soundtest = {}
soundtest.__index = soundtest

function soundtest.new(EntityWrapper)
	local instance = {
		entity = EntityWrapper
	}

	return setmetatable(instance, soundtest)
end

function soundtest:onInit()
end

function soundtest:onUpdate(dt)
	if not Engine.isPlaying() then
		return
	end

	if Input.isKeyRising(KeyCodes.KEY_N) then
		local sfx = self.entity:getComponent("AudioSFXComponent")
        local transform = self.entity:getComponent("Transform")
        local p = transform.translate
		sfx:PlayMySound(p)
	end
    if Input.isKeyRising(KeyCodes.KEY_M) then
        local music = self.entity:getComponent("AudioMusicComponent")
        music:PlayMyTrack()
    end
end

function soundtest:onDestroy()
end
