/* Prefix with xgeGraphicsShaderHeaderGet for desktop GL or GLES/WebGL2. */
static const char d3_sky_vertex_shader[] =
    "out vec2 ndc;\n"
    "void main(){\n"
    " ndc=vec2(gl_VertexID==1?3.0:-1.0,gl_VertexID==2?3.0:-1.0);\n"
    " gl_Position=vec4(ndc,1.0,1.0);\n"
    "}\n";
static const char d3_sky_fragment_shader[] =
    "#ifdef GL_ES\nprecision highp float;\n#endif\n"
    "in vec2 ndc; uniform mat4 skyInverse; uniform samplerCube sky; uniform float exposure; out vec4 frag;\n"
    "void main(){\n"
    " vec4 a=skyInverse*vec4(ndc,-1.0,1.0),b=skyInverse*vec4(ndc,1.0,1.0);\n"
    " vec3 c=texture(sky,b.xyz/b.w-a.xyz/a.w).rgb*exposure;\n"
    " frag=vec4(c,1.0);\n"
    "}\n";
static const char d3_vertex_shader[] =
    "layout(location=0) in vec3 position;\n"
    "layout(location=1) in vec3 normal;\n"
    "layout(location=2) in vec2 uv0;\n"
    "layout(location=3) in vec4 tangent;\n"
    "layout(location=4) in vec2 uv1;\n"
#if XGE3D_ENABLE_ANIMATION
    "layout(location=5) in vec4 joints;layout(location=6) in vec4 weights;\n"
    "uniform sampler2D jointPalette;uniform int skinEnabled;\n"
    "mat4 joint(int index){return mat4(texelFetch(jointPalette,ivec2(0,index),0),texelFetch(jointPalette,ivec2(1,index),0),texelFetch(jointPalette,ivec2(2,index),0),texelFetch(jointPalette,ivec2(3,index),0));}\n"
#endif
    "uniform mat4 mvp, world;\n"
    "uniform mat4 viewProjection;uniform sampler2D instancePalette;uniform int instanceEnabled,instanceOffset,instanceColumns;\n"
    "vec4 instanceColumn(int i,int c){return texelFetch(instancePalette,ivec2((i%instanceColumns)*4+c,i/instanceColumns),0);}\n"
    "out vec2 vUV0, vUV1; out vec3 vPosition, vNormal; out vec4 vTangent;\n"
    "void main(){\n"
    " mat4 skin=mat4(1.0);\n"
#if XGE3D_ENABLE_ANIMATION
    " if(skinEnabled!=0) skin=joint(int(joints.x))*weights.x+joint(int(joints.y))*weights.y+joint(int(joints.z))*weights.z+joint(int(joints.w))*weights.w;\n"
#endif
    " mat4 w=world; if(instanceEnabled!=0){int i=instanceOffset+gl_InstanceID;w=mat4(instanceColumn(i,0),instanceColumn(i,1),instanceColumn(i,2),instanceColumn(i,3));}\n"
    " vec3 p=position,n=normal;\n"
#if XGE3D_ENABLE_SHADER
    "#ifdef D3_CUSTOM_VERTEX\n xge3dVertex(p,n,uv0);\n#endif\n"
#endif
    " vec4 local=skin*vec4(p,1.0);mat3 basis=mat3(w*skin);\n"
    " gl_Position=instanceEnabled!=0?viewProjection*w*local:mvp*local; vUV0=uv0; vUV1=uv1;\n"
    " vPosition=(w*local).xyz;\n"
    " vNormal=transpose(inverse(basis))*n;\n"
    " vTangent=vec4(basis*tangent.xyz,tangent.w*sign(determinant(basis)));\n"
    "}\n";

static const char d3_fragment_shader[] =
    "#ifdef GL_ES\nprecision highp float;\n#endif\n"
    "in vec2 vUV0, vUV1; in vec3 vPosition, vNormal; in vec4 vTangent;\n"
    "uniform vec4 color; uniform vec3 emissive; uniform int alphaMode, mapMask;\n"
    "uniform float alphaCutoff; uniform sampler2D maps[5];\n"
#if XGE3D_ENABLE_DYNAMIC
    "uniform int targetMask;\n"
    "vec4 readMap(sampler2D image,int i,vec2 tc){bool target=(targetMask&(1<<i))!=0;if(target)tc.y=1.0-tc.y;vec4 c=texture(image,tc);\n"
    "if(target){vec3 s=c.a>0.0?c.rgb/c.a:vec3(0.0);c.rgb=mix(s/12.92,pow((s+0.055)/1.055,vec3(2.4)),step(vec3(0.04045),s));}return c;}\n"
