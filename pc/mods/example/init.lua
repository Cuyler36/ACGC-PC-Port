bind.load(function()
end)

bind.init(function()
end)

bind.postmove(function()
	local p = player.get()
	if p and actor.is_valid(p) then
		local x, y, z = actor.get_position(p)
	end
end)
