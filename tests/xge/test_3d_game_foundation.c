#include "xge.h"
#include "extlibs/xrt/xrt.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"foundation %d: %s\n",__LINE__,#x);return 1;}}while(0)
static xge3d_mat4_t identity(void){return (xge3d_mat4_t){{1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1}};}
#if XGE_ENABLE_INPUT_ACTIONS
static float read_action(const xge_action_binding_t *b,void *user){return ((float*)user)[b->code];}
static int actions(void)
{
    xge_actions *a=NULL;CHECK(!xgeActionsCreate(3,4,&a));float input[4]={0};
    xge_action_binding_t left={XGE_ACTION_KEY,0,0,-1},right={XGE_ACTION_KEY,0,1,1},axis={XGE_ACTION_GAMEPAD_AXIS,-1,2,1};
    CHECK(!xgeActionsBind(a,0,&left)&&!xgeActionsBind(a,0,&right)&&!xgeActionsBind(a,1,&axis)&&!xgeActionsSetDeadzone(a,1,.2f));
    input[0]=1;input[2]=.6f;CHECK(!xgeActionsUpdate(a,read_action,input));xge_action_state_t s,old;
    CHECK(!xgeActionsGet(a,0,&s)&&s.value==-1&&s.down&&s.pressed&&!s.released);
    CHECK(!xgeActionsGet(a,1,&s)&&fabsf(s.value-.5f)<1e-6f&&s.pressed);old=s;
    input[2]=NAN;CHECK(xgeActionsUpdate(a,read_action,input)==XGE_ERROR_INVALID_ARGUMENT);CHECK(!xgeActionsGet(a,1,&s)&&!memcmp(&s,&old,sizeof(s)));
    input[2]=.1f;input[1]=1;CHECK(!xgeActionsUpdate(a,read_action,input));CHECK(!xgeActionsGet(a,0,&s)&&!s.value&&!s.down&&s.released);
    CHECK(!xgeActionsGet(a,1,&s)&&!s.value&&s.released);
    input[0]=0;CHECK(!xgeActionsUpdate(a,read_action,input)&&!xgeActionsGet(a,0,&s)&&s.value==1&&s.pressed);
    CHECK(!xgeActionsSetEnabled(a,0,0)&&!xgeActionsUpdate(a,read_action,input)&&!xgeActionsGet(a,0,&s)&&!s.down&&!s.released);
    CHECK(!xgeActionsSetEnabled(a,0,1)&&!xgeActionsClearBindings(a,0));left.scale=1;CHECK(!xgeActionsBind(a,0,&left));
    input[0]=1;CHECK(!xgeActionsUpdate(a,read_action,input)&&!xgeActionsGet(a,0,&s)&&s.pressed);xgeActionsReset(a);
    CHECK(!xgeActionsGet(a,0,&s)&&!s.down&&!s.pressed&&!s.released);CHECK(xgeActionsSetDeadzone(a,1,1)==XGE_ERROR_INVALID_ARGUMENT);
    CHECK(!xgeActionsBind(a,2,&left)&&!xgeActionsBind(a,2,&right));
    CHECK(xgeActionsBind(a,2,&left)==XGE_ERROR_BUFFER_TOO_SMALL);
    xallocationscope scope={0};xallocationbudget *quota=NULL;
    CHECK(xrtAllocationBudgetCreate(1,&quota)&&xrtAllocationBudgetEnter(quota,&scope));
    CHECK(!xgeActionsUpdate(a,read_action,input)&&!scope.Rejections);CHECK(xrtAllocationBudgetLeave(&scope));xrtAllocationBudgetFree(quota);
    xgeActionsFree(a);return 0;
}
#endif
#if XGE3D_ENABLE_PARTICLES
static int particles(void)
{
    xge3d_particle_desc_t d=xge3dParticlesDefault();d.capacity=3;d.rate=0;d.life_min=d.life_max=.5f;d.speed_min=d.speed_max=2;d.spread=0;d.gravity=(xge3d_vec3_t){0,-4,0};d.columns=2;d.rows=2;d.fps=4;
    xge3d_particles *a=NULL,*b=NULL;CHECK(!xge3dParticlesCreate(&d,&a)&&!xge3dParticlesCreate(&d,&b));
    CHECK(!xge3dParticlesEmit(a,5)&&!xge3dParticlesEmit(b,5));xge3d_particle_info_t info={.size=sizeof(info)};
    CHECK(!xge3dParticlesInfo(a,&info)&&info.alive==3&&info.emitted==3&&info.dropped==2);
    CHECK(!xge3dParticlesUpdate(a,.25f,0)&&!xge3dParticlesUpdate(b,.25f,0));xge3d_billboard_t ba[3],bb[3];size_t n;
    CHECK(!xge3dParticlesBillboards(a,NULL,0,ba,3,&n)&&n==3&&!xge3dParticlesBillboards(b,NULL,0,bb,3,&n));
    for(int i=0;i<3;++i){CHECK(fabsf(ba[i].position.y-.375f)<1e-5f&&fabsf(ba[i].position.x)<1e-6f&&fabsf(ba[i].position.z)<1e-6f);
        CHECK(!memcmp(&ba[i].position,&bb[i].position,sizeof(ba[i].position))&&ba[i].rotation==bb[i].rotation&&ba[i].color[3]>.49f&&ba[i].color[3]<.51f&&ba[i].uv[0]==.5f);}
    CHECK(xge3dParticlesBillboards(a,NULL,0,ba,1,&n)==XGE_ERROR_BUFFER_TOO_SMALL&&n==3);
    CHECK(xge3dParticlesUpdate(a,2,0)==XGE_ERROR_INVALID_ARGUMENT&&!xge3dParticlesInfo(a,&info)&&info.alive==3);
    CHECK(!xge3dParticlesUpdate(a,.3f,0)&&!xge3dParticlesInfo(a,&info)&&!info.alive);
    xallocationscope scope={0};xallocationbudget *quota=NULL;
    CHECK(xrtAllocationBudgetCreate(1,&quota)&&xrtAllocationBudgetEnter(quota,&scope));
    CHECK(!xge3dParticlesEmit(a,3)&&!xge3dParticlesUpdate(a,.1f,0)&&!xge3dParticlesBillboards(a,NULL,0,ba,3,&n)&&!scope.Rejections);
    CHECK(xrtAllocationBudgetLeave(&scope));xrtAllocationBudgetFree(quota);
    xge3dParticlesFree(a);xge3dParticlesFree(b);return 0;
}
#endif
#if XGE3D_ENABLE_MOTION
static int wall(xge3d_scene *s,xge3d_node_t *node,xge3d_vec3_t center,xge3d_vec3_t half)
{xge3d_shape_t shape=xge3dShapeDefault(XGE3D_SHAPE_BOX);shape.center=center;shape.half_extents=half;int result=xge3dNodeCreate(s,(xge3d_node_t){0},node);return result?result:xge3dNodeSetCollider(s,*node,&shape);}
static int motion(void)
{
    xge3d_scene *s=NULL;CHECK(!xge3dSceneCreate(&s));xge3d_node_t first,second;
    CHECK(!wall(s,&first,(xge3d_vec3_t){2,0,0},(xge3d_vec3_t){.5f,5,5}));
    xge3d_shape_t body=xge3dShapeDefault(XGE3D_SHAPE_SPHERE);xge3d_mat4_t world=identity();xge3d_motion_result_t moved={.size=sizeof(moved)};
    CHECK(!xge3dSceneMoveShape(s,&body,&world,(xge3d_vec3_t){3,0,2},NULL,NULL,&moved));
    CHECK((moved.flags&XGE3D_MOTION_BLOCKED)&&!(moved.flags&XGE3D_MOTION_LIMITED)&&moved.displacement.x>.99f&&moved.displacement.x<1.001f&&fabsf(moved.displacement.z-2)<1e-4f);
    world.m[12]=1.25f;size_t n=0;xge3d_contact_t contact[4];CHECK(!xge3dSceneContactShape(s,&body,&world,NULL,contact,4,&n)&&n==1);
    CHECK(contact[0].depth>.249f&&contact[0].depth<.252f&&contact[0].hit.normal.x<-.999f);
    CHECK(!xge3dSceneMoveShape(s,&body,&world,(xge3d_vec3_t){0,0,1},NULL,NULL,&moved));
    CHECK((moved.flags&XGE3D_MOTION_RECOVERED)&&!(moved.flags&XGE3D_MOTION_LIMITED)&&moved.recovery.x<-.249f&&fabsf(moved.displacement.z-1)<1e-4f);
    xge3d_motion_settings_t settings=xge3dMotionDefault();settings.max_recovery=.1f;
    CHECK(!xge3dSceneMoveShape(s,&body,&world,(xge3d_vec3_t){0,0,1},NULL,&settings,&moved)&&(moved.flags&XGE3D_MOTION_LIMITED)&&!moved.displacement.z);
    world.m[12]=1;int away=xge3dSceneMoveShape(s,&body,&world,(xge3d_vec3_t){-1,0,0},NULL,NULL,&moved);
    if(away||fabsf(moved.displacement.x+1)>=1e-4f){fprintf(stderr,"touch away: result %d flags %u displacement %f,%f,%f\n",away,moved.flags,moved.displacement.x,moved.displacement.y,moved.displacement.z);}
    CHECK(!away&&fabsf(moved.displacement.x+1)<1e-4f);
    CHECK(!wall(s,&second,(xge3d_vec3_t){0,0,2},(xge3d_vec3_t){5,5,.5f}));world=identity();
    CHECK(!xge3dSceneMoveShape(s,&body,&world,(xge3d_vec3_t){3,1,3},NULL,NULL,&moved));
    CHECK(moved.contact_count>=2&&moved.displacement.x<1.001f&&moved.displacement.z<1.001f&&fabsf(moved.displacement.y-1)<1e-4f);
    xge3d_collision_filter_t filter={0,{0}};CHECK(!xge3dSceneMoveShape(s,&body,&world,(xge3d_vec3_t){3,1,3},&filter,NULL,&moved)&&moved.displacement.x==3&&!moved.flags);
    xge3d_motion_result_t old=moved;settings.margin=NAN;CHECK(xge3dSceneMoveShape(s,&body,&world,(xge3d_vec3_t){1,0,0},NULL,&settings,&moved)==XGE_ERROR_INVALID_ARGUMENT&&!memcmp(&old,&moved,sizeof(old)));
    xge3dSceneFree(s);return 0;
}
static int contacts(void)
{
    for(int a=0;a<3;++a){for(int b=0;b<3;++b){xge3d_scene *s=NULL;CHECK(!xge3dSceneCreate(&s));xge3d_node_t node;
        CHECK(!xge3dNodeCreate(s,(xge3d_node_t){0},&node));xge3d_shape_t query=xge3dShapeDefault(a),target=xge3dShapeDefault(b);CHECK(!xge3dNodeSetCollider(s,node,&target));
        xge3d_mat4_t other=identity();other.m[0]=cosf(.3f);other.m[2]=-sinf(.3f);other.m[8]=sinf(.3f);other.m[10]=cosf(.3f);other.m[4]=.25f;
        CHECK(!xge3dNodeSetMatrix(s,node,&other));xge3d_mat4_t at=identity();at.m[0]=1.1f;at.m[5]=.7f;at.m[10]=1.2f;at.m[12]=.2f;
        xge3d_contact_t c;size_t n=0;int result=xge3dSceneContactShape(s,&query,&at,NULL,&c,1,&n);
        if(result){fprintf(stderr,"affine contact %d/%d result %d\n",a,b,result);}CHECK(!result&&n==1&&c.depth>0);
        CHECK(fabsf(c.hit.normal.x*c.hit.normal.x+c.hit.normal.y*c.hit.normal.y+c.hit.normal.z*c.hit.normal.z-1)<1e-5f);
        at.m[12]+=c.hit.normal.x*(c.depth+.001f);at.m[13]+=c.hit.normal.y*(c.depth+.001f);at.m[14]+=c.hit.normal.z*(c.depth+.001f);
        CHECK(!xge3dSceneOverlapShape(s,&query,&at,NULL,NULL,0,&n)&&!n);xge3dSceneFree(s);
    }}
    xge3d_scene *s=NULL;CHECK(!xge3dSceneCreate(&s));xge3d_node_t node;CHECK(!xge3dNodeCreate(s,(xge3d_node_t){0},&node));
    xge3d_shape_t target=xge3dShapeDefault(XGE3D_SHAPE_SPHERE),query=target;CHECK(!xge3dNodeSetCollider(s,node,&target));
    for(int i=1;i<=80;++i){float x=.01f*i,y=.13f,z=.09f;float distance=sqrtf(x*x+y*y+z*z);xge3d_mat4_t at=identity();at.m[12]=x;at.m[13]=y;at.m[14]=z;
        xge3d_contact_t c;size_t n;CHECK(!xge3dSceneContactShape(s,&query,&at,NULL,&c,1,&n)&&n==1&&fabsf(c.depth-(1-distance))<1e-5f&&fabsf(c.hit.normal.x-x/distance)<1e-5f);
    }
    xge3d_vertex_t vertices[3]={0};vertices[0].position=(xge3d_vec3_t){-10,0,-10};vertices[1].position=(xge3d_vec3_t){10,0,-10};vertices[2].position=(xge3d_vec3_t){0,0,10};
    xge3d_mesh_desc_t mesh_desc={vertices,3};xge3d_mesh *mesh=NULL;CHECK(!xge3dMeshCreate(&mesh_desc,&mesh));target=xge3dShapeDefault(XGE3D_SHAPE_MESH);target.mesh=mesh;CHECK(!xge3dNodeSetCollider(s,node,&target));
    query=xge3dShapeDefault(XGE3D_SHAPE_CAPSULE);xge3d_mat4_t at=identity();at.m[13]=.3f;xge3d_contact_t c;size_t n;
    CHECK(!xge3dSceneContactShape(s,&query,&at,NULL,&c,1,&n)&&n==1&&fabsf(c.depth-.7f)<.001f&&c.hit.normal.y>.999f);
    xge3d_motion_result_t result={.size=sizeof(result)};CHECK(!xge3dSceneMoveShape(s,&query,&at,(xge3d_vec3_t){1,0,1},NULL,NULL,&result));
    CHECK((result.flags&XGE3D_MOTION_RECOVERED)&&!(result.flags&XGE3D_MOTION_LIMITED)&&result.recovery.y>.699f&&fabsf(result.displacement.x-1)<1e-4f);
    xge3dSceneFree(s);xge3dMeshFree(mesh);return 0;
}
#endif
#if XGE3D_ENABLE_DYNAMIC
static int dynamic(void)
{
    unsigned char red[]={255,0,0,255};xge_image_t image={.iWidth=1,.iHeight=1,.iStride=4,.iFormat=XGE_PIXEL_RGBA8,.pPixels=red};
    xge3d_texture_desc_t td={.image=&image};xge3d_texture *t=NULL;CHECK(!xge3dTextureCreate(&td,&t));xge3d_material *m=NULL;
    xge3d_material_desc_t d=xge3dMaterialDefault();d.maps[0].texture=t;CHECK(!xge3dMaterialCreate(&d,&m));
    d.base_color[0]=.25f;CHECK(!xge3dMaterialSetDesc(m,&d));xge3d_material_desc_t out;CHECK(!xge3dMaterialGetDesc(m,&out)&&out.base_color[0]==.25f);
    d.roughness=NAN;CHECK(xge3dMaterialSetDesc(m,&d)==XGE_ERROR_INVALID_ARGUMENT&&!xge3dMaterialGetDesc(m,&out)&&out.base_color[0]==.25f&&out.roughness==1);
    CHECK(!xge3dTextureUpdate(t,0,0,1,1,red,4)&&xge3dTextureUpdate(t,1,0,1,1,red,4)==XGE_ERROR_INVALID_ARGUMENT);
    xge3dMaterialFree(m);xge3dTextureFree(t);return 0;
}
#endif
int main(void)
{
    (void)identity;
#if XGE_ENABLE_INPUT_ACTIONS
    CHECK(!actions());
#endif
#if XGE3D_ENABLE_PARTICLES
    CHECK(!particles());
#endif
#if XGE3D_ENABLE_MOTION
    CHECK(!motion()&&!contacts());
#endif
#if XGE3D_ENABLE_DYNAMIC
    CHECK(!dynamic());
#endif
    puts("Game foundation CPU: actions, particles, contacts/slide/recovery and dynamic material contracts passed");return 0;
}
