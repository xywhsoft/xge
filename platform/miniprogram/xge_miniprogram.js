const XGE_TOUCH_BEGIN = 1;
const XGE_TOUCH_MOVE = 2;
const XGE_TOUCH_END = 4;
const XGE_TOUCH_CANCEL = 5;

class XgeMiniProgramBridge {
	constructor(module, options) {
		this.module = module;
		this.canvas = options && options.canvas ? options.canvas : null;
		this.gl = null;
		this.running = false;
		this.framePending = false;
		this.resourceLoader = options && options.resourceLoader ? options.resourceLoader : null;
		this.audioBridge = options && options.audioBridge ? options.audioBridge : null;
		this.width = 0;
		this.height = 0;
		this.dpr = 1;
	}

	_malloc(size) {
		if (!this.module || typeof this.module._malloc !== "function") {
			throw new Error("xge miniprogram bridge requires module._malloc");
		}
		return this.module._malloc(size);
	}

	_free(ptr) {
		if (ptr && this.module && typeof this.module._free === "function") {
			this.module._free(ptr);
		}
	}

	_ccall(name, returnType, argTypes, args) {
		if (this.module && typeof this.module.ccall === "function") {
			return this.module.ccall(name, returnType, argTypes, args);
		}
		const fn = this.module && this.module["_" + name];
		if (typeof fn !== "function") {
			throw new Error("missing native export: " + name);
		}
		return fn.apply(null, args || []);
	}

	init(width, height, dpr) {
		if (!this.canvas) {
			throw new Error("xge miniprogram canvas is required");
		}
		this.width = width || this.canvas.width || 1;
		this.height = height || this.canvas.height || 1;
		this.dpr = dpr || 1;
		this.gl = this.canvas.getContext("webgl2", {
			alpha: true,
			antialias: true,
			depth: true,
			stencil: false,
			premultipliedAlpha: true,
			preserveDrawingBuffer: false
		});
		if (!this.gl) {
			throw new Error("xge requires WebGL2 in miniprogram backend");
		}
		this.resize(this.width, this.height, this.dpr);
		const rc = this._ccall("xgeMiniProgramInitSimple", "number", ["number", "number", "number"], [this.width, this.height, this.dpr]);
		if (rc !== 0) {
			throw new Error("xgeMiniProgramInitSimple failed: " + rc);
		}
		return this.gl;
	}

	unit() {
		this.stop();
		this._ccall("xgeMiniProgramUnit", null, [], []);
	}

	start() {
		this.running = true;
		this.requestFrame();
	}

	stop() {
		this.running = false;
		this.framePending = false;
	}

	requestFrame() {
		if (this.framePending) {
			return 0;
		}
		this.framePending = true;
		const request = typeof requestAnimationFrame === "function" ? requestAnimationFrame : (fn) => setTimeout(() => fn(Date.now()), 16);
		request((time) => {
			this.framePending = false;
			if (!this.running) {
				return;
			}
			this._ccall("xgeMiniProgramFrame", "number", ["number"], [(time || 0) / 1000.0]);
			this.requestFrame();
		});
		return 0;
	}

	resize(width, height, dpr) {
		this.width = Math.max(1, Math.floor(width || 1));
		this.height = Math.max(1, Math.floor(height || 1));
		this.dpr = dpr && dpr > 0 ? dpr : 1;
		if (this.canvas) {
			this.canvas.width = Math.max(1, Math.floor(this.width * this.dpr));
			this.canvas.height = Math.max(1, Math.floor(this.height * this.dpr));
		}
		if (this.gl) {
			this.gl.viewport(0, 0, this.canvas.width, this.canvas.height);
		}
		return this._ccall("xgeMiniProgramResize", "number", ["number", "number", "number"], [this.width, this.height, this.dpr]);
	}

	touch(phase, touches) {
		const list = touches || [];
		let rc = 0;
		for (let i = 0; i < list.length; i++) {
			const t = list[i];
			const id = Number(t.id || t.identifier || i) | 0;
			const x = Number(t.x || t.clientX || 0);
			const y = Number(t.y || t.clientY || 0);
			const force = Number(t.force || 0);
			rc = this._ccall("xgeMiniProgramTouchOne", "number", ["number", "number", "number", "number", "number"], [phase, id, x, y, force]);
			if (rc !== 0) {
				return rc;
			}
		}
		return rc;
	}

	attachTouchEvents(target) {
		const source = target || this.canvas;
		if (!source || typeof source.addEventListener !== "function") {
			return;
		}
		source.addEventListener("touchstart", (ev) => this.touch(XGE_TOUCH_BEGIN, Array.from(ev.changedTouches || [])));
		source.addEventListener("touchmove", (ev) => this.touch(XGE_TOUCH_MOVE, Array.from(ev.changedTouches || [])));
		source.addEventListener("touchend", (ev) => this.touch(XGE_TOUCH_END, Array.from(ev.changedTouches || [])));
		source.addEventListener("touchcancel", (ev) => this.touch(XGE_TOUCH_CANCEL, Array.from(ev.changedTouches || [])));
	}

	text(codepoint) {
		return this._ccall("xgeMiniProgramText", "number", ["number"], [codepoint]);
	}

	attachTextInput(input) {
		if (!input || typeof input.addEventListener !== "function") {
			return;
		}
		input.addEventListener("input", (ev) => {
			const value = ev && ev.data ? ev.data : "";
			for (const ch of value) {
				const codepoint = ch.codePointAt(0);
				if (codepoint && codepoint <= 0xFFFF) {
					this.text(codepoint);
				}
			}
		});
	}

	loadResource(uri) {
		if (!this.resourceLoader) {
			throw new Error("xge miniprogram resourceLoader is not configured");
		}
		return this.resourceLoader(uri);
	}

	audioCommand(command, handle, data) {
		if (!this.audioBridge) {
			return -7;
		}
		const result = this.audioBridge(command, handle, data);
		this._ccall("xgeMiniProgramAudioCommand", "number", ["number", "number", "number", "number"], [command, handle, 0, 0]);
		return result;
	}
}

if (typeof module !== "undefined") {
	module.exports = {
		XgeMiniProgramBridge,
		XGE_TOUCH_BEGIN,
		XGE_TOUCH_MOVE,
		XGE_TOUCH_END,
		XGE_TOUCH_CANCEL
	};
}
