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
	-- Plays a player body animation by index (0 .. GetAnimationCount()-1) or name
	-- (e.g. "light_on1", "WAIT1"). Puts the player in wait, then applies the clip
	-- after the actor update so it is not overwritten. Stick input returns control.
	-- Options: loop (default false), speed (default 0.5), morph (default -5).
	PlayAnimation: (anim: number | string, options: { loop: boolean?, speed: number?, morph: number? }?) -> boolean,
	GetAnimationName: (index: number) -> string?,
	GetAnimationCount: () -> number,
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

export type QuestType = "delivery" | "errand" | "contest" | "none" | string

export type QuestTimeLimit = {
	year: number,
	month: number,
	day: number,
	hour: number,
	minute: number,
	second: number,
}

export type DeliveryQuest = {
	-- 1-based pocket/delivery slot
	Index: number,
	Type: QuestType,
	Kind: number,
	Progress: number,
	TimeLimitEnabled: boolean,
	TimeLimit: QuestTimeLimit?,
	IsActive: boolean,
	RecipientNpcId: number?,
	SenderNpcId: number?,
	Clear: (self: DeliveryQuest) -> (),
}

export type ErrandQuest = {
	-- 1-based errand slot
	Index: number,
	Type: QuestType,
	Kind: number,
	Progress: number,
	TimeLimitEnabled: boolean,
	TimeLimit: QuestTimeLimit?,
	IsActive: boolean,
	RecipientNpcId: number?,
	SenderNpcId: number?,
	Item: Item,
	-- 1-based pocket index, or nil
	PocketsIdx: number?,
	ErrandType: number,
	Clear: (self: ErrandQuest) -> (),
}

export type ContestQuest = {
	AnimalIndex: number,
	Type: QuestType,
	Kind: number,
	Progress: number,
	TimeLimitEnabled: boolean,
	TimeLimit: QuestTimeLimit?,
	IsActive: boolean,
	RequestedItem: Item,
	PlayerName: string?,
	ContestDataType: number,
	Clear: (self: ContestQuest) -> (),
}

export type Private = {
	Index: number,
	Valid: boolean,
	Wallet: number,
	Name: string,
	Loan: number,
	Pockets: Pockets,
	Equipped: Item,
	-- 1-based; slot i mirrors pocket i for delivery quests
	Deliveries: { [number]: DeliveryQuest },
	Errands: { [number]: ErrandQuest },
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
	Contest: ContestQuest,
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

declare Message: {
	-- True while dialogue text is actively on screen.
	IsOpen: () -> boolean,
	-- True when the window is fully closed (HIDE or WAIT) and can be opened.
	CanOpen: () -> boolean,
	-- Current message table index, or 0 outside gameplay.
	GetMsgId: () -> number,
	-- Opens from HIDE (cold start) or WAIT (after Close). clearFlag clears cursor state on WAIT reopen.
	Open: (clearFlag: boolean?) -> boolean,
	-- Closes an active message. Returns false if nothing is open or the request is rejected.
	Close: () -> boolean,
	-- Loads a message by table index without opening the window. Returns false if the id is invalid.
	SetMsgId: (msgId: number) -> boolean,
	-- SetMsgId then Open. Returns false if either step fails.
	OpenWithId: (msgId: number) -> boolean,
}

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

-- In-game RTC clock. Weekday: 0=Sunday .. 6=Saturday. Month: 1-12.
declare Time: {
	GetSecond: () -> number,
	GetMinute: () -> number,
	GetHour: () -> number,
	GetDay: () -> number,
	GetMonth: () -> number,
	GetYear: () -> number,
	GetWeekday: () -> number,
	-- year, month, day
	GetDate: () -> (number, number, number),
	-- hour, minute, second
	GetTime: () -> (number, number, number),
	-- year, month, day, hour, minute, second
	GetDateTime: () -> (number, number, number, number, number, number),

	-- Field setters keep other components. Returns false if the resulting clock is invalid.
	SetSecond: (second: number) -> boolean,
	SetMinute: (minute: number) -> boolean,
	SetHour: (hour: number) -> boolean,
	SetDay: (day: number) -> boolean,
	SetMonth: (month: number) -> boolean,
	SetYear: (year: number) -> boolean,
	SetDate: (year: number, month: number, day: number) -> boolean,
	-- second defaults to 0 when omitted.
	SetTime: (hour: number, minute: number, second: number?) -> boolean,
	SetDateTime: (year: number, month: number, day: number, hour: number, minute: number, second: number?) -> boolean,
}

-- Lighthouse period: 0=start day, 1=switch week, 2=reward window, 3=none
-- Event: 0=none, 1=jan contributed, 2=jan started, 3=feb contributed, 4=feb started
declare Quest: {
	Lighthouse: {
		GetPeriod: () -> number,
		IsActive: () -> boolean,
		GetDay: () -> number,
		GetRenewDate: () -> (number, number, number),
		GetDaysOn: () -> number,
		Start: () -> boolean,
		CanSwitch: () -> boolean,
		SwitchOn: () -> boolean,
		PlayerStarted: (playerIndex: number?) -> boolean,
		PlayerContributed: (playerIndex: number?) -> boolean,
		PlayerCompleted: (playerIndex: number?) -> boolean,
		GetEvent: (playerIndex: number?) -> number,
		ClearEvent: (playerIndex: number?) -> boolean,
		GetPresentItem: (playerIndex: number?) -> number,
		Clear: () -> boolean,
		SetupForSwitch: () -> boolean,
	},
}
