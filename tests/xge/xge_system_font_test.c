#include "xge.h"
#include "xui.h"

#include <stdio.h>
#include <string.h>
#include <math.h>

int main(void)
{
	xge_desc_t engine={0};
	xge_system_font_desc_t desc={0};
	xge_font_metrics_t metrics={0};
	xge_font_t font;
	xui_proxy proxy=NULL;
	xui_context context=NULL;
	xui_context_desc_t contextDesc;
	int status;
	memset(&font,0,sizeof(font));
	engine.iWidth=32;
	engine.iHeight=32;
	engine.sTitle="xge system font test";
	engine.iFlags=XGE_INIT_OFFSCREEN;
	engine.iRunMode=XGE_RUN_MANUAL;
	status=xgeInit(&engine);
	if(status!=XGE_OK){printf("xge_system_font_test: xgeInit=%d\n",status);return 1;}
	desc.iSize=sizeof(desc);
	desc.iRole=XGE_SYSTEM_FONT_UI;
	desc.fPixelSize=NAN;
	if(xgeFontLoadSystem(&font,&desc)!=XGE_ERROR_INVALID_ARGUMENT){xgeUnit();return 1;}
	desc.fPixelSize=INFINITY;
	if(xgeFontLoadSystem(&font,&desc)!=XGE_ERROR_INVALID_ARGUMENT){xgeUnit();return 1;}
	desc.fPixelSize=0;
	status=xgeFontLoadSystem(&font,&desc);
	if(status!=XGE_OK){printf("xge_system_font_test: load=%d\n",status);xgeUnit();return 1;}
	metrics.iSize=sizeof(metrics);
	status=xgeFontGetMetrics(&font,&metrics);
	if(status!=XGE_OK || metrics.fPixelSize<=0.0f || metrics.fLineHeight<=0.0f){
		printf("xge_system_font_test: metrics=%d size=%.2f line=%.2f\n",status,metrics.fPixelSize,metrics.fLineHeight);
		xgeFontFree(&font);xgeUnit();return 1;
	}
	printf("xge_system_font_test passed: %.2fpx line %.2f\n",metrics.fPixelSize,metrics.fLineHeight);
	xgeFontFree(&font);
	xuiContextDescDefault(&contextDesc);
	contextDesc.iSystemFontPolicy=XUI_SYSTEM_FONT_EAGER;
	status=xuiProxyXgeCreate(&proxy);
	if(status==XUI_OK)status=xuiCreateOwnedEx(proxy,&contextDesc,&context);
	if(status!=XUI_OK || !context || !xuiGetDefaultFont(context)){
		printf("xge_system_font_test: owned XUI eager=%d\n",status);
		if(context)xuiDestroy(context);
		if(proxy)xuiProxyReleaseChecked(proxy);
		xgeUnit();return 1;
	}
	status=xuiDestroyChecked(context);
	if(status!=XUI_OK){
		printf("xge_system_font_test: context destroy=%d\n",status);
		xuiProxyReleaseChecked(proxy);xgeUnit();return 1;
	}
	status=xuiProxyReleaseChecked(proxy);
	if(status!=XUI_OK){
		printf("xge_system_font_test: proxy release=%d\n",status);
		xgeUnit();return 1;
	}
	xgeUnit();
	return 0;
}
