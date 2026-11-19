// Link with emcc --js-library. Fixed imports require no dynamic table entries.
mergeInto(LibraryManager.library, {
	xgeMiniProgramJsOnFrame: function(id) {
		return Module["xgeMiniProgramHost"]["requestFrame"](id >>> 0);
	},
	xgeMiniProgramJsOnResource__deps: ["$UTF8ToString"],
	xgeMiniProgramJsOnResource: function(id, uri, size, output) {
		return Module["xgeMiniProgramHost"]["loadResource"](id >>> 0, UTF8ToString(uri >>> 0, size >>> 0), output >>> 0);
	},
	xgeMiniProgramJsOnAudio: function(id, command, handle, data, size) {
		return Module["xgeMiniProgramHost"]["audioCommand"](id >>> 0, command, handle, data >>> 0, size >>> 0);
	},
	xgeMiniProgramJsOnRetired: function(id) {
		Module["xgeMiniProgramHost"]["retire"](id >>> 0);
	}
});
