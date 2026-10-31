#include "xge3d_internal.h"
#if XGE3D_ENABLE_PARTICLES
typedef struct d3_particle {xge3d_vec3_t position,velocity;float age,life,rotation;} d3_particle;
struct xge3d_particles {xge3d_particle_desc_t desc;xarray live;uint32_t random;double emission;uint64_t emitted,dropped;};
static float particle_random(xge3d_particles *p){uint32_t x=p->random;x^=x<<13;x^=x>>17;x^=x<<5;p->random=x;return (x>>8)*(1.0f/16777216.0f);}
xge3d_particle_desc_t xge3dParticlesDefault(void)
{
    return (xge3d_particle_desc_t){.size=sizeof(xge3d_particle_desc_t),.capacity=256,.seed=1,.rate=30,.life_min=.5f,.life_max=1.5f,
        .speed_min=1,.speed_max=3,.size_start=.1f,.size_end=0,.spread=.5f,.direction={0,1,0},.gravity={0,-1,0},
        .color_start={1,.6f,.1f,1},.color_end={.3f,.1f,0,0},.columns=1,.rows=1};
}
static int particle_color(const float c[4]){for(int i=0;i<4;++i){if(!isfinite(c[i])||c[i]<0||c[i]>(i==3?1:64)){return 0;}}return 1;}
int xge3dParticlesCreate(const xge3d_particle_desc_t *d,xge3d_particles **out)
{
    if(!out){return XGE_ERROR_INVALID_ARGUMENT;}*out=NULL;
    if(!d||d->size!=sizeof(*d)||!d->capacity||d->capacity>1048576||!d3_finite3(d->position)||!d3_finite3(d->direction)||!d3_finite3(d->gravity)||
        !particle_color(d->color_start)||!particle_color(d->color_end)||d->columns<1||d->rows<1||d->columns>256||d->rows>256){return XGE_ERROR_INVALID_ARGUMENT;}
    const float values[]={d->rate,d->life_min,d->life_max,d->speed_min,d->speed_max,d->size_start,d->size_end,d->spread,d->drag,d->fps};
    for(size_t i=0;i<sizeof(values)/sizeof(*values);++i){if(!isfinite(values[i])||values[i]<0||values[i]>1000000){return XGE_ERROR_INVALID_ARGUMENT;}}
    if(d->life_min<=0||d->life_max<d->life_min||d->life_max>3600||d->speed_max<d->speed_min||d->spread>3.14159265f){return XGE_ERROR_INVALID_ARGUMENT;}
    const float *v=&d->position.x,*g=&d->gravity.x;for(int i=0;i<3;++i){if(fabsf(v[i])>1e12f||fabsf(g[i])>1e6f){return XGE_ERROR_INVALID_ARGUMENT;}}
    xge3d_vec3_t axis=d->direction;if(!d3_normalize(&axis)){return XGE_ERROR_INVALID_ARGUMENT;}
    xge3d_particles *p=xrtCalloc(1,sizeof(*p));if(!p){return XGE_ERROR_OUT_OF_MEMORY;}
    p->desc=*d;p->desc.direction=axis;p->random=d->seed?d->seed:1;xrtArrayInit(&p->live,sizeof(d3_particle));
    if(!xrtArrayReserve(&p->live,d->capacity)){xge3dParticlesFree(p);return XGE_ERROR_OUT_OF_MEMORY;}*out=p;return XGE_OK;
}
void xge3dParticlesFree(xge3d_particles *p){if(p){xrtArrayUnit(&p->live);xrtFree(p);}}
int xge3dParticlesSetPosition(xge3d_particles *p,xge3d_vec3_t position)
{if(!p||!d3_finite3(position)||fabsf(position.x)>1e12f||fabsf(position.y)>1e12f||fabsf(position.z)>1e12f){return XGE_ERROR_INVALID_ARGUMENT;}p->desc.position=position;return XGE_OK;}
int xge3dParticlesEmit(xge3d_particles *p,uint32_t count)
{
    if(!p){return XGE_ERROR_INVALID_ARGUMENT;}const xge3d_particle_desc_t *d=&p->desc;
    uint32_t available=d->capacity-(uint32_t)p->live.Count,emit=count<available?count:available;p->dropped+=count-emit;
    xge3d_vec3_t axis=d->direction,right=d3_cross(axis,fabsf(axis.y)<.9f?(xge3d_vec3_t){0,1,0}:(xge3d_vec3_t){1,0,0});d3_normalize(&right);
    xge3d_vec3_t up=d3_cross(right,axis);
    for(uint32_t i=0;i<emit;++i){float c=1-particle_random(p)*(1-cosf(d->spread)),s=sqrtf(fmaxf(0,1-c*c)),a=particle_random(p)*6.2831853f;
        xge3d_vec3_t direction=d3_add(d3_scale(axis,c),d3_add(d3_scale(right,s*cosf(a)),d3_scale(up,s*sinf(a))));
        d3_particle next={d->position,d3_scale(direction,d->speed_min+particle_random(p)*(d->speed_max-d->speed_min)),0,
            d->life_min+particle_random(p)*(d->life_max-d->life_min),particle_random(p)*6.2831853f};
        if(!xrtArrayPush(&p->live,&next)){return XGE_ERROR_OUT_OF_MEMORY;} /* Reserved at create. */
    }p->emitted+=emit;return XGE_OK;
}
int xge3dParticlesUpdate(xge3d_particles *p,float dt,int emit)
{
    if(!p||!isfinite(dt)||dt<0||dt>1||(emit!=0&&emit!=1)){return XGE_ERROR_INVALID_ARGUMENT;}
    int steps=(int)ceilf(dt*60);if(!steps){return XGE_OK;}float h=dt/steps;const xge3d_particle_desc_t *d=&p->desc;
    for(int step=0;step<steps;++step){d3_particle *items=(d3_particle*)p->live.Data;size_t n=0;
        for(size_t i=0;i<p->live.Count;++i){d3_particle v=items[i];v.age+=h;if(v.age>=v.life){continue;}
            if(d->drag>1e-5f){float decay=expf(-d->drag*h),t=-expm1f(-d->drag*h)/d->drag;
                v.position=d3_add(v.position,d3_add(d3_scale(v.velocity,t),d3_scale(d->gravity,(h-t)/d->drag)));
                v.velocity=d3_add(d3_scale(v.velocity,decay),d3_scale(d->gravity,t));
            }else{v.position=d3_add(v.position,d3_add(d3_scale(v.velocity,h),d3_scale(d->gravity,.5f*h*h)));v.velocity=d3_add(v.velocity,d3_scale(d->gravity,h));}
            items[n++]=v;
        }p->live.Count=n;
        if(emit){p->emission+=(double)d->rate*h;uint32_t count=(uint32_t)p->emission;p->emission-=count;int result=xge3dParticlesEmit(p,count);if(result){return result;}}
    }return XGE_OK;
}
int xge3dParticlesInfo(const xge3d_particles *p,xge3d_particle_info_t *out)
{if(!p||!out||out->size!=sizeof(*out)){return XGE_ERROR_INVALID_ARGUMENT;}*out=(xge3d_particle_info_t){sizeof(*out),(uint32_t)p->live.Count,p->emitted,p->dropped};return XGE_OK;}
int xge3dParticlesBillboards(const xge3d_particles *p,xge3d_texture *texture,int blend,xge3d_billboard_t *items,size_t capacity,size_t *count)
{
    if(!p||!count||(capacity&&!items)||(blend!=0&&blend!=1)){return XGE_ERROR_INVALID_ARGUMENT;}*count=p->live.Count;
    size_t n=capacity<p->live.Count?capacity:p->live.Count;const xge3d_particle_desc_t *d=&p->desc;
    for(size_t i=0;i<n;++i){const d3_particle *v=(const d3_particle*)p->live.Data+i;float t=v->age/v->life;
        xge3d_billboard_t b=xge3dBillboardDefault();b.position=v->position;b.width=b.height=d->size_start+(d->size_end-d->size_start)*t;b.rotation=v->rotation;
        for(int j=0;j<4;++j){b.color[j]=d->color_start[j]+(d->color_end[j]-d->color_start[j])*t;}
        uint64_t frame=(uint64_t)((double)v->age*d->fps)%((uint32_t)d->columns*d->rows);
        b.uv[0]=(float)(frame%d->columns)/d->columns;b.uv[1]=(float)(frame/d->columns)/d->rows;b.uv[2]=1.0f/d->columns;b.uv[3]=1.0f/d->rows;
        b.texture=texture;b.blend=blend;items[i]=b;
    }return items&&capacity<p->live.Count?XGE_ERROR_BUFFER_TOO_SMALL:XGE_OK;
}
#endif
