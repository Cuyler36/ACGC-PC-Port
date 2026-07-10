-- Type stubs for the player mod API (not loaded at runtime).
-- Luau LSP loads pc/mods/types/globals.d.luau instead.

declare player: {
	get: () -> Actor?,
	get_item_kind: () -> number?,
	-- Plays a player body animation by index (0 .. GetAnimationCount()-1) or name
	-- (e.g. "light_on1", "WAIT1"). Puts the player in wait, then applies the clip
	-- after the actor update so it is not overwritten. Stick input returns control.
	-- Options: loop (default false), speed (default 0.5), morph (default -5).
	PlayAnimation: (anim: number | string, options: { loop: boolean?, speed: number?, morph: number? }?) -> boolean,
	GetAnimationName: (index: number) -> string?,
	GetAnimationCount: () -> number,
}
