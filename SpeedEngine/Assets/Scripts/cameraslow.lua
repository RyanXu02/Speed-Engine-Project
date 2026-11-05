cameraslow = {}
cameraslow.__index = cameraslow

function cameraslow.new(EntityWrapper)
	local instance = {
		entity = EntityWrapper,
		center = vec3(0.0, 0.0, 0.0),
		radius = 20.0,
		height = 10.0,
		speed = 0.01,
		angle = 0.0
	}

	return setmetatable(instance, cameraslow)
end

function cameraslow:onInit()
	local cam = self.entity:getComponent("CameraComponent")
	EventSystem.publishSwitchCamera(cam.frustum)
end

function cameraslow:onUpdate(dt)
	if not Engine.isPlaying() then
		return
	end

	self.angle = self.angle + self.speed * dt

	local cam = self.entity:getComponent("CameraComponent")
	cam.frustum:setTarget(self.center)
	cam.frustum:setPosition(vec3(
		self.center.x + math.sin(self.angle) * self.radius,
		self.center.y + self.height,
		self.center.z + math.cos(self.angle) * self.radius
	))
end

function cameraslow:onDestroy()
	EventSystem.publishSwitchCamera(nil)
end
