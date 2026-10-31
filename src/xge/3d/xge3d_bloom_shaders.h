/* Small spatial-only pyramid; no history, compute shaders or external assets. */
static const char d3_bloom_fragment_shader[] =
    "#ifdef GL_ES\nprecision highp float;\n#endif\n"
    "in vec2 ndc;out vec4 frag;uniform sampler2D image;uniform vec4 filterParams;\n"
    "vec4 tap(vec2 uv){vec4 c=texture(image,uv);if(filterParams.w>0.0){\n"
    " float l=max(c.r,max(c.g,c.b))/max(c.a,0.00001);\n"
    " float soft=clamp(l-filterParams.x+filterParams.y,0.0,2.0*filterParams.y);\n"
    " soft=soft*soft/max(4.0*filterParams.y,0.00001);\n"
    " float weight=l>0.00001?clamp(max(l-filterParams.x,soft)/l,0.0,1.0):0.0;\n"
    " c*=weight*filterParams.w;}return c;}\n"
    "void main(){vec2 uv=ndc*0.5+0.5,px=1.0/vec2(textureSize(image,0));\n"
    " vec4 c=tap(uv)*0.5;\n"
    " c+=(tap(uv+px*vec2(-1,-1))+tap(uv+px*vec2(1,-1))+tap(uv+px*vec2(-1,1))+tap(uv+px*vec2(1,1)))*0.125;\n"
    " frag=c;}\n";
