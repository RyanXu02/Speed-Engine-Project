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

end

-- required for every class script
function Player:onUpdate()
    local nameComponent = self.entity:getComponent("NameComponent") 
    
    if(nameComponent == nil) then
        Logger.error("name component failed to retrieve from playerscript")
    else
        Logger.info(nameComponent.name)
    end
end

-- required for every class script
function Player:onDestroy()

end