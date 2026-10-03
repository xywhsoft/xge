/* Offline LDR environment preparation. No preprocessing runs in the renderer. */
#include "../src/xge3d_internal.h"
#include <stdio.h>
#include <stdlib.h>

static const char *face_names[]={"px","nx","py","ny","pz","nz"};
static xge_image_t input[6];
static const float pi=3.14159265358979323846f;
static float radical(unsigned n)
{
    n=(n<<16)|(n>>16);n=((n&0x55555555u)<<1)|((n&0xaaaaaaaau)>>1);
    n=((n&0x33333333u)<<2)|((n&0xccccccccu)>>2);
    n=((n&0x0f0f0f0fu)<<4)|((n&0xf0f0f0f0u)>>4);
    n=((n&0x00ff00ffu)<<8)|((n&0xff00ff00u)>>8);return n*2.3283064365386963e-10f;
}
static xge3d_vec3_t direction(int f,float u,float v)
{
    xge3d_vec3_t n;
    switch (f) {
        case 0:n=(xge3d_vec3_t){1,-v,-u};break;case 1:n=(xge3d_vec3_t){-1,-v,u};break;
        case 2:n=(xge3d_vec3_t){u,1,v};break;case 3:n=(xge3d_vec3_t){u,-1,-v};break;
        case 4:n=(xge3d_vec3_t){u,-v,1};break;default:n=(xge3d_vec3_t){-u,-v,-1};break;
    }
    d3_normalize(&n);return n;
}
static float linear(unsigned char byte)
{ float c=byte/255.0f;return c<=.04045f ? c/12.92f : powf((c+.055f)/1.055f,2.4f); }
static xge3d_vec3_t sample(xge3d_vec3_t n)
{
    float ax=fabsf(n.x),ay=fabsf(n.y),az=fabsf(n.z),u,v;int f;
    if (ax>=ay && ax>=az) { f=n.x>0 ? 0 : 1;u=n.x>0 ? -n.z/ax : n.z/ax;v=-n.y/ax; }
    else if (ay>=az) { f=n.y>0 ? 2 : 3;u=n.x/ay;v=n.y>0 ? n.z/ay : -n.z/ay; }
    else { f=n.z>0 ? 4 : 5;u=n.z>0 ? n.x/az : -n.x/az;v=-n.y/az; }
    const xge_image_t *i=&input[f];float x=(u*.5f+.5f)*i->iWidth-.5f,y=(v*.5f+.5f)*i->iHeight-.5f;
    int x0=(int)floorf(x),y0=(int)floorf(y);float fx=x-x0,fy=y-y0;xge3d_vec3_t result={0};
    for (int row=0;row<2;++row) for (int col=0;col<2;++col) {
        int px=x0+col,py=y0+row;px=px<0 ? 0 : px>=i->iWidth ? i->iWidth-1 : px;py=py<0 ? 0 : py>=i->iHeight ? i->iHeight-1 : py;
        const unsigned char *p=(const unsigned char*)i->pPixels+(size_t)py*i->iStride+px*4;
        float weight=(col ? fx : 1-fx)*(row ? fy : 1-fy);
        result=d3_add(result,d3_scale((xge3d_vec3_t){linear(p[0]),linear(p[1]),linear(p[2])},weight));
    }
    return result;
}
static xge3d_vec3_t hemisphere(xge3d_vec3_t n,float u,float cos_theta)
{
    xge3d_vec3_t up=fabsf(n.z)<.999f ? (xge3d_vec3_t){0,0,1} : (xge3d_vec3_t){1,0,0};
    xge3d_vec3_t t=d3_cross(up,n);d3_normalize(&t);xge3d_vec3_t b=d3_cross(n,t);
    float sine=sqrtf(fmaxf(0,1-cos_theta*cos_theta)),phi=2*pi*u;
    return d3_add(d3_scale(n,cos_theta),d3_add(d3_scale(t,cosf(phi)*sine),d3_scale(b,sinf(phi)*sine)));
}
static xge3d_vec3_t ggx(xge3d_vec3_t n,float u,float v,float rough)
{ float a=rough*rough;return hemisphere(n,u,sqrtf((1-v)/(1+(a*a-1)*v))); }
static unsigned char pack(float v)
{ return (unsigned char)lroundf(fminf(fmaxf(v,0),1)*255); }
static int cube(const char *prefix,int size,int samples,int irradiance,int level,int levels)
{
    xarray pixels;xrtArrayInit(&pixels,1);
    if (!xrtArrayResize(&pixels,(size_t)size*size*4)) return XGE_ERROR_OUT_OF_MEMORY;
    int result=XGE_OK;
    for (int f=0;f<6 && result==XGE_OK;++f) {
        for (int y=0;y<size;++y) for (int x=0;x<size;++x) {
            xge3d_vec3_t n=direction(f,2*(x+.5f)/size-1,2*(y+.5f)/size-1),color={0};float weights=0;
            float rough=levels>1 ? level/(float)(levels-1) : 0;
            for (int k=0;k<samples;++k) {
                float u=k/(float)samples,v=radical((unsigned)k);xge3d_vec3_t l;float weight=1;
                if (irradiance) l=hemisphere(n,u,sqrtf(1-v));
                else { xge3d_vec3_t h=ggx(n,u,v,rough);l=d3_sub(d3_scale(h,2*d3_dot(n,h)),n);weight=fmaxf(d3_dot(n,l),0); }
                if (weight>0) { color=d3_add(color,d3_scale(sample(l),weight));weights+=weight; }
            }
            color=d3_scale(color,1/fmaxf(weights,1e-8f));unsigned char *p=pixels.Data+((size_t)y*size+x)*4;
            p[0]=pack(color.x);p[1]=pack(color.y);p[2]=pack(color.z);p[3]=255;
        }
        char path[1024];int length=irradiance ? snprintf(path,sizeof(path),"%s-irr-%s.png",prefix,face_names[f]) :
            snprintf(path,sizeof(path),"%s-spec-%d-%s.png",prefix,level,face_names[f]);
        result=length<0 || (size_t)length>=sizeof(path) ? XGE_ERROR_INVALID_ARGUMENT :
            xgeImageSavePNGEx(path,size,size,pixels.Data,size*4,XGE_IMAGE_STRAIGHT_ALPHA);
    }
    xrtArrayUnit(&pixels);return result;
}
static int brdf(const char *prefix,int samples)
{
    const int size=64;unsigned char pixels[64*64*4];
    for (int y=0;y<size;++y) for (int x=0;x<size;++x) {
        float rough=(y+.5f)/size,nv=(x+.5f)/size,a=rough*rough,a2=a*a,A=0,B=0;
        xge3d_vec3_t v={sqrtf(1-nv*nv),0,nv};
        for (int k=0;k<samples;++k) {
            xge3d_vec3_t h=ggx((xge3d_vec3_t){0,0,1},k/(float)samples,radical((unsigned)k),rough);
            float vh=fmaxf(d3_dot(v,h),0);xge3d_vec3_t l=d3_sub(d3_scale(h,2*vh),v);float nl=fmaxf(l.z,0);
            if (nl<=0) continue;
            float visibility=.5f/fmaxf(nl*sqrtf(nv*nv*(1-a2)+a2)+nv*sqrtf(nl*nl*(1-a2)+a2),1e-6f);
            float weight=4*nl*visibility*vh/fmaxf(h.z,1e-6f),fresnel=powf(1-vh,5);
            A+=(1-fresnel)*weight;B+=fresnel*weight;
        }
        unsigned char *p=pixels+(y*size+x)*4;p[0]=pack(A/samples);p[1]=pack(B/samples);p[2]=0;p[3]=255;
    }
    char path[1024];int length=snprintf(path,sizeof(path),"%s-brdf.png",prefix);
    return length<0 || (size_t)length>=sizeof(path) ? XGE_ERROR_INVALID_ARGUMENT :
        xgeImageSavePNGEx(path,size,size,pixels,size*4,XGE_IMAGE_STRAIGHT_ALPHA);
}
int main(int argc,char **argv)
{
    if (argc<8) { fprintf(stderr,"usage: xge3d_ibl output-prefix +X.png -X.png +Y.png -Y.png +Z.png -Z.png [size=64] [samples=128]\n");return 2; }
    int size=argc>8 ? atoi(argv[8]) : 64,samples=argc>9 ? atoi(argv[9]) : 128;
    if (size<4 || size>256 || (size&(size-1)) || samples<16 || samples>4096) return 2;
    xge_desc_t desc={0};desc.iRunMode=XGE_RUN_MANUAL;
    if (xgeInit(&desc)!=XGE_OK) return 1;
    int result=XGE_OK;
    for (int i=0;i<6 && result==XGE_OK;++i) {
        result=xgeImageLoadEx(&input[i],argv[i+2],XGE_IMAGE_STRAIGHT_ALPHA);
        if (result==XGE_OK && (input[i].iWidth!=input[i].iHeight || input[i].iWidth!=input[0].iWidth ||
            input[i].iFormat!=XGE_PIXEL_RGBA8)) result=XGE_ERROR_INVALID_ARGUMENT;
    }
    int levels=1;for (int mip=size;mip>1;mip/=2) ++levels;
    if (result==XGE_OK) result=cube(argv[1],16,samples,1,0,1);
    for (int level=0;result==XGE_OK && level<levels;++level) result=cube(argv[1],size>>level,samples,0,level,levels);
    if (result==XGE_OK) result=brdf(argv[1],samples*2);
    for (int i=0;i<6;++i) xgeImageFree(&input[i]);
    xgeUnit();
    if (result!=XGE_OK) { fprintf(stderr,"IBL preparation failed: %d\n",result);return 1; }
    printf("IBL: %d roughness levels, six irradiance faces and 64x64 BRDF LUT; linear RGBA8 output\n",levels);return 0;
}
