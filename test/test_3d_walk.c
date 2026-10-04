/* Exercise the tutorial's actual movement/camera code through public XGE APIs. */
#define main tutorial_main
#include "../examples/xge_3d_walk/main.c"
#undef main
#define CHECK(x) do {if (!(x)) {fprintf(stderr,"%s:%d: %s\n",__FILE__,__LINE__,#x);goto done;}} while(0)
int main(void)
{
    int failed=1;demo d={.seed=42};float reference[10],height;xge3d_vec3_t normal;
    CHECK(xge3dSceneCreate(&d.scene)==XGE_OK && make_terrain(&d)==XGE_OK && make_actor(&d)==XGE_OK);
    CHECK(d.max_height-d.min_height>25);
    for (int i=0;i<10;++i) CHECK(ground(&d,i*3,i*-2,&reference[i],&normal)==XGE_OK);
    size_t nodes=xge3dSceneNodeCount(d.scene);
    CHECK(make_terrain(&d)==XGE_OK && xge3dSceneNodeCount(d.scene)==nodes);
    for (int i=0;i<10;++i) CHECK(ground(&d,i*3,i*-2,&height,&normal)==XGE_OK && height==reference[i]);
    ++d.seed;CHECK(make_terrain(&d)==XGE_OK && xge3dSceneNodeCount(d.scene)==nodes);
    CHECK(ground(&d,0,0,&height,&normal)==XGE_OK && fabsf(height-reference[0])>.01f);
    /* Normalizing input must preserve speed for diagonal movement. */
    CHECK(update_player(&d,(controls){1,0,0,0,0},.05f)==XGE_OK);float straight=d.travel;d.travel=0;CHECK(reset_player(&d)==XGE_OK);
    CHECK(update_player(&d,(controls){1,1,0,0,0},.05f)==XGE_OK && fabsf(d.travel-straight)<.0001f);
    d.travel=0;CHECK(reset_player(&d)==XGE_OK);
    CHECK(update_player(&d,(controls){1,0,0,1,0},.05f)==XGE_OK && fabsf(d.travel-2*straight)<.0001f && d.action==2);
    /* Fixed duration at different outer frame rates gives the same position. */
    CHECK(reset_player(&d)==XGE_OK);
    for (int i=0;i<60;++i) CHECK(update_player(&d,(controls){1,0,0,0,0},1.f/60)==XGE_OK);
    xge3d_vec3_t sixty=d.position;CHECK(reset_player(&d)==XGE_OK);
    for (int i=0;i<20;++i) CHECK(update_player(&d,(controls){1,0,0,0,0},.05f)==XGE_OK);
    CHECK(fabsf(d.position.x-sixty.x)<.001f && fabsf(d.position.y-sixty.y)<.001f && fabsf(d.position.z-sixty.z)<.001f);
    CHECK(update_player(&d,(controls){0,0,0,0,1},1.f/60)==XGE_OK && !d.grounded && d.action==3);
    float peak=d.position.y;
    for (int i=0;i<90;++i) {
        CHECK(update_player(&d,(controls){0},1.f/60)==XGE_OK);
        CHECK(ground(&d,d.position.x,d.position.z,&height,&normal)==XGE_OK && d.position.y>=height-.0001f);
        peak=fmaxf(peak,d.position.y);
    }
    CHECK(d.grounded && d.jumps==1 && d.landings==1 && peak-height>.9f && d.action==0);
    /* Find the positive-X shore and ensure walking stops on dry land. */
    for (float x=0;x<HALF;x+=.1f) {
        CHECK(ground(&d,x,0,&height,&normal)==XGE_OK);
        if (height>.25f && normal.y>.68f) d.position=(xge3d_vec3_t){x,height,0};
        else if (x>120) break;
    }
    d.yaw=0;
    for (int i=0;i<120;++i) CHECK(update_player(&d,(controls){0,1,0,0,0},1.f/60)==XGE_OK);
    CHECK(d.blocked>0 && ground(&d,d.position.x,d.position.z,&height,&normal)==XGE_OK && height>=.25f);
    CHECK(fabsf(d.position.y-height)<.0001f);
    /* Exercise camera terrain avoidance and both visibility modes. */
    xge_desc_t window={0};window.iWidth=640;window.iHeight=480;window.sTitle="XGE tutorial movement checks";
    CHECK(xgeInit(&window)==XGE_OK);CHECK(reset_player(&d)==XGE_OK);d.distance=18;d.pitch=.04f;
    xge3d_camera_t view;CHECK(camera(&d,&view)==XGE_OK && ground(&d,d.eye.x,d.eye.z,&height,&normal)==XGE_OK && d.eye.y>=height+.44f);
    d.first_person=1;CHECK(camera(&d,&view)==XGE_OK && fabsf(d.eye.y-d.position.y-1.67f)<.0001f);
    d.first_person=0;CHECK(camera(&d,&view)==XGE_OK);
    failed=0;puts("Island tutorial: seeded regeneration, unchanged live actor, diagonal/run speed, frame independence, triangle grounding, jump/landing, dry shoreline and camera modes passed");
done:
    release(&d);xgeUnit();return failed;
}
