/*
 * af_export.c - PNG sequence / GIF / Sprite sheet export
 *
 * Exports animation frames to various output formats.
 * Uses XGE render-to-texture for frame capture.
 */

#include "af_export.h"
#include "../core/af_doc.h"
#include "../anim/af_playback.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------ */
/* PNG export (via stb_image_write)                                   */
/* ------------------------------------------------------------------ */

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "lib/stb/stb_image_write.h"

/* ------------------------------------------------------------------ */
/* PNG Sequence export                                                */
/* ------------------------------------------------------------------ */

int afExportPNGSequence(af_doc pDoc, const char* sDir, int iStartFrame,
                        int iEndFrame, float fScale)
{
	int frame;
	int w = (int)(pDoc->fStageWidth * fScale);
	int h = (int)(pDoc->fStageHeight * fScale);
	char path[1024];

	if ( w <= 0 || h <= 0 )
		return -1;

	if ( iStartFrame < 0 ) iStartFrame = 0;
	if ( iEndFrame >= (int)pDoc->iFrameCount ) iEndFrame = (int)pDoc->iFrameCount - 1;

	printf("afExportPNGSequence: exporting frames %d-%d to '%s' (%dx%d)\n",
	       iStartFrame, iEndFrame, sDir, w, h);

	for ( frame = iStartFrame; frame <= iEndFrame; frame++ ) {
		unsigned char* pixels;

		/* Allocate pixel buffer (would be filled by render-to-texture) */
		pixels = (unsigned char*)calloc((size_t)(w * h * 4), 1);
		if ( pixels == NULL )
			return -2;

		/*
		 * In a full implementation, this would:
		 * 1. Set playback to frame
		 * 2. Render stage to FBO
		 * 3. glReadPixels to get RGBA data
		 * For now, write a placeholder frame.
		 */

		/* Fill with background color */
		{
			int px;
			unsigned char bgR = (pDoc->iBackgroundColor >> 24) & 0xFF;
			unsigned char bgG = (pDoc->iBackgroundColor >> 16) & 0xFF;
			unsigned char bgB = (pDoc->iBackgroundColor >> 8) & 0xFF;
			unsigned char bgA = pDoc->iBackgroundColor & 0xFF;
			for ( px = 0; px < w * h; px++ ) {
				pixels[px * 4 + 0] = bgR;
				pixels[px * 4 + 1] = bgG;
				pixels[px * 4 + 2] = bgB;
				pixels[px * 4 + 3] = bgA;
			}
		}

		/* Write PNG file */
		sprintf(path, "%s/frame_%04d.png", sDir, frame);
		if ( !stbi_write_png(path, w, h, 4, pixels, w * 4) ) {
			free(pixels);
			printf("afExportPNGSequence: failed to write '%s'\n", path);
			return -3;
		}

		free(pixels);
	}

	printf("afExportPNGSequence: exported %d frames\n", iEndFrame - iStartFrame + 1);
	return 0;
}

/* ------------------------------------------------------------------ */
/* GIF export (minimal uncompressed GIF writer)                       */
/* ------------------------------------------------------------------ */

static void __afWriteGifHeader(FILE* fp, int w, int h)
{
	unsigned char header[13];
	memcpy(header, "GIF89a", 6);
	header[6] = w & 0xFF;
	header[7] = (w >> 8) & 0xFF;
	header[8] = h & 0xFF;
	header[9] = (h >> 8) & 0xFF;
	header[10] = 0x70; /* no GCT, 8-bit color */
	header[11] = 0;    /* bg color index */
	header[12] = 0;    /* pixel aspect ratio */
	fwrite(header, 1, 13, fp);
}

int afExportGIF(af_doc pDoc, const char* sPath, int iStartFrame,
                int iEndFrame, float fScale, int iDelay)
{
	FILE* fp;
	int w = (int)(pDoc->fStageWidth * fScale);
	int h = (int)(pDoc->fStageHeight * fScale);

	if ( w <= 0 || h <= 0 )
		return -1;

	fp = fopen(sPath, "wb");
	if ( fp == NULL ) {
		printf("afExportGIF: cannot create '%s'\n", sPath);
		return -2;
	}

	printf("afExportGIF: exporting to '%s' (%dx%d, delay=%dms)\n",
	       sPath, w, h, iDelay);

	__afWriteGifHeader(fp, w, h);

	/*
	 * Full GIF encoding requires LZW compression and color quantization.
	 * This is a placeholder - a complete implementation would use
	 * a gif encoder library or implement LZW encoding.
	 */

	/* GIF trailer */
	fputc(0x3B, fp);
	fclose(fp);

	printf("afExportGIF: done (placeholder - full encoder not yet implemented)\n");
	(void)iStartFrame; (void)iEndFrame; (void)iDelay;
	return 0;
}

/* ------------------------------------------------------------------ */
/* Sprite sheet export                                                */
/* ------------------------------------------------------------------ */

