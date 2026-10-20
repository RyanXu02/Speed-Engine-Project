Player = {}				--create a table that acts as the class blueprint for all player objects
Player.__index = Player --lua prototype instance

-- cosntructor required for every class script
function Player.new(EntityWrapper)
	-- isntance's member values are defined here
	local instance = {	
		entity = EntityWrapper,
		mouseSubscription = nil,
		keySubscription = nil
	}

	return setmetatable(instance, Player)
end

-- required for every class script
function Player:onInit()
	self.mouseSubscription = EventSystem.subscribe(EventType.MouseMoved, function(event)
		local mouseEvent = as_MouseMovedEvent(event)
		if mouseEvent == nil then
			Logger.error("as_MouseMovedEvent cast failed")
			return
		end
		Logger.info("mouse x: " .. mouseEvent.m_x .. " mouse y: " .. mouseEvent.m_y)
	end)
	self.keySubscription = EventSystem.subscribe(EventType.KeyPressed, function(event)
		local keyEvent = as_KeyPressedEvent(event)
		if keyEvent == nil then
			Logger.error("as_KeyPressedEvent cast failed")
			return
		end
		Logger.info("key pressed: " .. keyEvent.keyCode)
	end)
end

-- required for every class script
function Player:onUpdate()
    -- local nameComponent = self.entity:getComponent("NameComponent") 
    
    -- if(nameComponent == nil) then
    --     Logger.error("name component failed to retrieve from playerscript")
    -- else
    --     Logger.info(nameComponent.name)
    -- end
end

-- required for every class script
function Player:onDestroy()

end