#endif
#if !XGE3D_ENABLE_DYNAMIC
    "#define readMap(image,i,tc) texture(image,tc)\n"
#endif
    "uniform float exposure;\n"
#if XGE3D_ENABLE_FOG
    "uniform vec4 fogColor,fogDepth;uniform vec2 fogRange;\n"
#endif
#if XGE3D_ENABLE_HEIGHT_FOG
    "uniform vec4 heightColor,heightParams,heightEye;uniform vec3 heightForward;\n"
    "vec3 heightFog(vec3 lit){\n"
    " if(heightColor.w==0.0)return lit;\n"
    " vec3 ray=vPosition-heightEye.xyz;if(heightEye.w>0.5)ray=heightForward*dot(ray,heightForward);\n"
    " float a=clamp(-heightParams.y*(vPosition.y-ray.y-heightParams.z),-80.0,80.0);\n"
    " float b=clamp(-heightParams.y*(vPosition.y-heightParams.z),-80.0,80.0);\n"
    " float integral=abs(b-a)<0.001?exp((a+b)*0.5):(exp(b)-exp(a))/(b-a);\n"
    " float amount=min(heightParams.w,1.0-exp(-min(80.0,heightParams.x*length(ray)*integral)));\n"
    " return mix(lit,heightColor.rgb,amount);\n"
    "}\n"
#endif
#if XGE3D_ENABLE_LIGHTING
    "uniform vec4 cameraPosition; uniform int lightCount, unlit; uniform vec4 materialParams;\n"
    "uniform vec4 lightPosition[8],lightDirection[8],lightColor[8]; uniform vec2 lightCone[8];\n"
#if XGE3D_ENABLE_IBL
    "uniform samplerCube irradianceMap,prefilteredMap;uniform sampler2D brdfMap;uniform float iblIntensity,iblMaxLod;\n"
#endif
#if XGE3D_ENABLE_SHADOW
    "uniform sampler2D shadowAtlas; uniform mat4 shadowMatrix[6]; uniform int lightMap[8],sunIndex,cascadeCount,receiveShadows;\n"
    "uniform vec4 cascadeSplits,shadowParams,viewDepth; uniform vec2 atlasTexel; uniform float cameraNear;\n"
#endif
#endif
    "uniform int texcoord[5]; uniform vec4 uvTransform[5]; uniform float uvRotation[5];\n"
    "out vec4 frag;\n"
    "vec2 uv(int i){\n"
    " vec2 p=(texcoord[i]==0?vUV0:vUV1)*uvTransform[i].zw;\n"
    " float c=cos(uvRotation[i]),s=sin(uvRotation[i]);\n"
    " return vec2(c*p.x-s*p.y,s*p.x+c*p.y)+uvTransform[i].xy;\n"
    "}\n"
#if XGE3D_ENABLE_LIGHTING
#if XGE3D_ENABLE_SHADOW
    "float shadowSample(int map,vec3 p){\n"
    " vec4 clip=shadowMatrix[map]*vec4(p,1.0); if(clip.w<=0.0) return 1.0;\n"
    " vec3 q=clip.xyz/clip.w*0.5+0.5; if(any(lessThan(q,vec3(0.0))) || any(greaterThan(q,vec3(1.0)))) return 1.0;\n"
    " vec2 lo=vec2(map%3,map/3)/vec2(3.0,2.0),hi=lo+1.0/vec2(3.0,2.0);\n"
    " vec2 tc=lo+q.xy/vec2(3.0,2.0);float sum=0.0;\n"
    " for(int y=-1;y<=1;++y) for(int x=-1;x<=1;++x){\n"
    "  vec2 sampleUV=clamp(tc+vec2(x,y)*atlasTexel*shadowParams.z,lo+atlasTexel*0.5,hi-atlasTexel*0.5);\n"
    "  sum+=q.z-shadowParams.x<=texture(shadowAtlas,sampleUV).r ? 1.0 : 0.0;\n"
    " }return sum/9.0;\n"
    "}\n"
    "float shadowVisibility(int light,vec3 N){\n"
    " int map=lightMap[light];if(map<0 || receiveShadows==0) return 1.0;vec3 p=vPosition+N*shadowParams.y;\n"
    " if(light!=sunIndex) return shadowSample(map,p);\n"
    " float depth=dot(viewDepth,vec4(vPosition,1.0)); if(depth>cascadeSplits[cascadeCount-1]) return 1.0;\n"
    " int c=0;for(int i=0;i<3;++i){if(c+1<cascadeCount && depth>cascadeSplits[c]) ++c;}\n"
    " float visibility=shadowSample(c,p);float lower=c==0?cameraNear:cascadeSplits[c-1];\n"
    " float start=mix(cascadeSplits[c],lower,shadowParams.w);\n"
    " if(c+1<cascadeCount && depth>start && shadowParams.w>0.0) visibility=mix(visibility,shadowSample(c+1,p),clamp((depth-start)/(cascadeSplits[c]-start),0.0,1.0));\n"
    " return visibility;\n"
    "}\n"
