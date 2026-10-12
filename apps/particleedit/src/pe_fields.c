#include "pe_fields.h"
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static const char *const shapes[] = {"点", "线段", "矩形", "圆盘", "圆环", "圆锥"};
static const char *const spaces[] = {"世界空间", "局部空间"};
static const char *const collisions[] = {"无", "反弹", "销毁"};
/* Keep the numeric order identical to XGE_BLEND_* in xge.h. */
static const char *const blends[] = {"不混合", "Alpha", "Add", "Multiply", "Screen"};
#define OFFSET(m) offsetof(xge_particle_emitter_t, m)
#define F(m, n, g, lo, hi, h) {#m, n, g, h, OFFSET(m), 0, PE_FLOAT, lo, hi, 1, NULL, 0}
#define A(m, n, g, h) {#m, n, g, h, OFFSET(m), 0, PE_FLOAT, -57295780, 57295780, 57.2957795131f, NULL, 0}
#define I(m, n, g, lo, hi, h) {#m, n, g, h, OFFSET(m), 0, PE_INT, lo, hi, 1, NULL, 0}
#define U(m, n, g, lo, hi, h) {#m, n, g, h, OFFSET(m), 0, PE_UINT, lo, hi, 1, NULL, 0}
#define B(m, n, g, h) {#m, n, g, h, OFFSET(m), 0, PE_BOOL, 0, 1, 1, NULL, 0}
#define S(m, n, g, h)                                                                                        \
	{#m, n, g, h, OFFSET(m), sizeof(((xge_particle_emitter_t *)0)->m), PE_STRING, 0, 0, 1, NULL, 0}
#define C(m, n, g, h) {#m, n, g, h, OFFSET(m), 0, PE_COLOR, 0, 0, 1, NULL, 0}
#define E(m, n, g, opts, h)                                                                                  \
	{#m,      n,         g,                                                                                  \
	 h,       OFFSET(m), 0,                                                                                  \
	 PE_ENUM, 0,         sizeof(opts) / sizeof(opts[0]) - 1,                                                 \
	 1,       opts,      sizeof(opts) / sizeof(opts[0])}
#define RANGE(m, n, g, lo, hi, h) F(m.fMin, n " · 最小", g, lo, hi, h), F(m.fMax, n " · 最大", g, lo, hi, h)
#define VECTOR(m, n, g, h) F(m.fX, n " X", g, -1e6, 1e6, h), F(m.fY, n " Y", g, -1e6, 1e6, h)
const pe_field pe_fields[] = {
    S(sName, "名称", "基本 / 发射", "时间轴与发射器列表中显示的名称"),
    U(iMaxParticles, "粒子容量", "基本 / 发射", 1, 1000000,
      "运行时合法上限 100 万；编辑器预览总预算 10 万，不会静默缩减资产容量"),
    B(bAutomatic, "自动发射", "基本 / 发射", "关闭后，仅由手动发射或子发射器事件生成粒子"),
    B(bLoop, "循环发射", "基本 / 发射", "循环周期由持续时间决定，时间轴显示第一轮"),
    F(fDelay, "启动延迟 / s", "基本 / 发射", 0, 1e6, "初次发射前等待时间"),
    F(fDuration, "持续时间 / s", "基本 / 发射", 0, 1e6,
      "0 表示不限时；非零值至少 0.001 秒，Burst 必须严格小于此值"),
    F(fRate, "每秒发射量", "基本 / 发射", 0, 1e6, "连续发射；Burst 独立于此参数"),
    F(fRateOverDistance, "每单位移动发射量", "基本 / 发射", 0, 1e6, "启用预览移动路径观察尾尘效果"),
    E(iSpace, "模拟空间", "形状 / 初始变换", spaces, "世界空间粒子不跟随发射器；局部空间粒子跟随"),
    E(iShape, "发射形状", "形状 / 初始变换", shapes, "只影响出生位置或方向，不改变现存粒子"),
    VECTOR(tOffset, "出生偏移", "形状 / 初始变换", "相对根变换，世界单位"),
    VECTOR(tShapeSize, "形状尺寸", "形状 / 初始变换", "线段或矩形的尺寸"),
    F(fRadius, "外半径", "形状 / 初始变换", 0, 1e6, "圆、环的外半径，必须 >= 内半径"),
    F(fInnerRadius, "内半径", "形状 / 初始变换", 0, 1e6, "圆环内半径，不能超过外半径"),
    A(fDirection, "方向 / °", "形状 / 初始变换", "界面使用度；文件和运行时使用弧度；Y 轴向下"),
    {"fSpread", "方向扩散 / °", "形状 / 初始变换", "0～360 度", OFFSET(fSpread), 0, PE_FLOAT, 0, 360,
     57.2957795131f, NULL, 0},
    RANGE(tLife, "寿命 / s", "出生随机范围", .0001, 3600, "随机范围要求最小 <= 最大；抬高范围时先改最大值"),
    RANGE(tSpeed, "速度", "出生随机范围", 0, 1e6, "出生随机值 × 速度生命周期曲线"),
    RANGE(tSize, "尺寸", "出生随机范围", 0, 1e6, "出生随机值 × 尺寸生命周期曲线"),
    A(tRotation.fMin, "旋转最小 / °", "出生随机范围", "文件中存储弧度"),
    A(tRotation.fMax, "旋转最大 / °", "出生随机范围", "最小 <= 最大"),
    A(tAngularVelocity.fMin, "角速度最小 / °/s", "出生随机范围", "文件中存储弧度每秒"),
    A(tAngularVelocity.fMax, "角速度最大 / °/s", "出生随机范围", "最小 <= 最大"),
    C(iColorMin, "出生颜色 · 最小", "出生随机范围", "RGBA：#RRGGBBAA；出生颜色 × 生命周期渐变 × Alpha 曲线"),
    C(iColorMax, "出生颜色 · 最大", "出生随机范围", "随机插值的另一端颜色"),
    VECTOR(tGravity, "重力", "运动 / 力场", "世界单位每平方秒，向下为正 Y"),
    F(fDrag, "阻力", "运动 / 力场", 0, 1e6, "线性阻尼系数"),
    F(fRadialAcceleration, "径向加速度", "运动 / 力场", -1e6, 1e6, "相对发射中心，正值向外"),
    F(fTangentialAcceleration, "切向加速度", "运动 / 力场", -1e6, 1e6, "相对发射中心的切向力"),
    F(fInheritVelocity, "继承根移动速度", "运动 / 力场", -1e6, 1e6, "新生粒子继承发射器的移动速度系数"),
    F(fNoiseStrength, "噪声强度", "运动 / 力场", -1e6, 1e6, "确定性噪声力；固定随机种子可复现"),
    F(fNoiseFrequency, "噪声频率", "运动 / 力场", 0, 1e6, "噪声的空间频率"),
    F(fNoiseScroll, "噪声滚动速度", "运动 / 力场", -1e6, 1e6, "噪声随时间的变化速度"),
    S(sTexture, "纹理绑定", "渲染 / 资源",
      "soft / sheet 为内置纹理；其他名称按相对粒子文件的图片路径预览；未绑定会警告"),
    S(sMaterial, "材质绑定", "渲染 / 资源",
      "空使用默认材质；ring 为范例材质；其他逻辑材质保留并显示未解析警告"),
    E(iBlend, "混合模式", "渲染 / 资源", blends, "纹理使用预乘 Alpha"),
    F(fAspect, "宽高比", "渲染 / 资源", .0001, 1e6, "必须大于零"),
    F(fStretch, "速度拉伸", "渲染 / 资源", 0, 1e6, "按速度拉长粒子"),
    B(bAlignVelocity, "朝向速度", "渲染 / 资源", "粒子的朝向跟随运动方向"),
    B(bScreenSpace, "屏幕空间", "渲染 / 资源", "预览舞台为 1000 × 680；该参数仍使用真实运行时语义"),
    I(iLayer, "渲染层", "渲染 / 资源", -1000000, 1000000, "优先按 layer 排序"),
    I(iOrder, "层内顺序", "渲染 / 资源", -1000000, 1000000, "相同 layer 下按 order 排序"),
    F(tTextureRect.fX, "纹理区域 X", "图集 / 翻页", -1e6, 1e6, "纹理像素坐标"),
    F(tTextureRect.fY, "纹理区域 Y", "图集 / 翻页", -1e6, 1e6, "纹理像素坐标"),
    F(tTextureRect.fW, "纹理区域宽", "图集 / 翻页", 0, 1e6, "0 使用整张纹理"),
    F(tTextureRect.fH, "纹理区域高", "图集 / 翻页", 0, 1e6, "0 使用整张纹理"),
    I(iColumns, "列数", "图集 / 翻页", 1, 256, "规则网格图集"),
    I(iRows, "行数", "图集 / 翻页", 1, 256, "规则网格图集"),
    RANGE(tStartFrame, "起始帧", "图集 / 翻页", 0, 65535, "起始随机帧范围"),
    F(fFramesPerSecond, "播放帧率", "图集 / 翻页", 0, 1e6, "0 将整段动画映射到粒子寿命"),
    E(iCollision, "碰撞响应", "碰撞", collisions, "需要碰撞查询回调；打开预览地面测试"),
    F(fCollisionRadius, "碰撞半径", "碰撞", 0, 1e6, "扫掠球半径，与绘制尺寸独立"),
    F(fRestitution, "弹性系数", "碰撞", 0, 1, "0 不反弹，1 完全弹性"),
    F(fFriction, "切向摩擦", "碰撞", 0, 1, "碰撞后的切向减速系数"),
    I(arrSubEmitters[0].iEmitter, "出生 → 目标索引", "事件 / 子发射器", -1, 15,
      "-1 关闭；使用左侧从 0 开始的索引，禁止循环引用"),
    U(arrSubEmitters[0].iCount, "出生 → 发射数", "事件 / 子发射器", 0, 1e6, "每个出生事件触发的粒子数"),
    F(arrSubEmitters[0].fInheritVelocity, "出生 → 继承速度", "事件 / 子发射器", -1e6, 1e6,
      "子粒子继承父粒子速度系数"),
    I(arrSubEmitters[1].iEmitter, "死亡 → 目标索引", "事件 / 子发射器", -1, 15,
      "-1 关闭；删除和重排发射器时自动修正引用"),
    U(arrSubEmitters[1].iCount, "死亡 → 发射数", "事件 / 子发射器", 0, 1e6, "每个死亡事件触发的粒子数"),
    F(arrSubEmitters[1].fInheritVelocity, "死亡 → 继承速度", "事件 / 子发射器", -1e6, 1e6,
      "子粒子继承父粒子速度系数"),
    I(arrSubEmitters[2].iEmitter, "碰撞 → 目标索引", "事件 / 子发射器", -1, 15,
      "-1 关闭；没有查询回调时不产生碰撞事件"),
    U(arrSubEmitters[2].iCount, "碰撞 → 发射数", "事件 / 子发射器", 0, 1e6, "每次碰撞事件触发的粒子数"),
    F(arrSubEmitters[2].fInheritVelocity, "碰撞 → 继承速度", "事件 / 子发射器", -1e6, 1e6,
      "子粒子继承父粒子速度系数")};
const int pe_field_count = sizeof(pe_fields) / sizeof(pe_fields[0]);
static int number(const char *text, double min, double max, double *value)
{
	char *end;
	double result;
	if (!text || !*text)
		return 0;
	errno = 0;
	result = strtod(text, &end);
	while (isspace((unsigned char)*end))
		++end;
	if (errno || end == text || *end || !isfinite(result) || result < min || result > max)
		return 0;
	*value = result;
	return 1;
}
int pe_parse_float(const char *text, float min, float max, float *value)
{
	double n;
	if (!number(text, min, max, &n))
		return 0;
	*value = (float)n;
	return 1;
}
int pe_parse_int(const char *text, int min, int max, int *value)
{
	double n;
	if (!number(text, min, max, &n) || floor(n) != n)
		return 0;
	*value = (int)n;
	return 1;
}
int pe_parse_color(const char *text, uint32_t *value)
{
	unsigned long rgba;
	char *end;
	if (!text || text[0] != '#' || strlen(text) != 9)
		return 0;
	for (int i = 1; i < 9; ++i)
		if (!isxdigit((unsigned char)text[i]))
			return 0;
	rgba = strtoul(text + 1, &end, 16);
	if (*end)
		return 0;
	*value = XGE_COLOR_RGBA((rgba >> 24) & 255, (rgba >> 16) & 255, (rgba >> 8) & 255, rgba & 255);
	return 1;
}
int pe_field_find(const char *id)
{
	for (int i = 0; i < pe_field_count; ++i)
		if (!strcmp(pe_fields[i].id, id))
			return i;
	return -1;
}
void pe_field_get(const pe_field *f, const xge_particle_emitter_t *e, char *out, size_t size)
{
	const void *p = (const char *)e + f->offset;
	if (f->type == PE_FLOAT)
		snprintf(out, size, "%.9g", *(const float *)p * f->scale);
	else if (f->type == PE_STRING)
		snprintf(out, size, "%s", (const char *)p);
	else if (f->type == PE_UINT)
		snprintf(out, size, "%u", *(const uint32_t *)p);
	else if (f->type == PE_BOOL)
		snprintf(out, size, "%s", *(const int *)p ? "true" : "false");
	else if (f->type == PE_COLOR)
	{
		uint32_t c = *(const uint32_t *)p;
		snprintf(out, size, "#%02X%02X%02X%02X", XGE_COLOR_GET_R(c), XGE_COLOR_GET_G(c), XGE_COLOR_GET_B(c),
		         XGE_COLOR_GET_A(c));
	}
	else
		snprintf(out, size, "%d", *(const int *)p);
}
int pe_field_set(const pe_field *f, xge_particle_emitter_t *e, const char *text)
{
	void *p = (char *)e + f->offset;
	double n;
	if (f->type == PE_STRING)
	{
		if (!text || strlen(text) >= f->capacity)
			return 0;
		memset(p, 0, f->capacity);
		strcpy(p, text);
		return 1;
	}
	if (f->type == PE_COLOR)
		return pe_parse_color(text, p);
	if (f->type == PE_BOOL)
	{
		if (!strcmp(text, "true") || !strcmp(text, "1"))
			*(int *)p = 1;
		else if (!strcmp(text, "false") || !strcmp(text, "0"))
			*(int *)p = 0;
		else
			return 0;
		return 1;
	}
	if (!number(text, f->min, f->max, &n))
		return 0;
	if (f->type == PE_FLOAT)
		*(float *)p = (float)(n / f->scale);
	else
	{
		if (floor(n) != n)
			return 0;
		if (f->type == PE_UINT)
			*(uint32_t *)p = (uint32_t)n;
		else
			*(int *)p = (int)n;
	}
	return 1;
}