int afExportSpriteSheet(af_doc pDoc, const char* sPath, int iStartFrame,
                        int iEndFrame, int iCols, float fScale)
{
	int frameCount = iEndFrame - iStartFrame + 1;
	int cellW = (int)(pDoc->fStageWidth * fScale);
	int cellH = (int)(pDoc->fStageHeight * fScale);
	int rows;
	int sheetW, sheetH;
	unsigned char* pixels;
	int frame, col, row;

	if ( frameCount <= 0 || cellW <= 0 || cellH <= 0 )
		return -1;

	if ( iCols <= 0 ) iCols = 1;
	rows = (frameCount + iCols - 1) / iCols;
	sheetW = cellW * iCols;
	sheetH = cellH * rows;

	printf("afExportSpriteSheet: %d frames, %dx%d grid, sheet %dx%d\n",
	       frameCount, iCols, rows, sheetW, sheetH);

	pixels = (unsigned char*)calloc((size_t)(sheetW * sheetH * 4), 1);
	if ( pixels == NULL )
		return -2;

	/* Fill with background */
	{
		int px;
		unsigned char bgR = (pDoc->iBackgroundColor >> 24) & 0xFF;
		unsigned char bgG = (pDoc->iBackgroundColor >> 16) & 0xFF;
		unsigned char bgB = (pDoc->iBackgroundColor >> 8) & 0xFF;
		for ( px = 0; px < sheetW * sheetH; px++ ) {
			pixels[px * 4 + 0] = bgR;
			pixels[px * 4 + 1] = bgG;
			pixels[px * 4 + 2] = bgB;
			pixels[px * 4 + 3] = 255;
		}
	}

	/*
	 * In full implementation, render each frame and blit into sheet.
	 * For now, write placeholder cells with frame number indicator.
	 */
	for ( frame = 0; frame < frameCount; frame++ ) {
		col = frame % iCols;
		row = frame / iCols;

		/* Draw a small marker in each cell corner */
		{
			int mx = col * cellW + 2;
			int my = row * cellH + 2;
			int ms = 4;
			int dx, dy;
			for ( dy = 0; dy < ms && my + dy < sheetH; dy++ ) {
				for ( dx = 0; dx < ms && mx + dx < sheetW; dx++ ) {
					int idx = ((my + dy) * sheetW + (mx + dx)) * 4;
					pixels[idx + 0] = 255;
					pixels[idx + 1] = 0;
					pixels[idx + 2] = 0;
					pixels[idx + 3] = 255;
				}
			}
		}
	}

	/* Write sprite sheet PNG */
	if ( !stbi_write_png(sPath, sheetW, sheetH, 4, pixels, sheetW * 4) ) {
		free(pixels);
		printf("afExportSpriteSheet: failed to write '%s'\n", sPath);
		return -3;
	}

	free(pixels);

	/* Write JSON metadata */
	{
		char jsonPath[1024];
		FILE* jf;
		sprintf(jsonPath, "%s.json", sPath);
		/* Remove .png extension from json path */
		{
			char* dot = strrchr(jsonPath, '.');
			if ( dot && strcmp(dot, ".png.json") == 0 ) {
				strcpy(dot, ".json");
			}
		}

		jf = fopen(jsonPath, "w");
		if ( jf ) {
			fprintf(jf, "{\n");
			fprintf(jf, "  \"image\": \"%s\",\n", sPath);
			fprintf(jf, "  \"frameWidth\": %d,\n", cellW);
			fprintf(jf, "  \"frameHeight\": %d,\n", cellH);
			fprintf(jf, "  \"columns\": %d,\n", iCols);
			fprintf(jf, "  \"rows\": %d,\n", rows);
			fprintf(jf, "  \"frameCount\": %d,\n", frameCount);
			fprintf(jf, "  \"frameRate\": %f\n", pDoc->fFrameRate);
			fprintf(jf, "}\n");
			fclose(jf);
		}
	}

	printf("afExportSpriteSheet: done\n");
	return 0;
}

/* ------------------------------------------------------------------ */
/* Unified export dispatch                                            */
/* ------------------------------------------------------------------ */

int afExport(af_doc pDoc, int iType, const char* sPath,
             int iStartFrame, int iEndFrame, float fScale)
{
	switch ( iType ) {
	case AF_EXPORT_TYPE_PNG_SEQ:
		return afExportPNGSequence(pDoc, sPath, iStartFrame, iEndFrame, fScale);

	case AF_EXPORT_TYPE_GIF:
		return afExportGIF(pDoc, sPath, iStartFrame, iEndFrame, fScale, 
		                   (int)(1000.0f / pDoc->fFrameRate));

	case AF_EXPORT_TYPE_SPRITE_SHEET:
		return afExportSpriteSheet(pDoc, sPath, iStartFrame, iEndFrame, 8, fScale);

	default:
		return -1;
	}
}
