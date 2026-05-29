#include "xge.h"
#include "xui.h"

#include <stdio.h>

int main(void)
{
	uint32_t iColor;
	xge_color_t tColor;
	xge_frame_stats_t tStats;

	if ( XUI_VERSION_MAJOR != 2 ) {
		printf("xge smoke failed: xui version got %d\n", XUI_VERSION_MAJOR);
		return 1;
	}

	iColor = xgeColorRGBA(1, 2, 3, 4);
	if ( iColor != 0x01020304u ) {
		printf("xge smoke failed: color pack got 0x%08x\n", iColor);
		return 1;
	}

	tColor = xgeColorUnpack(iColor);
	if ( (tColor.fR < 0.003f) || (tColor.fR > 0.005f) ||
	     (tColor.fG < 0.007f) || (tColor.fG > 0.009f) ||
	     (tColor.fB < 0.011f) || (tColor.fB > 0.013f) ||
	     (tColor.fA < 0.015f) || (tColor.fA > 0.017f) ) {
		printf("xge smoke failed: color unpack got %.4f,%.4f,%.4f,%.4f\n", tColor.fR, tColor.fG, tColor.fB, tColor.fA);
		return 1;
	}

	tStats = xgeFrameStatsGet();
	if ( tStats.iFrameCount != 0 ) {
		printf("xge smoke failed: uninitialized frame count got %d\n", tStats.iFrameCount);
		return 1;
	}

	printf("xge smoke passed\n");
	return 0;
}
