-- Cornell Box

function startup()
	push_transform()
		--translate({ 0, 0, -4})
		--scale({ 4.2, 2.7, 1 })
		quad_mesh({ 1, 1, 1 })
		pop_transform()
	push_transform()
		translate({-1, 0, 0})
	--	translate({-4, 0, 0})
		rotate({0, 1, 0}, 90)
	--	scale({4.2, 2.7, 1})
		quad_mesh({ 1, 0, 0 })
		pop_transform()
	--push_transform()
	--	translate({4, 0, -2})
	--	rotate({0, 1, 0}, -90)
	--	scale({4.2, 2.7, 1})
	--	quad_mesh({ 0, 1, 0 })
	--	pop_transform()
	--push_transform()
	--	translate({0, 2.5, -2})
	--	rotate({1, 0, 0}, 90)
	--	scale({4.2, 4.2, 1})
	--	quad_mesh({ 1, 1, 0 })
	--	pop_transform()
	--push_transform()
	--	translate({ 0, -2.5, -2 })
	--	rotate({ 1, 0, 0 }, -90)
	--	scale({4.2, 4.2, 1})
	--	quad_mesh({ 0, 0, 1 })
	--	pop_transform()
	perspective({0, 0, 4}, { 0, 0, 0 }, { 0, 1, 0 }, 60)
	point_light({ 0, 2, 4 }, { 1, 1, 1 })
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
