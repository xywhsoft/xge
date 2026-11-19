const XGE_TOUCH_BEGIN = 1;
const XGE_TOUCH_MOVE = 2;
const XGE_TOUCH_END = 4;
const XGE_TOUCH_CANCEL = 5;
const INVALID_ARGUMENT = -2;
const NOT_INITIALIZED = -3;
const OUT_OF_MEMORY = -5;
const UNSUPPORTED = -7;
const INVALID_STATE = -15;
const CALLBACK_FAILED = -16;
const RESOURCE = 1;
const AUDIO = 2;
const MAX_BYTES = 0x7fffffff;
const registries = new WeakMap();

function byteView(data) {
	if (data instanceof Uint8Array) return data;
	if (data instanceof ArrayBuffer) return new Uint8Array(data);
	return null;
}

function statusCode(value) {
	return Number.isInteger(value) && value === (value | 0) ? value : CALLBACK_FAILED;
}

function checked(operation, status) {
	if (status < 0) {
		const error = new Error(operation + " failed (" + status + ")");
		error.code = status;
		throw error;
	}
}

// Track offsets before an allocation can detach a borrowed HEAPU8 view.
function reserveBytes(module, bytes, reserve) {
	const shared = bytes.buffer === module.HEAPU8.buffer;
	const offset = bytes.byteOffset;
	const size = bytes.byteLength;
	const address = reserve(size) >>> 0;
	if (!address) return 0;
	const target = module.HEAPU8;
	const source = shared ? target.subarray(offset, offset + size) : bytes;
	target.set(source, address);
	return address;
}

function hostRegistry(module) {
	let registry = registries.get(module);
	if (registry) return registry;
	const entries = new Map();
	const host = {
		["requestFrame"](id) {
			const bridge = entries.get(id);
			if (!bridge) return INVALID_STATE;
			try { return bridge.requestFrame(); }
			catch { return CALLBACK_FAILED; }
		},
		["loadResource"](id, uri, output) {
			const bridge = entries.get(id);
			if (!bridge) return INVALID_STATE;
			try {
				const bytes = byteView(bridge.loadResource(uri));
				if (!bytes || bytes.byteLength > MAX_BYTES) return CALLBACK_FAILED;
				if (bytes.byteLength === 0) return 0;
				const address = reserveBytes(module, bytes,
					size => module._xgeMiniProgramJsResourceReserve(output, size));
				return address ? 0 : OUT_OF_MEMORY;
			} catch { return CALLBACK_FAILED; }
		},
		["audioCommand"](id, command, handle, data, size) {
			const bridge = entries.get(id);
			if (!bridge || !bridge.audioBridge) return UNSUPPORTED;
			try {
				const bytes = module.HEAPU8.subarray(data, data + size);
				return statusCode(bridge.audioBridge(command, handle, bytes));
			} catch { return CALLBACK_FAILED; }
		},
		["retire"](id) {
			const bridge = entries.get(id);
			entries.delete(id);
			if (bridge && bridge.registration === id) {
				bridge.registration = 0;
				bridge.running = false;
				bridge.framePending = null;
			}
		}
	};
	registry = {entries, nextId: 1};
	registries.set(module, registry);
	module["xgeMiniProgramHost"] = host;
	return registry;
}

class XgeMiniProgramBridge {
	constructor(module, options) {
		this.module = module;
		this.canvas = options && options.canvas ? options.canvas : null;
		this.gl = null;
		this.running = false;
		this.framePending = null;
		this.registration = 0;
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
		const install = this.module && this.module._xgeMiniProgramJsInstall;
		if (typeof install !== "function" || typeof this.module._xgeMiniProgramJsUnit !== "function" ||
			typeof this.module._xgeMiniProgramJsResourceReserve !== "function") {
			throw new Error("missing XGE wasm32 mini-program bridge exports");
		}
		const [w, h, ratio] = this._dimensions(width ?? this.canvas.width ?? 1, height ?? this.canvas.height ?? 1, dpr ?? 1);
		const gl = this.canvas.getContext("webgl2", {
			alpha: true,
			antialias: true,
			depth: true,
			stencil: false,
			premultipliedAlpha: true,
			preserveDrawingBuffer: false
		});
		if (!gl) {
			throw new Error("xge requires WebGL2 in miniprogram backend");
		}
		const registry = hostRegistry(this.module);
		if (registry.nextId > 0xffffffff) throw new Error("XGE mini-program registration IDs exhausted");
		const id = registry.nextId++;
		registry.entries.set(id, this);
		const flags = (this.resourceLoader ? RESOURCE : 0) | (this.audioBridge ? AUDIO : 0);
		const status = install(id, flags, w, h, ratio);
		// A checked error from the previous owner's retirement can follow publication.
		if (registry.entries.has(id)) {
			this.registration = id;
			this.gl = gl;
			this._resizeCanvas(w, h, ratio);
		}
		checked("install mini-program bridge", status);
		if (this.registration !== id) checked("install mini-program bridge", INVALID_STATE);
		return gl;
	}

