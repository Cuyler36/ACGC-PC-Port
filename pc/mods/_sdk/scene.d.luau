-- Type stubs for scene warp API. Not loaded at runtime.
-- Luau LSP loads pc/mods/types/globals.d.luau instead.

export type GotoSceneOptions = {
	-- Spawn position in the destination scene. Defaults to a known entry point.
	x: number?,
	y: number?,
	z: number?,
	-- Exit facing (mSc_DIRECT_*, 0-7). Default depends on the scene.
	facing: number?,
	-- Wipe type (WIPE_TYPE_*). Default is fade-to-black.
	wipe: number?,
	-- If true, request invade player mode during the transition.
	updatePlayer: boolean?,
}

-- Scene id (0 .. SCENE_NUM-1) or a friendly name such as "fg", "post_office",
-- "museum", "nooks_cranny", "needlework". Names are case-insensitive; optional
-- "scene_" prefix and hyphens/spaces are accepted.
declare function gotoscene(scene: number | string, options: GotoSceneOptions?): boolean
