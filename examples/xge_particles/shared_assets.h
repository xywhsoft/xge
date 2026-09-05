#ifndef PARTICLE_EXAMPLE_ASSETS_H
#define PARTICLE_EXAMPLE_ASSETS_H
/* Shared procedural resources used by the seven examples and the particle editor.
 * No bundled font/image licence or runtime/editor dependency is introduced. */
#include "../../xge.h"
#include <math.h>
#include <stdio.h>

static int particle_example_textures(xge_texture soft, xge_texture sheet)
{
	unsigned char pixels[64 * 64 * 4 * 4];
	int x, y, cell, index, result;
	for (cell = 0; cell < 4; cell++)
		for (y = 0; y < 64; y++)
			for (x = 0; x < 64; x++)
			{
				float u = (x - 31.5f) / 31.5f, v = (y - 31.5f) / 31.5f;
				float warp = 1 + .1f * sinf(9 * u + cell * 2) * sinf(7 * v - cell);
				float radius = sqrtf(u * u + v * v) * warp, alpha = powf(fmaxf(0, 1 - radius), 2);
				unsigned char a = (unsigned char)(alpha * 255);
				index = (y * 256 + cell * 64 + x) * 4;
				pixels[index] = pixels[index + 1] = pixels[index + 2] = pixels[index + 3] = a;
			}
	result = xgeTextureCreateRGBA(sheet, 256, 64, pixels);
	if (result != XGE_OK)
		return result;
	for (y = 0; y < 64; y++)
		for (x = 0; x < 64; x++)
		{
			float u = (x - 31.5f) / 31.5f, v = (y - 31.5f) / 31.5f,
			      alpha = powf(fmaxf(0, 1 - u * u - v * v), 2);
			unsigned char a = (unsigned char)(alpha * 255);
			index = (y * 64 + x) * 4;
			pixels[index] = pixels[index + 1] = pixels[index + 2] = pixels[index + 3] = a;
		}
	return xgeTextureCreateRGBA(soft, 64, 64, pixels);
}

static int particle_example_ring(xge_shader shader, xge_material material)
{
	int result;
	const char *vsBody =
	    "layout(location=0) in vec4 aPos; layout(location=1) in vec2 aUV; layout(location=2) in vec4 aColor; "
	    "uniform vec2 uResolution; out vec2 vUV; out vec4 vColor; void main(){vec2 "
	    "p=aPos.xy/uResolution*2.-1.; gl_Position=vec4(p.x,-p.y,0.,1.);vUV=aUV;vColor=aColor;}";
	const char *fsBody = "in vec2 vUV; in vec4 vColor; uniform vec4 uColor; uniform float uTime; out vec4 "
	                     "FragColor; void main(){vec2 p=vUV*2.-1.;float r=length(p),a=atan(p.y,p.x);float "
	                     "ring=exp(-pow((r-.75)*40.,2.));float inner=exp(-pow((r-.52)*70.,2.));float "
	                     "marks=pow(max(0.,cos(a*12.+uTime)),20.)*exp(-pow((r-.64)*24.,2.));float "
	                     "glow=clamp(ring+inner*.5+marks,0.,1.);FragColor=vColor*uColor*glow;}";
	{
		char header[128], vs[1024], fs[1536];
		result = xgeGraphicsShaderHeaderGet(XGE_GPU_BACKEND_NONE, header, sizeof(header));
		if (result != XGE_OK)
			return result;
		snprintf(vs, sizeof(vs), "%s%s", header, vsBody);
		snprintf(fs, sizeof(fs), "%s%s", header, fsBody);
		result = xgeShaderCreate(shader, vs, fs);
		if (result != XGE_OK)
			return result;
		xgeMaterialInit(material);
		xgeMaterialSetShader(material, shader);
		xgeMaterialSetBlend(material, XGE_BLEND_ADD);
	}
	return XGE_OK;
}
#endif