	unit() {
		const id = this.registration;
		if (id === 0) return;
		const status = this.module._xgeMiniProgramJsUnit(id);
		if (status === 0 && this.registration === id) {
			// An in-flight callback may still own the registered JS entry.
			this.registration = 0;
			this.stop();
		}
		checked("close mini-program bridge", status);
	}

	start() {
		if (!this.registration) checked("start mini-program bridge", NOT_INITIALIZED);
		this.running = true;
		const status = this.requestFrame();
		if (status < 0) this.stop();
		checked("schedule mini-program frame", status);
	}

	stop() {
		this.running = false;
		this.framePending = null;
	}

	requestFrame() {
		if (!this.running || this.framePending) {
			return 0;
		}
		const ticket = {};
		this.framePending = ticket;
		const request = typeof this.canvas.requestAnimationFrame === "function"
			? this.canvas.requestAnimationFrame.bind(this.canvas)
			: typeof requestAnimationFrame === "function" ? requestAnimationFrame
			: fn => setTimeout(() => fn(typeof performance === "object" ? performance.now() : Date.now()), 16);
		try {
			request(time => {
				if (this.framePending !== ticket) return;
				this.framePending = null;
				const status = this._ccall("xgeMiniProgramFrame", "number", ["number"], [time / 1000]);
				if (status < 0) this.stop();
				checked("run mini-program frame", status);
				checked("schedule mini-program frame", this.requestFrame());
			});
		} catch {
			this.framePending = null;
			return CALLBACK_FAILED;
		}
		return 0;
	}

	_dimensions(width, height, dpr) {
		const w = Math.max(1, Math.floor(width));
		const h = Math.max(1, Math.floor(height));
		const ratio = Math.fround(dpr);
		if (!Number.isFinite(w) || !Number.isFinite(h) || w > MAX_BYTES || h > MAX_BYTES ||
			!Number.isFinite(ratio) || ratio <= 0 || w * ratio < 1 || h * ratio < 1 ||
			w * ratio > MAX_BYTES || h * ratio > MAX_BYTES) {
			checked("set mini-program canvas size", INVALID_ARGUMENT);
		}
		return [w, h, ratio];
	}

	_resizeCanvas(width, height, dpr) {
		this.width = width;
		this.height = height;
		this.dpr = dpr;
		if (this.canvas) {
			this.canvas.width = Math.max(1, Math.floor(this.width * this.dpr));
			this.canvas.height = Math.max(1, Math.floor(this.height * this.dpr));
		}
		if (this.gl) {
			this.gl.viewport(0, 0, this.canvas.width, this.canvas.height);
		}
	}

	resize(width, height, dpr) {
		if (!this.registration) return NOT_INITIALIZED;
		let size;
		try { size = this._dimensions(width, height, dpr); }
		catch { return INVALID_ARGUMENT; }
		const status = this._ccall("xgeMiniProgramResize", "number", ["number", "number", "number"], size);
		if (status === 0) this._resizeCanvas(...size);
		return status;
	}

	touch(phase, touches) {
		if (!this.registration) return NOT_INITIALIZED;
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
		if (!this.registration) return NOT_INITIALIZED;
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
				if (codepoint) {
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
		if (!this.registration) return NOT_INITIALIZED;
		if (!this.audioBridge) return UNSUPPORTED;
		if (command !== (command | 0) || handle !== (handle | 0)) return INVALID_ARGUMENT;
		const bytes = typeof data === "string" ? new TextEncoder().encode(data) : byteView(data);
		if (data != null && !bytes) return INVALID_ARGUMENT;
		const size = bytes ? bytes.byteLength : 0;
		if (size > MAX_BYTES) return INVALID_ARGUMENT;
		let address = 0;
		try {
			if (size) {
				reserveBytes(this.module, bytes, size => {
					address = this._malloc(size) >>> 0;
					return address;
				});
				if (!address) return OUT_OF_MEMORY;
			}
			return this._ccall("xgeMiniProgramAudioCommand", "number", ["number", "number", "number", "number"],
				[command, handle, address, size]);
		} finally {
			this._free(address);
		}
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
