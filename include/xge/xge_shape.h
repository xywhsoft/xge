/**
 * @file xge_shape.h
 * @brief XGE 形状绘制模块
 */

#ifndef XGE_SHAPE_H
#define XGE_SHAPE_H

#include <math.h>

/* ============================================================================
 * 形状渲染器状态
 * ============================================================================ */

typedef struct XgeShapeRenderer {
	GLuint hProgram;
	GLuint hVAO;
	GLuint hVBO;
	GLint iLocColor;
	int bInitialized;
} XgeShapeRenderer;

static XgeShapeRenderer g_shapeRenderer = {0};

/* 着色器源码 */
static const char* s_sShapeVertexShader =
	"#version 330 core\n"
	"layout (location = 0) in vec2 aPos;\n"
	"uniform vec2 uResolution;\n"
	"void main() {\n"
	"    vec2 pos = (aPos / uResolution) * 2.0 - 1.0;\n"
	"    pos.y = -pos.y;\n"
	"    gl_Position = vec4(pos, 0.0, 1.0);\n"
	"}\n";

static const char* s_sShapeFragmentShader =
	"#version 330 core\n"
	"uniform vec4 uColor;\n"
	"out vec4 FragColor;\n"
	"void main() {\n"
	"    FragColor = uColor;\n"
	"}\n";

/* ============================================================================
 * 内部函数
 * ============================================================================ */

static GLuint xge_CompileShader(GLenum iType, const char* sSource)
{
	GLuint hShader = glCreateShader(iType);
	glShaderSource(hShader, 1, &sSource, NULL);
	glCompileShader(hShader);
	
	GLint iSuccess;
	glGetShaderiv(hShader, GL_COMPILE_STATUS, &iSuccess);
	if ( !iSuccess ) {
		char sLog[512];
		glGetShaderInfoLog(hShader, 512, NULL, sLog);
		XGE_ERROR("Shader compile error: %s", sLog);
		glDeleteShader(hShader);
		return 0;
	}
	
	return hShader;
}

static int xge_ShapeRendererInit(void)
{
	if ( g_shapeRenderer.bInitialized ) {
		return 1;
	}
	
	/* 编译着色器 */
	GLuint hVert = xge_CompileShader(GL_VERTEX_SHADER, s_sShapeVertexShader);
	GLuint hFrag = xge_CompileShader(GL_FRAGMENT_SHADER, s_sShapeFragmentShader);
	
	if ( (!hVert) || (!hFrag) ) {
		return 0;
	}
	
	/* 链接程序 */
	g_shapeRenderer.hProgram = glCreateProgram();
	glAttachShader(g_shapeRenderer.hProgram, hVert);
	glAttachShader(g_shapeRenderer.hProgram, hFrag);
	glLinkProgram(g_shapeRenderer.hProgram);
	
	GLint iSuccess;
	glGetProgramiv(g_shapeRenderer.hProgram, GL_LINK_STATUS, &iSuccess);
	if ( !iSuccess ) {
		char sLog[512];
		glGetProgramInfoLog(g_shapeRenderer.hProgram, 512, NULL, sLog);
		XGE_ERROR("Program link error: %s", sLog);
		return 0;
	}
	
	glDeleteShader(hVert);
	glDeleteShader(hFrag);
	
	/* 获取 Uniform 位置 */
	g_shapeRenderer.iLocColor = glGetUniformLocation(g_shapeRenderer.hProgram, "uColor");
	
	/* 创建 VAO 和 VBO */
	glGenVertexArrays(1, &g_shapeRenderer.hVAO);
	glGenBuffers(1, &g_shapeRenderer.hVBO);
	
	glBindVertexArray(g_shapeRenderer.hVAO);
	glBindBuffer(GL_ARRAY_BUFFER, g_shapeRenderer.hVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 1024, NULL, GL_DYNAMIC_DRAW);
	
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	
	g_shapeRenderer.bInitialized = 1;
	return 1;
}

static void xge_ShapeBegin(unsigned int uColor)
{
	if ( !g_shapeRenderer.bInitialized ) {
		xge_ShapeRendererInit();
	}
	
	glUseProgram(g_shapeRenderer.hProgram);
	
	/* 设置分辨率 */
	GLint iLocRes = glGetUniformLocation(g_shapeRenderer.hProgram, "uResolution");
	int iW = g_xge.pCurrentTarget ? g_xge.pCurrentTarget->iWidth : g_xge.iWidth;
	int iH = g_xge.pCurrentTarget ? g_xge.pCurrentTarget->iHeight : g_xge.iHeight;
	glUniform2f(iLocRes, (float)iW, (float)iH);
	
	/* 设置颜色 */
	float fR = XGE_GETR(uColor) / 255.0f;
	float fG = XGE_GETG(uColor) / 255.0f;
	float fB = XGE_GETB(uColor) / 255.0f;
	float fA = XGE_GETA(uColor) / 255.0f;
	glUniform4f(g_shapeRenderer.iLocColor, fR, fG, fB, fA);
	
	glBindVertexArray(g_shapeRenderer.hVAO);
	glBindBuffer(GL_ARRAY_BUFFER, g_shapeRenderer.hVBO);
}

