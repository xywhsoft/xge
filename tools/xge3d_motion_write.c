#include "xge3d_motion_internal.h"

static int name(xjsonwriter *w,const char *key) {return xrtJsonWriterName(w,xrtStrView(key));}
static int number(xjsonwriter *w,const char *key,double value) {return name(w,key) && xrtJsonWriterFloat(w,value);}
static int string(xjsonwriter *w,const char *key,const char *value) {return name(w,key) && xrtJsonWriterString(w,xrtStrView(value));}
static int vector(xjsonwriter *w,const char *key,const double *v,int n)
{
    if (!name(w,key) || !xrtJsonWriterArray(w)) return 0;
    for (int i=0;i<n;++i) if (!xrtJsonWriterFloat(w,v[i])) return 0;
    return xrtJsonWriterEnd(w);
}
#define J(call) do {if (!(call)) goto done;} while (0)
int motion_write(const char *path,const motion_asset *a)
{
    size_t nodes=a->nodes.Count,frames=a->times.Count;
    if (!nodes || !frames || a->poses.Count!=nodes*frames || frames>(MOTION_MAX_BYTES/sizeof(float))/(1+nodes*10)) return 0;
    int ok=0;xarray binary;xrtArrayInit(&binary,sizeof(float));char *encoded=NULL,*uri=NULL,*json=NULL;xjsonwriter *w=NULL;
    J(xrtArrayResize(&binary,frames*(1+nodes*10)));float *out=(float*)binary.Data;
    memcpy(out,a->times.Data,frames*sizeof(float));size_t offset=frames;
    for (size_t n=0;n<nodes;++n) for (int path_index=0;path_index<3;++path_index) {
        int components=path_index==1 ? 4 : 3;ufbx_quat previous={.w=1};
        for (size_t f=0;f<frames;++f) {
            ufbx_transform pose=((ufbx_transform*)a->poses.Data)[f*nodes+n];
            double *value=path_index==0 ? pose.translation.v : path_index==1 ? pose.rotation.v : pose.scale.v;
            if (path_index==1) {
                double dot=0;for (int i=0;i<4;++i) dot+=previous.v[i]*pose.rotation.v[i];
                if (dot<0) for (int i=0;i<4;++i) pose.rotation.v[i]=-pose.rotation.v[i];previous=pose.rotation;
            }
            for (int i=0;i<components;++i) {float number=(float)value[i];J(isfinite(number));out[offset++]=number;}
        }
    }
    encoded=xrtBase64EncodeNew(binary.Data,binary.Count*sizeof(float),NULL);J(encoded);size_t encoded_size=strlen(encoded);
    const char *prefix="data:application/octet-stream;base64,";size_t prefix_size=strlen(prefix);
    uri=xrtMalloc(prefix_size+encoded_size+1);J(uri);memcpy(uri,prefix,prefix_size);memcpy(uri+prefix_size,encoded,encoded_size+1);
    xjsonwriteconfig config;xrtJsonWriteConfigInit(&config);config.MaxOutputBytes=MOTION_MAX_BYTES*2u;
    w=xrtJsonWriterCreate(&config);J(w);J(xrtJsonWriterObject(w));
    J(name(w,"asset"));J(xrtJsonWriterObject(w));J(string(w,"version","2.0"));J(string(w,"generator","xge3d_motion C offline converter"));J(xrtJsonWriterEnd(w));
    J(number(w,"scene",0));J(name(w,"scenes"));J(xrtJsonWriterArray(w));J(xrtJsonWriterObject(w));J(name(w,"nodes"));J(xrtJsonWriterArray(w));
    for (size_t i=0;i<nodes;++i) if (((motion_node*)a->nodes.Data)[i].parent==SIZE_MAX) J(xrtJsonWriterUInt(w,i));
    J(xrtJsonWriterEnd(w));J(xrtJsonWriterEnd(w));J(xrtJsonWriterEnd(w));
    J(name(w,"nodes"));J(xrtJsonWriterArray(w));
    for (size_t i=0;i<nodes;++i) {
        const motion_node *node=xrtArrayConstGet(&a->nodes,i);J(xrtJsonWriterObject(w));J(string(w,"name",node->name));
        J(vector(w,"translation",node->rest.translation.v,3));J(vector(w,"rotation",node->rest.rotation.v,4));J(vector(w,"scale",node->rest.scale.v,3));
        int children=0;for (size_t j=0;j<nodes;++j) children+=((motion_node*)a->nodes.Data)[j].parent==i;
        if (children) {
            J(name(w,"children"));J(xrtJsonWriterArray(w));
            for (size_t j=0;j<nodes;++j) if (((motion_node*)a->nodes.Data)[j].parent==i) J(xrtJsonWriterUInt(w,j));
            J(xrtJsonWriterEnd(w));
        }
        J(xrtJsonWriterEnd(w));
    }
    J(xrtJsonWriterEnd(w));J(name(w,"buffers"));J(xrtJsonWriterArray(w));J(xrtJsonWriterObject(w));
    J(number(w,"byteLength",binary.Count*sizeof(float)));J(string(w,"uri",uri));J(xrtJsonWriterEnd(w));J(xrtJsonWriterEnd(w));
    J(name(w,"bufferViews"));J(xrtJsonWriterArray(w));offset=0;
    for (size_t i=0;i<1+nodes*3;++i) {
        size_t size=frames*(i==0 ? 1 : (i-1)%3==1 ? 4 : 3);
        J(xrtJsonWriterObject(w));J(number(w,"buffer",0));J(number(w,"byteOffset",offset*sizeof(float)));J(number(w,"byteLength",size*sizeof(float)));
        J(xrtJsonWriterEnd(w));offset+=size;
    }
    J(xrtJsonWriterEnd(w));J(name(w,"accessors"));J(xrtJsonWriterArray(w));
    for (size_t i=0;i<1+nodes*3;++i) {
        J(xrtJsonWriterObject(w));J(number(w,"bufferView",i));J(number(w,"componentType",5126));J(number(w,"count",frames));
        J(string(w,"type",i==0 ? "SCALAR" : (i-1)%3==1 ? "VEC4" : "VEC3"));
        if (!i) {double first=((float*)a->times.Data)[0],last=((float*)a->times.Data)[frames-1];J(vector(w,"min",&first,1));J(vector(w,"max",&last,1));}
        J(xrtJsonWriterEnd(w));
    }
    J(xrtJsonWriterEnd(w));J(name(w,"animations"));J(xrtJsonWriterArray(w));J(xrtJsonWriterObject(w));J(string(w,"name",a->name));
    J(name(w,"samplers"));J(xrtJsonWriterArray(w));
    for (size_t i=0;i<nodes*3;++i) {J(xrtJsonWriterObject(w));J(number(w,"input",0));J(number(w,"output",i+1));J(string(w,"interpolation","LINEAR"));J(xrtJsonWriterEnd(w));}
    J(xrtJsonWriterEnd(w));J(name(w,"channels"));J(xrtJsonWriterArray(w));
    const char *paths[]={"translation","rotation","scale"};
    for (size_t i=0;i<nodes*3;++i) {
        J(xrtJsonWriterObject(w));J(number(w,"sampler",i));J(name(w,"target"));J(xrtJsonWriterObject(w));J(number(w,"node",i/3));J(string(w,"path",paths[i%3]));
        J(xrtJsonWriterEnd(w));J(xrtJsonWriterEnd(w));
    }
    J(xrtJsonWriterEnd(w));J(xrtJsonWriterEnd(w));J(xrtJsonWriterEnd(w));J(xrtJsonWriterEnd(w));J(xrtJsonWriterFinish(w));
    size_t size;json=xrtJsonWriterTake(w,&size);J(json);J(xrtFileWriteAtomic(path,(xbytesview){(const unsigned char*)json,size}));ok=1;
done:
    if (!ok) fprintf(stderr,"Cannot write motion: %s (invalid data, allocation or file write failed)\n",path);
    xrtJsonWriterFree(w);xrtFree(json);xrtFree(uri);xrtFree(encoded);xrtArrayUnit(&binary);return ok;
}
