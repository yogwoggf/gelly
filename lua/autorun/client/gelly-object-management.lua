---@module "gelly.logging"
local logging = include("gelly/logging.lua")

local objects = {}

--- Returns a list of the individual meshes and their vertices of the given model.
---@param modelPath string
local function getVerticesOfModel(modelPath)
	local meshes = util.GetModelMeshes(modelPath, 1, 1)
	local vertices = {}

	for _, mesh in ipairs(meshes) do
		local vertsForMesh = {}
		for _, vertex in ipairs(mesh.triangles) do
			table.insert(vertsForMesh, vertex.pos)
		end

		table.insert(vertices, vertsForMesh)
	end

	return vertices
end

local function addObject(entity)
	logging.info("Adding object #%d to gelly", entity:EntIndex())

	local meshes = getVerticesOfModel(entity:GetModel())

	local objectHandles = {}
	for _, mesh in ipairs(meshes) do
		table.insert(objectHandles, gelly.AddObject(mesh))
	end

	objects[entity] = objectHandles
end

local function removeObject(entity)
	local objectHandles = objects[entity]
	logging.info("Removing object #%d from gelly", entity:EntIndex())

	for _, objectHandle in ipairs(objectHandles) do
		gelly.RemoveObject(objectHandle)
	end

	objects[entity] = nil
end

local function updateObject(entity)
	local objectHandles = objects[entity]

	for _, objectHandle in ipairs(objectHandles) do
		local transform = entity:GetWorldTransformMatrix()
		gelly.SetObjectPosition(objectHandle, transform:GetTranslation())
		gelly.SetObjectRotation(objectHandle, transform:GetAngles())
	end
end

hook.Add("GellyLoaded", "gelly.object-management-initialize", function()
	hook.Add("OnEntityCreated", "gelly.object-add", function(entity)
		if not IsValid(entity) or entity:GetClass() ~= "prop_physics" then
			return
		end

		addObject(entity)
	end)

	hook.Add("EntityRemoved", "gelly.object-remove", function(entity)
		if not objects[entity] then
			return
		end

		removeObject(entity)
	end)

	hook.Add("Think", "gelly.object-update", function()
		for entity, _ in pairs(objects) do
			updateObject(entity)
		end
	end)
end)