static void xge_ShapeEnd(void)
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);
}

/* ============================================================================
 * 形状 API 实现
 * ============================================================================ */

void xgePset(int iX, int iY, unsigned int uColor)
{
	float arrVertices[] = { (float)iX, (float)iY };
	
	xge_ShapeBegin(uColor);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(arrVertices), arrVertices);
	glDrawArrays(GL_POINTS, 0, 1);
	xge_ShapeEnd();
}

void xgeLine(int iX1, int iY1, int iX2, int iY2, unsigned int uColor)
{
	float arrVertices[] = {
		(float)iX1, (float)iY1,
		(float)iX2, (float)iY2
	};
	
	xge_ShapeBegin(uColor);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(arrVertices), arrVertices);
	glDrawArrays(GL_LINES, 0, 2);
	xge_ShapeEnd();
}

void xgeBox(int iX, int iY, int iW, int iH, unsigned int uColor)
{
	float fX1 = (float)iX;
	float fY1 = (float)iY;
	float fX2 = (float)(iX + iW);
	float fY2 = (float)(iY + iH);
	
	float arrVertices[] = {
		fX1, fY1,
		fX2, fY1,
		fX2, fY1,
		fX2, fY2,
		fX2, fY2,
		fX1, fY2,
		fX1, fY2,
		fX1, fY1
	};
	
	xge_ShapeBegin(uColor);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(arrVertices), arrVertices);
	glDrawArrays(GL_LINES, 0, 8);
	xge_ShapeEnd();
}

void xgeBoxFill(int iX, int iY, int iW, int iH, unsigned int uColor)
{
	float fX1 = (float)iX;
	float fY1 = (float)iY;
	float fX2 = (float)(iX + iW);
	float fY2 = (float)(iY + iH);
	
	float arrVertices[] = {
		fX1, fY1,
		fX2, fY1,
		fX1, fY2,
		fX2, fY2
	};
	
	xge_ShapeBegin(uColor);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(arrVertices), arrVertices);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	xge_ShapeEnd();
}

void xgeCircle(int iX, int iY, int iRadius, unsigned int uColor)
{
	const int iSegments = 36;
	float arrVertices[iSegments * 2 * 2];
	float fAngleStep = 2.0f * 3.14159265f / iSegments;
	int i;
	
	for ( i = 0; i < iSegments; i++ ) {
		float fAngle1 = i * fAngleStep;
		float fAngle2 = (i + 1) * fAngleStep;
		
		arrVertices[i * 4 + 0] = iX + cosf(fAngle1) * iRadius;
		arrVertices[i * 4 + 1] = iY + sinf(fAngle1) * iRadius;
		arrVertices[i * 4 + 2] = iX + cosf(fAngle2) * iRadius;
		arrVertices[i * 4 + 3] = iY + sinf(fAngle2) * iRadius;
	}
	
	xge_ShapeBegin(uColor);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(arrVertices), arrVertices);
	glDrawArrays(GL_LINES, 0, iSegments * 2);
	xge_ShapeEnd();
}

void xgeCircleFill(int iX, int iY, int iRadius, unsigned int uColor)
{
	const int iSegments = 36;
	float arrVertices[(iSegments + 2) * 2];
	float fAngleStep = 2.0f * 3.14159265f / iSegments;
	int i;
	
	/* 中心点 */
	arrVertices[0] = (float)iX;
	arrVertices[1] = (float)iY;
	
	/* 圆周点 */
	for ( i = 0; i <= iSegments; i++ ) {
		float fAngle = i * fAngleStep;
		arrVertices[(i + 1) * 2 + 0] = iX + cosf(fAngle) * iRadius;
		arrVertices[(i + 1) * 2 + 1] = iY + sinf(fAngle) * iRadius;
	}
	
	xge_ShapeBegin(uColor);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(arrVertices), arrVertices);
	glDrawArrays(GL_TRIANGLE_FAN, 0, iSegments + 2);
	xge_ShapeEnd();
}

void xgeTriangle(int iX1, int iY1, int iX2, int iY2, int iX3, int iY3, unsigned int uColor)
{
	float arrVertices[] = {
		(float)iX1, (float)iY1,
		(float)iX2, (float)iY2,
		(float)iX2, (float)iY2,
		(float)iX3, (float)iY3,
		(float)iX3, (float)iY3,
		(float)iX1, (float)iY1
	};
	
	xge_ShapeBegin(uColor);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(arrVertices), arrVertices);
	glDrawArrays(GL_LINES, 0, 6);
	xge_ShapeEnd();
}

void xgeTriangleFill(int iX1, int iY1, int iX2, int iY2, int iX3, int iY3, unsigned int uColor)
{
	float arrVertices[] = {
		(float)iX1, (float)iY1,
		(float)iX2, (float)iY2,
		(float)iX3, (float)iY3
	};
	
	xge_ShapeBegin(uColor);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(arrVertices), arrVertices);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	xge_ShapeEnd();
}

#endif /* XGE_SHAPE_H */
