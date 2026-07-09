-- AC PC Port mod API globals (loaded by luau-lsp only; not executed at runtime).

export type Callback = () -> ()
export type FrameCallback = (dt: number) -> ()

export type Actor = {}
export type ActorPart = "fg" | "item" | "unused" | "player" | "npc" | "bg" | "effect" | "control"
export type ActorIter = () -> Actor?
export type Vec3 = { x: number, y: number, z: number }
export type BlockPos = { x: number, z: number }

export type ActorMakeOptions = {
	rotation: { number }?,
	block: { number }?,
	npc_id: number?,
	villager_id: number?,
	arg: number?,
}

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

declare actor: {
	get_player: () -> Actor?,
	is_valid: (actor: Actor) -> boolean,
	get_profile: (actor: Actor) -> number,
	get_part: (actor: Actor) -> ActorPart,
	get_npc_id: (actor: Actor) -> number,
	get_position: (actor: Actor) -> (number, number, number),
	set_position: (actor: Actor, x: number, y: number, z: number) -> (),
	get_rotation: (actor: Actor) -> (number, number, number),
	set_rotation: (actor: Actor, x: number, y: number, z: number) -> (),
	get_distance_to_player: (actor: Actor) -> number,
	each: (part: ActorPart | number, callback: (actor: Actor) -> boolean) -> (),
	iter: (part: ActorPart | number) -> ActorIter,
	make: (profile: number, x: number, y: number, z: number, options: ActorMakeOptions?) -> Actor?,
}

declare player: {
	get: () -> Actor?,
	get_item_kind: () -> number?,
}

export type ItemKind =
	| "item0"
	| "furniture"
	| "item1"
	| "warp"
	| "struct"
	| "item2"
	| "actor"
	| "props"
	| "spnpc"
	| "npc"
	| "unknown"

export type ItemCategory =
	| "paper"
	| "money"
	| "tool"
	| "fish"
	| "cloth"
	| "etc"
	| "carpet"
	| "wall"
	| "fruit"
	| "plant"
	| "minidisk"
	| "dummy"
	| "ticket"
	| "insect"
	| "hukubukuro"
	| "kabu"

export type Item = {
	Id: number,
	Name: string,
	FullName: string,
	IsEmpty: boolean,
	Kind: ItemKind,
	Category: ItemCategory?,
	Article: string,
	GetFullName: (self: Item) -> string,
}

declare class ItemModule
	fromId: (id: number) -> Item
	empty: () -> Item
end

declare Item: ItemModule

export type ItemCondition = "normal" | "present" | "quest"

export type ItemSlot = {
	Item: Item,
	Condition: ItemCondition,
}

export type Pockets = { [number]: ItemSlot }

export type Private = {
	Index: number,
	Valid: boolean,
	Wallet: number,
	Name: string,
	Loan: number,
	Pockets: Pockets,
	Equipped: Item,
}

declare Private: {
	fromIndex: (index: number) -> Private?,
	current: () -> Private?,
	isReady: () -> boolean,
}

export type AnimalHome = {
	-- 1-based acre coordinates and unit position; 255 means unset.
	bx: number,
	bz: number,
	ux: number,
	uz: number,
}

export type Animal = {
	Index: number,
	Valid: boolean,
	NpcId: number,
	Name: string,
	Home: AnimalHome,
	-- Errors if this is the last villager. Invalidates this handle afterward.
	Delete: (self: Animal) -> (),
}
export type AnimalIter = () -> Animal?

declare class AnimalModule
	-- npcId: villager species id (NAME_TYPE_NPC, e.g. 0xE05D).
	-- Adds save data, builds a house on a reserve plot, and syncs runtime town state.
	-- Errors if save is not ready, the id is invalid/duplicate, town is full, or house placement fails.
	-- bypassValidation: skip pre-checks and house-build rollback; villager may be left without a home.
	new: (npcId: number, bypassValidation: boolean?) -> Animal
	iter: () -> AnimalIter
	fromIndex: (index: number) -> Animal?
	count: () -> number
	function __iter(self): AnimalIter
end

declare Animal: AnimalModule

export type TownMelody = { number } -- 16 town tune notes, each 0-15

export type Town = {
	Ready: boolean,
	Name: string,
	Id: number,
	VillagerCount: number,
	Fruit: number,
	Melody: TownMelody,
}

declare Town: {
	current: () -> Town?,
	isReady: () -> boolean,
}
