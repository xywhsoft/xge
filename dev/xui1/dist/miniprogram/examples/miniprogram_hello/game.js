"use strict";

const {
	XgeMiniProgramBridge,
	XGE_TOUCH_BEGIN,
	XGE_TOUCH_MOVE,
	XGE_TOUCH_END,
	XGE_TOUCH_CANCEL
} = require("../../platform/miniprogram/xge_miniprogram.js");

function __xgeMiniHasWx() {
	return typeof wx !== "undefined";
}

function __xgeMiniCreateCanvas() {
	if (__xgeMiniHasWx() && typeof wx.createCanvas === "function") {
		return wx.createCanvas();
	}
	if (typeof document !== "undefined") {
		const canvas = document.getElementById("xge-canvas") || document.createElement("canvas");
		if (!canvas.parentNode && document.body) {
			document.body.appendChild(canvas);
		}
		return canvas;
	}
	throw new Error("xge mini program hello requires a canvas provider");
}

function __xgeMiniWindowSize() {
	if (__xgeMiniHasWx() && typeof wx.getSystemInfoSync === "function") {
		const info = wx.getSystemInfoSync();
		return {
			width: info.windowWidth || 320,
			height: info.windowHeight || 200,
			dpr: info.pixelRatio || 1
		};
	}
	if (typeof window !== "undefined") {
		return {
			width: window.innerWidth || 320,
			height: window.innerHeight || 200,
			dpr: window.devicePixelRatio || 1
		};
	}
	return { width: 320, height: 200, dpr: 1 };
}

function __xgeMiniResolveResource(uri) {
	const prefix = "res://";
	if (typeof uri !== "string") {
		throw new Error("invalid resource uri");
	}
	if (uri.indexOf(prefix) === 0) {
		return "assets/" + uri.substring(prefix.length);
	}
	return uri;
}

function __xgeMiniReadResource(uri) {
	const path = __xgeMiniResolveResource(uri);
	if (__xgeMiniHasWx() && typeof wx.getFileSystemManager === "function") {
		const fs = wx.getFileSystemManager();
		return fs.readFileSync(path);
	}
	throw new Error("sync resource loading is not available: " + path);
}

function __xgeMiniLoadModule() {
	if (typeof Module !== "undefined") {
		return Promise.resolve(Module);
	}
	if (typeof require === "function") {
		try {
			const mod = require("./xge.js");
			if (mod && typeof mod.then === "function") {
				return mod;
			}
			if (typeof mod === "function") {
				return mod();
			}
			return Promise.resolve(mod);
		} catch (err) {
			return Promise.reject(err);
		}
	}
	return Promise.reject(new Error("xge.js is not available"));
}

function __xgeMiniTouches(list) {
	return Array.prototype.map.call(list || [], (touch, index) => ({
		id: touch.identifier != null ? touch.identifier : index,
		x: touch.clientX != null ? touch.clientX : touch.x,
		y: touch.clientY != null ? touch.clientY : touch.y,
		force: touch.force || 0
	}));
}

function __xgeMiniAttachWxTouch(bridge) {
	if (!__xgeMiniHasWx()) {
		return false;
	}
	if (typeof wx.onTouchStart === "function") {
		wx.onTouchStart((ev) => bridge.touch(XGE_TOUCH_BEGIN, __xgeMiniTouches(ev.changedTouches)));
	}
	if (typeof wx.onTouchMove === "function") {
		wx.onTouchMove((ev) => bridge.touch(XGE_TOUCH_MOVE, __xgeMiniTouches(ev.changedTouches)));
	}
	if (typeof wx.onTouchEnd === "function") {
		wx.onTouchEnd((ev) => bridge.touch(XGE_TOUCH_END, __xgeMiniTouches(ev.changedTouches)));
	}
	if (typeof wx.onTouchCancel === "function") {
		wx.onTouchCancel((ev) => bridge.touch(XGE_TOUCH_CANCEL, __xgeMiniTouches(ev.changedTouches)));
	}
	return true;
}

function xgeMiniProgramHelloStart(options) {
	const canvas = options && options.canvas ? options.canvas : __xgeMiniCreateCanvas();
	const size = __xgeMiniWindowSize();

	canvas.width = Math.max(1, Math.floor(size.width * size.dpr));
	canvas.height = Math.max(1, Math.floor(size.height * size.dpr));

	return __xgeMiniLoadModule().then((module) => {
		const bridge = new XgeMiniProgramBridge(module, {
			canvas,
			resourceLoader: __xgeMiniReadResource,
			audioBridge(command, handle) {
				console.warn("xge mini audio bridge is not wired yet", command, handle);
				return 0;
			}
		});

		bridge.init(size.width, size.height, size.dpr);
		if (!__xgeMiniAttachWxTouch(bridge)) {
			bridge.attachTouchEvents(canvas);
		}
		bridge.start();
		return bridge;
	});
}

if (__xgeMiniHasWx()) {
	xgeMiniProgramHelloStart().catch((err) => {
		console.error(err);
	});
}

module.exports = {
	xgeMiniProgramHelloStart
};
