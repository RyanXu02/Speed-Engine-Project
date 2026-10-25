Player = {}
Player.__index = Player

function Player.new(EntityWrapper)
	local instance = {
		entity = EntityWrapper,
		moveSpeed = 4.0
	}

	return setmetatable(instance, Player)
end

function Player:onInit()
end

function Player:onUpdate(dt)
	if not Engine.isPlaying() then
		return
	end

	local dx, dz = 0.0, 0.0
	if Input.isKeyDown(KeyCodes.KEY_W) then dz = dz - 1.0 end
	if Input.isKeyDown(KeyCodes.KEY_S) then dz = dz + 1.0 end
	if Input.isKeyDown(KeyCodes.KEY_A) then dx = dx - 1.0 end
	if Input.isKeyDown(KeyCodes.KEY_D) then dx = dx + 1.0 end

	if dx == 0.0 and dz == 0.0 then
		return
	end

	local transform = self.entity:getComponent("Transform")
	local p = transform:getPosition()
	transform:setPosition(vec3(p.x + dx * self.moveSpeed * dt, p.y, p.z + dz * self.moveSpeed * dt))
end

function Player:onDestroy()
end
