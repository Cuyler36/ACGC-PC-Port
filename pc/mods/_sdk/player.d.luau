-- Type stubs for the player mod API (not loaded at runtime).
-- Luau LSP loads pc/mods/types/globals.d.luau instead.

declare player: {
	get: () -> Actor?,
	get_item_kind: () -> number?,
}
