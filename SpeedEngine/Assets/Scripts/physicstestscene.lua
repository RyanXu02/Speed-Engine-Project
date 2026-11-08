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
		for i = 1, 10 do
			local activeScene = SceneSystem.getCurrentScene()
		
			local entityName = "cube_" .. self.numAdded
			local id = activeScene:addEntity(entityName)
			local entityNew = activeScene:getEntity(id)
		
			entityNew:addComponent("MeshComponent", {})
			local mesh = entityNew:getComponent("MeshComponent")
			if self.numAdded % 2 == 0 then
				mesh:setPath("Assets/Meshes/Cube.obj")
			else
				mesh:setPath("Assets/Meshes/Ball.glb")
			end

			entityNew:addComponent("RigidBody", {})
			local rigidBody = entityNew:getComponent("RigidBody")
			rigidBody:setRestitution(0.5)
			rigidBody:activateBody()

			if self.numAdded % 2 == 0 then
				entityNew:addComponent("PhysicsShapeBox", {})
				local boxCollider = entityNew:getComponent("PhysicsShapeBox")
				boxCollider:setHalfExtents(vec3(0.5, 0.5, 0.5))
			else
				entityNew:addComponent("PhysicsShapeSphere", {})
				local sphereCollider = entityNew:getComponent("PhysicsShapeSphere")
				sphereCollider:setRadius(0.5)
			end

			local transform = entityNew:getComponent("Transform")
			transform.translate = vec3((self.numAdded % 10 - 5.0), 15 + (self.numAdded // 100), (self.numAdded // 10) % 10)
			transform.scale = vec3(0.5, 0.5, 0.5)

			self.numAdded = self.numAdded + 1
		end
	end
end

function physicstestscene:onDestroy()
end
