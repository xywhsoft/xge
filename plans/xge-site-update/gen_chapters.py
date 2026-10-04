#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Generate tutorial chapters ch215-ch227 from shared boilerplate + bespoke bodies.
Existing ch212-214 were hand-written as the template. Console output ASCII.
"""
import os

SITE = "D:/GIT/home/host/xge/wwwroot/tutorial"

PART22 = [
    (212, "场景与节点层级"), (213, "相机"), (214, "glTF 模型加载"), (215, "材质"),
    (216, "直接光照"), (217, "阴影"), (218, "IBL 与环境"), (219, "骨骼动画"),
    (220, "地形高度场"), (221, "雾与天空"), (222, "异步加载与预算"),
    (223, "拾取与射线"), (224, "实战：岛屿行走"), (225, "综合集成与裁剪构建"),
]
CH226 = (226, "富文档与事务")
CH227 = (227, "Markdown 三模式")

def sidebar(cur):
    rows = []
    for n, t in PART22:
        on = ' class="On"' if n == cur else ""
        rows.append('\t\t\t\t\t<a href="ch%d.html"%s>%d %s</a>' % (n, on, n, t))
    return "\n".join(rows)

def doc_sidebar(cur):
    def row(n, t):
        on = ' class="On"' if n == cur else ""
        return '\t\t\t\t\t<a href="ch%d.html"%s>%d %s</a>' % (n, on, n, t)
    return "\n".join([
        '\t\t\t\t\t<h5>文档控件扩展</h5>',
        row(*CH226), row(*CH227),
        '\t\t\t\t\t<h5>相关章节</h5>',
        '\t\t\t\t\t<a href="ch173.html">173 CodeEdit 架构</a>',
        '\t\t\t\t\t<a href="ch130.html">130 事件系统</a>',
    ])

def pager(prev, nxt):
    def one(cls, href, label):
        return '<a class="%s" href="%s">%s</a>' % (cls, href, label)
    p = one("TutPagerPrev", prev[0], "← " + prev[1]) if prev else ""
    n = one("TutPagerNext", nxt[0], nxt[1] + " →") if nxt else ""
    return ('\t\t\t\t\t<div class="TutPager">\n\t\t\t\t\t\t%s\n\t\t\t\t\t\t%s\n\t\t\t\t\t</div>'
            % (p, n)) if p or n else ""

def page(num, title, kick, lead, side, body, pg):
    return f"""<!DOCTYPE html>
<html lang="zh-CN">
<head>
	<meta charset="utf-8">
	<title>第 {num} 章 {title} — XGE 教程</title>
	<meta name="viewport" content="width=device-width, initial-scale=1">
	<link rel="preconnect" href="https://fonts.googleapis.com">
	<link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
	<link href="https://fonts.googleapis.com/css2?family=Chakra+Petch:wght@500;600;700&family=JetBrains+Mono:wght@400;500&display=swap" rel="stylesheet">
	<link rel="stylesheet" href="../res/css/xge.css">
	<link rel="stylesheet" href="../res/css/tutorial.css">
	<link rel="icon" type="image/svg+xml" href="../res/img/favicon.svg">
</head>
<body>
	<div class="Ambient"></div>
	<nav class="Nav">
		<div class="NavInner">
			<a class="Brand" href="../index.html"><span class="BrandMark"><svg viewBox="0 0 34 34" fill="none"><rect x="2" y="2" width="30" height="30" stroke="#4fd8c2" stroke-width="2"/><path d="M9 9 L25 25 M25 9 L9 25" stroke="#ffb454" stroke-width="2.6" stroke-linecap="square"/><circle cx="17" cy="17" r="4" fill="#081114" stroke="#ff6b5e" stroke-width="2"/></svg></span><span class="BrandName">X<em>GE</em></span></a>
			<div class="NavLinks">
				<a href="../index.html#features">特性</a>
				<a href="../docs/index.html">文档</a>
				<a href="index.html" class="On">教程</a>
				<a href="../examples/index.html">示例</a>
			</div>
			<div class="NavRight">
				<span class="VerBadge">v2.0.0</span>
				<a class="NavBtn" href="../download/index.html">获取引擎</a>
			</div>
		</div>
	</nav>

	<header class="PageHero">
		<div class="Wrap">
			<div class="Kick">{kick}</div>
			<h1>{title}</h1>
			<p>{lead}</p>
		</div>
		<div class="Deco">{num}</div>
	</header>

	<section class="Section" style="padding-top:60px">
		<div class="Wrap">
			<div class="TutGrid">
				<aside class="TutSide">
					<h5>本篇章节</h5>
{side}
				</aside>

				<div class="TutMain">
{body}

{pg}
				</div>
			</div>
		</div>
	</section>

	<footer class="Footer">
		<div class="Wrap">
			<div class="FootBar">
				<span>Copyright © 2015-2026 星月无痕工作室 · <a href="https://beian.miit.gov.cn" target="_blank">京 ICP 备 2022013391 号</a></span>
				<span>XGE v2.0.0 · XUI v2.0.0</span>
			</div>
		</div>
	</footer>
	<script src="../res/js/xge.js"></script>
</body>
</html>
"""

def shot(n, alt, cap):
    return ('\t\t\t\t\t<figure class="TutShot">\n'
            '\t\t\t\t\t\t<img src="img/ch%d_1.png" alt="%s" loading="lazy">\n'
            '\t\t\t\t\t\t<figcaption>%s</figcaption>\n'
            '\t\t\t\t\t</figure>' % (n, alt, cap))

B = {}  # num -> (title, kick, lead, body, prev, next, side_mode)

B[215] = ("材质", "Chapter 215 · 3D 图形",
    "PBR metallic-roughness 工作流：一份描述结构体、五张贴图槽位、三种透明模式——从 Default 出发两行代码得到可用的材质。",
    """\t\t\t\t\t<h2>从默认值开始</h2>
