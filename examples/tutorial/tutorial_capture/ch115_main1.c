/* ch115 - Keyboard transitions through ordered input events.
 * The replay covers a press and release; an interactive game uses the same
 * event fields to update its own action state, then maps actions to gameplay.
 */
#include "tut_capture.h"

static xge_font_t g_font;
static int g_font_loaded, g_ran, g_verified, g_events, g_space_down;

static const char* find_font(void) { static const char* p[] = {"C:\\Windows\\Fonts\\segoeui.ttf", "C:\\Windows\\Fonts\\arial.ttf"}; int i; for(i=0;i<2;i++){FILE* f=fopen(p[i],"rb");if(f){fclose(f);return p[i];}} return NULL; }
static void label(const char* s,float x,float y,uint32_t c) { if(g_font_loaded) xgeTextDraw(&g_font,s,x,y,c); }

static void post_key(int type, int key)
{
	xge_input_event_t e;
	memset(&e, 0, sizeof(e)); e.iSize = sizeof(e); e.iType = type; e.iKey = key;
	(void)xgeInputEventPost(&e);
}

static void replay_and_consume(void)
{
	xge_input_event_t e;
	post_key(XGE_EVENT_KEY_DOWN, XGE_KEY_SPACE);
	post_key(XGE_EVENT_KEY_UP, XGE_KEY_SPACE);
	while ( xgeInputEventGet(&e) > 0 ) {
		g_events++;
		if ( e.iKey == XGE_KEY_SPACE && e.iType == XGE_EVENT_KEY_DOWN ) g_space_down = 1;
		if ( e.iKey == XGE_KEY_SPACE && e.iType == XGE_EVENT_KEY_UP ) g_space_down = 0;
	}
	g_verified = (g_events == 2 && g_space_down == 0);
	printf("ch115 keyboard self-test: %s\n", g_verified ? "PASS" : "FAIL");
}

static void draw_scene(void)
{
	uint32_t status;
	if(!g_ran){const char* p=find_font();if(p && xgeFontLoad(&g_font,p,22.0f)==XGE_OK)g_font_loaded=1;replay_and_consume();g_ran=1;}
	status=g_verified?XGE_COLOR_RGBA(79,216,194,255):XGE_COLOR_RGBA(255,107,94,255);
	xgeShapeRectFill((xge_rect_t){70,58,660,80},XGE_COLOR_RGBA(23,34,45,255));xgeShapeRectStroke((xge_rect_t){70,58,660,80},2,status);xgeShapeCircleFill(108,98,16,status);
	label("XGE ordered keyboard input",142,78,XGE_COLOR_RGBA(231,238,246,255));label(g_verified?"KEY_DOWN -> KEY_UP replay: PASS":"keyboard self-test: FAIL",142,106,status);
	xgeShapeRectFill((xge_rect_t){208,235,384,112},XGE_COLOR_RGBA(29,42,55,255));xgeShapeRectStroke((xge_rect_t){208,235,384,112},2,XGE_COLOR_RGBA(109,179,242,255));
	label("SPACE",330,260,XGE_COLOR_RGBA(246,248,252,255));label("pressed edge",240,315,XGE_COLOR_RGBA(79,216,194,255));label("released edge",443,315,XGE_COLOR_RGBA(255,180,84,255));
	xgeShapeLine(380,390,380,450,3,XGE_COLOR_RGBA(72,93,115,255));xgeShapeTriangleFill((xge_vec2_t){380,470},(xge_vec2_t){365,445},(xge_vec2_t){395,445},XGE_COLOR_RGBA(255,180,84,255));
	label("Keep action state in your control/game; event queue preserves arrival order.",92,530,XGE_COLOR_RGBA(148,163,184,255));
}
int main(int argc,char** argv){int r=tut_run(draw_scene,"ch115-keyboard",argc,argv);return(r==0&&(!g_ran||g_verified))?0:1;}
