#include "xge3d_motion_internal.h"

void motion_init(motion_asset *a)
{
    memset(a,0,sizeof(*a));xrtArrayInit(&a->nodes,sizeof(motion_node));xrtArrayInit(&a->times,sizeof(float));xrtArrayInit(&a->poses,sizeof(ufbx_transform));
}
char *motion_string(const char *s)
{
    size_t n=strlen(s)+1;char *out=xrtMalloc(n);if (out) memcpy(out,s,n);return out;
}
void motion_free(motion_asset *a)
{
    for (size_t i=0;i<a->nodes.Count;++i) xrtFree(((motion_node*)a->nodes.Data)[i].name);
    xrtArrayUnit(&a->nodes);xrtArrayUnit(&a->times);xrtArrayUnit(&a->poses);xrtFree(a->name);memset(a,0,sizeof(*a));
}
static int option_number(const char *text,double *out)
{
    char *end;*out=strtod(text,&end);return *text && !*end && isfinite(*out);
}
int main(int argc,char **argv)
{
    if (argc<3) {fprintf(stderr,"Usage: xge3d_motion list INPUT\n       xge3d_motion convert INPUT OUTPUT.gltf [--clip NAME|INDEX] [--fps 30] [--start SEC] [--end SEC] [--unit 0.01] [--up y|z] [--target MODEL.gltf --map BONES.json]\n");return 1;}
    motion_options options={.fps=30,.end=-1,.unit=.01};const char *target=NULL,*mapping=NULL;
    options.list=!strcmp(argv[1],"list");
    if ((!options.list && strcmp(argv[1],"convert")) || (!options.list && argc<4)) return 1;
    for (int i=options.list ? 3 : 4;i<argc;++i) {
        const char *flag=argv[i];if (++i>=argc) {fprintf(stderr,"Missing value for %s\n",flag);return 1;}
        if (!strcmp(flag,"--clip")) options.clip=argv[i];
        else if (!strcmp(flag,"--target")) target=argv[i];
        else if (!strcmp(flag,"--map")) mapping=argv[i];
        else if (!strcmp(flag,"--up")) {if (strcmp(argv[i],"y") && strcmp(argv[i],"z")) return 1;options.z_up=!strcmp(argv[i],"z");}
        else {
            double number;if (!option_number(argv[i],&number)) return 1;
            if (!strcmp(flag,"--fps")) options.fps=number;
            else if (!strcmp(flag,"--start")) options.start=number;
            else if (!strcmp(flag,"--end")) options.end=number;
            else if (!strcmp(flag,"--unit")) options.unit=number;
            else {fprintf(stderr,"Unknown option %s\n",flag);return 1;}
        }
    }
    if (options.fps<1 || options.fps>240 || options.start<0 || (options.end<0 && options.end!=-1) || options.unit<=0 || (!!target!=!!mapping)) return 1;
    motion_asset asset;motion_init(&asset);
    int ok=motion_load(argv[2],&options,&asset);
    if (ok && target && !options.list) ok=motion_retarget(&asset,target,mapping);
    if (ok && !options.list) ok=motion_write(argv[3],&asset);
    if (ok && !options.list) printf("%s: %zu nodes, %zu samples, %.6g seconds, %s\n",argv[3],asset.nodes.Count,asset.times.Count,
        asset.times.Count ? ((float*)asset.times.Data)[asset.times.Count-1] : 0,asset.name);
    motion_free(&asset);return ok ? 0 : 1;
}