\t\t\t\t\t<p><code>xge3dMaterialDefault()</code> 返回初始化好 glTF metallic/roughness 因子与恒等 UV 变换的描述结构体，改需要的字段即可：</p>
\t\t\t\t\t<div class="CodeBlock"><pre><code><span class="tk-t">xge3d_material_desc_t</span> md = <span class="tk-f">xge3dMaterialDefault</span>();
md.metallic = <span class="tk-n">0.0f</span>;  md.roughness = <span class="tk-n">1.0f</span>;   <span class="tk-c">// 哑光地表</span>
md.base_color[<span class="tk-n">0</span>] = <span class="tk-n">.31f</span>; md.base_color[<span class="tk-n">1</span>] = <span class="tk-n">.44f</span>; md.base_color[<span class="tk-n">2</span>] = <span class="tk-n">.17f</span>;

<span class="tk-t">xge3d_material</span> *pMaterial;
<span class="tk-f">xge3dMaterialCreate</span>(&amp;md, &amp;pMaterial);
<span class="tk-f">xge3dNodeSetMaterial</span>(pScene, tNode, pMaterial);</code></pre></div>

\t\t\t\t\t<h2>五张贴图槽位</h2>
\t\t\t\t\t<p><code>maps[XGE3D_MAP_*]</code> 共五槽：BASE_COLOR、METALLIC_ROUGHNESS、NORMAL、OCCLUSION、EMISSIVE。每个绑定携带纹理指针、texcoord 通道（0/1）、offset/scale/rotation UV 变换。纹理创建时显式声明 sRGB——颜色图填 1，法线/MR/遮蔽数据填 0：</p>
\t\t\t\t\t<div class="CodeBlock"><pre><code><span class="tk-t">xge3d_texture_desc_t</span> td = { &amp;tImage, <span class="tk-n">1</span>, { <span class="tk-n">0</span> } };  <span class="tk-c">// sRGB=1</span>
<span class="tk-t">xge3d_texture</span> *pTex;
<span class="tk-f">xge3dTextureCreate</span>(&amp;td, &amp;pTex);
md.maps[XGE3D_MAP_BASE_COLOR].texture = pTex;</code></pre></div>

\t\t\t\t\t<h2>透明与双面</h2>
\t\t\t\t\t<ul class="TutList">
\t\t\t\t\t\t<li><b>alpha_mode</b>：OPAQUE / MASK（alpha_cutoff 硬裁剪）/ BLEND（排序透明）。</li>
\t\t\t\t\t\t<li><b>double_sided</b>：双面渲染，配合法线翻转适合植被卡片。</li>
\t\t\t\t\t\t<li><b>unlit</b>：跳过光照，直接输出基色——海面、UI 板块常用（岛屿示例的海面就是 unlit 材质）。</li>
\t\t\t\t\t</ul>
\t\t\t\t\t<p>不想换整份材质时，<code>xge3dNodeSetColor(scene, node, rgba)</code> 可以对节点叠加纯色 tint；<code>xge3dMaterialGetDesc</code> 回读描述（贴图为借用指针，不要释放或修改）。</p>""",
    ("ch214.html", "第 214 章：glTF 模型加载"), ("ch216.html", "第 216 章：直接光照"), "part22")

B[216] = ("直接光照", "Chapter 216 · 3D 图形",
    "方向光、点光、聚光灯各三行描述——单位是物理的：lux 与坎德拉，距离衰减是平方反比。",
    """\t\t\t\t\t<h2>三种灯型</h2>
\t\t\t\t\t<div class="CodeBlock"><pre><code><span class="tk-t">xge3d_light_desc_t</span> tSun = <span class="tk-f">xge3dLightDefault</span>(XGE3D_LIGHT_DIRECTIONAL);
tSun.direction = (<span class="tk-t">xge3d_vec3_t</span>){ <span class="tk-n">-0.4f</span>, <span class="tk-n">-0.75f</span>, <span class="tk-n">0.5f</span> };  <span class="tk-c">// 光行进方向</span>
tSun.intensity = <span class="tk-n">3.2f</span>;   <span class="tk-c">// lux</span>

<span class="tk-t">xge3d_node_t</span> tLamp;
<span class="tk-f">xge3dNodeCreate</span>(pScene, (<span class="tk-t">xge3d_node_t</span>){0}, &amp;tLamp);
<span class="tk-f">xge3dNodeSetLight</span>(pScene, tLamp, &amp;tSun);</code></pre></div>
\t\t\t\t\t<p>点光与聚光灯用 <code>XGE3D_LIGHT_POINT / XGE3D_LIGHT_SPOT</code>：intensity 单位是坎德拉，平方反比衰减；<code>range</code> 为 0 表示无界，聚光角的 inner/outer 满足 0 ≤ inner &lt; outer &lt; π/2（弧度）。颜色是线性 RGB。最多 8 盏灯（<code>XGE3D_MAX_LIGHTS</code>）。</p>
\t\t\t\t\t<div class="TutNote">
\t\t\t\t\t\t<b>灯在节点上：</b>位置与朝向来自节点变换——把灯节点挂到角色手里，它就跟着手走。direction 是"光的行进方向"，不是"指向光源的方向"，别弄反。
\t\t\t\t\t</div>

\t\t\t\t\t<h2>本章 API 速查</h2>
\t\t\t\t\t<table class="ApiTable">
\t\t\t\t\t\t<thead><tr><th>函数 / 宏</th><th>说明</th></tr></thead>
\t\t\t\t\t\t<tbody>
\t\t\t\t\t\t\t<tr><td><code>xge3dLightDefault(type)</code></td><td>按灯型返回默认描述</td></tr>
\t\t\t\t\t\t\t<tr><td><code>xge3dNodeSetLight(scene, node, desc)</code></td><td>把灯挂到节点；替换即换灯</td></tr>
\t\t\t\t\t\t\t<tr><td><code>XGE3D_LIGHT_DIRECTIONAL / POINT / SPOT</code></td><td>三种灯型</td></tr>
\t\t\t\t\t\t\t<tr><td><code>casts_shadow</code>（描述字段）</td><td>方向/聚光可投影；点光阴影不支持</td></tr>
\t\t\t\t\t\t</tbody>
\t\t\t\t\t</table>""",
    ("ch215.html", "第 215 章：材质"), ("ch217.html", "第 217 章：阴影"), "part22")

B[217] = ("阴影", "Chapter 217 · 3D 图形",
    "一个太阳加两个聚光的深度阴影，1-4 级联由一份设置结构体全权控制——分辨率、级联数、偏置与过滤半径。",
    """\t\t\t\t\t<h2>启用阴影</h2>
