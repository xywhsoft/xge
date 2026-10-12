#include "xge3d_internal.h"

#if XGE_ENABLE_3D
static int d3_box_valid(const xge3d_aabb_t *b)
{return b && d3_finite3(b->min) && d3_finite3(b->max) && b->min.x<=b->max.x && b->min.y<=b->max.y && b->min.z<=b->max.z;}
static int d3_sphere_valid(const xge3d_sphere_t *s)
{return s && d3_finite3(s->center) && isfinite(s->radius) && s->radius>=0;}
static int d3_ray_valid(const xge3d_ray_t *r,float max,xge3d_vec3_t *direction)
{
    if (!r || !d3_finite3(r->origin) || !d3_finite3(r->direction) || !isfinite(max) || max<=0) return 0;
    *direction=r->direction;return d3_normalize(direction);
}
int xge3dMeshGetBounds(const xge3d_mesh *mesh,xge3d_aabb_t *out)
{
    if (!out) return XGE_ERROR_INVALID_ARGUMENT;
    *out=(xge3d_aabb_t){0};if (!mesh) return XGE_ERROR_INVALID_ARGUMENT;*out=mesh->bounds;return XGE_OK;
}
int xge3dNodeGetMesh(xge3d_scene *scene,xge3d_node_t node,const xge3d_mesh **out)
{
    if (!out) return XGE_ERROR_INVALID_ARGUMENT;
    *out=NULL;xge3d_node_data *n=d3_node(scene,node);if (!n) return XGE_ERROR_INVALID_ARGUMENT;
    *out=n->mesh;return *out ? XGE_OK : XGE_ERROR_NOT_FOUND;
}
static int d3_transform_box(xge3d_aabb_t box,xge3d_mat4_t world,xge3d_aabb_t *out,int *found)
{
    for (int i=0;i<8;++i) {
        xge3d_vec3_t p={i&1 ? box.max.x : box.min.x,i&2 ? box.max.y : box.min.y,i&4 ? box.max.z : box.min.z};p=d3_point(world,p);
        if (!d3_finite3(p)) return XGE_ERROR_INVALID_ARGUMENT;
        if (!*found) {*out=(xge3d_aabb_t){p,p};*found=1;}else d3_bounds_point(out,p);
    }
    return XGE_OK;
}
int d3_node_bounds(xge3d_scene *s,xge3d_node_data *n,xge3d_aabb_t *out)
{
    if (!n->mesh) return XGE_ERROR_NOT_FOUND;
    int lod_changed=0;
    if (n->lods) for (size_t i=0;i<n->lods->count;++i) if (n->lods->versions[i]!=n->lods->levels[i].mesh->version) lod_changed=1;
    if (!lod_changed && n->bounds_version==s->version && n->mesh_version==n->mesh->version) {*out=n->bounds;return XGE_OK;}
    xge3d_mat4_t world;int result=xge3dNodeGetWorldMatrix(s,n->handle,&world);if (result!=XGE_OK) return result;
    xge3d_aabb_t box={0};int found=0;
#if XGE3D_ENABLE_ANIMATION
    if (n->skin) {
        result=d3_skin_update(s,n);if (result!=XGE_OK) return result;
        /* A normalized weighted vertex is inside the union of its joint boxes.
         * Transform eight corners per shared joint envelope, not every vertex. */
        for (size_t i=0;i<n->mesh->joint_bounds.Count;++i) {
            const d3_joint_bounds *b=xrtArrayConstGet(&n->mesh->joint_bounds,i);if (!b->valid) continue;
            const xge3d_mat4_t *palette=xrtArrayConstGet(&n->skin->palette,i);if (!palette) return XGE_ERROR_INVALID_STATE;
            result=d3_transform_box(b->box,d3_mul(world,*palette),&box,&found);if (result!=XGE_OK) return result;
        }
        if (!found) return XGE_ERROR_INVALID_STATE;
    } else
#endif
    {
        result=d3_transform_box(n->mesh->bounds,world,&box,&found);if (result!=XGE_OK) return result;
        if (n->lods) for (size_t i=0;i<n->lods->count;++i) {
            result=d3_transform_box(n->lods->levels[i].mesh->bounds,world,&box,&found);if (result!=XGE_OK) return result;
        }
    }
    if (n->lods) for (size_t i=0;i<n->lods->count;++i) n->lods->versions[i]=n->lods->levels[i].mesh->version;
    n->bounds=box;n->bounds_version=s->version;n->mesh_version=n->mesh->version;*out=box;return XGE_OK;
}
int xge3dNodeGetBounds(xge3d_scene *scene,xge3d_node_t node,int descendants,xge3d_aabb_t *out)
{
    if (!out) return XGE_ERROR_INVALID_ARGUMENT;
    *out=(xge3d_aabb_t){0};xge3d_node_data *root=d3_node(scene,node);if (!root) return XGE_ERROR_INVALID_ARGUMENT;
    int found=0;
    for (size_t i=0;i<scene->nodes.Count;++i) {
        xge3d_node_data *n=*(xge3d_node_data**)xrtArrayGet(&scene->nodes,i),*p=n;if (!n->mesh) continue;
        if (descendants) {for (;p && p!=root;p=d3_node(scene,p->parent)) {}if (!p) continue;}else if (n!=root) continue;
        xge3d_aabb_t box;int result=d3_node_bounds(scene,n,&box);if (result!=XGE_OK) {*out=(xge3d_aabb_t){0};return result;}
        if (!found) {*out=box;found=1;}else {d3_bounds_point(out,box.min);d3_bounds_point(out,box.max);}
    }
    return found ? XGE_OK : XGE_ERROR_NOT_FOUND;
}
int d3_frustum_box(xge3d_mat4_t m,xge3d_aabb_t box)
{
    for (int axis=0;axis<3;++axis) for (int sign=-1;sign<=1;sign+=2) {
        double p[4];for (int i=0;i<4;++i) p[i]=(double)m.m[i*4+3]+sign*(double)m.m[i*4+axis];
        double distance=p[3]+p[0]*(p[0]>=0 ? box.max.x : box.min.x)+p[1]*(p[1]>=0 ? box.max.y : box.min.y)+p[2]*(p[2]>=0 ? box.max.z : box.min.z);
        if (distance<-1e-5*(1+fabs(p[0])+fabs(p[1])+fabs(p[2]))) return 0;
    }
    return 1;
}
int xge3dCameraIntersectsAabb(const xge3d_camera_t *c,const xge3d_aabb_t *b)
{return c && d3_finite_matrix(&c->view) && d3_finite_matrix(&c->projection) && d3_box_valid(b) && d3_frustum_box(d3_mul(c->projection,c->view),*b);}
int xge3dAabbIntersects(const xge3d_aabb_t *a,const xge3d_aabb_t *b)
{
    return d3_box_valid(a) && d3_box_valid(b) && a->min.x<=b->max.x && a->max.x>=b->min.x &&
        a->min.y<=b->max.y && a->max.y>=b->min.y && a->min.z<=b->max.z && a->max.z>=b->min.z;
}
int xge3dSphereIntersectsAabb(const xge3d_sphere_t *s,const xge3d_aabb_t *b)
{
    if (!d3_sphere_valid(s) || !d3_box_valid(b)) return 0;
    const float *center=&s->center.x,*min=&b->min.x,*max=&b->max.x;double distance=0;
    for (int i=0;i<3;++i) {double d=(double)center[i]-fmaxf(min[i],fminf(max[i],center[i]));distance+=d*d;}
    return distance<=(double)s->radius*s->radius;
}
int xge3dRayAabb(const xge3d_ray_t *r,const xge3d_aabb_t *b,float max,float *out)
{
    if (!out) return XGE_ERROR_INVALID_ARGUMENT;
    *out=0;xge3d_vec3_t direction;if (!d3_box_valid(b) || !d3_ray_valid(r,max,&direction)) return XGE_ERROR_INVALID_ARGUMENT;
    const float *o=&r->origin.x,*d=&direction.x,*min=&b->min.x,*maximum=&b->max.x;double lo=0,hi=max;
    for (int i=0;i<3;++i) {
        if (!d[i]) {if (o[i]<min[i] || o[i]>maximum[i]) return XGE_ERROR_NOT_FOUND;continue;}
        double x=((double)min[i]-o[i])/d[i],y=((double)maximum[i]-o[i])/d[i];lo=fmax(lo,fmin(x,y));hi=fmin(hi,fmax(x,y));
        if (lo>hi) return XGE_ERROR_NOT_FOUND;
    }
    *out=(float)lo;return XGE_OK;
}
int xge3dRaySphere(const xge3d_ray_t *r,const xge3d_sphere_t *s,float max,float *out)
{
    if (!out) return XGE_ERROR_INVALID_ARGUMENT;
    *out=0;xge3d_vec3_t direction;if (!d3_sphere_valid(s) || !d3_ray_valid(r,max,&direction)) return XGE_ERROR_INVALID_ARGUMENT;
    double x=(double)r->origin.x-s->center.x,y=(double)r->origin.y-s->center.y,z=(double)r->origin.z-s->center.z;
    double distance=x*x+y*y+z*z-(double)s->radius*s->radius;if (distance<=0) return XGE_OK;
    double a=(double)direction.x*direction.x+(double)direction.y*direction.y+(double)direction.z*direction.z;
    double b=x*direction.x+y*direction.y+z*direction.z,discriminant=b*b-a*distance;if (discriminant<0) return XGE_ERROR_NOT_FOUND;
    double t=(-b-sqrt(discriminant))/a;if (t<0 || t>max) return XGE_ERROR_NOT_FOUND;
    *out=(float)t;return XGE_OK;
}
int xge3dRayTriangle(const xge3d_ray_t *r,const xge3d_vec3_t v[3],float max,xge3d_hit_t *out)
{
    if (!out) return XGE_ERROR_INVALID_ARGUMENT;
    memset(out,0,sizeof(*out));xge3d_vec3_t direction;
    if (!v || !d3_ray_valid(r,max,&direction) || !d3_finite3(v[0]) || !d3_finite3(v[1]) || !d3_finite3(v[2])) return XGE_ERROR_INVALID_ARGUMENT;
    xge3d_vec3_t e1=d3_sub(v[1],v[0]),e2=d3_sub(v[2],v[0]),cross=d3_cross(direction,e2),delta=d3_sub(r->origin,v[0]);
    double det=d3_dot(e1,cross);if (fabs(det)<1e-20) return XGE_ERROR_NOT_FOUND;
    double u=d3_dot(delta,cross)/det;if (u<0 || u>1) return XGE_ERROR_NOT_FOUND;
    cross=d3_cross(delta,e1);double bary=d3_dot(direction,cross)/det;if (bary<0 || u+bary>1) return XGE_ERROR_NOT_FOUND;
    double distance=d3_dot(e2,cross)/det;out->normal=d3_cross(e1,e2);
    if (!isfinite(distance) || distance<0 || distance>max || !d3_normalize(&out->normal)) {memset(out,0,sizeof(*out));return XGE_ERROR_NOT_FOUND;}
    out->distance=(float)distance;out->position=d3_add(r->origin,d3_scale(direction,out->distance));return XGE_OK;
}
int xge3dSceneQueryAabb(xge3d_scene *s,const xge3d_aabb_t *box,xge3d_node_t *nodes,size_t capacity,size_t *count)
{
    if (!count) return XGE_ERROR_INVALID_ARGUMENT;
    *count=0;if (!s || !d3_box_valid(box) || (capacity && !nodes)) return XGE_ERROR_INVALID_ARGUMENT;
    for (size_t i=0;i<s->nodes.Count;++i) {
        xge3d_node_data *n=*(xge3d_node_data**)xrtArrayGet(&s->nodes,i),*p=n;if (!n->mesh) continue;
        for (;p && p->visible;p=d3_node(s,p->parent)) {}if (p) continue;
        xge3d_aabb_t bounds;int result=d3_node_bounds(s,n,&bounds);if (result!=XGE_OK) return result;
        if (xge3dAabbIntersects(box,&bounds)) {if (*count<capacity) nodes[*count]=n->handle;++*count;}
    }
    return nodes && *count>capacity ? XGE_ERROR_BUFFER_TOO_SMALL : XGE_OK;
}
int xge3dCameraScreenRay(const xge3d_camera_t *camera, float x, float y,
    float width, float height, xge3d_ray_t *out)
{
    if (!out) return XGE_ERROR_INVALID_ARGUMENT;
    memset(out,0,sizeof(*out));
    if (!camera || !d3_finite_matrix(&camera->view) || !d3_finite_matrix(&camera->projection) ||
        !isfinite(x) || !isfinite(y) || !isfinite(width) || !isfinite(height) || width<=0 || height<=0)
        return XGE_ERROR_INVALID_ARGUMENT;
    xge3d_mat4_t inverse;
    if (!d3_inverse(d3_mul(camera->projection,camera->view),&inverse)) return XGE_ERROR_INVALID_ARGUMENT;
    xge3d_vec3_t p[2];
    float nx=2*x/width-1, ny=1-2*y/height;
    for (int i=0;i<2;++i) {
        float z=i ? 1 : -1;
        float w=inverse.m[3]*nx+inverse.m[7]*ny+inverse.m[11]*z+inverse.m[15];
        if (!isfinite(w) || fabsf(w)<1e-30f) return XGE_ERROR_INVALID_ARGUMENT;
        p[i]=d3_scale(d3_point(inverse,(xge3d_vec3_t){nx,ny,z}),1/w);
        if (!d3_finite3(p[i])) return XGE_ERROR_INVALID_ARGUMENT;
    }
    xge3d_vec3_t direction=d3_sub(p[1],p[0]);
    if (!d3_normalize(&direction)) return XGE_ERROR_INVALID_ARGUMENT;
    *out=(xge3d_ray_t){p[0],direction}; return XGE_OK;
}

