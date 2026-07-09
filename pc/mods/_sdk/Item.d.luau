-- Type stubs for game item ids (mActor_name_t). Not loaded at runtime.
-- Luau LSP loads pc/mods/types/globals.d.luau instead.

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

export type ItemCondition = "normal" | "present" | "quest"

export type ItemSlot = {
	Item: Item,
	Condition: ItemCondition,
}

declare class ItemModule
	fromId: (id: number) -> Item
	empty: () -> Item
end

declare Item: ItemModule
