physicstestscene = {}
physicstestscene.__index = physicstestscene

function physicstestscene.new(EntityWrapper)
	local instance = {
		entity = EntityWrapper,
		numAdded = 0
	}

	return setmetatable(instance, physicstestscene)
end

function physicstestscene:onInit()
end

function physicstestscene:onUpdate(dt)
	if not Engine.isPlaying() then
		return
	end

	if Input.isKeyDown(KeyCodes.KEY_C) then
		local activeScene = SceneSystem.getCurrentScene()
		
		local entityName = "cube_" .. self.numAdded
		local id = activeScene:addEntity(entityName)
		local entityNew = activeScene:getEntity(id)
		
		entityNew:addComponent("MeshComponent", {})
		local mesh = entityNew:getComponent("MeshComponent")
		mesh:setPath("Assets/Meshes/Cube.obj")

		entityNew:addComponent("RigidBody", {})
		local rigidBody = entityNew:getComponent("RigidBody")
		rigidBody:setRestitution(0.5)
		rigidBody:activateBody()

		entityNew:addComponent("PhysicsShapeBox", {})
		local boxCollider = entityNew:getComponent("PhysicsShapeBox")
		boxCollider:setHalfExtents(vec3(0.5, 0.5, 0.5))

		local transform = entityNew:getComponent("Transform")
		transform:setPosition(vec3((self.numAdded % 10 - 5.0), 15 + (self.numAdded // 100), (self.numAdded // 10) % 10))
		transform:setScale(vec3(0.5, 0.5, 0.5))

		self.numAdded = self.numAdded + 1
	end
end

function physicstestscene:onDestroy()
end
