-- CORNELL BOX
-- Default box is 2x2x2 centered at the origin
-- X component of every tranlation gets multiplied by aspect ratio
-- Z-location of camera is 1 (outmost edge of the box is at Z=+1) + ctg(vertFov/2)
-- For 60 degrees Zc = 1 + ctg(30deg) = 1 + 1.732 = 2.732

function startup()
	push_transform()
		translate({ 0, 0, -1})
		quad_mesh({ 1, 1, 1 })
		pop_transform()
	push_transform()
		translate({-1, 0, 0})
		rotate({0, 1, 0}, 90)
		quad_mesh({ 1, 0, 0 })
		pop_transform()
	push_transform()
		translate({1, 0, 0})
		rotate({0, 1, 0}, -90)
		quad_mesh({ 0, 1, 0 })
		pop_transform()
	push_transform()
		translate({0, 1, 0})
		rotate({1, 0, 0}, 90)
		quad_mesh({ 1, 1, 1 })
		pop_transform()
	push_transform()
		translate({ 0, -1, 0 })
		rotate({ 1, 0, 0 }, -90)
		quad_mesh({ 1, 1, 1 })
		pop_transform()
	local vFov = 60.0
	local Zc = 1.0 + math_ctg(vFov * 0.5)
	perspective({ 0, 0, Zc }, { 0, 0, 0 }, { 0, 1, 0 }, vFov)
	point_light({ 0, 0.95, 0 }, { 1, 1, 1 }, { 1, 0, 0.3 })
end

function update()
	--
end

function shutdown()
	--
end

__main__ = {
	startup,
	update,
	shutdown
}
