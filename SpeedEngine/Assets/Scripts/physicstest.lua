physicstest = {}
physicstest.__index = physicstest

function physicstest.new(EntityWrapper)
	local instance = {
		entity = EntityWrapper
	}

	return setmetatable(instance, physicstest)
end

function physicstest:onInit()
end

function physicstest:onUpdate(dt)
	if not Engine.isPlaying() then
		return
	end

	if Input.isKeyDown(KeyCodes.KEY_SPACE) then
		local rigidbody = self.entity:getComponent("RigidBody")
		rigidbody:addForce(vec3(0.0, 100000.0, 0.0))
	end
end

function physicstest:onDestroy()
end
