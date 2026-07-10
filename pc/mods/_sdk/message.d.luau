-- Type stubs for the in-game message window API. Not loaded at runtime.
-- Luau LSP loads pc/mods/types/globals.d.luau instead.

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
