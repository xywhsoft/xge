#include "xui.h"
#include "xui_test_proxy.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#define CHECK(e) do{++checks;if(!(e)){fprintf(stderr,"style_registration:%d: %s\n",__LINE__,#e);failed=1;goto cleanup;}}while(0)
static const char* value(xui_widget widget,const char* key){xui_style_property_t p={0};return xuiWidgetGetResolvedStyleProperty(widget,key,&p)==XUI_OK ? p.tValue.sText:NULL;}
int main(void){int checks=0,failed=0;xui_context context=NULL;xui_widget root=NULL,child=NULL;xui_msgbox box=NULL;xui_font font=NULL;xui_test_proxy_state_t proxy;xuiTestProxyInit(&proxy);
 CHECK(xuiCreate(&context)==XUI_OK && xuiSetProxy(context,&proxy.tProxy)==XUI_OK);
 CHECK(xuiWidgetCreate(context,&root)==XUI_OK && xuiWidgetCreate(context,&child)==XUI_OK && xuiSetRootWidget(context,root)==XUI_OK && xuiWidgetAddChild(root,child)==XUI_OK);
 uint32_t g=xuiStyleGetGeneration(context);xui_widget_type window=xuiWindowGetType(context);CHECK(window && xuiStyleGetGeneration(context)==g+1);g=xuiStyleGetGeneration(context);
 for(int i=0;i<100;++i)CHECK(xuiWindowGetType(context)==window && xuiStyleGetGeneration(context)==g);
 CHECK(xuiStyleBeginUpdate(context)==XUI_OK);xui_widget_type picker=xuiIconPickerGetType(context);CHECK(picker && xuiStyleGetGeneration(context)==g);CHECK(xuiStyleEndUpdate(context)==XUI_OK && xuiStyleGetGeneration(context)==g+1);g=xuiStyleGetGeneration(context);
 for(int i=0;i<100;++i)CHECK(xuiIconPickerGetType(context)==picker && xuiStyleGetGeneration(context)==g);

 /* A type lookup must not undo a caller's explicit registered definition. */
 xui_style_property_info_t custom={0};uint32_t windowMin=xuiStyleFindProperty(context,"window.min_width");CHECK(windowMin && xuiStyleGetPropertyInfo(context,windowMin,&custom)==XUI_OK);custom.tDefaultValue=(xui_style_value_t){.iSize=sizeof(xui_style_value_t),.iType=XUI_STYLE_VALUE_FLOAT,.fFloat=321};CHECK(xuiStyleRegisterProperty(context,&custom,NULL)==XUI_OK);g=xuiStyleGetGeneration(context);CHECK(xuiWindowGetType(context)==window && xuiStyleGetGeneration(context)==g);CHECK(xuiStyleGetPropertyInfo(context,windowMin,&custom)==XUI_OK && custom.tDefaultValue.fFloat==321);
 CHECK(proxy.tProxy.fontLoadFile(&proxy.tProxy,&font,"mock.ttf",14,0)==XUI_OK && xuiSetDefaultFont(context,font)==XUI_OK);
 g=xuiStyleGetGeneration(context);xui_msgbox_desc_t desc={0};desc.iSize=sizeof(desc);desc.sMessage="registration";CHECK(xuiMsgBoxCreate(context,&box,&desc)==XUI_OK && xuiStyleGetGeneration(context)==g+1);xuiMsgBoxDestroy(box);box=NULL;g=xuiStyleGetGeneration(context);
 CHECK(xuiMsgBoxCreate(context,&box,&desc)==XUI_OK && xuiStyleGetGeneration(context)==g);xuiMsgBoxDestroy(box);box=NULL;
 xui_style_property_info_t info={0};info.iSize=sizeof(info);info.sName="test.inherited";info.iValueType=XUI_STYLE_VALUE_STRING;info.iFlags=XUI_STYLE_PROPERTY_INHERITED;info.tDefaultValue.iSize=sizeof(info.tDefaultValue);info.tDefaultValue.iType=XUI_STYLE_VALUE_STRING;info.tDefaultValue.sText="default";uint32_t id=0;CHECK(xuiStyleRegisterProperty(context,&info,&id)==XUI_OK && id);
 xui_style_property_info_t borrowed={0};CHECK(xuiStyleGetPropertyInfo(context,id,&borrowed)==XUI_OK);const char* original=borrowed.tDefaultValue.sText;g=xuiStyleGetGeneration(context);uint32_t again=0;
 CHECK(xuiStyleRegisterProperty(context,&borrowed,&again)==XUI_OK && again==id && xuiStyleGetGeneration(context)==g);CHECK(xuiStyleGetPropertyInfo(context,id,&borrowed)==XUI_OK && borrowed.tDefaultValue.sText==original);
 xui_style_property_t prop={0};prop.iSize=sizeof(prop);prop.sName=info.sName;prop.tValue.iSize=sizeof(prop.tValue);prop.tValue.iType=XUI_STYLE_VALUE_STRING;prop.tValue.sText="parent";CHECK(xuiWidgetSetInlineStyle(root,&prop,1)==XUI_OK && xuiStyleRefresh(context)==XUI_OK && !strcmp(value(child,info.sName),"parent"));
 borrowed.iFlags=0;CHECK(xuiStyleRegisterProperty(context,&borrowed,&again)==XUI_OK && again==id && !strcmp(value(child,info.sName),"default"));
 CHECK(xuiStyleGetPropertyInfo(context,id,&borrowed)==XUI_OK);borrowed.tDefaultValue.sText="changed";CHECK(xuiStyleRegisterProperty(context,&borrowed,NULL)==XUI_OK && !strcmp(value(child,info.sName),"changed"));
 /* Colliding numeric buckets and a registry larger than the index must retain
    exact name/ID lookup, values and the original newest-first ID semantics. */
 CHECK(xuiStyleBeginUpdate(context)==XUI_OK);
 for(int i=0;i<1024;++i){char key[64];snprintf(key,sizeof(key),"collision.%d",i);xui_style_property_info_t row={0};row.iSize=sizeof(row);row.sName=key;row.iPropertyId=100000u+(uint32_t)i*256u;row.iValueType=XUI_STYLE_VALUE_INT;row.tDefaultValue.iSize=sizeof(row.tDefaultValue);row.tDefaultValue.iType=XUI_STYLE_VALUE_INT;row.tDefaultValue.iInt=i;CHECK(xuiStyleRegisterProperty(context,&row,NULL)==XUI_OK);}
 CHECK(xuiStyleEndUpdate(context)==XUI_OK);
 for(int i=0;i<1024;++i){char key[64];snprintf(key,sizeof(key),"collision.%d",i);uint32_t expected=100000u+(uint32_t)i*256u;xui_style_property_info_t row={0};CHECK(xuiStyleFindProperty(context,key)==expected && xuiStyleGetPropertyInfo(context,expected,&row)==XUI_OK && row.tDefaultValue.iInt==i);}
 info=(xui_style_property_info_t){0};info.iSize=sizeof(info);info.sName="float.bits";info.iValueType=XUI_STYLE_VALUE_FLOAT;info.tDefaultValue.iSize=sizeof(info.tDefaultValue);info.tDefaultValue.iType=XUI_STYLE_VALUE_FLOAT;info.tDefaultValue.fFloat=NAN;CHECK(xuiStyleRegisterProperty(context,&info,&id)==XUI_OK);g=xuiStyleGetGeneration(context);CHECK(xuiStyleRegisterProperty(context,&info,NULL)==XUI_OK && xuiStyleGetGeneration(context)==g);
 info.tDefaultValue.fFloat=0.0f;CHECK(xuiStyleRegisterProperty(context,&info,NULL)==XUI_OK);g=xuiStyleGetGeneration(context);info.tDefaultValue.fFloat=-0.0f;CHECK(xuiStyleRegisterProperty(context,&info,NULL)==XUI_OK && xuiStyleGetGeneration(context)==g+1);CHECK(xuiStyleGetPropertyInfo(context,id,&info)==XUI_OK && signbit(info.tDefaultValue.fFloat));
 CHECK(xuiStyleEndUpdate(context)==XUI_ERROR_INVALID_ARGUMENT);
cleanup:if(box)xuiMsgBoxDestroy(box);if(context)xuiDestroy(context);if(font)proxy.tProxy.fontDestroy(&proxy.tProxy,font);if(!failed)printf("style_registration passed: %d checks; stable cached getters, nested initialization, native MsgBox, borrowed strings, explicit updates/inheritance, 1024 colliding IDs and float bit identity\n",checks);return failed;}
