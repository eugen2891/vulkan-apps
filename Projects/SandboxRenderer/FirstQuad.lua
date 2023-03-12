-- First Quad

function startup()
	quad_mesh()
	perspective({0, 0, 4}, { 0, 0, 0 }, { 0, 1, 0 }, 60)
end

__main__ = {
	startup,
	nil,
	nil
}
