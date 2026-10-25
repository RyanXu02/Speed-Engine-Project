Camera = {}
Camera.__index = Camera

function Camera.new(EntityWrapper)
	local instance = {	
		entity = EntityWrapper,
		
	}

	return setmetatable(instance, Camera)
end

function Camera:onInit()
	
end

function Camera:onUpdate()
	
end

function Camera:onDestroy()
	
end