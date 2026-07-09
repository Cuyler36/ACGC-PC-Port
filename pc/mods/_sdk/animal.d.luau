-- Type stubs for villager save data (Animal_c). Not loaded at runtime.
-- Luau LSP loads pc/mods/types/globals.d.luau instead.

export type AnimalHome = {
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
}
export type AnimalIter = () -> Animal?

declare class AnimalModule
	iter: () -> AnimalIter
	fromIndex: (index: number) -> Animal?
	count: () -> number
	function __iter(self): AnimalIter
end

declare Animal: AnimalModule
