/* ch188 — VirtualJoystick 虚拟摇杆 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_virtual_joystick_desc_t desc;
	xui_widget pJoystick;
	int ret;

	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.fRadius = 80.0f;
	desc.fKnobSize = 36.0f;
	desc.fDeadZone = 0.1f;
	desc.bUseBuiltinAtlas = 1;
	ret = xuiVirtualJoystickCreate(ctx->pContext, &pJoystick, &desc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetMargin(pJoystick, (xui_thickness_t){100.0f, 100.0f, 0.0f, 0.0f});
	xuiWidgetSetPreferredSize(pJoystick, (xui_vec2_t){180.0f, 180.0f});
	return xuiWidgetAddChild(ctx->pRoot, pJoystick);
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch188", argc, argv); }