\t\t\t\t\t<div class="CodeBlock"><pre><code><span class="tk-c">// 灯要开 casts_shadow（方向/聚光有效）</span>
<span class="tk-t">xge3d_light_desc_t</span> tSun = <span class="tk-f">xge3dLightDefault</span>(XGE3D_LIGHT_DIRECTIONAL);
tSun.<span class="tk-f">casts_shadow</span> = <span class="tk-n">1</span>;

<span class="tk-c">// 渲染描述里带阴影设置；NULL 则完全跳过 shadow pass</span>
<span class="tk-t">xge3d_shadow_settings_t</span> tShadow = <span class="tk-f">xge3dShadowDefault</span>();
tShadow.resolution = <span class="tk-n">2048</span>;
tShadow.cascades   = <span class="tk-n">4</span>;        <span class="tk-c">// 1..4</span>
tDesc.shadows = &amp;tShadow;
<span class="tk-f">xge3dRender</span>(pRenderer, pScene, &amp;tDesc, &amp;tStats);</code></pre></div>

\t\t\t\t\t<h2>调参直觉</h2>
\t\t\t\t\t<ul class="TutList">
\t\t\t\t\t\t<li><b>distance</b>：阴影覆盖的相机远界。级联把这段距离按 <code>split_lambda</code> 非线性切成 cascades 段，近处密远处疏。</li>
\t\t\t\t\t\t<li><b>bias / normal_bias</b>：痤疮与漏光的拉锯。先调 normal_bias。</li>
\t\t\t\t\t\t<li><b>filter_radius / blend</b>：级联间过渡与柔化半径，掩盖接缝。</li>
\t\t\t\t\t\t<li><b>预算</b>：<code>XGE3D_MAX_CASCADES 4</code>、<code>XGE3D_MAX_SHADOW_SPOTS 2</code>——一个方向光 + 两个聚光阴影。</li>
\t\t\t\t\t</ul>
\t\t\t\t\t<p>渲染统计 <code>xge3d_render_stats_t</code> 里有 shadow_draw_calls 与 shadow_maps，实时验证阴影开销；LOD 与阴影使用同一相机层级选择。</p>""",
    ("ch216.html", "第 216 章：直接光照"), ("ch218.html", "第 218 章：IBL 与环境"), "part22")

B[218] = ("IBL 与环境", "Chapter 218 · 3D 图形",
    "立方图环境同时是天空与间接光源：六张等尺寸面组成天空盒，预计算的辐照度 / 预滤波 / BRDF 三件套构成静态 IBL。",
    """\t\t\t\t\t<h2>创建环境</h2>
\t\t\t\t\t<div class="CodeBlock"><pre><code><span class="tk-t">xge3d_environment_desc_t</span> ed = { <span class="tk-n">0</span> };
ed.srgb = <span class="tk-n">1</span>;                       <span class="tk-c">// 颜色立方图</span>
<span class="tk-k">for</span> (<span class="tk-k">int</span> f = <span class="tk-n">0</span>; f &lt; <span class="tk-n">6</span>; ++f) ed.faces[f] = &amp;tFaces[f];  <span class="tk-c">// +X,-X,+Y,-Y,+Z,-Z</span>

<span class="tk-t">xge3d_environment</span> *pSky;
<span class="tk-f">xge3dEnvironmentCreate</span>(&amp;ed, &amp;pSky);

tDesc.environment = pSky;   <span class="tk-c">// 渲染时借用；NULL 则无天空</span></code></pre></div>
\t\t\t\t\t<p>六面的行列朝向遵循 GL 立方图约定（头文件注释给出了每个面的左→右、上→下世界轴对），岛屿示例在 CPU 上程序化生成六面渐变天空再交给引擎。环境的 CPU 拷贝在创建时完成，首次渲染一次性上传。</p>

\t\t\t\t\t<h2>静态 IBL 三件套</h2>
\t\t\t\t\t<div class="CodeBlock"><pre><code><span class="tk-t">xge3d_ibl_desc_t</span> ibl = { <span class="tk-n">0</span> };
ibl.irradiance   = tCosineFaces;      <span class="tk-c">// 余弦卷积 / π，线性 RGBA8</span>
ibl.prefiltered  = tGgxMips;          <span class="tk-c">// GGX 全 mip 链，roughness 0..1</span>
ibl.level_count  = tMipCount;
ibl.brdf         = &amp;tBrdfLut;         <span class="tk-c">// R=scale G=bias；X=NdotV Y=roughness</span>
<span class="tk-f">xge3dEnvironmentSetIBL</span>(pSky, &amp;ibl);

tDesc.ibl_intensity = <span class="tk-n">1.0f</span>;        <span class="tk-c">// 0 关闭间接光</span></code></pre></div>
\t\t\t\t\t<div class="TutNote">
\t\t\t\t\t\t<b>离线生成：</b>辐照度/预滤波贴图由离线工具从环境图卷积产出（工具不入运行库）；<code>EnvironmentSetIBL</code> 事务式拷贝预计算数据，传 NULL 移除 IBL 但保留天空面。
\t\t\t\t\t</div>""",
    ("ch217.html", "第 217 章：阴影"), ("ch219.html", "第 219 章：骨骼动画"), "part22")

B[219] = ("骨骼动画", "Chapter 219 · 3D 图形",
    "剪辑保留源数据、动画器借用场景：两层覆盖式混合、每层事件与根运动提取，骨骼节点还能当挂点用。",
    """\t\t\t\t\t<h2>剪辑与动画器</h2>
\t\t\t\t\t<div class="CodeBlock"><pre><code><span class="tk-c">// 从模型取剪辑，或加载独立的"纯动作" glTF</span>
<span class="tk-t">xge3d_clip</span> *pWalk, *pRun;
<span class="tk-f">xge3dClipFromModel</span>(pModel, <span class="tk-n">0</span>, &amp;pWalk);
<span class="tk-f">xge3dClipLoad</span>(<span class="tk-s">"actions/run.gltf"</span>, <span class="tk-n">0</span>, &amp;pRun);

<span class="tk-t">xge3d_animator</span> *pAnim;
<span class="tk-f">xge3dAnimatorCreate</span>(pScene, tActorRoot, &amp;pAnim);</code></pre></div>