#else
    "float shadowVisibility(int light,vec3 N){return 1.0;}\n"
#endif
    "vec3 surfaceNormal(){\n"
    " vec3 N=normalize(vNormal); if(!gl_FrontFacing) N=-N;\n"
    " if((mapMask&4)==0) return N;\n"
    " vec2 tc=uv(2); vec3 dp1=dFdx(vPosition),dp2=dFdy(vPosition);\n"
    " vec2 a=dFdx(tc),b=dFdy(tc); float det=a.x*b.y-a.y*b.x;\n"
    " if(abs(det)<1e-12) return N;\n"
    " vec3 T=normalize((dp1*b.y-dp2*a.y)/det); T=normalize(T-N*dot(T,N));\n"
    " vec3 B=normalize((-dp1*b.x+dp2*a.x)/det);\n"
    " vec3 m=readMap(maps[2],2,tc).xyz*2.0-1.0; m.xy*=materialParams.z;\n"
    " return normalize(T*m.x+B*m.y+N*m.z);\n"
    "}\n"
    "vec3 shade(vec3 base){\n"
    " if(unlit!=0) return base;\n"
    " float metal=materialParams.x,rough=materialParams.y;\n"
    " if((mapMask&2)!=0){vec4 mr=readMap(maps[1],1,uv(1));metal*=mr.b;rough*=mr.g;}\n"
    " rough=max(rough,0.045); float a=rough*rough,a2=a*a;\n"
    " vec3 N=surfaceNormal(),V=normalize(cameraPosition.w>0.5?cameraPosition.xyz:cameraPosition.xyz-vPosition);\n"
    " float nv=max(dot(N,V),0.0001); vec3 f0=mix(vec3(0.04),base,metal),result=vec3(0.0);\n"
    " for(int i=0;i<8;++i){\n"
    "  if(i>=lightCount) break; vec3 L=-lightDirection[i].xyz; float attenuation=1.0;\n"
    "  if(lightPosition[i].w>0.5){\n"
    "   vec3 delta=lightPosition[i].xyz-vPosition; float d2=dot(delta,delta); L=delta*inversesqrt(max(d2,1e-8));\n"
    "   float ratio=lightDirection[i].w>0.0?d2/(lightDirection[i].w*lightDirection[i].w):0.0;\n"
    "   float falloff=max(1.0-ratio*ratio,0.0); attenuation=falloff*falloff/max(d2,1e-4);\n"
    "   if(lightPosition[i].w>1.5){float cone=clamp((dot(-L,lightDirection[i].xyz)-lightCone[i].y)/(lightCone[i].x-lightCone[i].y),0.0,1.0);attenuation*=cone*cone;}\n"
    "  }\n"
    "  float nl=max(dot(N,L),0.0); if(nl<=0.0 || attenuation==0.0) continue;\n"
    "  vec3 H=normalize(L+V); float nh=max(dot(N,H),0.0),vh=max(dot(V,H),0.0);\n"
    "  float denominator=nh*nh*(a2-1.0)+1.0; float D=a2/(3.14159265*denominator*denominator);\n"
    "  float visibility=0.5/max(nl*sqrt(nv*nv*(1.0-a2)+a2)+nv*sqrt(nl*nl*(1.0-a2)+a2),1e-6);\n"
    "  vec3 F=f0+(1.0-f0)*pow(1.0-vh,5.0);\n"
    "  vec3 brdf=(1.0-F)*(1.0-metal)*base/3.14159265+D*visibility*F;\n"
    "  result+=brdf*lightColor[i].rgb*(nl*attenuation*shadowVisibility(i,N));\n"
    " }\n"
