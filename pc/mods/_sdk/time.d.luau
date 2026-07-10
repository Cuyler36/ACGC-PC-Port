-- Type stubs for the in-game clock API. Not loaded at runtime.
-- Luau LSP loads pc/mods/types/globals.d.luau instead.
-- Weekday: 0=Sunday .. 6=Saturday. Month: 1-12. Year: typically 2001-2099.

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
