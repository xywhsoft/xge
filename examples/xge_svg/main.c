#include "../../xge.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_W 1160
#define DEMO_H 400
#define SVG_RENDER_DEFAULT_W 512
#define SVG_RENDER_DEFAULT_H 512
#define SVG_RENDER_MAX_SIZE 8192

typedef struct xge_svg_demo_t {
	xge_render_target_t tTarget;
	xge_svg pMemorySvg;
	xge_svg pFileSvg;
	xge_texture_t tRasterSvg;
	char sCapturePath[260];
	char sRenderPath[260];
	char sBoundsPath[260];
	char sAspect[64];
	int bRasterLoaded;
	int bCaptureDone;
	int iFrame;
	int iMaxFrames;
	int iRenderWidth;
	int iRenderHeight;
	int iRenderRepeat;
	float fRenderTolerance;
	int iRenderResult;
} xge_svg_demo_t;

static const char g_sMemorySvg[] =
	"<svg viewBox=\"0,0,64,64\" fill=\"none\" stroke=\"#24364b\" stroke-width=\"2\">"
	"<style>.cssAccent{fill:#13579b;stroke:none}.cssCurrent{color:goldenrod;fill:currentColor;stroke:transparent}#idAccent{fill:#2468ac;stroke:none}polygon{fill:#579b13}path.cssCompound.hot{fill:#b51a62;stroke:none}.cssPair.extra{fill:#0f9675;stroke:none}stop.cssStop{stop-color:#19c37d;stop-opacity:100%}*.cssUniversal{fill:#c001d0;stroke:none}</style>"
	"<style>path.cssSpecific{fill:#22b8cf;stroke:none}.cssSpecific{fill:#111111;stroke:none}.cssAttr{fill:#7a5cff;stroke:none}</style>"
	"<style>.cssImportant{fill:#010101;stroke:none}.cssImportant{fill:#303132 !important}.cssImportant{fill:#ff0000}.cssLast{fill:#010101;fill:#363738;stroke:none}</style>"
	"<style>g.cssScope path.cssDesc{fill:#515253;stroke:none}g.cssScope>path.cssChild{fill:#545556;stroke:none}defs linearGradient>stop.cssScopedStop{stop-color:#575859;stop-opacity:100%}</style>"
	"<style>rect[probe]{fill:#5a5b5c;stroke:none}rect[data-eq=match]{fill:#5d5e5f;stroke:none}rect[data-word~=beta]{fill:#606162;stroke:none}rect[lang|=en]{fill:#636465;stroke:none}rect[data-prefix^=pre]{fill:#666768;stroke:none}rect[data-suffix$=end]{fill:#696a6b;stroke:none}rect[data-sub*=mid]{fill:#6c6d6e;stroke:none}</style>"
	"<style>.cssFontEm{font-size:4px;fill:#6f7071;stroke:none}.cssFontEx{font-size:6px;fill:#727374;stroke:none}</style>"
	"<style>:root{font-size:4px}.cssRootFont{fill:#757677;stroke:none}</style>"
	"<style>.cssNotProbe:not(.cssNotSkip){fill:#ff9f1c;stroke:none}.cssNotProbe.cssNotSkip{fill:#1d3557;stroke:none}.cssIsProbe:is(.cssIsMatch, .cssUnused){fill:#06d6a0;stroke:none}.cssWhereProbe:where(.cssWhereMatch, .cssUnused){fill:#ef476f;stroke:none}.cssSiblingPrev+.cssAdjacentProbe{fill:#118ab2;stroke:none}.cssSiblingBase~.cssGeneralProbe{fill:#fb8500;stroke:none}.cssFirstChildProbe:first-child{fill:#8338ec;stroke:none}.cssFirstTypeProbe:first-of-type{fill:#3a86ff;stroke:none}.cssNthChildProbe:nth-child(3){fill:#ff006e;stroke:none}.cssNthTypeProbe:nth-of-type(2n){fill:#ffbe0b;stroke:none}.cssLastChildProbe:last-child{fill:#7209b7;stroke:none}.cssLastTypeProbe:last-of-type{fill:#f72585;stroke:none}.cssOnlyChildProbe:only-child{fill:#4cc9f0;stroke:none}.cssOnlyTypeProbe:only-of-type{fill:#b5179e;stroke:none}.cssNthLastChildProbe:nth-last-child(2){fill:#4895ef;stroke:none}.cssNthLastTypeProbe:nth-last-of-type(even){fill:#f77f00;stroke:none}.cssEmptyProbe:empty{fill:#00a6fb;stroke:none}.cssHasDescProbe:has(path.cssHasNeedle){fill:#d81159;stroke:none}.cssHasChildProbe:has(>path.cssHasChildNeedle){fill:#8f2d56;stroke:none}.cssHasAdjacentProbe:has(+path.cssHasAdjacentNeedle){fill:#218380;stroke:none}</style>"
	"<style><![CDATA[/* cdata style */@font-face{font-family:ignore;src:url(ignore)}path.cssCData{fill:/*inline*/#e07124;stroke:none}]]></style>"
	"<defs><linearGradient id=\"cardGrad\" x1=\"0%\" y1=\"0%\" x2=\"100%\" y2=\"0%\" spreadMethod=\"reflect\" gradientTransform=\"translate(0.1 0) scale(0.8 1)\">"
	"<stop offset=\"0%\" stop-color=\"#22a06b\"/>"
	"<stop offset=\"100%\" stop-color=\"#2d7dd2\"/>"
	"</linearGradient></defs>"
	"<defs><radialGradient id=\"sunGrad\" cx=\"50%\" cy=\"45%\" r=\"55%\" spreadMethod=\"repeat\" gradientTransform=\"scale(0.95 0.95) translate(0.03 0.02)\">"
	"<stop offset=\"0%\" stop-color=\"#ffffff\"/>"
	"<stop offset=\"100%\" stop-color=\"#f04f64\"/>"
	"</radialGradient></defs>"
	"<defs><linearGradient id=\"cardGradAlias\" href=\"#lateSharedStops\" x1=\"0%\" y1=\"0%\" x2=\"100%\" y2=\"100%\"/>"
	"<linearGradient id=\"cardGradChain\" xlink:href=\"#cardGradAlias\" spreadMethod=\"repeat\"/>"
	"<linearGradient id=\"lateSharedStops\"><stop offset=\"0%\" stop-color=\"#24364b\"/><stop offset=\"100%\" stop-color=\"#ffb02e\"/></linearGradient></defs>"
	"<defs><linearGradient id=\"currentStopGrad\" color=\"#abcdef\"><stop offset=\"0\" stop-color=\"currentColor\"/><stop offset=\"1\" style=\"stop-color:currentColor\"/></linearGradient>"
	"<linearGradient id=\"cssStopGrad\"><stop class=\"cssStop\" offset=\"0\"/><stop class=\"cssStop\" offset=\"1\"/></linearGradient></defs>"
	"<defs><linearGradient id=\"scopedStopGrad\"><stop class=\"cssScopedStop\" offset=\"0\"/><stop class=\"cssScopedStop\" offset=\"1\"/></linearGradient></defs>"
	"<defs><clipPath id=\"sunCrop\" clipPathUnits=\"objectBoundingBox\"><g transform=\"translate(0.05 0)\"><rect x=\"0\" y=\"0\" width=\"0.55\" height=\"1\"/><rect x=\"0.35\" y=\"0\" width=\"0.55\" height=\"1\"/></g></clipPath></defs>"
	"<defs><clipPath id=\"rectCrop\"><rect x=\"4\" y=\"28\" width=\"18\" height=\"10\"/></clipPath></defs>"
	"<defs><clipPath id=\"boxCrop\" clipPathUnits=\"objectBoundingBox\"><rect x=\"0.2\" y=\"0\" width=\"0.6\" height=\"1\"/></clipPath></defs>"
	"<defs><clipPath id=\"multiRectCrop\"><rect x=\"1\" y=\"20\" width=\"4\" height=\"4\"/><rect x=\"9\" y=\"20\" width=\"4\" height=\"4\"/></clipPath></defs>"
	"<defs><clipPath id=\"overlapCrop\"><rect x=\"16\" y=\"20\" width=\"7\" height=\"4\"/><rect x=\"20\" y=\"20\" width=\"7\" height=\"4\"/></clipPath></defs>"
	"<defs><mask id=\"cardMask\" maskContentUnits=\"objectBoundingBox\"><rect x=\"0.08\" y=\"0\" width=\"0.55\" height=\"1\" fill=\"#909090\"/><rect x=\"0.38\" y=\"0\" width=\"0.55\" height=\"1\" fill=\"#c8c8c8\"/></mask></defs>"
	"<defs><mask id=\"lumaMask\" maskContentUnits=\"objectBoundingBox\"><rect x=\"0\" y=\"0\" width=\"1\" height=\"1\" fill=\"#ffffff\"/></mask></defs>"
	"<defs><pattern id=\"checkerPattern\" patternUnits=\"userSpaceOnUse\" width=\"4\" height=\"4\"><rect x=\"0\" y=\"0\" width=\"2\" height=\"2\" fill=\"#0f172a\" stroke=\"none\"/><rect x=\"2\" y=\"2\" width=\"2\" height=\"2\" fill=\"#38bdf8\" stroke=\"none\"/></pattern></defs>"
	"<defs><pattern id=\"mixedPattern\" patternUnits=\"userSpaceOnUse\" width=\"8\" height=\"8\"><rect x=\"0\" y=\"0\" width=\"8\" height=\"8\" fill=\"#f8fafc\" stroke=\"none\"/><text x=\"1\" y=\"6\" font-size=\"5\" fill=\"#ff31c7\" stroke=\"none\">T</text><image x=\"4\" y=\"1\" width=\"3\" height=\"3\" href=\"data:image/svg+xml,%3Csvg viewBox='0 0 4 4' xmlns='http://www.w3.org/2000/svg'%3E%3Crect width='4' height='4' fill='%23f97316'/%3E%3C/svg%3E\"/></pattern></defs>"
	"<defs><pattern id=\"hrefPattern\" href=\"#mixedPattern\" patternTransform=\"translate(1 1)\"/></defs>"
	"<defs><path id=\"sparkMark\" d=\"M0 -5 L2 0 L0 5 L-2 0 Z\" fill=\"#f04f64\" stroke=\"none\"/></defs>"
	"<defs><g id=\"miniPair\"><circle cx=\"0\" cy=\"0\" r=\"2\" fill=\"#2d7dd2\" stroke=\"none\"/><rect x=\"4\" y=\"-2\" width=\"4\" height=\"4\" fill=\"#22a06b\" stroke=\"none\"/></g></defs>"
	"<defs><rect id=\"entityBox\" width=\"4\" height=\"4\" fill=\"#222324\" stroke=\"none\"/></defs>"
	"<defs><symbol id=\"scaledSymbol\" viewBox=\"-1,-1,2,2\" preserveAspectRatio=\"none\"><rect x=\"-1\" y=\"-1\" width=\"2\" height=\"2\" fill=\"#cafe44\" stroke=\"none\"/></symbol></defs>"
	"<defs><marker id=\"arrowMarker\" viewBox=\"0 0 6 6\" refX=\"6\" refY=\"3\" markerWidth=\"2\" markerHeight=\"2\" orient=\"auto\"><path d=\"M0 0 L6 3 L0 6 Z\" fill=\"#838485\" stroke=\"none\"/></marker></defs>"
	"<defs><marker id=\"pathMarker\" viewBox=\"0 0 6 6\" refX=\"6\" refY=\"3\" markerWidth=\"2\" markerHeight=\"2\" orient=\"auto\"><path d=\"M0 0 L6 3 L0 6 Z\" fill=\"#898a8b\" stroke=\"none\"/></marker></defs>"
	"<defs><marker id=\"reverseMarker\" viewBox=\"0 0 6 6\" refX=\"6\" refY=\"3\" markerWidth=\"3\" markerHeight=\"3\" orient=\"auto-start-reverse\"><path d=\"M0 0 L6 3 L0 6 Z\" fill=\"#31ff31\" stroke=\"none\"/></marker></defs>"
	"<path id=\"textPathGuide\" d=\"M4 17 C14 5 28 5 38 17 S54 29 60 14\" fill=\"none\" stroke=\"#657282\" stroke-width=\"0.6\"/>"
	"<text font-size=\"5\" fill=\"#31ff7f\" stroke=\"none\" text-decoration=\"underline\"><textPath href=\"#textPathGuide\" startOffset=\"4%\" textLength=\"34\" lengthAdjust=\"spacing\">TEXT PATH</textPath></text>"
	"<path d=\"M8 40 C18 14 28 14 38 40 S54 58 58 28\" stroke=\"url(#cardGrad)\" stroke-width=\"4\" stroke-linecap=\"round\"/>"
	"<rect x=\"2\" y=\"26\" width=\"24\" height=\"14\" fill=\"tomato\" stroke=\"none\" clip-path=\"url(#rectCrop)\"/>"
	"<rect x=\"30\" y=\"40\" width=\"24\" height=\"10\" fill=\"royalblue\" stroke=\"none\" clip-path=\"url(#boxCrop)\"/>"
	"<rect x=\"25%\" y=\"2mm\" width=\"12.5%\" height=\"4mm\" fill=\"#22b8cf\" stroke=\"#7a5cff\" stroke-width=\"2%\" opacity=\"0.85\"/>"
	"<svg x=\"34\" y=\"4\" width=\"12\" height=\"8\" viewBox=\"0,0,120,80\" preserveAspectRatio=\"none\"><rect x=\"0\" y=\"0\" width=\"120\" height=\"80\" fill=\"#39ff88\" stroke=\"none\"/></svg>"
	"<g visibility=\"hidden\"><rect x=\"46\" y=\"4\" width=\"5\" height=\"5\" fill=\"#ff0001\" stroke=\"none\"/><rect x=\"46\" y=\"4\" width=\"5\" height=\"5\" fill=\"#aa33ff\" stroke=\"none\" visibility=\"visible\"/></g>"
	"<g display=\"none\"><rect x=\"52\" y=\"4\" width=\"5\" height=\"5\" fill=\"#12feef\" stroke=\"none\" visibility=\"visible\"/></g>"
	"<rect x=\"52\" y=\"2\" width=\"5\" height=\"3\" fill=\"rgb(100% 50% 0%)\" stroke=\"none\"/>"
	"<rect x=\"58\" y=\"2\" width=\"4\" height=\"3\" fill=\"rgba(18 52 86 / 100%)\" stroke=\"none\"/>"
	"<rect x=\"52\" y=\"6\" width=\"4\" height=\"2\" fill=\"#0f0f\" stroke=\"none\"/>"
	"<rect x=\"58\" y=\"6\" width=\"4\" height=\"2\" fill=\"#336699ff\" stroke=\"none\"/>"
	"<rect x=\"2\" y=\"2\" width=\"4\" height=\"3\" fill=\"hsl(210 100% 50%)\" stroke=\"none\"/>"
	"<rect x=\"7\" y=\"2\" width=\"4\" height=\"3\" fill=\"hsla(300, 100%, 50%, 1)\" stroke=\"none\"/>"
	"<rect x=\"12\" y=\"2\" width=\"4\" height=\"3\" fill=\"#fedcba\" opacity=\"100%\" stroke=\"none\"/>"
	"<rect x=\"17\" y=\"2\" width=\"4\" height=\"3\" fill=\"#55aa11\" fill-opacity=\"100%\" stroke=\"none\"/>"
	"<rect x=\"0\" y=\"68\" width=\"3\" height=\"3\" fill=\"#0d0bad\" transform=\"translate(60-12)\" stroke=\"none\"/>"
	"<rect x=\"1\" y=\"58\" width=\"4\" height=\"4\" fill=\"url(#currentStopGrad)\" stroke=\"none\"/>"
	"<rect x=\"6\" y=\"58\" width=\"4\" height=\"4\" fill=\"url(#cssStopGrad)\" stroke=\"none\"/>"
	"<rect x=\"12\" y=\"58\" width=\"10\" height=\"4\" ry=\"2\" fill=\"#bada55\" stroke=\"none\"/>"
	"<path d=\"M12 60 H30\" fill=\"none\" stroke=\"#fe00aa\" stroke-width=\"3\" stroke-linecap=\"butt\" stroke-dasharray=\"0 6\"/>"
	"<rect class=\"cssUniversal\" x=\"24\" y=\"58\" width=\"4\" height=\"4\"/>"
	"<rect x=\"29\" y=\"58\" width=\"4\" height=\"4\" fill=\"url(#missingPaint) #1f2021\" stroke=\"none\"/>"
	"<use href=\"&#35;entityBox\" x=\"34\" y=\"58\"/>"
	"<path d=\"M48 60 H62\" fill=\"none\" stroke=\"#232425\" stroke-width=\"2\" stroke-linecap=\"butt\" stroke-dasharray=\"3\"/>"
	"<rect x=\"2\" y=\"52\" width=\"8\" height=\"5\" fill=\"#454647\" stroke=\"#48494a\" stroke-width=\"3\" paint-order=\"markers stroke\"/>"
	"<rect x=\"12\" y=\"52\" width=\"8\" height=\"5\" fill=\"#4b4c4d\" stroke=\"#4e4f50\" stroke-width=\"3\" paint-order=\"normal\"/>"
	"<g class=\"cssScope\"><g><path class=\"cssDesc\" d=\"M52 52 L56 52 L56 56 Z\"/></g><path class=\"cssChild\" d=\"M57 52 L61 52 L61 56 Z\"/></g>"
	"<rect x=\"57\" y=\"57\" width=\"4\" height=\"4\" fill=\"url(#scopedStopGrad)\" stroke=\"none\"/>"
	"<rect x=\"1\" y=\"62\" width=\"3\" height=\"2\" probe=\"1\"/><rect x=\"5\" y=\"62\" width=\"3\" height=\"2\" data-eq=\"match\"/><rect x=\"9\" y=\"62\" width=\"3\" height=\"2\" data-word=\"alpha beta\"/><rect x=\"13\" y=\"62\" width=\"3\" height=\"2\" lang=\"en-US\"/><rect x=\"17\" y=\"62\" width=\"3\" height=\"2\" data-prefix=\"prefix\"/><rect x=\"21\" y=\"62\" width=\"3\" height=\"2\" data-suffix=\"the-end\"/><rect x=\"25\" y=\"62\" width=\"3\" height=\"2\" data-sub=\"in-the-middle\"/>"
	"<rect class=\"cssFontEm\" x=\"30\" y=\"62\" width=\"1em\" height=\"1ex\"/><rect class=\"cssFontEx\" x=\"35\" y=\"62\" width=\"1ex\" height=\"2\"/><rect class=\"cssRootFont\" x=\"39\" y=\"62\" width=\"1em\" height=\"1ex\"/>"
	"<rect x=\"10\" y=\"10\" width=\"16\" height=\"16\" rx=\"4\" fill=\"url(#cardGrad)\" stroke=\"none\" mask=\"url(#cardMask)\"/>"
	"<circle cx=\"42\" cy=\"20\" r=\"10\" fill=\"url(#sunGrad)\" stroke=\"none\" clip-path=\"url(#sunCrop)\"/>"
	"<polygon points=\"18,52 32,34 46,52\" fill=\"#ffb02e\" stroke=\"#24364b\" stroke-width=\"2\"/>"
	"<use href=\"#sparkMark\" x=\"54\" y=\"48\" transform=\"rotate(30 54 48)\"/>"
	"<use href=\"#miniPair\" x=\"12\" y=\"56\"/>"
	"<use href=\"#lateSymbol\" x=\"22\" y=\"56\" width=\"6\" height=\"4\"/>"
	"<use href=\"#latePair\" x=\"42\" y=\"56\"/>"
	"<use href=\"#scaledSymbol\" x=\"30\" y=\"56\" width=\"8\" height=\"4\"/>"
	"<path class=\"cssAccent\" d=\"M50 8 L58 8 L58 14 Z\"/>"
	"<path class=\"cssCurrent\" d=\"M50 28 L58 28 L58 34 Z\"/>"
	"<path class=\"cssCompound hot\" d=\"M50 38 L58 38 L58 44 Z\"/>"
	"<path class=\"cssPair extra\" d=\"M50 48 L58 48 L58 54 Z\"/>"
	"<path class=\"cssCData\" d=\"M6 26 L14 26 L14 32 Z\"/>"
	"<path class=\"cssSpecific\" d=\"M6 36 L14 36 L14 42 Z\"/>"
	"<path class=\"cssAttr\" fill=\"#111111\" d=\"M6 46 L14 46 L14 52 Z\"/>"
	"<rect x=\"52\" y=\"18\" width=\"6\" height=\"6\" fill=\"url(#cardGradChain)\" stroke=\"none\"/>"
	"<path id=\"idAccent\" d=\"M6 6 L14 6 L14 12 Z\"/>"
	"<polygon points=\"6,16 14,16 14,22\"/>"
	"<rect class=\"cssImportant\" x=\"38\" y=\"58\" width=\"4\" height=\"4\" style=\"fill:#ffffff\"/>"
	"<rect class=\"cssImportant\" x=\"43\" y=\"58\" width=\"4\" height=\"4\" style=\"fill:#333435 !important\"/>"
	"<rect class=\"cssLast\" x=\"48\" y=\"58\" width=\"4\" height=\"4\"/>"
	"<g transform=\"translate(38 50)\"><path d=\"M0 0 H6\" transform=\"scale(3)\" fill=\"none\" stroke=\"#3c3d3e\" stroke-width=\"1\"/><path d=\"M0 4 H6\" transform=\"scale(3)\" fill=\"none\" stroke=\"#404142\" stroke-width=\"1\" vector-effect=\"non-scaling-stroke\"/></g>"
	"<line x1=\"62\" y1=\"42\" x2=\"62\" y2=\"42\" stroke=\"#797a7b\" stroke-width=\"4\" stroke-linecap=\"round\"/>"
	"<line x1=\"62\" y1=\"49\" x2=\"62\" y2=\"49\" stroke=\"#7c7d7e\" stroke-width=\"4\" stroke-linecap=\"square\"/>"
	"<line x1=\"58\" y1=\"49\" x2=\"58\" y2=\"49\" stroke=\"#7f8081\" stroke-width=\"4\" stroke-linecap=\"round\" stroke-dasharray=\"4 2\"/>"
	"<line x1=\"2\" y1=\"33\" x2=\"22\" y2=\"33\" stroke=\"#fe12ca\" stroke-width=\"1\" stroke-dasharray=\"10 10\"/>"
	"<line x1=\"2\" y1=\"35\" x2=\"22\" y2=\"35\" stroke=\"#12cafe\" stroke-width=\"1\" stroke-dasharray=\"10 10\" pathLength=\"10\"/>"
	"<g stroke-dasharray=\"2 2\"><line x1=\"32\" y1=\"33\" x2=\"46\" y2=\"33\" stroke=\"#0b5dee\" stroke-width=\"1\" stroke-dasharray=\" NONE \"/></g>"
	"<line x1=\"2\" y1=\"49\" x2=\"16\" y2=\"49\" stroke=\"#858687\" stroke-width=\"2\" marker-end=\"url(#arrowMarker)\"/>"
	"<line x1=\"18\" y1=\"45\" x2=\"30\" y2=\"45\" stroke=\"#858687\" stroke-width=\"1\" marker-start=\"url(#reverseMarker)\"/>"
	"<path d=\"M18 49 C24 45 30 53 36 49 L44 49\" fill=\"none\" stroke=\"#8c8d8e\" stroke-width=\"1\" marker-start=\"url(#pathMarker)\" marker-mid=\"url(#pathMarker)\" marker-end=\"url(#pathMarker)\"/>"
	"<text x=\"2\" y=\"45\" font-size=\"5\" fill=\"#8f9091\" stroke=\"none\">SVG</text>"
	"<image id=\"embeddedImg\" x=\"54\" y=\"36\" width=\"5\" height=\"5\" href=\"data:image/svg+xml,%3Csvg%20viewBox%3D%270%200%2010%2010%27%20xmlns%3D%27http%3A%2F%2Fwww.w3.org%2F2000%2Fsvg%27%3E%3Crect%20width%3D%2710%27%20height%3D%2710%27%20fill%3D%27%23929394%27%2F%3E%3C%2Fsvg%3E\"/>"
	"<use href=\"#embeddedImg\" transform=\"translate(-8 0)\"/>"
	"<path id=\"useMan\" d=\"M53 44 L58 44 L58 47 Z\" fill=\"#9b9c9d\" stroke=\"none\"/><use href=\"#useMan\" transform=\"translate(-8 0)\"/>"
	"<path class=\"cssNotProbe cssNotSkip\" d=\"M4 44 L10 44 L10 50 Z\"/><path class=\"cssNotProbe\" d=\"M12 44 L18 44 L18 50 Z\"/><path class=\"cssIsProbe cssIsMatch\" d=\"M20 44 L26 44 L26 50 Z\"/><path class=\"cssWhereProbe cssWhereMatch\" d=\"M28 44 L34 44 L34 50 Z\"/><path class=\"cssSiblingPrev\" d=\"M36 44 L42 44 L42 50 Z\"/><path class=\"cssAdjacentProbe\" d=\"M44 44 L50 44 L50 50 Z\"/><path class=\"cssSiblingBase\" d=\"M36 52 L40 52 L40 56 Z\"/><path class=\"cssSiblingSkip\" d=\"M42 52 L46 52 L46 56 Z\"/><path class=\"cssGeneralProbe\" d=\"M48 52 L52 52 L52 56 Z\"/><g><path class=\"cssFirstChildProbe\" d=\"M54 44 L58 44 L58 50 Z\"/><path d=\"M60 44 L62 44 L62 50 Z\"/></g><g><rect x=\"54\" y=\"52\" width=\"2\" height=\"4\"/><path class=\"cssFirstTypeProbe\" d=\"M57 52 L59 52 L59 56 Z\"/></g><g><path d=\"M2 50 L4 50 L4 54 Z\"/><path d=\"M5 50 L7 50 L7 54 Z\"/><path class=\"cssNthChildProbe\" d=\"M8 50 L10 50 L10 54 Z\"/></g><g><rect x=\"12\" y=\"50\" width=\"2\" height=\"4\"/><path d=\"M15 50 L17 50 L17 54 Z\"/><path class=\"cssNthTypeProbe\" d=\"M18 50 L20 50 L20 54 Z\"/></g>"
	"<text id=\"textUse\" x=\"2\" y=\"24\" font-size=\"5\" fill=\"#a102f3\" stroke=\"none\">Q</text><use href=\"&nbsp;#textUse&nbsp;\" transform=\"translate(10 0)\"/>"
	"<defs><symbol id=\"lateSymbol\" viewBox=\"0,0,12,8\" preserveAspectRatio=\"none\"><rect x=\"0\" y=\"0\" width=\"12\" height=\"8\" fill=\"#ff66aa\" stroke=\"none\"/></symbol></defs>"
	"<defs><g id=\"latePair\"><circle cx=\"0\" cy=\"0\" r=\"2\" fill=\"#0badf0\" stroke=\"none\"/><rect x=\"5\" y=\"-2\" width=\"4\" height=\"4\" fill=\"#0fdbad\" stroke=\"none\"/></g></defs>"
	"<text x=\"38\" y=\"30\" font-size=\"5\" fill=\"#ff31c7\" stroke=\"none\" text-anchor=\"middle\">M</text>"
	"<text x=\"50\" y=\"30\" font-size=\"5\" fill=\"#ff31ff\" stroke=\"none\" text-anchor=\"middle\"><tspan>A</tspan><tspan dx=\"4\">B</tspan></text>"
	"<text x=\"38\" y=\"36\" dx=\"3\" dy=\"2\" font-size=\"5\" fill=\"#31ffc7\" stroke=\"none\">D</text>"
	"<text x=\"2\" y=\"30\" font-size=\"5\" fill=\"#8f9091\" stroke=\"none\"><tspan fill=\"#02a7f0\">T</tspan><tspan dx=\"4\" fill=\"#f07818\">S</tspan></text>"
	"<text x=\"2\" y=\"36\" font-size=\"5\" fill=\"#8f9091\" stroke=\"none\"><tspan>N<tspan fill=\"#c731ff\">E</tspan>S</tspan></text>"
	"<text x=\"44\" y=\"22\" font-size=\"5\" fill=\"#ff7f31\" stroke=\"none\" transform=\"rotate(-32 44 22)\">ROT</text>"
	"<rect x=\"30\" y=\"4\" width=\"28\" height=\"10\" fill=\"#eef2f7\" stroke=\"none\"/>"
	"<text x=\"30\" y=\"12\" font-size=\"7\" fill=\"#ff0055\" stroke=\"none\" letter-spacing=\"1.2\" word-spacing=\"2.4\">A B</text>"
	"<rect x=\"24\" y=\"14\" width=\"38\" height=\"6\" fill=\"#ffffff\" stroke=\"none\"/>"
	"<text x=\"25\" y=\"19\" font-size=\"4\" fill=\"#0047ff\" stroke=\"none\" textLength=\"18\" lengthAdjust=\"spacing\">ABC</text>"
	"<text x=\"44\" y=\"19\" font-size=\"4\" fill=\"#ff0047\" stroke=\"none\" textLength=\"12\" lengthAdjust=\"spacingAndGlyphs\">WIDE</text>"
	"<text x=\"28\" y=\"43\" font-size=\"5\" fill=\"#ee1199\" stroke=\"none\">   W</text>"
	"<text x=\"28\" y=\"37\" font-size=\"5\" fill=\"#11ee99\" stroke=\"none\" xml:space=\"preserve\">   W</text>"
	"<rect x=\"32\" y=\"36\" width=\"30\" height=\"12\" fill=\"#ffffff\" stroke=\"none\"/>"
	"<text x=\"34\" y=\"42\" font-size=\"5\" fill=\"#0aa0ff\" stroke=\"none\" dominant-baseline=\"text-before-edge\">T</text>"
	"<text x=\"40\" y=\"42\" font-size=\"5\" fill=\"#ff6d00\" stroke=\"none\" dominant-baseline=\"middle\">M</text>"
	"<text x=\"47\" y=\"42\" font-size=\"5\" fill=\"#2a9d8f\" stroke=\"none\" alignment-baseline=\"text-after-edge\">B</text>"
	"<text x=\"52\" y=\"46\" font-size=\"4\" fill=\"#5f27cd\" stroke=\"none\" text-decoration=\"underline overline line-through\">D</text>"
	"<g><path class=\"cssFirstChildProbe\" d=\"M2 55 L5 55 L5 60 Z\"/><path d=\"M6 55 L8 55 L8 60 Z\"/></g>"
	"<g><rect x=\"10\" y=\"55\" width=\"2\" height=\"5\"/><path class=\"cssFirstTypeProbe\" d=\"M13 55 L16 55 L16 60 Z\"/></g>"
	"<g><path d=\"M18 55 L20 55 L20 60 Z\"/><path d=\"M21 55 L23 55 L23 60 Z\"/><path class=\"cssNthChildProbe\" d=\"M24 55 L27 55 L27 60 Z\"/></g>"
	"<g><rect x=\"29\" y=\"55\" width=\"2\" height=\"5\"/><path d=\"M32 55 L34 55 L34 60 Z\"/><path class=\"cssNthTypeProbe\" d=\"M36 55 L39 55 L39 60 Z\"/></g>"
	"<g><path d=\"M41 55 L43 55 L43 60 Z\"/><path class=\"cssLastChildProbe\" d=\"M44 55 L47 55 L47 60 Z\"/></g>"
	"<g><path class=\"cssLastTypeProbe\" d=\"M49 55 L52 55 L52 60 Z\"/><rect x=\"53\" y=\"55\" width=\"2\" height=\"5\"/></g>"
	"<g><path class=\"cssOnlyChildProbe\" d=\"M57 55 L60 55 L60 60 Z\"/></g>"
	"<g><rect x=\"2\" y=\"49\" width=\"2\" height=\"4\"/><path class=\"cssOnlyTypeProbe\" d=\"M5 49 L8 49 L8 53 Z\"/></g>"
	"<g><path d=\"M10 49 L12 49 L12 53 Z\"/><path class=\"cssNthLastChildProbe\" d=\"M13 49 L16 49 L16 53 Z\"/><path d=\"M17 49 L19 49 L19 53 Z\"/></g>"
	"<g><path d=\"M21 49 L23 49 L23 53 Z\"/><path class=\"cssNthLastTypeProbe\" d=\"M24 49 L27 49 L27 53 Z\"/><path d=\"M28 49 L30 49 L30 53 Z\"/></g>"
	"<path class=\"cssEmptyProbe\" d=\"M31 49 L34 49 L34 53 Z\"/>"
	"<g class=\"cssHasDescProbe\"><g><path class=\"cssHasNeedle\" d=\"M35 49 L38 49 L38 53 Z\"/></g></g>"
	"<g class=\"cssHasChildProbe\"><path class=\"cssHasChildNeedle\" d=\"M40 49 L43 49 L43 53 Z\"/></g>"
	"<path class=\"cssHasAdjacentProbe\" d=\"M45 49 L48 49 L48 53 Z\"/><path class=\"cssHasAdjacentNeedle\" d=\"M49 49 L51 49 L51 53 Z\" fill=\"#111111\" stroke=\"none\"/>"
	"<rect x=\"1\" y=\"20\" width=\"12\" height=\"4\" fill=\"#00e5a8\" stroke=\"none\" clip-path=\"url(#multiRectCrop)\"/>"
	"<rect x=\"16\" y=\"20\" width=\"11\" height=\"4\" fill=\"#ffffff\" stroke=\"none\"/>"
	"<rect x=\"16\" y=\"20\" width=\"11\" height=\"4\" fill=\"#ff0000\" fill-opacity=\"0.5\" stroke=\"none\" clip-path=\"url(#overlapCrop)\"/>"
	"<rect x=\"28\" y=\"20\" width=\"5\" height=\"4\" fill=\"#12f0f0\" stroke=\"none\" style=\"opacity:nan;fill-opacity:inf;stroke-opacity:infinity;stroke-width:nan\" transform=\"translate(nan 0)\"/>"
	"<rect x=\"56\" y=\"32\" width=\"5\" height=\"4\" fill=\"#f012e5\" stroke=\"none\" mask=\"url(#lumaMask)\"/>"
	"<rect x=\"34\" y=\"24\" width=\"12\" height=\"8\" fill=\"url(#checkerPattern)\" stroke=\"#24364b\" stroke-width=\"0.8\"/>"
	"<rect x=\"48\" y=\"24\" width=\"12\" height=\"8\" fill=\"url(#mixedPattern)\" stroke=\"#24364b\" stroke-width=\"0.8\"/>"
	"<rect x=\"48\" y=\"34\" width=\"12\" height=\"6\" fill=\"url(#hrefPattern)\" stroke=\"#24364b\" stroke-width=\"0.8\"/>"
	"</svg>";