\t\t\t\t\t<h2>层混合与更新</h2>
\t\t\t\t\t<div class="CodeBlock"><pre><code><span class="tk-t">xge3d_animation_layer_t</span> tLayer = <span class="tk-f">xge3dAnimationLayerDefault</span>();
tLayer.speed = <span class="tk-n">1.0f</span>;  tLayer.loop = <span class="tk-n">1</span>;
<span class="tk-f">xge3dAnimatorSetLayer</span>(pAnim, <span class="tk-n">0</span>, pWalk, &amp;tLayer);   <span class="tk-c">// 基础层</span>
tLayer.weight = <span class="tk-n">0.6f</span>;
<span class="tk-f">xge3dAnimatorSetLayer</span>(pAnim, <span class="tk-n">1</span>, pRun, &amp;tLayer);    <span class="tk-c">// 覆盖层，对静息姿混合</span>

<span class="tk-f">xge3dAnimatorUpdate</span>(pAnim, fDeltaSeconds);</code></pre></div>
\t\t\t\t\t<p>绑定失败保留旧层；自动绑定按唯一骨骼名，或对来自同一模型的剪辑按源索引。层可带遮罩（mask 数组按目标源节点序）与跨骨架骨绑定表。事件用 <code>xge3dAnimatorSetEvents</code> 登记，<code>xge3dAnimatorNextEvent</code> 按剪辑时间序取出（每层最多 1024，待处理超 4096 时 Update 直接失败防积压）。</p>

\t\t\t\t\t<h2>根运动与挂点</h2>
\t\t\t\t\t<div class="CodeBlock"><pre><code><span class="tk-f">xge3dAnimatorSetRootMotion</span>(pAnim, tTargetBone,
                            XGE3D_ROOT_X | XGE3D_ROOT_Z);   <span class="tk-c">// 抽取水平位移</span>
<span class="tk-t">xge3d_root_motion_t</span> tMotion;
<span class="tk-f">xge3dAnimatorTakeRootMotion</span>(pAnim, &amp;tMotion);  <span class="tk-c">// 应用前先做你自己的碰撞检查</span>

<span class="tk-t">xge3d_node_t</span> tHand;
<span class="tk-f">xge3dAnimatorBoneNode</span>(pAnim, <span class="tk-s">"hand_r"</span>, &amp;tHand);  <span class="tk-c">// 骨骼节点当挂点（socket）</span></code></pre></div>
\t\t\t\t\t<div class="TutNote TutNoteWarn">
\t\t\t\t\t\t<b>注意：</b>运行时按名/索引绑定<b>不</b>调整骨骼长度与轴——不同骨架的动作必须先走离线重定向（C 工具已验证两套不同参考姿态的目标骨架）。
\t\t\t\t\t</div>""",
    ("ch218.html", "第 218 章：IBL 与环境"), ("ch220.html", "第 220 章：地形高度场"), "part22")

B[220] = ("地形高度场", "Chapter 220 · 3D 图形",
    "把高度网格交给引擎：固定分块、1-4 级 LOD、垂直裙边——采样按真实三角形插值，射线连裙边一起测。",
    """\t\t\t\t\t<h2>从高度数组到地形</h2>
\t\t\t\t\t<div class="CodeBlock"><pre><code><span class="tk-t">xge3d_terrain_desc_t</span> td = <span class="tk-f">xge3dTerrainDefault</span>();
td.heights     = pHeights;             <span class="tk-c">// uint16 或 float 数组（原样拷贝，无 8bit 转换）</span>
td.width = td.depth = <span class="tk-n">257</span>;
td.format      = XGE3D_HEIGHT_F32;
td.cell_size   = <span class="tk-n">1.5f</span>;
td.height_scale = <span class="tk-n">1.0f</span>;  td.height_offset = <span class="tk-n">0.0f</span>;
td.chunk_cells = <span class="tk-n">32</span>;                   <span class="tk-c">// 每块边长（格）</span>
td.lod_count   = <span class="tk-n">4</span>;                    <span class="tk-c">// 1..4</span>
td.lod_distances[<span class="tk-n">0</span>] = <span class="tk-n">90</span>;  td.lod_distances[<span class="tk-n">1</span>] = <span class="tk-n">180</span>;  td.lod_distances[<span class="tk-n">2</span>] = <span class="tk-n">300</span>;
td.material    = pGroundMaterial;

<span class="tk-t">xge3d_terrain</span> *pTerrain;
<span class="tk-f">xge3dTerrainCreate</span>(&amp;td, &amp;pTerrain);

<span class="tk-t">xge3d_node_t</span> tRoot;
<span class="tk-f">xge3dTerrainInstantiate</span>(pScene, pTerrain, (<span class="tk-t">xge3d_node_t</span>){0}, &amp;tRoot);
<span class="tk-f">xge3dTerrainFree</span>(pTerrain);   <span class="tk-c">// 实例持有网格与材质后即可释放</span></code></pre></div>

\t\t\t\t\t<h2>查询：采样与射线</h2>
\t\t\t\t\t<div class="CodeBlock"><pre><code><span class="tk-k">float</span> fHeight; <span class="tk-t">xge3d_vec3_t</span> tNormal;
<span class="tk-f">xge3dTerrainSample</span>(pTerrain, <span class="tk-n">0</span>, fLocalX, fLocalZ, &amp;fHeight, &amp;tNormal);

<span class="tk-t">xge3d_hit_t</span> tHit; <span class="tk-k">size_t</span> stChunk;
<span class="tk-f">xge3dTerrainRaycast</span>(pTerrain, <span class="tk-n">0</span>, &amp;tRay, <span class="tk-n">500.0f</span>, &amp;tHit, &amp;stChunk);</code></pre></div>
\t\t\t\t\t<div class="TutNote">
\t\t\t\t\t\t<b>三角形级精度：</b>Sample 对实际三角形插值（不是双线性近似）；Raycast 含裙边。两者都指定网格层级（0 为精细面），而渲染中的块可用不同层级——查询与显示解耦。
\t\t\t\t\t</div>
\t\t\t\t\t<p><code>xge3dTerrainInfo</code> 返回块数、LOD 数与表面包围盒（不含裙边）；<code>xge3dTerrainChunkMesh</code> 借出指定块的网格与块内位置，供自定义碰撞或导航使用。引擎不提供世界流式加载与游戏规则——地形数据是一次性常驻的。</p>""",
    ("ch219.html", "第 219 章：骨骼动画"), ("ch221.html", "第 221 章：雾与天空"), "part22")

B[221] = ("雾与天空", "Chapter 221 · 3D 图形",
    "线性距离雾在曝光与 sRGB 转换之前混合，天空不受雾影响；天空本体就是第 218 章的环境立方图。",
    """\t\t\t\t\t<h2>三行雾</h2>