#if XGE3D_ENABLE_IBL
    " if(iblIntensity>0.0){\n"
    "  vec3 F=f0+(max(vec3(1.0-rough),f0)-f0)*pow(1.0-nv,5.0);\n"
    "  vec3 diffuse=(1.0-F)*(1.0-metal)*base*texture(irradianceMap,N).rgb;\n"
    "  vec2 lut=texture(brdfMap,vec2(nv,rough)).rg;\n"
    "  vec3 specular=textureLod(prefilteredMap,reflect(-V,N),rough*iblMaxLod).rgb*(f0*lut.x+lut.y);\n"
    "  float ao=(mapMask&8)!=0 ? mix(1.0,readMap(maps[3],3,uv(3)).r,materialParams.w) : 1.0;\n"
    "  result+=(diffuse+specular)*iblIntensity*ao;\n"
    " }\n"
#endif
    " return result;\n"
    "}\n"
#else
    "vec3 shade(vec3 base){return base;}\n"
#endif
    "void main(){\n"
    " vec4 base=color; if((mapMask&1)!=0) base*=readMap(maps[0],0,uv(0));\n"
    " vec3 emission=emissive;\n"
#if XGE3D_ENABLE_SHADER
    "#ifdef D3_CUSTOM_SURFACE\n xge3dSurface(base,emission,vUV0);\n#endif\n"
#endif
    " if(alphaMode==1 && base.a<alphaCutoff) discard;\n"
    " if((mapMask&16)!=0) emission*=readMap(maps[4],4,uv(4)).rgb;\n"
    " vec3 lit=shade(base.rgb)+emission;\n"
#if XGE3D_ENABLE_FOG
    " if(fogColor.w>0.5) lit=mix(lit,fogColor.rgb,clamp((dot(fogDepth,vec4(vPosition,1.0))-fogRange.x)/(fogRange.y-fogRange.x),0.0,1.0));\n"
#endif
#if XGE3D_ENABLE_HEIGHT_FOG
    " lit=heightFog(lit);\n"
#endif
    " frag=vec4(lit*exposure,alphaMode==2?base.a:1.0);\n"
    "}\n";

/* Intermediate pixels are linear associated RGBA. Public targets remain
 * display-encoded associated RGBA, matching the existing 2D texture contract. */
static const char d3_composite_fragment_shader[] =
    "#ifdef GL_ES\nprecision highp float;\n#endif\n"
    "in vec2 ndc;out vec4 frag;uniform sampler2D image;uniform int importing;\n"
#if XGE3D_ENABLE_FXAA
    "uniform int antialiasing;\n"
#endif
#if XGE3D_ENABLE_TONEMAP || XGE3D_ENABLE_COLOR_GRADING
    "uniform int colorEnabled;\n"
#endif
#if XGE3D_ENABLE_TONEMAP
    "uniform int colorTone;\n"
    /* Narkowicz's fitted ACES curve, CC0 option: https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/ */
    "vec3 filmicCurve(vec3 x){return clamp((x*(2.51*x+0.03))/(x*(2.43*x+0.59)+0.14),0.0,1.0);}\n"
#endif
#if XGE3D_ENABLE_COLOR_GRADING
    "uniform vec3 colorGain;uniform vec2 colorGrade;\n"
#endif
#if XGE3D_ENABLE_BLOOM
    "uniform sampler2D bloomImage;uniform float bloomStrength;\n"
#endif
    "vec3 encode(vec3 x){x=clamp(x,0.0,1.0);return mix(12.92*x,1.055*pow(x,vec3(1.0/2.4))-0.055,step(vec3(0.0031308),x));}\n"
    "vec3 decode(vec3 x){return mix(x/12.92,pow((x+0.055)/1.055,vec3(2.4)),step(vec3(0.04045),x));}\n"
    "vec4 display(vec2 uv){vec4 c=texture(image,uv);\n"
#if XGE3D_ENABLE_BLOOM
    " if(bloomStrength>0.0){vec4 b=texture(bloomImage,uv)*bloomStrength;c.rgb+=b.rgb;c.a+=(1.0-c.a)*clamp(b.a,0.0,1.0);}\n"
#endif
    " vec3 v=c.a>0.0?c.rgb/c.a:vec3(0.0);\n"
#if XGE3D_ENABLE_TONEMAP || XGE3D_ENABLE_COLOR_GRADING
    " if(colorEnabled!=0){\n"