static size_t d3_index(const xge3d_mesh_desc_t *mesh, size_t i)
{
    if (!mesh->index_count) return i;
    return mesh->index_bits==16 ? ((const uint16_t*)mesh->indices)[i] : ((const uint32_t*)mesh->indices)[i];
}

int xge3dSceneRaycast(xge3d_scene *scene, const xge3d_ray_t *ray, float max_distance, xge3d_hit_t *out)
{
    if (!out) return XGE_ERROR_INVALID_ARGUMENT;
    memset(out,0,sizeof(*out));
    if (!scene || !ray || !d3_finite3(ray->origin) || !d3_finite3(ray->direction) ||
        !isfinite(max_distance) || max_distance<=0) return XGE_ERROR_INVALID_ARGUMENT;
    xge3d_vec3_t direction=ray->direction;
    if (!d3_normalize(&direction)) return XGE_ERROR_INVALID_ARGUMENT;
    float nearest=max_distance;
    int found=0;
    for (size_t i=0;i<scene->nodes.Count;++i) {
        xge3d_node_data *n=*(xge3d_node_data**)xrtArrayGet(&scene->nodes,i), *p=n;
        if (!n->mesh) continue;
        for (;p && p->visible;p=d3_node(scene,p->parent)) {}
        if (p) continue;
        xge3d_aabb_t bounds;float entry;
        int result=d3_node_bounds(scene,n,&bounds);if (result!=XGE_OK) {memset(out,0,sizeof(*out));return result;}
        if (xge3dRayAabb(ray,&bounds,nearest,&entry)==XGE_ERROR_NOT_FOUND) continue;
        xge3d_mat4_t world;
        result=xge3dNodeGetWorldMatrix(scene,n->handle,&world);
        if (result!=XGE_OK) { memset(out,0,sizeof(*out)); return result; }
        xge3d_mesh_desc_t mesh;
        xge3dMeshGetData(n->mesh,&mesh);
#if XGE3D_ENABLE_ANIMATION
        result=d3_skin_update(scene,n);if (result!=XGE_OK) {memset(out,0,sizeof(*out));return result;}
#endif
        size_t count=mesh.index_count ? mesh.index_count : mesh.vertex_count;
        for (size_t t=0;t<count;t+=3) {
            xge3d_vec3_t v[3];
            for (int k=0;k<3;++k) {
                const xge3d_vertex_t *vertex=&mesh.vertices[d3_index(&mesh,t+k)];
                xge3d_vec3_t position=vertex->position;
#if XGE3D_ENABLE_ANIMATION
                if (n->skin) position=d3_skin_point(n->skin,vertex);
#endif
                v[k]=d3_point(world,position);
            }
            xge3d_vec3_t e1=d3_sub(v[1],v[0]), e2=d3_sub(v[2],v[0]);
            xge3d_vec3_t cross=d3_cross(direction,e2);
            float det=d3_dot(e1,cross);
            if (fabsf(det)<1e-20f) continue;
            xge3d_vec3_t delta=d3_sub(ray->origin,v[0]);
            float u=d3_dot(delta,cross)/det;
            if (u<0 || u>1) continue;
            cross=d3_cross(delta,e1);
            float bary_v=d3_dot(direction,cross)/det;
            if (bary_v<0 || u+bary_v>1) continue;
            float distance=d3_dot(e2,cross)/det;
            if (!isfinite(distance) || distance<0 || distance>nearest) continue;
            xge3d_vec3_t normal=d3_cross(e1,e2);
            if (!d3_normalize(&normal)) continue;
            xge3d_vec3_t wx={world.m[0],world.m[1],world.m[2]},
                wy={world.m[4],world.m[5],world.m[6]}, wz={world.m[8],world.m[9],world.m[10]};
            if (d3_dot(wx,d3_cross(wy,wz))<0) normal=d3_scale(normal,-1);
            out->node=n->handle; out->distance=distance; out->triangle=t/3;
            out->position=d3_add(ray->origin,d3_scale(direction,distance)); out->normal=normal;
#if XGE3D_ENABLE_MODEL
            out->model_root=n->instance_root;
#endif
            nearest=distance; found=1;
        }
    }
    return found ? XGE_OK : XGE_ERROR_NOT_FOUND;
}
#endif