\t\t\t\t\t<div class="CodeBlock"><pre><code><span class="tk-t">xge3d_fog_settings_t</span> tFog = <span class="tk-f">xge3dFogDefault</span>();
tFog.color = (<span class="tk-t">xge3d_vec3_t</span>){ <span class="tk-n">.78f</span>, <span class="tk-n">.87f</span>, <span class="tk-n">.93f</span> };  <span class="tk-c">// 线性 RGB</span>
tFog.start = <span class="tk-n">60.0f</span>;  tFog.end = <span class="tk-n">420.0f</span>;               <span class="tk-c">// 沿相机前向，米；0 ≤ start &lt; end</span>
tDesc.fog = &amp;tFog;   <span class="tk-c">// NULL 关闭；只依赖 3D 开关，无光照配置也可用</span></code></pre></div>
\t\t\t\t\t<div class="TutNote">
\t\t\t\t\t\t<b>混合位置很重要：</b>雾色先与场景线性色混合，再进曝光（<code>exposure</code>，0 选 1）与 sRGB 转换——所以雾色给线性值，天空保持原样不被雾化，地平线处的过渡是自然的。
\t\t\t\t\t</div>

\t\t\t\t\t<h2>天空的两种做法</h2>
\t\t\t\t\t<ul class="TutList">
\t\t\t\t\t\t<li><b>预烘焙立方图</b>：离线渲染或全景图转六面，配合 IBL 三件套一步到位。</li>
\t\t\t\t\t\t<li><b>程序化生成</b>：岛屿示例在 CPU 上按面法线/切线基生成渐变 + 云 + 太阳高光的六面图，再交给 <code>xge3dEnvironmentCreate</code>——几十行代码得到风格化天空。</li>
\t\t\t\t\t</ul>
\t\t\t\t\t<p>渲染时把环境挂到 <code>tDesc.environment</code> 即同时得到天空盒与（若有 IBL 的）间接光；关掉天空传 NULL。</p>""",
    ("ch220.html", "第 220 章：地形高度场"), ("ch222.html", "第 222 章：异步加载与预算"), "part22")

B[222] = ("异步加载与预算", "Chapter 222 · 3D 图形",
    "CPU 解码走 worker 线程，GPU 上传由你按预算泵送：字节、操作数硬上限加时间软上限——帧率由你说了算。",
    """\t\t\t\t\t<h2>请求生命周期</h2>
\t\t\t\t\t<div class="CodeBlock"><pre><code><span class="tk-t">xge3d_loader_desc_t</span> ld = { <span class="tk-n">0</span> };
ld.max_requests = <span class="tk-n">64</span>;   <span class="tk-c">// 0 取默认 64，上限 1024</span>

<span class="tk-t">xge3d_loader</span> *pLoader;
<span class="tk-f">xge3dLoaderCreate</span>(&amp;ld, &amp;pLoader);

<span class="tk-t">xge3d_request_t</span> tReq;
<span class="tk-f">xge3dLoaderRequest</span>(pLoader, <span class="tk-s">"assets/scene.glb"</span>, &amp;tReq);</code></pre></div>
\t\t\t\t\t<p>状态机：QUEUED → LOADING → CPU_READY → UPLOADING → READY（或 FAILED / CANCELLED）。<code>xge3dLoaderStatus</code> 随时查询进度字节与结果。</p>

\t\t\t\t\t<h2>按预算泵送</h2>
\t\t\t\t\t<div class="CodeBlock"><pre><code><span class="tk-t">xge3d_upload_budget_t</span> tBudget = { <span class="tk-n">2</span>&lt;&lt;<span class="tk-n">20</span>, <span class="tk-n">2000</span>, <span class="tk-n">16</span> };  <span class="tk-c">// 字节 / 微秒 / 操作数</span>
<span class="tk-t">xge3d_upload_stats_t</span> tStats;
<span class="tk-f">xge3dLoaderPump</span>(pLoader, &amp;tBudget, &amp;tStats);
<span class="tk-c">// 字节与操作数是硬顶（大缓冲/纹理分片上传），时间是软限（操作间隙检查）</span>
<span class="tk-c">// 被挡住时的最小需求通过 min_next_bytes 报告</span></code></pre></div>
\t\t\t\t\t<div class="CodeBlock"><pre><code><span class="tk-t">xge3d_request_info_t</span> tInfo;
<span class="tk-f">xge3dLoaderStatus</span>(pLoader, tReq, &amp;tInfo);
<span class="tk-k">if</span> (tInfo.state == XGE3D_REQUEST_READY) {
    <span class="tk-t">xge3d_model</span> *pModel;
    <span class="tk-f">xge3dLoaderTake</span>(pLoader, tReq, &amp;pModel);  <span class="tk-c">// 成功即失效句柄</span>
}</code></pre></div>

\t\t\t\t\t<h2>取消、释放与线程</h2>
\t\t\t\t\t<ul class="TutList">
\t\t\t\t\t\t<li><b>Cancel 幂等</b>：立即阻止发布并在拥有线程释放 GPU 残留；CPU 清理可以稍后完成。</li>
\t\t\t\t\t\t<li><b>Release 使句柄失效</b>——防止陈旧结果落到复用的 slot 上。</li>
\t\t\t\t\t\t<li><b>线程规则</b>：一个 loader 一条 xrt CPU worker；loader 的全部公开调用都走创建线程；Pump 必须在 GPU 上下文线程。</li>
\t\t\t\t\t\t<li><b>资源提供者</b>：<code>XGE3D_LOADER_RESOURCE_PROVIDERS</code> 标志接入 xgeResource 回调（工作线程执行，须线程安全）；默认 IO 是普通文件与 file://。</li>
\t\t\t\t\t</ul>""",
    ("ch221.html", "第 221 章：雾与天空"), ("ch223.html", "第 223 章：拾取与射线"), "part22")

B[223] = ("拾取与射线", "Chapter 223 · 3D 图形",
    "屏幕一点反投影成射线，场景求交返回命中节点与三角形；包围盒/球/三角形的原子测试与容量协商式区域查询一起组成完整的拾取工具箱。",
    """\t\t\t\t\t<h2>屏幕到命中</h2>
