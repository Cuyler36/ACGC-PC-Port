-- Type stubs for the actor mod API (not loaded at runtime).
-- Luau LSP loads pc/mods/types/globals.d.luau instead (definition files cannot use `return`).

export type Actor = {}
export type ActorPart = "fg" | "item" | "unused" | "player" | "npc" | "bg" | "effect" | "control"
export type Vec3 = (number, number, number)
export type BlockPos = (number, number)

export type ActorMakeOptions = {
	rotation: Vec3?,
	block: BlockPos?,
	npc_id: number?,
	villager_id: number?,
	arg: number?,
}

declare actor: {
	get_player: () -> Actor?,
	is_valid: (actor: Actor) -> boolean,
	get_profile: (actor: Actor) -> number,
	get_part: (actor: Actor) -> ActorPart,
	get_npc_id: (actor: Actor) -> number,
	get_position: (actor: Actor) -> Vec3,
	set_position: (actor: Actor, x: number, y: number, z: number) -> (),
	get_rotation: (actor: Actor) -> Vec3,
	set_rotation: (actor: Actor, x: number, y: number, z: number) -> (),
	get_distance_to_player: (actor: Actor) -> number,
	each: (part: ActorPart | number, callback: (actor: Actor) -> boolean) -> (),
	iter: (part: ActorPart | number) -> (() -> Actor?),
	make: (profile: number, x: number, y: number, z: number, options: ActorMakeOptions?) -> Actor?,
}
