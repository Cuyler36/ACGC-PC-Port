-- Type stubs for the Quest API. Not loaded at runtime.
-- Luau LSP loads pc/mods/types/globals.d.luau instead.
--
-- Typed quests (Delivery / Errand / Contest) hang off Private / Animal.
-- Town-global systems live under Quest.<Name> (e.g. Quest.Lighthouse).

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

-- Lighthouse period: 0=start day, 1=switch week, 2=reward window, 3=none
-- Event: 0=none, 1=jan contributed, 2=jan started, 3=feb contributed, 4=feb started
declare Quest: {
	Lighthouse: {
		GetPeriod: () -> number,
		IsActive: () -> boolean,
		-- Day index within the switch week (0-6).
		GetDay: () -> number,
		-- renew_time year, month, day (day 0 means inactive).
		GetRenewDate: () -> (number, number, number),
		-- Bitmask of days the switch was flipped (bits 0-6).
		GetDaysOn: () -> number,
		-- Clears prior data, sets renew_time to today, marks current player started.
		Start: () -> boolean,
		-- True when the player can flip the switch tonight (period 1, 18-21, not yet today).
		CanSwitch: () -> boolean,
		SwitchOn: () -> boolean,
		PlayerStarted: (playerIndex: number?) -> boolean,
		PlayerContributed: (playerIndex: number?) -> boolean,
		PlayerCompleted: (playerIndex: number?) -> boolean,
		GetEvent: (playerIndex: number?) -> number,
		ClearEvent: (playerIndex: number?) -> boolean,
		-- Present furniture item id for the current renew month.
		GetPresentItem: (playerIndex: number?) -> number,
		Clear: () -> boolean,
		-- Start quest, advance to next day at 20:00, reset switch bits, turn light off.
		SetupForSwitch: () -> boolean,
	},
}
