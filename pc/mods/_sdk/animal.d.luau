-- Type stubs for villager save data (Animal_c). Not loaded at runtime.
-- Luau LSP loads pc/mods/types/globals.d.luau instead.

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