\t\t\t\t\t<div class="CodeBlock"><pre><code><span class="tk-t">xge3d_ray_t</span> tRay;
<span class="tk-f">xge3dCameraScreenRay</span>(&amp;tCam, fMouseX, fMouseY, fWidth, fHeight, &amp;tRay);
<span class="tk-c">// 像素坐标自左上、帧缓冲像素；射线原点在近平面上</span>

<span class="tk-t">xge3d_hit_t</span> tHit;
<span class="tk-k">int</span> r = <span class="tk-f">xge3dSceneRaycast</span>(pScene, &amp;tRay, <span class="tk-n">1000.0f</span>, &amp;tHit);
<span class="tk-k">if</span> (r == XGE_OK) {
    <span class="tk-c">// tHit.node / model_root / distance(米) / position / normal / triangle</span>
}</code></pre></div>
\t\t\t\t\t<p>SceneRaycast 归一化非零方向、测试可见网格三角形的双面；无命中返回 NOT_FOUND 并清空输出。骨骼网格的包围随当前姿势收紧。</p>

\t\t\t\t\t<h2>原子测试与区域查询</h2>
\t\t\t\t\t<div class="CodeBlock"><pre><code><span class="tk-t">xge3d_aabb_t</span> tBox; <span class="tk-k">float</span> fDist;
<span class="tk-f">xge3dRayAabb</span>(&amp;tRay, &amp;tBox, <span class="tk-n">100.0f</span>, &amp;fDist);
<span class="tk-f">xge3dRaySphere</span>(&amp;tRay, &amp;tSphere, <span class="tk-n">100.0f</span>, &amp;fDist);
<span class="tk-f">xge3dRayTriangle</span>(&amp;tRay, tTriangle, <span class="tk-n">100.0f</span>, &amp;tHit);

<span class="tk-c">// 容量协商：先数后取</span>
<span class="tk-k">size_t</span> stCount;
<span class="tk-f">xge3dSceneQueryAabb</span>(pScene, &amp;tBox, <span class="tk-m">NULL</span>, <span class="tk-n">0</span>, &amp;stCount);
<span class="tk-t">xge3d_node_t</span> *pNodes = <span class="tk-f">malloc</span>(stCount * <span class="tk-k">sizeof</span>(*pNodes));
<span class="tk-f">xge3dSceneQueryAabb</span>(pScene, &amp;tBox, pNodes, stCount, &amp;stCount);</code></pre></div>
\t\t\t\t\t<div class="TutNote">
\t\t\t\t\t\t<b>保守两字箴言：</b>查询是保守宽相（含视锥/包围盒外扩），命中后如需精确再做三角形级验证；短缓冲会收到前缀并返回 BUFFER_TOO_SMALL。
\t\t\t\t\t</div>""",
    ("ch222.html", "第 222 章：异步加载与预算"), ("ch224.html", "第 224 章：实战：岛屿行走"), "part22")

B[224] = ("实战：岛屿行走", "Chapter 224 · 3D 图形",
    "把前七章串成一个经典教学范例：程序化岛屿 + glTF 角色 + 骨骼动作 + 地面采样行走——应用规则全部留在示例代码里。",
    """\t\t\t\t\t<h2>范例结构</h2>
\t\t\t\t\t<p><code>examples/xge_3d_walk</code> 按经典 3D 教学组织：<b>初始化 → 创建场景 → 读取输入 → 更新角色与相机 → 绘制</b>。教程策略（岛屿生成、移动、重力、相机）全部留在示例内；XGE 只提供高度场拷贝、三角形采样、蒙皮、天空与渲染。</p>

\t\t\t\t\t<h2>程序化岛屿</h2>
\t\t\t\t\t<div class="CodeBlock"><pre><code><span class="tk-c">// 257×257 高度场：多八度值噪声 + 径向衰减，srgb 顶点色烘焙进 257² 纹理</span>
<span class="tk-k">for</span> (<span class="tk-k">int</span> step = <span class="tk-n">64</span>; step &gt;= <span class="tk-n">4</span>; step /= <span class="tk-n">2</span>, amplitude *= <span class="tk-n">.5f</span>) { ... }
<span class="tk-c">// 海滩→草地→岩石按高度混色，再乘逐点颗粒噪声</span>

<span class="tk-t">xge3d_material_desc_t</span> md = <span class="tk-f">xge3dMaterialDefault</span>();
md.metallic = <span class="tk-n">0</span>; md.roughness = <span class="tk-n">1</span>;
md.maps[XGE3D_MAP_BASE_COLOR].texture = pTexture;  <span class="tk-c">// 顶点色贴图</span></code></pre></div>
\t\t\t\t\t<p>海面是一个 unlit 大面片（基色深青、粗糙度 0.25），天空按六面法线基程序化生成渐变+云+太阳高光——两段代码都在 main.c 里，可以直接抄。</p>

\t\t\t\t\t<h2>角色与行走规则</h2>
\t\t\t\t\t<div class="CodeBlock"><pre><code><span class="tk-c">// 加载 explorer.gltf，四段剪辑交给动画器</span>
<span class="tk-f">xge3dModelInstantiate</span>(pScene, pModel, (<span class="tk-t">xge3d_node_t</span>){0}, &amp;d.actor);
<span class="tk-f">xge3dAnimatorCreate</span>(pScene, d.actor, &amp;d.animator);

