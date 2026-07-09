-- Type stubs for town-level save data (Save_t). Not loaded at runtime.
-- Luau LSP loads pc/mods/types/globals.d.luau instead.

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
