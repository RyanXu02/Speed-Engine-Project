Camera = {}
Camera.__index = Camera

function Camera.new(EntityWrapper)
	local instance = {
		entity = EntityWrapper,
		center = vec3(0.0, 0.0, 0.0),
		radius = 12.0,
		height = 5.0,
		speed = 0.5,
		angle = 0.0
	}

	return setmetatable(instance, Camera)
end

function Camera:onInit()
	local cam = self.entity:getComponent("CameraComponent")
	EventSystem.publishSwitchCamera(cam.frustum)
end

function Camera:onUpdate(dt)
	if not Engine.isPlaying() then
		return
	end

	--self.angle = self.angle + self.speed * dt

	local cam = self.entity:getComponent("CameraComponent")
	cam.frustum:setTarget(self.center)
	cam.frustum:setPosition(vec3(
		self.center.x + math.sin(self.angle) * self.radius,
		self.center.y + self.height,
		self.center.z + math.cos(self.angle) * self.radius
	))
end

function Camera:onDestroy()
	EventSystem.publishSwitchCamera(nil)
end
