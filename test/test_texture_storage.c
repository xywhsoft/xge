/* Deterministic backend-contract tests, including unavailable capabilities.
 * Include the implementation to replace GL entry points without a window. */
#include "../xge.c"

#include <assert.h>

typedef struct mock_texture_t {
	int internal, width, height;
	int swizzle[4];
	unsigned char* pixels;
} mock_texture_t;
static mock_texture_t mock_textures[512];
static GLuint mock_next, mock_bound;
static GLint mock_alignment = 8;
static GLenum mock_error;
static unsigned int mock_reject;
static int mock_swizzle_fail, mock_oom, mock_sub_fail, mock_attempts, checks;
#define CHECK(c) do { checks++; if (!(c)) { fprintf(stderr, "FAIL texture storage line %d: %s\n", __LINE__, #c); exit(1); } } while (0)

static int mock_storage(int internal)
{
	switch ( internal ) {
	case GL_R8: return XGE_TEXTURE_STORAGE_R8;
	case GL_RG8: return XGE_TEXTURE_STORAGE_RG8;
	case GL_RGB565: return XGE_TEXTURE_STORAGE_RGB565;
	case GL_RGB5_A1: return XGE_TEXTURE_STORAGE_RGB5_A1;
	case GL_RGBA4: return XGE_TEXTURE_STORAGE_RGBA4;
	default: return XGE_TEXTURE_STORAGE_RGBA8;
	}
}
static GLenum APIENTRY mock_get_error(void) { GLenum e = mock_error; mock_error = 0; return e; }
static void APIENTRY mock_get_integer(GLenum name, GLint* value)
{
	*value = name == GL_TEXTURE_BINDING_2D ? (GLint)mock_bound : name == GL_UNPACK_ALIGNMENT ? mock_alignment : 0;
}
static void APIENTRY mock_pixel_store(GLenum name, GLint value) { CHECK(name == GL_UNPACK_ALIGNMENT); mock_alignment = value; }
static void APIENTRY mock_bind(GLenum target, GLuint id) { CHECK(target == GL_TEXTURE_2D); mock_bound = id; }
static void APIENTRY mock_gen(GLsizei n, GLuint* ids)
{
	int i;
	for ( i = 0; i < n; i++ ) {
		CHECK(++mock_next < 512);
		ids[i] = mock_next;
		mock_textures[mock_next].swizzle[0] = GL_RED;
		mock_textures[mock_next].swizzle[1] = GL_GREEN;
		mock_textures[mock_next].swizzle[2] = GL_BLUE;
		mock_textures[mock_next].swizzle[3] = GL_ALPHA;
	}
}
static void APIENTRY mock_delete(GLsizei n, const GLuint* ids)
{
	int i;
	for ( i = 0; i < n; i++ ) {
		free(mock_textures[ids[i]].pixels);
		memset(&mock_textures[ids[i]], 0, sizeof(mock_texture_t));
		if ( mock_bound == ids[i] ) mock_bound = 0;
	}
}
static void APIENTRY mock_parameter(GLenum target, GLenum name, GLint value)
{
	CHECK(target == GL_TEXTURE_2D && mock_bound != 0);
	if ( name >= GL_TEXTURE_SWIZZLE_R && name <= GL_TEXTURE_SWIZZLE_A ) {
		if ( mock_swizzle_fail ) { mock_error = GL_INVALID_ENUM; return; }
		mock_textures[mock_bound].swizzle[name - GL_TEXTURE_SWIZZLE_R] = value;
	}
}
static void mock_decode(const unsigned char* src, unsigned char* dst, GLenum format, GLenum type, int count)
{
	int i;
	for ( i = 0; i < count; i++, dst += 4 ) {
		uint16_t v;
		dst[0] = dst[1] = dst[2] = 0; dst[3] = 255;
		if ( type == GL_UNSIGNED_BYTE ) {
			int n = format == GL_RED ? 1 : format == GL_RG ? 2 : 4;
			memcpy(dst, src, n); src += n;
		} else {
			memcpy(&v, src, 2); src += 2;
			if ( type == GL_UNSIGNED_SHORT_5_6_5 ) {
				dst[0] = (unsigned char)(((v >> 11) * 255u + 15) / 31);
				dst[1] = (unsigned char)((((v >> 5) & 63) * 255u + 31) / 63);
				dst[2] = (unsigned char)(((v & 31) * 255u + 15) / 31);
			} else if ( type == GL_UNSIGNED_SHORT_5_5_5_1 ) {
				dst[0] = (unsigned char)(((v >> 11) * 255u + 15) / 31);
				dst[1] = (unsigned char)((((v >> 6) & 31) * 255u + 15) / 31);
				dst[2] = (unsigned char)((((v >> 1) & 31) * 255u + 15) / 31);
				dst[3] = (v & 1) ? 255 : 0;
			} else {
				dst[0] = (unsigned char)((v >> 12) * 17);
				dst[1] = (unsigned char)(((v >> 8) & 15) * 17);
				dst[2] = (unsigned char)(((v >> 4) & 15) * 17);
				dst[3] = (unsigned char)((v & 15) * 17);
			}
		}
	}
}
static void APIENTRY mock_image(GLenum target, GLint level, GLint internal, GLsizei w, GLsizei h, GLint border, GLenum format, GLenum type, const void* pixels)
{
	mock_texture_t* t = &mock_textures[mock_bound];
	(void)level; (void)border;
	CHECK(target == GL_TEXTURE_2D && mock_bound != 0 && mock_alignment == 1);
	mock_attempts++;
	if ( mock_oom ) { mock_error = GL_OUT_OF_MEMORY; return; }
	if ( mock_reject & (1u << mock_storage(internal)) ) { mock_error = GL_INVALID_ENUM; return; }
	t->internal = internal; t->width = w; t->height = h;
	t->pixels = (unsigned char*)malloc((size_t)w * h * 4u);
	CHECK(t->pixels != NULL);
	mock_decode((const unsigned char*)pixels, t->pixels, format, type, w * h);
}
static void APIENTRY mock_sub_image(GLenum target, GLint level, GLint x, GLint y, GLsizei w, GLsizei h, GLenum format, GLenum type, const void* pixels)
{
	mock_texture_t* t = &mock_textures[mock_bound];
	int row, bytes = format == GL_RED ? 1 : format == GL_RG ? 2 : 4;
	(void)target; (void)level;
	CHECK(mock_alignment == 1);
	if ( mock_sub_fail ) { mock_error = GL_INVALID_OPERATION; return; }
	for ( row = 0; row < h; row++ )
		mock_decode((const unsigned char*)pixels + (size_t)row * w * bytes,
		            t->pixels + ((size_t)(y + row) * t->width + x) * 4u, format, type, w);
}
static void mock_sample(xge_texture texture, unsigned char* rgba)
{
	mock_texture_t* t = &mock_textures[texture->iBackendId];
	int i, c;
	for ( i = 0; i < t->width * t->height; i++ ) for ( c = 0; c < 4; c++ ) {
		int swizzle = t->swizzle[c];
		int channel = swizzle == GL_RED ? 0 : swizzle == GL_GREEN ? 1 : swizzle == GL_BLUE ? 2 : 3;
		rgba[i * 4 + c] = swizzle == GL_ONE ? 255 : swizzle == GL_ZERO ? 0 : t->pixels[i * 4 + channel];
	}
}
static xge_texture_storage_info_t info(xge_texture texture)
{
	xge_texture_storage_info_t result = {0};
	result.iSize = sizeof(result);
	CHECK(xgeTextureGetStorageInfo(texture, &result) == XGE_OK);
	return result;
}
static void fill(unsigned char* pixels, int kind)
{
	int i;
	for ( i = 0; i < 81; i++ ) {
		unsigned char v = (unsigned char)(i * 3);
		pixels[i * 4] = pixels[i * 4 + 1] = pixels[i * 4 + 2] = v;
		pixels[i * 4 + 3] = kind == 0 ? v : kind == 1 ? 255 : (unsigned char)(255 - v);
		if ( kind == 3 ) pixels[i * 4] = (unsigned char)(v ^ 127);
		if ( kind == 4 || kind == 5 ) pixels[i * 4] = pixels[i * 4 + 1] = pixels[i * 4 + 2] = kind == 4 ? 255 : 0;
	}
}
static void verify_pixels(xge_texture texture, const unsigned char* expected)
{
	unsigned char pixels[81 * 4];
	CHECK(xgeTextureReadPixels(texture, pixels, 36) == XGE_OK && memcmp(pixels, expected, sizeof(pixels)) == 0);
	mock_sample(texture, pixels);
	CHECK(memcmp(pixels, expected, sizeof(pixels)) == 0);
	CHECK(mock_alignment == 8 && mock_bound == 0);
}
int main(void)
{
	xge_texture_t texture = {0};
	unsigned char pixels[81 * 4], readback[81 * 4], patch[24];
	int k, before;
	GLuint id;
	glGenTextures = mock_gen; glDeleteTextures = mock_delete; glBindTexture = mock_bind;
	glTexImage2D = mock_image; glTexSubImage2D = mock_sub_image; glTexParameteri = mock_parameter;
	glGetError = mock_get_error; glGetIntegerv = mock_get_integer; glPixelStorei = mock_pixel_store;
	g_xge.bSokolRunning = 1;
	g_xge.tGraphicsBackend.iType = XGE_GPU_BACKEND_OPENGL33;
	for ( k = 0; k < 6; k++ ) {
		int storage = k == 2 ? XGE_TEXTURE_STORAGE_RG8 : k == 3 ? XGE_TEXTURE_STORAGE_RGBA8 : XGE_TEXTURE_STORAGE_R8;
		fill(pixels, k);
		CHECK(xgeTextureCreateRGBA(&texture, 9, 9, pixels) == XGE_OK);
		CHECK(info(&texture).iStorage == storage);
		CHECK(info(&texture).iCpuBytes == sizeof(pixels));
		verify_pixels(&texture, pixels);
		xgeTextureFree(&texture);
		CHECK(g_xge.iTextureMemoryBytes == 0 && g_xge.iTextureCount == 0);
	}
	/* Explicit NONE and small textures never scan/pack into new formats. */
	fill(pixels, 0);
	CHECK(xgeTextureCreateRGBAEx(&texture, 9, 9, pixels, XGE_TEXTURE_COMPRESS_NONE) == XGE_OK);
	CHECK(info(&texture).iStorage == XGE_TEXTURE_STORAGE_RGBA8); xgeTextureFree(&texture);
	CHECK(xgeTextureCreateRGBA(&texture, 1, 1, pixels) == XGE_OK);
	CHECK(info(&texture).iStorage == XGE_TEXTURE_STORAGE_RGBA8); xgeTextureFree(&texture);
	CHECK(xgeTextureCreateRGBAEx(&texture, 9, 9, pixels, XGE_TEXTURE_COMPRESS_MASK) == XGE_ERROR_INVALID_ARGUMENT);
	/* No WebGL2 swizzle tokens; native GLES3 does support exact packing. */
	g_xge.tGraphicsBackend.iType = XGE_GPU_BACKEND_WEBGL2;
	mock_swizzle_fail = 1;
	CHECK(xgeTextureCreateRGBA(&texture, 9, 9, pixels) == XGE_OK);
	CHECK(info(&texture).iStorage == XGE_TEXTURE_STORAGE_RGBA8); verify_pixels(&texture, pixels); xgeTextureFree(&texture);
	mock_swizzle_fail = 0; g_xge.tGraphicsBackend.iType = XGE_GPU_BACKEND_GLES30;
	CHECK(xgeTextureCreateRGBA(&texture, 9, 9, pixels) == XGE_OK);
	CHECK(info(&texture).iStorage == XGE_TEXTURE_STORAGE_R8); xgeTextureFree(&texture);
	/* Reject R8, then reject both exact candidates, then reject swizzle itself. */
	mock_reject = 1u << XGE_TEXTURE_STORAGE_R8;
	CHECK(xgeTextureCreateRGBA(&texture, 9, 9, pixels) == XGE_OK);
	CHECK(info(&texture).iStorage == XGE_TEXTURE_STORAGE_RG8); verify_pixels(&texture, pixels); xgeTextureFree(&texture);
	mock_reject |= 1u << XGE_TEXTURE_STORAGE_RG8;
	CHECK(xgeTextureCreateRGBA(&texture, 9, 9, pixels) == XGE_OK);
	CHECK(info(&texture).iStorage == XGE_TEXTURE_STORAGE_RGBA8); verify_pixels(&texture, pixels); xgeTextureFree(&texture);
	mock_reject = 0; mock_swizzle_fail = 1;
	CHECK(xgeTextureCreateRGBA(&texture, 9, 9, pixels) == XGE_OK);
	CHECK(info(&texture).iStorage == XGE_TEXTURE_STORAGE_RGBA8); verify_pixels(&texture, pixels); xgeTextureFree(&texture);
	mock_swizzle_fail = 0;
	/* Full same-class lossy chain followed by the exact chain. */
	fill(pixels, 1);
	for ( k = XGE_TEXTURE_STORAGE_RGB565; k <= XGE_TEXTURE_STORAGE_RGBA4 + 1; k++ ) {
		CHECK(xgeTextureCreateRGBAEx(&texture, 9, 9, pixels, XGE_TEXTURE_COMPRESS_LOSSY) == XGE_OK);
		CHECK(info(&texture).iStorage == (k <= XGE_TEXTURE_STORAGE_RGBA4 ? k : XGE_TEXTURE_STORAGE_R8));
		CHECK(xgeTextureReadPixels(&texture, readback, 36) == XGE_OK && memcmp(pixels, readback, sizeof(pixels)) == 0);
		mock_sample(&texture, readback);
		for ( before = 0; before < (int)sizeof(pixels); before++ ) CHECK(abs((int)pixels[before] - readback[before]) <= 8);
		xgeTextureFree(&texture);
		mock_reject |= 1u << k;
	}
	mock_reject = 0;
	/* Non-binary alpha only tries RGBA4, then gives up lossy storage on update. */
	fill(pixels, 3);
	CHECK(xgeTextureCreateRGBAEx(&texture, 9, 9, pixels, XGE_TEXTURE_COMPRESS_LOSSY) == XGE_OK);
	CHECK(info(&texture).iStorage == XGE_TEXTURE_STORAGE_RGBA4);
	CHECK(xgeTextureUpdateRGBA(&texture, 0, 0, 1, 1, pixels, 4) == XGE_OK);
	CHECK(info(&texture).iStorage == XGE_TEXTURE_STORAGE_RGBA8); verify_pixels(&texture, pixels); xgeTextureFree(&texture);
	/* Strided, odd-width exact updates keep R8 and restore pixel-store state. */
	fill(pixels, 0); memset(patch, 71, sizeof(patch));
	CHECK(xgeTextureCreateRGBA(&texture, 9, 9, pixels) == XGE_OK); id = texture.iBackendId;
	CHECK(xgeTextureUpdateRGBA(&texture, 2, 3, 2, 2, patch, 12) == XGE_OK);
	memcpy(pixels + (3 * 9 + 2) * 4, patch, 8); memcpy(pixels + (4 * 9 + 2) * 4, patch + 12, 8);
	CHECK(texture.iBackendId == id && info(&texture).iGpuBytes == 81); verify_pixels(&texture, pixels);
	patch[0] = 23; /* Break grayscale, test atomic failure before promotion. */
	mock_oom = 1; before = mock_attempts;
	CHECK(xgeTextureUpdateRGBA(&texture, 1, 1, 1, 1, patch, 4) == XGE_ERROR_OUT_OF_MEMORY);
	CHECK(mock_attempts == before + 1 && texture.iBackendId == id && info(&texture).iGpuBytes == 81);
	verify_pixels(&texture, pixels); mock_oom = 0;
	CHECK(xgeTextureUpdateRGBA(&texture, 1, 1, 1, 1, patch, 4) == XGE_OK);
	memcpy(pixels + (1 * 9 + 1) * 4, patch, 4);
	CHECK(info(&texture).iGpuBytes == sizeof(pixels)); verify_pixels(&texture, pixels);
	fill(pixels, 0); CHECK(xgeTextureUpdateRGBA(&texture, 0, 0, 9, 9, pixels, 36) == XGE_OK);
	CHECK(info(&texture).iStorage == XGE_TEXTURE_STORAGE_RGBA8); xgeTextureFree(&texture);
	/* A backend rejecting compact subimage updates promotes losslessly. */
	CHECK(xgeTextureCreateRGBA(&texture, 9, 9, pixels) == XGE_OK);
	mock_sub_fail = 1;
	CHECK(xgeTextureUpdateRGBA(&texture, 0, 0, 1, 1, pixels, 4) == XGE_OK);
	CHECK(info(&texture).iStorage == XGE_TEXTURE_STORAGE_RGBA8); verify_pixels(&texture, pixels);
	mock_sub_fail = 0; xgeTextureFree(&texture);
	/* Fatal allocation failure is not retried as a larger format. */
	mock_oom = 1; before = mock_attempts;
	CHECK(xgeTextureCreateRGBA(&texture, 9, 9, pixels) == XGE_ERROR_OUT_OF_MEMORY);
	CHECK(mock_attempts == before + 1 && texture.pBackend == NULL && g_xge.iTextureCount == 0);
	mock_oom = 0;
	/* Creation before a context preserves policy; selection happens on upload. */
	g_xge.bSokolRunning = 0;
	CHECK(xgeTextureCreateRGBA(&texture, 9, 9, pixels) == XGE_OK);
	CHECK(info(&texture).iGpuBytes == 0 && info(&texture).iCpuBytes == sizeof(pixels));
	g_xge.bSokolRunning = 1;
	CHECK(xgeTextureUploadFlush() == 1 && info(&texture).iStorage == XGE_TEXTURE_STORAGE_R8);
	verify_pixels(&texture, pixels); xgeTextureFree(&texture);
	/* File/memory creation flags survive decoding. */
	{
		int length;
		unsigned char* png = stbi_write_png_to_mem(pixels, 36, 9, 9, 4, &length);
		CHECK(png != NULL);
		CHECK(xgeTextureLoadMemoryEx(&texture, png, length, XGE_IMAGE_STRAIGHT_ALPHA | XGE_TEXTURE_COMPRESS_NONE) == XGE_OK);
		CHECK(info(&texture).iStorage == XGE_TEXTURE_STORAGE_RGBA8); verify_pixels(&texture, pixels); xgeTextureFree(&texture);
		STBIW_FREE(png);
	}
	/* Resource fallback must not override the caller's storage policy. */
	CHECK(xgeTextureFallbackSetRGBA(9, 9, pixels) == XGE_OK);
	CHECK(xgeTextureLoadMemoryEx(&texture, "bad", 3, XGE_TEXTURE_COMPRESS_NONE) == XGE_OK);
	CHECK((texture.iFlags & XGE_TEXTURE_FALLBACK) != 0 && info(&texture).iStorage == XGE_TEXTURE_STORAGE_RGBA8);
	verify_pixels(&texture, pixels); xgeTextureFree(&texture);
	CHECK(xgeTextureLoadMemoryEx(&texture, "bad", 3, XGE_TEXTURE_COMPRESS_MASK) == XGE_ERROR_INVALID_ARGUMENT);
	xgeTextureFallbackClear();
	/* Both synchronous and threaded async entry points preserve NONE. */
	fill(pixels, 1);
	CHECK(xgeImageSavePNG("build/xge_texture_storage_async.png", 9, 9, pixels, 36) == XGE_OK);
	for ( k = 0; k < 2; k++ ) {
		xge_async_request_t request;
		int state = XGE_ASYNC_LOADING, poll;
		xgeAsyncRequestInit(&request);
		CHECK(xgeAsyncThreadingSet(k) == XGE_OK);
		CHECK(xgeAsyncTextureLoad(&request, &texture, "build/xge_texture_storage_async.png", XGE_TEXTURE_COMPRESS_NONE, NULL, NULL) == XGE_OK);
		for ( poll = 0; poll < 2000 && state == XGE_ASYNC_LOADING; poll++ ) {
			state = xgeAsyncPoll(&request);
			if ( state == XGE_ASYNC_LOADING ) xgeSleep(1);
		}
		CHECK(state == XGE_ASYNC_READY);
		CHECK(info(&texture).iCompression == XGE_TEXTURE_COMPRESS_NONE && info(&texture).iStorage == XGE_TEXTURE_STORAGE_RGBA8);
		verify_pixels(&texture, pixels); xgeTextureFree(&texture); xgeAsyncRequestFree(&request);
	}
	CHECK(xgeAsyncThreadingSet(0) == XGE_OK);
	CHECK(remove("build/xge_texture_storage_async.png") == 0);
	g_xge.bSokolRunning = 0;
	CHECK(xgeTextureCreateYUV420P(&texture, 9, 9) == XGE_OK);
	CHECK(info(&texture).iStorage == XGE_TEXTURE_STORAGE_YUV420P && info(&texture).iGpuBytes == 0);
	CHECK(info(&texture).iCpuBytes == 131); xgeTextureFree(&texture);
	g_xge.bSokolRunning = 1;
	CHECK(g_xge.iTextureMemoryBytes == 0 && g_xge.iTextureCount == 0);
	for ( k = 1; k <= (int)mock_next; k++ ) CHECK(mock_textures[k].pixels == NULL);
	printf("PASS texture storage: %d checks (packing, fallback, updates, canonical readback, state, lifetime)\n", checks);
	return 0;
}