<span class="tk-c">// 每步移动前做地面采样：高度与坡度门槛 = 可行走</span>
<span class="tk-k">int</span> valid = <span class="tk-f">xge3dTerrainSample</span>(pTerrain, <span class="tk-n">0</span>, nx, nz, &amp;height, &amp;normal);
<span class="tk-k">if</span> (valid != XGE_OK || height &lt; <span class="tk-n">.25f</span> || normal.y &lt; <span class="tk-n">.68f</span>) {
    ++d.blocked;   <span class="tk-c">// 拒绝这步：太低（水下）或太陡</span>
} <span class="tk-k">else</span> { d.position.x = nx; d.position.z = nz; }
<span class="tk-c">// 空中时积分重力，落地回贴地面高度</span></code></pre></div>
\t\t\t\t\t<div class="TutNote">
\t\t\t\t\t\t<b>规则归属：</b>"水下不走、陡坡不走、跳跃与落地"是示例的策略，不是引擎的——XGE 只负责告诉你那一点的高度和法线。这正是 API 的分层哲学。
\t\t\t\t\t</div>

\t\t\t\t\t<h2>命令行</h2>
\t\t\t\t\t<div class="Term">
<span class="P">$</span> examples\\xge_3d_walk\\run.bat<br>
<span class="P">$</span> build\\xge-3d-walk\\xge_3d_walk.exe <span class="Hl">--seed 20261003 --autopilot --first-person</span><br>
<span class="Cm"># --frames N 定帧退出；--capture out.png 落盘截图；构建脚本自动生成确定性资产</span>
\t\t\t\t\t</div>""",
    ("ch223.html", "第 223 章：拾取与射线"), ("ch225.html", "第 225 章：综合集成与裁剪构建"), "part22")

B[225] = ("综合集成与裁剪构建", "Chapter 225 · 3D 图形",
    "一份把 3D 全家桶与真实 XUI 同场运行的集成范例，加上七个构建档位——本章也是 3D 篇的收官。",
    """\t\t\t\t\t<h2>集成范例覆盖什么</h2>
\t\t\t\t\t<p><code>examples/xge_3d_integration</code> 用纯公开 API 把下列能力组合运行，十五项异步资源全部完成后开始计帧：</p>
\t\t\t\t\t<ul class="TutList">
\t\t\t\t\t\t<li>模型、<b>共享材质与静态实例</b>、分块高度场、太阳与聚光阴影、天空、两种骨架的六类外部动作、预算加载与真实 XUI 界面。</li>
\t\t\t\t\t\t<li>应用侧导出静态三角形、<b>过滤可行走表面</b>、地面射线查询写回位置；验证创建/移动/删除与 <b>1e9 全局坐标下的原点重定位</b>——这些应用数据与规则不进 XGE。</li>
\t\t\t\t\t</ul>
\t\t\t\t\t<div class="Term">
<span class="P">$</span> examples\\xge_3d_integration\\build.bat full-dev --run <span class="Hl">--frames 360 --capture out.png</span><br>
<span class="Cm"># 也可用 3d 档构建（界面与二维合成随宏裁掉）</span>
\t\t\t\t\t</div>

\t\t\t\t\t<h2>可裁剪构建档位</h2>
\t\t\t\t\t<table class="ApiTable">
\t\t\t\t\t\t<thead><tr><th>档位</th><th>能力</th><th>实测体积（2026-10，x64 -O2）</th></tr></thead>
\t\t\t\t\t\t<tbody>
\t\t\t\t\t\t\t<tr><td>core</td><td>窗口、输入、资源、GPU 底座</td><td>672,176 B</td></tr>
\t\t\t\t\t\t\t<tr><td>3d</td><td>三维 + 底座（关 2D/文本/音频/XUI）</td><td>873,737 B</td></tr>
\t\t\t\t\t\t\t<tr><td>full-dev</td><td>全部能力含三维</td><td>8,466,461 B</td></tr>
\t\t\t\t\t\t</tbody>
\t\t\t\t\t</table>
\t\t\t\t\t<p>3D 相比 core 只增加约 201 KB。八个 <code>XGE3D_ENABLE_*</code> 子模块（MODEL/LIGHTING/SHADOW/IBL/ANIMATION/TERRAIN/ASYNC/FOG）可单独关闭验证依赖收敛；65 个 <code>XUI_ENABLE_*</code> 控件开关同理，依赖表在 <code>tools/features.json</code>，冲突由编译器报错。库与消费者必须同配置——构建产物自带完整 <code>xge_build_config.h</code> 强制对齐。增量编译实测：新增一个异步 TU 0.59 s，热构建约 0.14 s。</p>

\t\t\t\t\t<h2>下一步</h2>
\t\t\t\t\t<p>3D 篇到此收官。公开接口、所有权与限制的权威文本在仓库 <code>docs/3D.md</code>；交付实测在 <code>docs/3D_RELEASE.md</code>；配置细节在 <code>docs/BUILD_PROFILES.md</code>。祝渲染愉快。</p>""",
    ("ch224.html", "第 224 章：实战：岛屿行走"), ("index.html", "返回教程首页"), "part22")

B[226] = ("富文档与事务", "Chapter 226 · 文档控件扩展",
    "xuiDocument 是一个共享句柄的文档对象：事务保证编辑原子性，快照提供不可变视图，修订号驱动撤销与协同。",
    """\t\t\t\t\t<h2>文档是共享句柄</h2>
\t\t\t\t\t<div class="CodeBlock"><pre><code><span class="tk-t">xui_document</span> pDoc;
<span class="tk-f">xuiDocumentCreate</span>(&amp;tDesc, &amp;pDoc);
<span class="tk-f">xuiDocumentRetain</span>(pDoc);    <span class="tk-c">// 编辑器与预览各自持有引用</span>
...
<span class="tk-f">xuiDocumentRelease</span>(pDoc);</code></pre></div>
\t\t\t\t\t<p>创建描述可指定 profile 与 Markdown 方言（<code>xuiDocumentGetProfile / GetMarkdownDialect</code> 可回读）。同一份文档可以同时挂<b>富文本编辑器、Markdown 编辑器与只读预览</b>——三个视图共享内容与撤销历史。</p>

