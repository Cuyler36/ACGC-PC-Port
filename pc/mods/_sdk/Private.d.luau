-- Type stubs for player private save data (Private_c). Not loaded at runtime.
-- Luau LSP loads pc/mods/types/globals.d.luau instead.

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
