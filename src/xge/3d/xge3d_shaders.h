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
    " c=mix(12.92*c,1.055*pow(max(c,vec3(0.0)),vec3(1.0/2.4))-0.055,step(vec3(0.0031308),c));\n"
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
    " vec4 local=skin*vec4(position,1.0);mat3 basis=mat3(w*skin);\n"
    " gl_Position=instanceEnabled!=0?viewProjection*w*local:mvp*local; vUV0=uv0; vUV1=uv1;\n"
    " vPosition=(w*local).xyz;\n"
    " vNormal=transpose(inverse(basis))*normal;\n"
    " vTangent=vec4(basis*tangent.xyz,tangent.w*sign(determinant(basis)));\n"
    "}\n";

static const char d3_fragment_shader[] =
    "#ifdef GL_ES\nprecision highp float;\n#endif\n"
    "in vec2 vUV0, vUV1; in vec3 vPosition, vNormal; in vec4 vTangent;\n"
    "uniform vec4 color; uniform vec3 emissive; uniform int alphaMode, mapMask;\n"
    "uniform float alphaCutoff; uniform sampler2D maps[5];\n"
    "uniform float exposure;\n"
#if XGE3D_ENABLE_FOG
    "uniform vec4 fogColor,fogDepth;uniform vec2 fogRange;\n"
#endif
#if XGE3D_ENABLE_LIGHTING
    "uniform vec4 cameraPosition; uniform int lightCount, unlit; uniform vec4 materialParams;\n"
    "uniform vec4 lightPosition[8],lightDirection[8],lightColor[8]; uniform vec2 lightCone[8];\n"
#if XGE3D_ENABLE_IBL
    "uniform samplerCube irradianceMap,prefilteredMap;uniform sampler2D brdfMap;uniform float iblIntensity,iblMaxLod;\n"
#endif
#if XGE3D_ENABLE_SHADOW
    "uniform sampler2D shadowAtlas; uniform mat4 shadowMatrix[6]; uniform int lightMap[8],sunIndex,cascadeCount;\n"
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
    "vec3 srgb(vec3 c){\n"
    " return mix(12.92*c,1.055*pow(max(c,vec3(0.0)),vec3(1.0/2.4))-0.055,step(vec3(0.0031308),c));\n"
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
    " int map=lightMap[light];if(map<0) return 1.0;vec3 p=vPosition+N*shadowParams.y;\n"
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
    " vec3 m=texture(maps[2],tc).xyz*2.0-1.0; m.xy*=materialParams.z;\n"
    " return normalize(T*m.x+B*m.y+N*m.z);\n"
    "}\n"
    "vec3 shade(vec3 base){\n"
    " if(unlit!=0) return base;\n"
    " float metal=materialParams.x,rough=materialParams.y;\n"
    " if((mapMask&2)!=0){vec4 mr=texture(maps[1],uv(1));metal*=mr.b;rough*=mr.g;}\n"
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
    "  float ao=(mapMask&8)!=0 ? mix(1.0,texture(maps[3],uv(3)).r,materialParams.w) : 1.0;\n"
    "  result+=(diffuse+specular)*iblIntensity*ao;\n"
    " }\n"
#endif
    " return result;\n"
    "}\n"
#else
    "vec3 shade(vec3 base){return base;}\n"
#endif
    "void main(){\n"
    " vec4 base=color; if((mapMask&1)!=0) base*=texture(maps[0],uv(0));\n"
    " if(alphaMode==1 && base.a<alphaCutoff) discard;\n"
    " vec3 emission=emissive; if((mapMask&16)!=0) emission*=texture(maps[4],uv(4)).rgb;\n"
    " vec3 lit=shade(base.rgb)+emission;\n"
#if XGE3D_ENABLE_FOG
    " if(fogColor.w>0.5) lit=mix(lit,fogColor.rgb,clamp((dot(fogDepth,vec4(vPosition,1.0))-fogRange.x)/(fogRange.y-fogRange.x),0.0,1.0));\n"
#endif
    " frag=vec4(srgb(lit*exposure),alphaMode==2?base.a:1.0);\n"
    "}\n";

#if XGE3D_ENABLE_SHADOW
static const char d3_depth_fragment_shader[] =
    "#ifdef GL_ES\nprecision highp float;\n#endif\n"
    "in vec2 vUV0,vUV1; uniform vec4 color; uniform int alphaMode,hasMap,texcoord;\n"
    "uniform float alphaCutoff,uvRotation; uniform vec4 uvTransform; uniform sampler2D baseMap;\n"
    "void main(){\n"
    " float alpha=color.a;if(hasMap!=0){\n"
    "  vec2 p=(texcoord==0?vUV0:vUV1)*uvTransform.zw;float c=cos(uvRotation),s=sin(uvRotation);\n"
    "  alpha*=texture(baseMap,vec2(c*p.x-s*p.y,s*p.x+c*p.y)+uvTransform.xy).a;\n"
    " } if(alphaMode==1 && alpha<alphaCutoff) discard;\n"
    "}\n";
#endif
