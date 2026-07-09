-- Ambient type stubs for the PC port mod API (not loaded at runtime).
-- Luau LSP loads pc/mods/types/globals.d.luau instead.

export type Callback = () -> ()

declare bind: {
	load: (callback: Callback) -> (),
	init: (callback: Callback) -> (),
	beginframe: (callback: Callback) -> (),
	endframe: (callback: Callback) -> (),
	premove: (callback: Callback) -> (),
	postmove: (callback: Callback) -> (),
	predraw: (callback: Callback) -> (),
	postdraw: (callback: Callback) -> (),
}
