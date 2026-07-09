-- Ambient type stubs for the PC port mod API (not loaded at runtime).
-- Luau LSP loads pc/mods/types/globals.d.luau instead.

export type Callback = () -> ()
export type FrameCallback = (dt: number) -> ()

declare bind: {
	load: (callback: Callback) -> (),
	init: (callback: Callback) -> (),
	-- After save data is loaded and town init completes (mSDI_StartInitAfter).
	saveload: (callback: Callback) -> (),
	beginframe: (callback: FrameCallback) -> (),
	endframe: (callback: FrameCallback) -> (),
	premove: (callback: FrameCallback) -> (),
	postmove: (callback: FrameCallback) -> (),
	predraw: (callback: FrameCallback) -> (),
	postdraw: (callback: FrameCallback) -> (),
}