\t\t\t\t\t<h2>事务：要么全有要么全无</h2>
\t\t\t\t\t<div class="CodeBlock"><pre><code><span class="tk-t">xui_document_transaction</span> t;
<span class="tk-f">xuiDocumentBeginTransaction</span>(pDoc, &amp;tTxnDesc, &amp;t);
<span class="tk-f">xuiDocumentTxnInsertFragment</span>(t, ...);
<span class="tk-f">xuiDocumentTxnInsertTable</span>(t, <span class="tk-n">1</span>, XUI_DOCUMENT_APPEND, <span class="tk-n">3</span>, <span class="tk-n">2</span>, <span class="tk-n">1</span>, &amp;tTable);
<span class="tk-f">xuiDocumentTxnCommit</span>(t, <span class="tk-m">NULL</span>);
<span class="tk-f">xuiDocumentTxnRelease</span>(t);</code></pre></div>
\t\t\t\t\t<div class="TutNote">
\t\t\t\t\t\t<b>原子保证：</b>任一步失败则整体不提交——文档不会留下半截结构。UI 侧的失败反馈就是一句"Operation returned %d. The document was not partially committed."
\t\t\t\t\t</div>

\t\t\t\t\t<h2>快照与修订</h2>
\t\t\t\t\t<p><code>xuiDocumentAcquireSnapshot</code> 取不可变快照，配合 <code>SnapshotRetain / Release</code> 与一族遍历 API（GetNode / GetChild / GetBlockSyntax / GetInlineSyntax / GetSourceSegment / GetSourceLine / GetTableToken…）做导出、搜索或语法着色；<code>xuiDocumentGetRevision</code> 返回单调修订号，是撤销栈与协同合并的基准。</p>

\t\t\t\t\t<h2>编辑器控件</h2>
\t\t\t\t\t<div class="CodeBlock"><pre><code><span class="tk-t">xui_doc_editor_desc_t</span> ed = { <span class="tk-n">0</span> };
ed.tView.pDocument = pDoc;
<span class="tk-f">xuiDocumentEditorCreate</span>(pContext, &amp;ed, &amp;pEditor);   <span class="tk-c">// 三参数创建模式</span>

<span class="tk-f">xuiDocumentEditorExecute</span>(pEditor, XUI_DOC_EDIT_BOLD); <span class="tk-c">// 结构化命令</span>
<span class="tk-f">xuiDocumentEditorInsertText</span>(pEditor, ...);
<span class="tk-f">xuiDocumentUndo</span>(pDoc, <span class="tk-m">NULL</span>);                          <span class="tk-c">// 共享撤销</span></code></pre></div>""",
    ("ch185.html", "第 185 章：MessageList 消息列表"), ("ch227.html", "第 227 章：Markdown 三模式"), "doc")

B[227] = ("Markdown 三模式", "Chapter 227 · 文档控件扩展",
    "Source、Visual、Live MD 三种投影视图读的是同一份文档、同一条修订历史——切模式不丢撤销。",
    """\t\t\t\t\t<h2>切换投影</h2>
\t\t\t\t\t<div class="CodeBlock"><pre><code><span class="tk-c">// 编辑器创建时选起点</span>
ed.iMode = XUI_DOC_SOURCE_TEXT;
<span class="tk-f">xuiDocumentEditorCreate</span>(pContext, &amp;ed, &amp;pEditor);

<span class="tk-c">// 运行时切换：源码 / 可视化 / 实时 MD</span>
<span class="tk-f">xuiDocumentViewSetMode</span>(pEditor, XUI_DOC_VISUAL);
<span class="tk-f">xuiDocumentViewSetMode</span>(pEditor, XUI_DOC_LIVE_MARKDOWN);</code></pre></div>
\t\t\t\t\t<p>Visual 模式下 Bold/Italic 等按钮变成结构化命令（直接改文档树）；Source 模式编辑纯文本；Live MD 把当前顶层容器显示为源码、其余容器照常渲染——三种模式共享修订与 <code>xuiDocumentUndo</code> 历史。</p>

\t\t\t\t\t<h2>预览与图片资源</h2>
\t\t\t\t\t<div class="CodeBlock"><pre><code><span class="tk-c">// 只读预览：与编辑器共用同一 View 描述</span>
<span class="tk-t">xui_doc_view_desc_t</span> vd = ed.tView;
<span class="tk-f">xuiDocumentViewCreate</span>(pContext, &amp;vd, &amp;pPreview);

<span class="tk-c">// 图片：同步解码或异步加载（不卡 UI 线程）</span>
<span class="tk-f">xuiDocumentImageResourceLoadFile</span>(pContext, <span class="tk-s">"demo.surface"</span>, <span class="tk-s">"res/msgbox_info.png"</span>, <span class="tk-m">NULL</span>, &amp;pRes);
<span class="tk-f">xuiDocumentImageResourceLoadFileAsync</span>(pContext, <span class="tk-s">"demo.async"</span>, <span class="tk-s">"res/msgbox_info.png"</span>, <span class="tk-m">NULL</span>, &amp;pReq);
<span class="tk-c">// Poll 直到不再是 BUSY；缺图时渲染占位框而非布局抖动</span></code></pre></div>

\t\t\t\t\t<h2>示例与验证</h2>
\t\t\t\t\t<div class="Term">
<span class="P">$</span> call examples\\xui_document\\build.bat<br>
<span class="P">$</span> build\\xui_document.exe <span class="Cm"># 三栏：富文本 / MD 编辑 / 实时预览</span><br>
<span class="P">$</span> build\\xui_document.exe <span class="Hl">--verify</span><br>
<span class="Cm"># Ctrl+Z/Y 撤销重做 · Ctrl+B/I 格式 · Ctrl+滚轮缩放 · 旧 xui_richedit 示例已并入本入口</span>
\t\t\t\t\t</div>
\t\t\t\t\t<p>自动化验证覆盖渲染、异步发布次序（源先变、预览后变）与缺图占位三条链路。至此 227 章教程全部完成——回头见目录页。</p>""",
    ("ch226.html", "第 226 章：富文档与事务"), ("ch186.html", "第 186 章：InventoryGrid 物品栏"), "doc")

def main():
    for num, (title, kick, lead, body, prev, nxt, side_mode) in B.items():
        side = sidebar(num) if side_mode == "part22" else doc_sidebar(num)
        html = page(num, title, kick, lead, side, body, pager(prev, nxt))
        out = os.path.join(SITE, "ch%d.html" % num)
        with open(out, "w", encoding="utf-8", newline="\n") as f:
            f.write(html)
        print("wrote ch%d %s (%d bytes)" % (num, title, len(html.encode("utf-8"))))

if __name__ == "__main__":
    main()