#if XGE3D_ENABLE_COLOR_GRADING
    " v*=colorGain;v=mix(vec3(dot(v,vec3(0.2126,0.7152,0.0722))),v,colorGrade.x);v=max(vec3(0.0),(v-0.18)*colorGrade.y+0.18);\n"
#endif
#if XGE3D_ENABLE_TONEMAP
    " if(colorTone==1)v=v/(1.0+v);else if(colorTone==2)v=filmicCurve(v);\n"
#endif
    " }\n"
#endif
    " return vec4(encode(v)*c.a,c.a);}\n"
#if XGE3D_ENABLE_FXAA
    "float luma(vec4 c){return dot(c.rgb,vec3(0.299,0.587,0.114))+c.a*0.25;}\n"
    "vec4 fxaa(vec2 uv,vec4 m){\n"
    " vec2 px=1.0/vec2(textureSize(image,0));\n"
    " float a=luma(display(uv+px*vec2(-1,-1))),b=luma(display(uv+px*vec2(1,-1)));\n"
    " float c=luma(display(uv+px*vec2(-1,1))),d=luma(display(uv+px*vec2(1,1))),e=luma(m);\n"
    " float lo=min(e,min(min(a,b),min(c,d))),hi=max(e,max(max(a,b),max(c,d)));\n"
    " if(hi-lo<max(0.0312,hi*0.125))return m;\n"
    " vec2 dir=vec2(-((a+b)-(c+d)),(a+c)-(b+d));\n"
    " float reduce=max((a+b+c+d)*0.03125,0.0078125);\n"
    " dir=clamp(dir/(min(abs(dir.x),abs(dir.y))+reduce),vec2(-8.0),vec2(8.0))*px;\n"
    " vec4 p=0.5*(display(uv+dir*(1.0/3.0-0.5))+display(uv+dir*(2.0/3.0-0.5)));\n"
    " vec4 q=p*0.5+0.25*(display(uv-dir*0.5)+display(uv+dir*0.5));\n"
    " float l=luma(q);return l<lo||l>hi?p:q;\n"
    "}\n"
#endif
    "void main(){vec2 uv=ndc*0.5+0.5;vec4 c=texture(image,uv);\n"
    " if(importing!=0){frag=vec4(c.a>0.0?decode(clamp(c.rgb/c.a,0.0,1.0))*c.a:vec3(0.0),c.a);return;}\n"
    " c=display(uv);\n"
#if XGE3D_ENABLE_FXAA
    " if(antialiasing!=0)c=fxaa(uv,c);\n"
#endif
    " frag=c;\n"
    "}\n";

#if XGE3D_ENABLE_SHADOW
static const char d3_depth_fragment_shader[] =
    "#ifdef GL_ES\nprecision highp float;\n#endif\n"
    "in vec2 vUV0,vUV1; uniform vec4 color; uniform int alphaMode,hasMap,texcoord;\n"
    "uniform float alphaCutoff,uvRotation; uniform vec4 uvTransform; uniform sampler2D baseMap;\n"
#if XGE3D_ENABLE_DYNAMIC
    "uniform int baseTarget;\n"
#endif
#if XGE3D_ENABLE_SHADER
    "#ifdef D3_CUSTOM_SURFACE\nuniform vec3 emissive;\n#endif\n"
#endif
    "void main(){\n"
    " vec4 base=color;if(hasMap!=0){\n"
    "  vec2 p=(texcoord==0?vUV0:vUV1)*uvTransform.zw;float c=cos(uvRotation),s=sin(uvRotation);\n"
    "  vec2 uv=vec2(c*p.x-s*p.y,s*p.x+c*p.y)+uvTransform.xy;\n"
#if XGE3D_ENABLE_DYNAMIC
    "  if(baseTarget!=0)uv.y=1.0-uv.y;\n"
#endif
    "  vec4 t=texture(baseMap,uv);\n"
#if XGE3D_ENABLE_DYNAMIC
    "  if(baseTarget!=0){vec3 s=t.a>0.0?t.rgb/t.a:vec3(0.0);t.rgb=mix(s/12.92,pow((s+0.055)/1.055,vec3(2.4)),step(vec3(0.04045),s));}\n"
#endif
    "  base*=t;}\n"
#if XGE3D_ENABLE_SHADER
    "#ifdef D3_CUSTOM_SURFACE\n vec3 emission=emissive;xge3dSurface(base,emission,vUV0);\n#endif\n"
#endif
    " if(alphaMode==1 && base.a<alphaCutoff) discard;\n"
    "}\n";
#endif