static void __xgeSvgDemoUsage(void)
{
	printf("usage: xge_svg [--frames N] [--capture PATH]\n");
	printf("       xge_svg --render PATH --capture PATH [--width N] [--height N] [--aspect VALUE] [--repeat N] [--tolerance F]\n");
	printf("       xge_svg --bounds PATH [--width N] [--height N] [--aspect VALUE]\n");
	printf("       no duration option means run until the window is closed.\n");
}

static int __xgeSvgDemoParseArgs(xge_svg_demo_t* pDemo, int argc, char** argv)
{
	int i;

	for ( i = 1; i < argc; i++ ) {
		if ( strcmp(argv[i], "--frames") == 0 ) {
			if ( i + 1 >= argc ) return XGE_ERROR_INVALID_ARGUMENT;
			pDemo->iMaxFrames = atoi(argv[++i]);
			if ( pDemo->iMaxFrames <= 0 ) return XGE_ERROR_INVALID_ARGUMENT;
		} else if ( strncmp(argv[i], "--frames=", 9) == 0 ) {
			pDemo->iMaxFrames = atoi(argv[i] + 9);
			if ( pDemo->iMaxFrames <= 0 ) return XGE_ERROR_INVALID_ARGUMENT;
		} else if ( strcmp(argv[i], "--capture") == 0 ) {
			if ( i + 1 >= argc ) return XGE_ERROR_INVALID_ARGUMENT;
			snprintf(pDemo->sCapturePath, sizeof(pDemo->sCapturePath), "%s", argv[++i]);
			pDemo->sCapturePath[sizeof(pDemo->sCapturePath) - 1] = '\0';
		} else if ( strncmp(argv[i], "--capture=", 10) == 0 ) {
			snprintf(pDemo->sCapturePath, sizeof(pDemo->sCapturePath), "%s", argv[i] + 10);
			pDemo->sCapturePath[sizeof(pDemo->sCapturePath) - 1] = '\0';
		} else if ( strcmp(argv[i], "--output") == 0 ) {
			if ( i + 1 >= argc ) return XGE_ERROR_INVALID_ARGUMENT;
			snprintf(pDemo->sCapturePath, sizeof(pDemo->sCapturePath), "%s", argv[++i]);
			pDemo->sCapturePath[sizeof(pDemo->sCapturePath) - 1] = '\0';
		} else if ( strncmp(argv[i], "--output=", 9) == 0 ) {
			snprintf(pDemo->sCapturePath, sizeof(pDemo->sCapturePath), "%s", argv[i] + 9);
			pDemo->sCapturePath[sizeof(pDemo->sCapturePath) - 1] = '\0';
		} else if ( strcmp(argv[i], "--render") == 0 ) {
			if ( i + 1 >= argc ) return XGE_ERROR_INVALID_ARGUMENT;
			snprintf(pDemo->sRenderPath, sizeof(pDemo->sRenderPath), "%s", argv[++i]);
			pDemo->sRenderPath[sizeof(pDemo->sRenderPath) - 1] = '\0';
		} else if ( strncmp(argv[i], "--render=", 9) == 0 ) {
			snprintf(pDemo->sRenderPath, sizeof(pDemo->sRenderPath), "%s", argv[i] + 9);
			pDemo->sRenderPath[sizeof(pDemo->sRenderPath) - 1] = '\0';
		} else if ( strcmp(argv[i], "--bounds") == 0 ) {
			if ( i + 1 >= argc ) return XGE_ERROR_INVALID_ARGUMENT;
			snprintf(pDemo->sBoundsPath, sizeof(pDemo->sBoundsPath), "%s", argv[++i]);
			pDemo->sBoundsPath[sizeof(pDemo->sBoundsPath) - 1] = '\0';
		} else if ( strncmp(argv[i], "--bounds=", 9) == 0 ) {
			snprintf(pDemo->sBoundsPath, sizeof(pDemo->sBoundsPath), "%s", argv[i] + 9);
			pDemo->sBoundsPath[sizeof(pDemo->sBoundsPath) - 1] = '\0';
		} else if ( strcmp(argv[i], "--width") == 0 ) {
			if ( i + 1 >= argc ) return XGE_ERROR_INVALID_ARGUMENT;
			pDemo->iRenderWidth = atoi(argv[++i]);
		} else if ( strncmp(argv[i], "--width=", 8) == 0 ) {
			pDemo->iRenderWidth = atoi(argv[i] + 8);
		} else if ( strcmp(argv[i], "--height") == 0 ) {
			if ( i + 1 >= argc ) return XGE_ERROR_INVALID_ARGUMENT;
			pDemo->iRenderHeight = atoi(argv[++i]);
		} else if ( strncmp(argv[i], "--height=", 9) == 0 ) {
			pDemo->iRenderHeight = atoi(argv[i] + 9);
		} else if ( strcmp(argv[i], "--repeat") == 0 ) {
			if ( i + 1 >= argc ) return XGE_ERROR_INVALID_ARGUMENT;
			pDemo->iRenderRepeat = atoi(argv[++i]);
			if ( pDemo->iRenderRepeat <= 0 ) return XGE_ERROR_INVALID_ARGUMENT;
		} else if ( strncmp(argv[i], "--repeat=", 9) == 0 ) {
			pDemo->iRenderRepeat = atoi(argv[i] + 9);
			if ( pDemo->iRenderRepeat <= 0 ) return XGE_ERROR_INVALID_ARGUMENT;
		} else if ( strcmp(argv[i], "--tolerance") == 0 ) {
			if ( i + 1 >= argc ) return XGE_ERROR_INVALID_ARGUMENT;
			pDemo->fRenderTolerance = strtof(argv[++i], NULL);
			if ( !isfinite(pDemo->fRenderTolerance) || (pDemo->fRenderTolerance <= 0.0f) ) return XGE_ERROR_INVALID_ARGUMENT;
		} else if ( strncmp(argv[i], "--tolerance=", 12) == 0 ) {
			pDemo->fRenderTolerance = strtof(argv[i] + 12, NULL);
			if ( !isfinite(pDemo->fRenderTolerance) || (pDemo->fRenderTolerance <= 0.0f) ) return XGE_ERROR_INVALID_ARGUMENT;
		} else if ( strcmp(argv[i], "--aspect") == 0 || strcmp(argv[i], "--preserveAspectRatio") == 0 ) {
			if ( i + 1 >= argc ) return XGE_ERROR_INVALID_ARGUMENT;
			snprintf(pDemo->sAspect, sizeof(pDemo->sAspect), "%s", argv[++i]);
			pDemo->sAspect[sizeof(pDemo->sAspect) - 1] = '\0';
		} else if ( strncmp(argv[i], "--aspect=", 9) == 0 ) {
			snprintf(pDemo->sAspect, sizeof(pDemo->sAspect), "%s", argv[i] + 9);
			pDemo->sAspect[sizeof(pDemo->sAspect) - 1] = '\0';
		} else if ( strncmp(argv[i], "--preserveAspectRatio=", 22) == 0 ) {
			snprintf(pDemo->sAspect, sizeof(pDemo->sAspect), "%s", argv[i] + 22);
			pDemo->sAspect[sizeof(pDemo->sAspect) - 1] = '\0';
		} else if ( (strcmp(argv[i], "--help") == 0) || (strcmp(argv[i], "-h") == 0) ) {
			__xgeSvgDemoUsage();
			return 1;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	if ( (pDemo->sRenderPath[0] != '\0') && (pDemo->sBoundsPath[0] != '\0') ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pDemo->sRenderPath[0] != '\0' ) {
		if ( pDemo->sCapturePath[0] == '\0' ) return XGE_ERROR_INVALID_ARGUMENT;
		if ( pDemo->iRenderRepeat == 0 ) pDemo->iRenderRepeat = 1;
		if ( pDemo->iRenderWidth == 0 ) pDemo->iRenderWidth = SVG_RENDER_DEFAULT_W;
		if ( pDemo->iRenderHeight == 0 ) pDemo->iRenderHeight = SVG_RENDER_DEFAULT_H;
		if ( (pDemo->iRenderWidth <= 0) || (pDemo->iRenderHeight <= 0) ||
			(pDemo->iRenderWidth > SVG_RENDER_MAX_SIZE) || (pDemo->iRenderHeight > SVG_RENDER_MAX_SIZE) ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	if ( pDemo->sBoundsPath[0] != '\0' ) {
		if ( pDemo->iRenderWidth == 0 ) pDemo->iRenderWidth = SVG_RENDER_DEFAULT_W;
		if ( pDemo->iRenderHeight == 0 ) pDemo->iRenderHeight = SVG_RENDER_DEFAULT_H;
		if ( (pDemo->iRenderWidth <= 0) || (pDemo->iRenderHeight <= 0) ||
			(pDemo->iRenderWidth > SVG_RENDER_MAX_SIZE) || (pDemo->iRenderHeight > SVG_RENDER_MAX_SIZE) ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static int __xgeSvgDemoPrintBounds(xge_svg_demo_t* pDemo)
{
	xge_svg pSvg;
	xge_rect_t tLocal;
	xge_rect_t tDraw;
	xge_rect_t tDst;
	int iRet;

	if ( (pDemo == NULL) || (pDemo->sBoundsPath[0] == '\0') ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pSvg = NULL;
	iRet = xgeSvgCreate(&pSvg);
	if ( iRet == XGE_OK ) {
		iRet = xgeSvgLoad(pSvg, pDemo->sBoundsPath);
	}
	if ( (iRet == XGE_OK) && (pDemo->sAspect[0] != '\0') ) {
		iRet = xgeSvgSetPreserveAspectRatio(pSvg, pDemo->sAspect);
	}
	if ( iRet == XGE_OK ) {
		iRet = xgeSvgGetBounds(pSvg, 0.05f, &tLocal);
	}
	if ( iRet == XGE_OK ) {
		tDst.fX = 0.0f;
		tDst.fY = 0.0f;
		tDst.fW = (float)pDemo->iRenderWidth;
		tDst.fH = (float)pDemo->iRenderHeight;
		iRet = xgeSvgGetDrawBounds(pSvg, tDst, 0.05f, &tDraw);
	}
	if ( iRet == XGE_OK ) {
		printf("xge_svg bounds: %s\n", pDemo->sBoundsPath);
		printf("local %.6f %.6f %.6f %.6f\n", tLocal.fX, tLocal.fY, tLocal.fW, tLocal.fH);
		printf("draw %.6f %.6f %.6f %.6f (%dx%d)\n",
			tDraw.fX,
			tDraw.fY,
			tDraw.fW,
			tDraw.fH,
			pDemo->iRenderWidth,
			pDemo->iRenderHeight);
	} else {
		printf("xge_svg bounds failed: %d (%s)\n", iRet, pDemo->sBoundsPath);
	}
	xgeSvgDestroy(pSvg);
	xgeSvgCacheClear();
	return iRet;
}

static void __xgeSvgDemoPixelsToStraightAlpha(unsigned char* pPixels, int iWidth, int iHeight, int iStride)
{
	int x;
	int y;

	if ( (pPixels == NULL) || (iWidth <= 0) || (iHeight <= 0) || (iStride < (iWidth * 4)) ) return;
	for ( y = 0; y < iHeight; y++ ) {
		unsigned char* pRow = pPixels + (y * iStride);

		for ( x = 0; x < iWidth; x++ ) {
			unsigned char* pPixel = pRow + (x * 4);
			unsigned int iAlpha = pPixel[3];
			unsigned int iValue;

			if ( iAlpha == 0u ) {
				pPixel[0] = 0;
				pPixel[1] = 0;
				pPixel[2] = 0;
			} else if ( iAlpha < 255u ) {
				iValue = ((unsigned int)pPixel[0] * 255u + (iAlpha / 2u)) / iAlpha;
				pPixel[0] = (unsigned char)(iValue > 255u ? 255u : iValue);
				iValue = ((unsigned int)pPixel[1] * 255u + (iAlpha / 2u)) / iAlpha;
				pPixel[1] = (unsigned char)(iValue > 255u ? 255u : iValue);
				iValue = ((unsigned int)pPixel[2] * 255u + (iAlpha / 2u)) / iAlpha;
				pPixel[2] = (unsigned char)(iValue > 255u ? 255u : iValue);
			}
		}
	}
}

static int __xgeSvgDemoRenderCapture(xge_svg_demo_t* pDemo)
{
	xge_svg pSvg;
	xge_render_target_t tTarget;
	xge_pass_t tPass;
	xge_rect_t tDst;
	unsigned char* pPixels;
	double fRenderStart;
	double fRenderSeconds;
	int iStride;
	int iRepeat;
	int iRet;

	if ( (pDemo == NULL) || (pDemo->sRenderPath[0] == '\0') || (pDemo->sCapturePath[0] == '\0') ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pDemo->iRenderWidth > (INT32_MAX / 4) || pDemo->iRenderHeight > (INT32_MAX / (pDemo->iRenderWidth * 4)) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	iStride = pDemo->iRenderWidth * 4;
	pPixels = (unsigned char*)malloc((size_t)iStride * (size_t)pDemo->iRenderHeight);
	if ( pPixels == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pSvg = NULL;
	memset(&tTarget, 0, sizeof(tTarget));
	memset(&tPass, 0, sizeof(tPass));
	iRet = xgeSvgCreate(&pSvg);
	if ( iRet == XGE_OK ) iRet = xgeSvgLoad(pSvg, pDemo->sRenderPath);
	if ( (iRet == XGE_OK) && (pDemo->sAspect[0] != '\0') ) {
		iRet = xgeSvgSetPreserveAspectRatio(pSvg, pDemo->sAspect);
	}
	if ( iRet == XGE_OK ) {
		iRet = xgeRenderTargetCreate(&tTarget, pDemo->iRenderWidth, pDemo->iRenderHeight);
	}
	tDst.fX = 0.0f;
	tDst.fY = 0.0f;
	tDst.fW = (float)pDemo->iRenderWidth;
	tDst.fH = (float)pDemo->iRenderHeight;
	fRenderStart = xgeTimer();
	for ( iRepeat = 0; (iRet == XGE_OK) && (iRepeat < pDemo->iRenderRepeat); iRepeat++ ) {
		xgePassInit(&tPass, &tTarget, XGE_PASS_CLEAR_COLOR, XGE_COLOR_RGBA(0, 0, 0, 0));
		iRet = xgePassBegin(&tPass);
		if ( iRet == XGE_OK ) iRet = xgeSvgDrawPx(pSvg, tDst, pDemo->fRenderTolerance);
		if ( tPass.bActive ) {
			int iEndRet = xgePassEnd(&tPass);
			if ( iRet == XGE_OK ) iRet = iEndRet;
		}
	}
	fRenderSeconds = xgeTimer() - fRenderStart;
	if ( (iRet == XGE_OK) && (pDemo->iRenderRepeat > 1) ) {
		printf(
			"xge_svg render-benchmark repeats=%d tolerance=%.3f total_ms=%.3f average_ms=%.3f\n",
			pDemo->iRenderRepeat, pDemo->fRenderTolerance,
			fRenderSeconds * 1000.0, fRenderSeconds * 1000.0 / (double)pDemo->iRenderRepeat
		);
	}
	if ( iRet == XGE_OK ) iRet = xgeRenderTargetReadPixels(&tTarget, pPixels, iStride);
	xgeRenderTargetFree(&tTarget);
	xgeSvgDestroy(pSvg);
	if ( iRet == XGE_OK ) {
		__xgeSvgDemoPixelsToStraightAlpha(pPixels, pDemo->iRenderWidth, pDemo->iRenderHeight, iStride);
		iRet = xgeImageSavePNG(pDemo->sCapturePath, pDemo->iRenderWidth, pDemo->iRenderHeight, pPixels, iStride);
	}
	free(pPixels);
	if ( iRet == XGE_OK ) {
		printf("xge_svg render saved: %s (%dx%d from %s)\n",
			pDemo->sCapturePath,
			pDemo->iRenderWidth,
			pDemo->iRenderHeight,
			pDemo->sRenderPath);
	}
	return iRet;
}

static int __xgeSvgDemoRenderFrame(void* pUser)
{
	xge_svg_demo_t* pDemo;

	pDemo = (xge_svg_demo_t*)pUser;
	if ( pDemo == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	pDemo->iRenderResult = __xgeSvgDemoRenderCapture(pDemo);
	if ( pDemo->iRenderResult != XGE_OK ) {
		printf("xge_svg render failed: %d\n", pDemo->iRenderResult);
	}
	xgeQuit();
	return pDemo->iRenderResult;
}

static int __xgeSvgDemoCapture(xge_svg_demo_t* pDemo)
{
	unsigned char* pPixels;
	int iStride;
	int iRet;

	if ( (pDemo == NULL) || (pDemo->sCapturePath[0] == '\0') || (pDemo->bCaptureDone != 0) ) {
		return XGE_OK;
	}
	iStride = DEMO_W * 4;
	pPixels = (unsigned char*)malloc((size_t)iStride * DEMO_H);
	if ( pPixels == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	iRet = xgeRenderTargetReadPixels(&pDemo->tTarget, pPixels, iStride);
	if ( iRet == XGE_OK ) {
		__xgeSvgDemoPixelsToStraightAlpha(pPixels, DEMO_W, DEMO_H, iStride);
		iRet = xgeImageSavePNG(pDemo->sCapturePath, DEMO_W, DEMO_H, pPixels, iStride);
	}
	free(pPixels);
	if ( iRet == XGE_OK ) {
		pDemo->bCaptureDone = 1;
		printf("xge_svg capture saved: %s\n", pDemo->sCapturePath);
	}
	return iRet;
}

static xge_rect_t __xgeSvgDemoRect(float fX, float fY, float fW, float fH)
{
	xge_rect_t tRect;

	tRect.fX = fX;
	tRect.fY = fY;
	tRect.fW = fW;
	tRect.fH = fH;
	return tRect;
}

static void __xgeSvgDemoPanel(xge_rect_t tPanel)
{
	xgeShapeRectFillPx(tPanel, XGE_COLOR_RGBA(247, 250, 253, 255));
	xgeShapeRectStrokePx(tPanel, 1.0f, XGE_COLOR_RGBA(189, 200, 212, 255));
}

static int __xgeSvgDemoLoadCachedAsset(const char* sPath, const char* sBuildPath, xge_svg* ppSvg)
{
	int iRet;

	iRet = xgeSvgLoadCached(sPath, ppSvg);
	if ( iRet == XGE_OK ) {
		return XGE_OK;
	}
	return xgeSvgLoadCached(sBuildPath, ppSvg);
}

static int __xgeSvgDemoLoadRasterTexture(xge_texture pTexture, const char* sPath, const char* sBuildPath, int iWidth, int iHeight)
{
	int iRet;

	iRet = xgeSvgTextureLoad(pTexture, sPath, iWidth, iHeight);
	if ( iRet == XGE_OK ) {
		return XGE_OK;
	}
	xgeTextureFree(pTexture);
	memset(pTexture, 0, sizeof(*pTexture));
	return xgeSvgTextureLoad(pTexture, sBuildPath, iWidth, iHeight);
}

static const char* __xgeSvgDemoFindTtf(void)
{
	static const char* arrPaths[] = {
		"C:\\Windows\\Fonts\\segoeui.ttf",
		"C:\\Windows\\Fonts\\arial.ttf",
		"C:\\Windows\\Fonts\\calibri.ttf",
		"C:\\Windows\\Fonts\\msyh.ttc",
		"C:\\Windows\\Fonts\\simhei.ttf"
	};
	FILE* pFile;
	int i;

	for ( i = 0; i < (int)(sizeof(arrPaths) / sizeof(arrPaths[0])); i++ ) {
		pFile = fopen(arrPaths[i], "rb");
		if ( pFile != NULL ) {
			fclose(pFile);
			return arrPaths[i];
		}
	}
	return NULL;
}

static int __xgeSvgDemoFrame(void* pUser)
{
	xge_svg_demo_t* pDemo;
	xge_rect_t tPanelA;
	xge_rect_t tPanelB;
	xge_rect_t tPanelC;
	xge_rect_t tSvgA;
	xge_rect_t tSvgB;
	xge_rect_t tSvgC;
	xge_draw_t tDraw;
	xge_pass_t tPass;
	xge_texture pTargetTexture;
	int iRet;

	pDemo = (xge_svg_demo_t*)pUser;
	if ( pDemo == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	if ( xgeKeyPressed(XGE_KEY_ESCAPE) ) {
		xgeQuit();
	}
	if ( pDemo->bRasterLoaded == 0 ) {
		iRet = __xgeSvgDemoLoadRasterTexture(&pDemo->tRasterSvg, "examples\\xge_svg\\assets\\compat.svg", "..\\examples\\xge_svg\\assets\\compat.svg", 320, 240);
		if ( iRet != XGE_OK ) {
			printf("xge_svg failed to load svg raster texture: %d\n", iRet);
			return iRet;
		}
		pDemo->bRasterLoaded = 1;
	}
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) return iRet;
	xgePassInit(&tPass, &pDemo->tTarget, XGE_PASS_CLEAR_COLOR, XGE_COLOR_RGBA(22, 26, 32, 255));
	iRet = xgePassBegin(&tPass);
	if ( iRet != XGE_OK ) return iRet;
	xgeClear(XGE_COLOR_RGBA(22, 26, 32, 255));

	tPanelA = __xgeSvgDemoRect(32.0f, 48.0f, 332.0f, 304.0f);
	tPanelB = __xgeSvgDemoRect(414.0f, 48.0f, 332.0f, 304.0f);
	tPanelC = __xgeSvgDemoRect(796.0f, 48.0f, 332.0f, 304.0f);
	tSvgA = __xgeSvgDemoRect(70.0f, 80.0f, 256.0f, 240.0f);
	tSvgB = __xgeSvgDemoRect(452.0f, 80.0f, 256.0f, 240.0f);
	tSvgC = __xgeSvgDemoRect(834.0f, 80.0f, 256.0f, 240.0f);
	__xgeSvgDemoPanel(tPanelA);
	__xgeSvgDemoPanel(tPanelB);
	__xgeSvgDemoPanel(tPanelC);

	(void)xgeSvgDrawPx(pDemo->pMemorySvg, tSvgA, 0.75f);
	(void)xgeSvgDrawPx(pDemo->pFileSvg, tSvgB, 0.75f);
	memset(&tDraw, 0, sizeof(tDraw));
	tDraw.pTexture = &pDemo->tRasterSvg;
	tDraw.tDst = tSvgC;
	tDraw.iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	tDraw.iFlags = XGE_DRAW_SCREEN_SPACE;
	xgeDrawEx(&tDraw);

	iRet = xgePassEnd(&tPass);
	if ( iRet != XGE_OK ) return iRet;
	iRet = __xgeSvgDemoCapture(pDemo);
	if ( iRet != XGE_OK ) return iRet;
	xgeClear(XGE_COLOR_RGBA(10, 13, 17, 255));
	memset(&tDraw, 0, sizeof(tDraw));
	pTargetTexture = xgeRenderTargetTexture(&pDemo->tTarget);
	tDraw.pTexture = pTargetTexture;
	tDraw.tSrc = __xgeSvgDemoRect(0.0f, 0.0f, (float)DEMO_W, (float)DEMO_H);
	tDraw.tDst = __xgeSvgDemoRect(0.0f, 0.0f, (float)DEMO_W, (float)DEMO_H);
	tDraw.iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	tDraw.iFlags = XGE_DRAW_SCREEN_SPACE;
	xgeDrawEx(&tDraw);
	iRet = xgeEnd();
	if ( iRet != XGE_OK ) return iRet;
	pDemo->iFrame++;
	if ( pDemo->bCaptureDone || ((pDemo->iMaxFrames > 0) && (pDemo->iFrame >= pDemo->iMaxFrames)) ) {
		printf("xge_svg final-summary frames=%d memorySvg=loaded fileSvg=loaded rasterTexture=%dx%d\n",
			pDemo->iFrame,
			pDemo->tRasterSvg.iWidth,
			pDemo->tRasterSvg.iHeight);
		xgeQuit();
	}
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xge_svg_demo_t tDemo;
	xge_desc_t tDesc;
	int iRet;

	memset(&tDemo, 0, sizeof(tDemo));
	tDemo.fRenderTolerance = 0.25f;
	iRet = __xgeSvgDemoParseArgs(&tDemo, argc, argv);
	if ( iRet == 1 ) return 0;
	if ( iRet != XGE_OK ) {
		__xgeSvgDemoUsage();
		return 1;
	}
	if ( tDemo.sBoundsPath[0] != '\0' ) {
		iRet = __xgeSvgDemoPrintBounds(&tDemo);
		return (iRet == XGE_OK) ? 0 : 1;
	}
	if ( tDemo.sRenderPath[0] == '\0' ) {
		iRet = xgeSvgCreate(&tDemo.pMemorySvg);
		if ( iRet == XGE_OK ) iRet = xgeSvgLoadMemory(tDemo.pMemorySvg, g_sMemorySvg, (int)sizeof(g_sMemorySvg) - 1);
		if ( iRet == XGE_OK ) iRet = __xgeSvgDemoLoadCachedAsset("examples\\xge_svg\\assets\\shapes.svg", "..\\examples\\xge_svg\\assets\\shapes.svg", &tDemo.pFileSvg);
		if ( iRet == XGE_OK ) iRet = xgeSvgSetPreserveAspectRatio(tDemo.pFileSvg, "xMidYMid slice");
		if ( iRet != XGE_OK ) {
			printf("xge_svg failed to load svg assets: %d\n", iRet);
			xgeSvgDestroy(tDemo.pMemorySvg);
			xgeSvgDestroy(tDemo.pFileSvg);
			xgeSvgCacheClear();
			return 1;
		}
	}

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.sTitle = (tDemo.sRenderPath[0] != '\0') ? "XGE SVG Render" : "XGE SVG";
	tDesc.iWidth = (tDemo.sRenderPath[0] != '\0') ? tDemo.iRenderWidth : DEMO_W;
	tDesc.iHeight = (tDemo.sRenderPath[0] != '\0') ? tDemo.iRenderHeight : DEMO_H;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		xgeTextureFree(&tDemo.tRasterSvg);
		xgeSvgDestroy(tDemo.pMemorySvg);
		xgeSvgDestroy(tDemo.pFileSvg);
		xgeSvgCacheClear();
		return 1;
	}
	{
		const char* sFontPath = __xgeSvgDemoFindTtf();
		if ( sFontPath != NULL ) {
			(void)xgeFontFallbackSet(sFontPath, 16.0f);
		}
	}
	if ( tDemo.sRenderPath[0] != '\0' ) {
		tDemo.iRenderResult = XGE_OK;
		iRet = xgeRun(__xgeSvgDemoRenderFrame, &tDemo);
		if ( (iRet == XGE_OK) && (tDemo.iRenderResult != XGE_OK) ) {
			iRet = tDemo.iRenderResult;
		}
		xgeSvgCacheClear();
		xgeUnit();
		return (iRet == XGE_OK) ? 0 : 1;
	}
	iRet = xgeRenderTargetCreate(&tDemo.tTarget, DEMO_W, DEMO_H);
	if ( iRet != XGE_OK ) {
		printf("xge_svg failed to create render target: %d\n", iRet);
		xgeTextureFree(&tDemo.tRasterSvg);
		xgeSvgDestroy(tDemo.pMemorySvg);
		xgeSvgDestroy(tDemo.pFileSvg);
		xgeSvgCacheClear();
		xgeUnit();
		return 1;
	}
	iRet = xgeRun(__xgeSvgDemoFrame, &tDemo);
	xgeRenderTargetFree(&tDemo.tTarget);
	xgeTextureFree(&tDemo.tRasterSvg);
	xgeSvgDestroy(tDemo.pMemorySvg);
	xgeSvgDestroy(tDemo.pFileSvg);
	xgeUnit();
	return (iRet == XGE_OK) ? 0 : 1;
}
