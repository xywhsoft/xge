export interface XgeMiniProgramBridgeOptions {
	canvas: any;
	resourceLoader?: (uri: string) => ArrayBuffer | Uint8Array;
	audioBridge?: (command: number, handle: number, data?: ArrayBuffer | Uint8Array | object) => number;
}

export interface XgeMiniProgramTouch {
	id: number;
	x: number;
	y: number;
	force?: number;
}

export class XgeMiniProgramBridge {
	constructor(module: any, options: XgeMiniProgramBridgeOptions);
	init(width: number, height: number, dpr: number): any;
	unit(): void;
	start(): void;
	stop(): void;
	requestFrame(): number;
	resize(width: number, height: number, dpr: number): number;
	touch(phase: number, touches: XgeMiniProgramTouch[]): number;
	attachTouchEvents(target?: any): void;
	text(codepoint: number): number;
	attachTextInput(input: any): void;
	loadResource(uri: string): ArrayBuffer | Uint8Array;
	audioCommand(command: number, handle: number, data?: ArrayBuffer | Uint8Array | object): number;
}

export const XGE_TOUCH_BEGIN: number;
export const XGE_TOUCH_MOVE: number;
export const XGE_TOUCH_END: number;
export const XGE_TOUCH_CANCEL: number;
