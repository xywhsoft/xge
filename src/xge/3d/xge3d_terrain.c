#include "xge3d_internal.h"
#include "xge_graphics_internal.h"
#include <limits.h>
#if XGE3D_ENABLE_TERRAIN
typedef struct d3_terrain_chunk {
    size_t x,z,width,depth;xge3d_vec3_t position;xge3d_mesh *meshes[4];
} d3_terrain_chunk;
struct xge3d_terrain {
    xge3d_terrain_desc_t desc;xge3d_terrain_info_t info;xarray heights,chunks;size_t columns;
    uint64_t owner;
};
xge3d_terrain_options_t xge3dTerrainOptionsDefault(void)
{
    return (xge3d_terrain_options_t){.format=XGE3D_HEIGHT_F32,.cell_size=1,.height_scale=1,
        .chunk_cells=32,.lod_count=3,.lod_distances={40,100,200}};
}
static xge3d_terrain_desc_t d3_terrain_desc(const xge3d_terrain_options_t *o)
{
    return (xge3d_terrain_desc_t){.width=o->width,.depth=o->depth,.stride=o->stride,.format=o->format,
        .cell_size=o->cell_size,.height_scale=o->height_scale,.height_offset=o->height_offset,
        .chunk_cells=o->chunk_cells,.lod_count=o->lod_count,
        .lod_distances={o->lod_distances[0],o->lod_distances[1],o->lod_distances[2]},.skirt_depth=o->skirt_depth};
}
xge3d_terrain_desc_t xge3dTerrainDefault(void)
{
    xge3d_terrain_options_t options=xge3dTerrainOptionsDefault();
    return d3_terrain_desc(&options);
}
static int d3_terrain_status(const xge3d_terrain *t)
{
    if(!t)return XGE_ERROR_INVALID_ARGUMENT;
    return t->owner && t->owner!=xrtThreadCurrentId()?XGE_ERROR_INVALID_STATE:XGE_OK;
}
static float d3_height(const xge3d_terrain *t,size_t x,size_t z)
{return ((const float*)t->heights.Data)[z*t->info.width+x];}
static xge3d_vec3_t d3_terrain_normal(const xge3d_terrain *t,size_t x,size_t z)
{
    size_t a=x ? x-1 : x,b=x+1<t->info.width ? x+1 : x,c=z ? z-1 : z,d=z+1<t->info.depth ? z+1 : z;
    double dx=((double)d3_height(t,b,z)-d3_height(t,a,z))/((b-a)*t->desc.cell_size);
    double dz=((double)d3_height(t,x,d)-d3_height(t,x,c))/((d-c)*t->desc.cell_size),length=sqrt(dx*dx+1+dz*dz);
    return (xge3d_vec3_t){(float)(-dx/length),(float)(1/length),(float)(-dz/length)};
}
static int d3_terrain_mesh(xge3d_terrain *t,d3_terrain_chunk *c,int lod,xge3d_mesh **out)
{
    size_t step=(size_t)1<<lod,nx=(c->width+step-1)/step+1,nz=(c->depth+step-1)/step+1,grid=nx*nz,ring=2*(nx+nz)-4;
    xarray vertices,indices,edges;xrtArrayInit(&vertices,sizeof(xge3d_vertex_t));xrtArrayInit(&indices,sizeof(uint32_t));xrtArrayInit(&edges,sizeof(uint32_t));
    int result=XGE_ERROR_OUT_OF_MEMORY;
    if (!xrtArrayResize(&vertices,grid+ring) || !xrtArrayResize(&indices,(nx-1)*(nz-1)*6+ring*6) || !xrtArrayResize(&edges,ring)) goto done;
    memset(vertices.Data,0,vertices.Count*vertices.ItemSize);xge3d_vertex_t *v=(xge3d_vertex_t*)vertices.Data;uint32_t *ix=(uint32_t*)indices.Data,*e=(uint32_t*)edges.Data;
    for (size_t z=0;z<nz;++z) for (size_t x=0;x<nx;++x) {
        size_t gx=c->x+(x*step<c->width ? x*step : c->width),gz=c->z+(z*step<c->depth ? z*step : c->depth),i=z*nx+x;
        v[i].position=(xge3d_vec3_t){gx*t->desc.cell_size-c->position.x,d3_height(t,gx,gz),gz*t->desc.cell_size-c->position.z};
        v[i].normal=d3_terrain_normal(t,gx,gz);xge3d_vec3_t tangent={v[i].normal.y,-v[i].normal.x,0};d3_normalize(&tangent);
        v[i].tangent[0]=tangent.x;v[i].tangent[1]=tangent.y;v[i].tangent[2]=tangent.z;v[i].tangent[3]=-1;
        v[i].uv[0]=gx/(float)(t->info.width-1);v[i].uv[1]=gz/(float)(t->info.depth-1);
    }
    size_t index=0;
    for (size_t z=0;z<nz-1;++z) for (size_t x=0;x<nx-1;++x) {
        uint32_t a=(uint32_t)(z*nx+x),b=a+1,c0=a+(uint32_t)nx,d=c0+1;
        uint32_t cell[]={a,c0,d,a,d,b};memcpy(ix+index,cell,sizeof(cell));index+=6;
    }
    size_t edge=0;
    for (size_t x=0;x<nx;++x) e[edge++]=(uint32_t)x;
    for (size_t z=1;z<nz;++z) e[edge++]=(uint32_t)(z*nx+nx-1);
    for (size_t x=nx-1;x>0;--x) e[edge++]=(uint32_t)((nz-1)*nx+x-1);
    for (size_t z=nz-1;z>1;--z) e[edge++]=(uint32_t)((z-1)*nx);
    for (size_t i=0;i<ring;++i) {
        v[grid+i]=v[e[i]];v[grid+i].position.y-=t->desc.skirt_depth;
        uint32_t a=e[i],b=e[(i+1)%ring],c0=(uint32_t)(grid+i),d=(uint32_t)(grid+(i+1)%ring);
        uint32_t side[]={a,b,d,a,d,c0};memcpy(ix+index,side,sizeof(side));index+=6;
    }
    if(t->owner){
        result=xge3dMeshCreateU32Owned(v,vertices.Count,ix,indices.Count,out);
        if(result==XGE_OK)(*out)->immutable=true;
    }else{
        xge3d_mesh_desc_t md={v,vertices.Count,ix,indices.Count,32};
        result=xge3dMeshCreate(&md,out);
    }
done:
    xrtArrayUnit(&vertices);xrtArrayUnit(&indices);xrtArrayUnit(&edges);return result;
}
static void d3_terrain_dispose(xge3d_terrain *t)
{
    for (size_t i=0;i<t->chunks.Count;++i) {d3_terrain_chunk *c=xrtArrayGet(&t->chunks,i);for (int k=0;k<4;++k) xge3dMeshFree(c->meshes[k]);}
    xge3dMaterialFree(t->desc.material);xrtArrayUnit(&t->chunks);xrtArrayUnit(&t->heights);xrtFree(t);
}
static int d3_terrain_release(xge3d_terrain *t,bool *consumed)
{
    if(!t)return XGE_OK;
    int status=d3_terrain_status(t);
    if(status!=XGE_OK)return status;
    bool managed=t->owner!=0;
    uint64_t context=managed?__xge3dContext():0;
    for(size_t i=0;context && i<t->chunks.Count;++i){
        const d3_terrain_chunk *chunk=xrtArrayConstGet(&t->chunks,i);
        for(int lod=0;lod<t->info.lod_count;++lod){
            const xge3d_mesh *mesh=chunk->meshes[lod];
            if(mesh->refs==1 && mesh->context && mesh->context!=context)return XGE_ERROR_INVALID_STATE;
        }
    }
    d3_terrain_dispose(t);
    if(consumed)*consumed=true;
    return managed?xgeInternalGraphicsOwnerRelease():XGE_OK;
}
void xge3dTerrainFree(xge3d_terrain *t)
{(void)d3_terrain_release(t,NULL);}
int xge3dTerrainReleaseOwnerChecked(xge3d_terrain *t,bool *consumed)
{
    if(!consumed)return XGE_ERROR_INVALID_ARGUMENT;
    *consumed=false;
    if(!t||!t->owner)return XGE_ERROR_INVALID_ARGUMENT;
    return d3_terrain_release(t,consumed);
}
int xge3dTerrainReleaseChecked(xge3d_terrain *t)
{
    bool consumed;
    return xge3dTerrainReleaseOwnerChecked(t,&consumed);
}
static int d3_terrain_create(const xge3d_terrain_desc_t *d,size_t bytes,bool managed,xge3d_terrain **out)
{
    if (!out) return XGE_ERROR_INVALID_ARGUMENT;
    *out=NULL;
    if (!d || !d->heights || d->width<2 || d->depth<2 || d->width>65537 || d->depth>65537 ||
        (d->format!=XGE3D_HEIGHT_U16 && d->format!=XGE3D_HEIGHT_F32) || !isfinite(d->cell_size) || d->cell_size<=0 ||
        !isfinite(d->height_scale) || !isfinite(d->height_offset) || !isfinite(d->skirt_depth) || d->skirt_depth<0 ||
        d->chunk_cells<4 || d->chunk_cells>256 || (d->chunk_cells&(d->chunk_cells-1)) || d->lod_count<1 || d->lod_count>4)
        return XGE_ERROR_INVALID_ARGUMENT;
    size_t sample=d->format==XGE3D_HEIGHT_U16 ? 2 : 4,row=d->width*sample,stride=d->stride ? d->stride : row;
    if (stride<row || d->depth-1>(SIZE_MAX-row)/stride || d->width>SIZE_MAX/d->depth/sizeof(float) ||
        !isfinite((float)((d->width-1)*(double)d->cell_size)) || !isfinite((float)((d->depth-1)*(double)d->cell_size))) return XGE_ERROR_INVALID_ARGUMENT;
    if(managed && (d->depth-1)*stride+row>bytes)return XGE_ERROR_BUFFER_TOO_SMALL;
    float previous=0;for (int i=0;i<d->lod_count-1;++i) {
        if (!isfinite(d->lod_distances[i]) || d->lod_distances[i]<=previous) return XGE_ERROR_INVALID_ARGUMENT;
        previous=d->lod_distances[i];
    }
    xge3d_terrain *t=xrtMalloc(sizeof(*t));if (!t) return XGE_ERROR_OUT_OF_MEMORY;
    memset(t,0,sizeof(*t));t->desc=*d;t->desc.heights=NULL;t->desc.material=NULL;
    if(managed)t->owner=xrtThreadCurrentId();
    t->info.width=d->width;t->info.depth=d->depth;t->info.lod_count=d->lod_count;
    xrtArrayInit(&t->heights,sizeof(float));xrtArrayInit(&t->chunks,sizeof(d3_terrain_chunk));
    int result=XGE_ERROR_OUT_OF_MEMORY;
    if (!xrtArrayResize(&t->heights,d->width*d->depth)) goto failed;
    float minimum=0,maximum=0;
    for (size_t z=0;z<d->depth;++z) for (size_t x=0;x<d->width;++x) {
        const unsigned char *p=(const unsigned char*)d->heights+z*stride+x*sample;double h;
        if (d->format==XGE3D_HEIGHT_U16) {uint16_t value;memcpy(&value,p,2);h=value;}else {float value;memcpy(&value,p,4);h=value;}
        float height=(float)(h*d->height_scale+d->height_offset);
        if (!isfinite(height)) {result=XGE_ERROR_INVALID_ARGUMENT;goto failed;}
        ((float*)t->heights.Data)[z*d->width+x]=height;
        if (!x && !z) minimum=maximum=height;else {minimum=fminf(minimum,height);maximum=fmaxf(maximum,height);}
    }
    if (!t->desc.skirt_depth) t->desc.skirt_depth=(float)((double)maximum-minimum+d->cell_size*.1);
    if (!isfinite(t->desc.skirt_depth) || !isfinite(minimum-t->desc.skirt_depth)) {result=XGE_ERROR_INVALID_ARGUMENT;goto failed;}
    t->info.bounds=(xge3d_aabb_t){{0,minimum,0},{(d->width-1)*d->cell_size,maximum,(d->depth-1)*d->cell_size}};
    if (d->material) {
        if(managed)result=xge3dMaterialShareOwned(d->material,&t->desc.material);
        else if(d3_material_retain(d->material)){t->desc.material=d->material;result=XGE_OK;}
        else result=XGE_ERROR_INVALID_STATE;
        if(result!=XGE_OK)goto failed;
    }
    else {xge3d_material_desc_t material=xge3dMaterialDefault();material.metallic=0;result=xge3dMaterialCreate(&material,&t->desc.material);if (result!=XGE_OK) goto failed;}
    if(managed && !d->material){
        xge3d_material *owned=NULL;
        result=xge3dMaterialShareOwned(t->desc.material,&owned);
        if(result!=XGE_OK)goto failed;
        xge3dMaterialFree(owned);
    }
    t->columns=(d->width-2)/d->chunk_cells+1;size_t rows=(d->depth-2)/d->chunk_cells+1;
    if (!xrtArrayResize(&t->chunks,t->columns*rows)) {result=XGE_ERROR_OUT_OF_MEMORY;goto failed;}
    memset(t->chunks.Data,0,t->chunks.Count*t->chunks.ItemSize);t->info.chunk_count=t->chunks.Count;
    for (size_t i=0;i<t->chunks.Count;++i) {
        d3_terrain_chunk *c=xrtArrayGet(&t->chunks,i);c->x=(i%t->columns)*d->chunk_cells;c->z=(i/t->columns)*d->chunk_cells;
        c->width=d->width-1-c->x;if (c->width>d->chunk_cells) c->width=d->chunk_cells;
        c->depth=d->depth-1-c->z;if (c->depth>d->chunk_cells) c->depth=d->chunk_cells;
        c->position=(xge3d_vec3_t){(c->x+c->width*.5)*d->cell_size,0,(c->z+c->depth*.5)*d->cell_size};
        for (int k=0;k<d->lod_count;++k) {result=d3_terrain_mesh(t,c,k,&c->meshes[k]);if (result!=XGE_OK) goto failed;}
    }
    *out=t;return XGE_OK;
failed:
    d3_terrain_dispose(t);return result;
}
int xge3dTerrainCreate(const xge3d_terrain_desc_t *d,xge3d_terrain **out)
{return d3_terrain_create(d,0,false,out);}
int xge3dTerrainCreateOwned(const void *heights,size_t bytes,const xge3d_terrain_options_t *options,
    xge3d_material *material,xge3d_terrain **out)
{
    if(!out)return XGE_ERROR_INVALID_ARGUMENT;
    *out=NULL;
    if(!options)return XGE_ERROR_INVALID_ARGUMENT;
    xge3d_terrain_desc_t desc=d3_terrain_desc(options);
    desc.heights=heights;desc.material=material;
    int status=xgeInternalGraphicsOwnerAcquire();
    if(status!=XGE_OK)return status;
    status=d3_terrain_create(&desc,bytes,true,out);
    if(status!=XGE_OK)(void)xgeInternalGraphicsOwnerRelease();
    return status;
}
xge3d_terrain_info_t xge3dTerrainInfo(const xge3d_terrain *t)
{return d3_terrain_status(t)==XGE_OK?t->info:(xge3d_terrain_info_t){0};}
int xge3dTerrainInfoChecked(const xge3d_terrain *t,xge3d_terrain_info_t *out)
{
    if(!out)return XGE_ERROR_INVALID_ARGUMENT;
    *out=(xge3d_terrain_info_t){0};
    int status=d3_terrain_status(t);
    if(status!=XGE_OK)return status;
    *out=t->info;return XGE_OK;
}
int xge3dTerrainChunkMesh(const xge3d_terrain *t,size_t chunk,int lod,const xge3d_mesh **out,xge3d_vec3_t *position)
{
    if (!out || !position) return XGE_ERROR_INVALID_ARGUMENT;
    *out=NULL;*position=(xge3d_vec3_t){0};
    int status=d3_terrain_status(t);if(status!=XGE_OK)return status;
    if (chunk>=t->chunks.Count || lod<0 || lod>=t->info.lod_count) return XGE_ERROR_INVALID_ARGUMENT;
    const d3_terrain_chunk *c=xrtArrayConstGet(&t->chunks,chunk);*out=c->meshes[lod];*position=c->position;return XGE_OK;
}
int xge3dTerrainChunkMeshOwned(const xge3d_terrain *t,size_t chunk,int lod,xge3d_mesh **out)
{
    if(!out)return XGE_ERROR_INVALID_ARGUMENT;
    *out=NULL;
    if(!t||!t->owner)return XGE_ERROR_INVALID_ARGUMENT;
    const xge3d_mesh *mesh=NULL;
    xge3d_vec3_t position;
    int status=xge3dTerrainChunkMesh(t,chunk,lod,&mesh,&position);
    return status==XGE_OK?xge3dMeshShareOwned((xge3d_mesh*)mesh,out):status;
}
int xge3dTerrainChunkPosition(const xge3d_terrain *t,size_t chunk,xge3d_vec3_t *out)
{
    const xge3d_mesh *mesh=NULL;
    return xge3dTerrainChunkMesh(t,chunk,0,&mesh,out);
}
int xge3dTerrainInstantiate(xge3d_scene *s,xge3d_terrain *t,xge3d_node_t parent,xge3d_node_t *out)
{
    if (!out) return XGE_ERROR_INVALID_ARGUMENT;
    *out=(xge3d_node_t){0};if (!s) return XGE_ERROR_INVALID_ARGUMENT;
    int access=d3_terrain_status(t);if(access!=XGE_OK)return access;
    /* Reserve rollback traversal before publishing any node, so an allocation
     * failure during instance creation cannot prevent subtree removal. */
    if (!xrtArrayReserve(&s->scratch,t->chunks.Count+1)) return XGE_ERROR_OUT_OF_MEMORY;
    xge3d_node_t root;int result=xge3dNodeCreate(s,parent,&root);if (result!=XGE_OK) return result;
    for (size_t i=0;i<t->chunks.Count;++i) {
        const d3_terrain_chunk *c=xrtArrayConstGet(&t->chunks,i);xge3d_node_t node;
        result=xge3dNodeCreate(s,root,&node);if (result!=XGE_OK) break;
        xge3d_transform_t transform=XGE3D_TRANSFORM_IDENTITY;transform.position=c->position;
        result=xge3dNodeSetTransform(s,node,&transform);
        if (result==XGE_OK) result=xge3dNodeSetMesh(s,node,c->meshes[0]);
        if (result==XGE_OK) result=xge3dNodeSetMaterial(s,node,t->desc.material);
        xge3d_lod_t levels[3];for (int k=1;k<t->info.lod_count;++k) levels[k-1]=(xge3d_lod_t){c->meshes[k],t->desc.lod_distances[k-1]};
        if (result==XGE_OK) result=xge3dNodeSetLods(s,node,levels,t->info.lod_count-1);
        if (result!=XGE_OK) break;
    }
    if (result!=XGE_OK) {xge3dNodeDestroy(s,root);return result;}
    *out=root;return XGE_OK;
}
int xge3dTerrainSample(const xge3d_terrain *t,int lod,float x,float z,float *out,xge3d_vec3_t *normal)
{
    if (!out || !normal) return XGE_ERROR_INVALID_ARGUMENT;
    *out=0;*normal=(xge3d_vec3_t){0};
    int status=d3_terrain_status(t);if(status!=XGE_OK)return status;
    if (lod<0 || lod>=t->info.lod_count || !isfinite(x) || !isfinite(z)) return XGE_ERROR_INVALID_ARGUMENT;
    if (x<0 || z<0 || x>t->info.bounds.max.x || z>t->info.bounds.max.z) return XGE_ERROR_NOT_FOUND;
    double gx=fmin(x/(double)t->desc.cell_size,t->info.width-1),gz=fmin(z/(double)t->desc.cell_size,t->info.depth-1);
    size_t cx=(size_t)(gx/t->desc.chunk_cells),cz=(size_t)(gz/t->desc.chunk_cells),rows=t->chunks.Count/t->columns;
    if (cx>=t->columns) cx=t->columns-1;
    if (cz>=rows) cz=rows-1;
    const d3_terrain_chunk *c=xrtArrayConstGet(&t->chunks,cz*t->columns+cx);
    size_t step=(size_t)1<<lod,nx=(c->width+step-1)/step,nz=(c->depth+step-1)/step;
    size_t ix=(size_t)fmax(0,floor((gx-c->x)/step)),iz=(size_t)fmax(0,floor((gz-c->z)/step));
    if (ix>=nx) ix=nx-1;
    if (iz>=nz) iz=nz-1;
    size_t x0=c->x+ix*step,z0=c->z+iz*step,x1=x0+step,z1=z0+step;
    if (x1>c->x+c->width) x1=c->x+c->width;
    if (z1>c->z+c->depth) z1=c->z+c->depth;
    double a=(gx-x0)/(x1-x0),b=(gz-z0)/(z1-z0),h00=d3_height(t,x0,z0),h10=d3_height(t,x1,z0),h01=d3_height(t,x0,z1),h11=d3_height(t,x1,z1);
    double dx,dz;
    if (a<=b) {*out=(float)(h00+(h11-h01)*a+(h01-h00)*b);dx=h11-h01;dz=h01-h00;}
    else {*out=(float)(h00+(h10-h00)*a+(h11-h10)*b);dx=h10-h00;dz=h11-h10;}
    dx/=(x1-x0)*t->desc.cell_size;dz/=(z1-z0)*t->desc.cell_size;double length=sqrt(dx*dx+1+dz*dz);
    *normal=(xge3d_vec3_t){(float)(-dx/length),(float)(1/length),(float)(-dz/length)};return XGE_OK;
}
int xge3dTerrainRaycast(const xge3d_terrain *t,int lod,const xge3d_ray_t *ray,float max,xge3d_hit_t *out,size_t *chunk)
{
    if (!out) return XGE_ERROR_INVALID_ARGUMENT;
    memset(out,0,sizeof(*out));if (chunk) *chunk=SIZE_MAX;
    int status=d3_terrain_status(t);if(status!=XGE_OK)return status;
    if (lod<0 || lod>=t->info.lod_count || !ray || !d3_finite3(ray->origin) || !d3_finite3(ray->direction) || !isfinite(max) || max<=0) return XGE_ERROR_INVALID_ARGUMENT;
    xge3d_vec3_t direction=ray->direction;if (!d3_normalize(&direction)) return XGE_ERROR_INVALID_ARGUMENT;
    int found=0;
    for (size_t i=0;i<t->chunks.Count;++i) {
        const d3_terrain_chunk *c=xrtArrayConstGet(&t->chunks,i);const xge3d_mesh *m=c->meshes[lod];float entry;
        xge3d_aabb_t bounds={d3_add(m->bounds.min,c->position),d3_add(m->bounds.max,c->position)};
        if (xge3dRayAabb(ray,&bounds,max,&entry)==XGE_ERROR_NOT_FOUND) continue;
        const uint32_t *indices=(const uint32_t*)m->indices.Data;const xge3d_vertex_t *vertices=(const xge3d_vertex_t*)m->vertices.Data;
        for (size_t k=0;k<m->indices.Count;k+=3) {
            xge3d_vec3_t points[3];for (int v=0;v<3;++v) points[v]=d3_add(vertices[indices[k+v]].position,c->position);
            xge3d_hit_t hit;if (xge3dRayTriangle(ray,points,max,&hit)!=XGE_OK) continue;
            *out=hit;out->triangle=k/3;if (chunk) *chunk=i;max=hit.distance;found=1;
        }
    }
    return found ? XGE_OK : XGE_ERROR_NOT_FOUND;
}
#endif
