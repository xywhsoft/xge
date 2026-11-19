const assert = require("node:assert/strict");
const path = require("node:path");
const createModule = require(path.resolve(process.argv[2]));
const { XgeMiniProgramBridge } = require("../../platform/miniprogram/xge_miniprogram.js");

function canvas() {
	const frames = [];
	const gl = { viewport() {} };
	return { width: 64, height: 32, frames, gl, getContext: () => gl,
		requestAnimationFrame: callback => frames.push(callback) };
}

async function main() {
	const wasm = await createModule();
	assert.equal(wasm._testEngineInit(), 0);
	const load = uri => wasm.ccall("testResourceLoad", "number", ["string"], [uri]);
	const request = () => wasm._xgeMiniProgramRequestFrame();
	const payload = () => Array.from({length: wasm._testResourceSize()}, (_, i) => wasm._testResourceByte(i));
	const firstCanvas = canvas();
	let audioCalls = 0;
	let returnedAudio = 17;
	let expectedAudio = [97, 0, 255];
	const first = new XgeMiniProgramBridge(wasm, {
		canvas: firstCanvas,
		resourceLoader(uri) {
			if (uri.endsWith("empty")) return new Uint8Array();
			if (uri.endsWith("invalid")) return Promise.resolve(new Uint8Array());
			if (uri.endsWith("throws")) throw new Error("loader rejected");
			assert.equal(uri, "res://中文");
			return Uint8Array.of(97, 0, 255);
		},
		audioBridge(command, handle, data) {
			++audioCalls;
			assert.equal(command, 2);
			assert.equal(handle, 3);
			assert.equal(data.buffer, wasm.HEAPU8.buffer);
			assert.deepEqual(Array.from(data), expectedAudio);
			return returnedAudio;
		}
	});
	assert.equal(first.init(64, 32, 2), firstCanvas.gl);
	assert.equal(firstCanvas.width, 128);
	assert.equal(wasm._xgeGetWidth(), 128);
	assert.equal(load("res://中文"), 0);
	assert.deepEqual(payload(), [97, 0, 255]);
	assert.equal(load("res://empty"), 0);
	assert.deepEqual(payload(), []);
	assert.equal(load("res://invalid"), -16);
	assert.deepEqual(payload(), []);
	assert.equal(load("res://throws"), -16);
	assert.deepEqual(payload(), []);
	assert.equal(wasm._testMemoryFail(0), 1);
	assert.equal(load("res://中文"), -5);
	assert.equal(wasm._testMemoryFail(-1), 1);
	assert.deepEqual(payload(), []);

	assert.equal(first.audioCommand(2, 3, Uint8Array.of(97, 0, 255)), 17);
	returnedAudio = -11;
	assert.equal(first.audioCommand(2, 3, Uint8Array.of(97, 0, 255)), -11);
	assert.equal(audioCalls, 2);
	assert.equal(first.audioCommand(2, 3, {}), -2);
	assert.equal(audioCalls, 2);
	expectedAudio = [228, 184, 173, 0, 97];
	returnedAudio = 4;
	assert.equal(first.audioCommand(2, 3, "中\0a"), 4);
	assert.equal(audioCalls, 3);

	let rendered = 0;
	const nativeFrame = wasm._xgeMiniProgramFrame;
	wasm._xgeMiniProgramFrame = time => { ++rendered; return nativeFrame(time); };
	first.start();
	assert.equal(request(), 0);
	assert.equal(firstCanvas.frames.length, 1);
	first.stop();
	first.start();
	assert.equal(firstCanvas.frames.length, 2);
	firstCanvas.frames.shift()(10);
	assert.equal(rendered, 0);
	assert.equal(request(), 0);
	assert.equal(firstCanvas.frames.length, 1);
	firstCanvas.frames.shift()(20);
	assert.equal(rendered, 1);
	assert.equal(firstCanvas.frames.length, 1);

	const secondCanvas = canvas();
	const second = new XgeMiniProgramBridge(wasm, {
		canvas: secondCanvas,
		resourceLoader() { return Uint8Array.of(42); }
	});
	for (let budget = 0; budget < 4; ++budget) {
		assert.equal(wasm._testMemoryFail(budget), 1);
		assert.throws(() => second.init(81, 37, 1), { code: -5 });
		assert.equal(wasm._testMemoryFail(-1), 1);
		assert.equal(wasm._xgeGetWidth(), 128);
		assert.equal(load("res://中文"), 0);
		assert.deepEqual(payload(), [97, 0, 255]);
	}

	first.audioBridge = () => {
		second.init(80, 40, 1);
		first.unit();
		return 29;
	};
	assert.equal(first.audioCommand(2, 3, new Uint8Array()), 29);
	assert.equal(first.registration, 0);
	assert.equal(wasm._xgeGetWidth(), 80);
	assert.deepEqual(payload(), [97, 0, 255]);
	firstCanvas.frames.shift()(30);
	assert.equal(rendered, 1);
	assert.equal(second.audioCommand(2, 3, new Uint8Array()), -7);
	assert.equal(load("res://中文"), 0);
	assert.deepEqual(payload(), [42]);

	// Return a view of the old linear memory and grow it while reserving the reply.
	const offset = wasm._malloc(4);
	wasm.HEAPU8.set([0, 127, 0, 255], offset);
	second.resourceLoader = () => wasm.HEAPU8.subarray(offset, offset + 4);
	const reserve = wasm._xgeMiniProgramJsResourceReserve;
	let grew = false;
	wasm._xgeMiniProgramJsResourceReserve = (output, size) => {
		const oldHeap = wasm.HEAPU8.buffer;
		const large = wasm._malloc(wasm.HEAPU8.byteLength + 65536);
		assert.ok(large);
		wasm._free(large);
		grew = wasm.HEAPU8.buffer !== oldHeap;
		return reserve(output, size);
	};
	assert.equal(load("res://shared"), 0);
	assert.ok(grew);
	assert.deepEqual(payload(), [0, 127, 0, 255]);
	wasm._xgeMiniProgramJsResourceReserve = reserve;
	wasm._free(offset);
	for (let i = 0; i < 32; ++i) second.init(80, 40, 1);
	assert.equal(wasm._testEngineUnit(), 0);
	assert.equal(second.registration, 0);
	assert.equal(request(), -7);
	assert.equal(second.resize(16, 16, 1), -3);
	console.log("XGE_MINIPROGRAM_WASM_OK");
}

main().catch(error => { console.error(error); process.exitCode = 1; });
