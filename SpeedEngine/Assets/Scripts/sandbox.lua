local a = 5
Logger.info("value: " .. a)

local activeScene = SceneSystem.getCurrentScene()
Logger.info("scene name: ".. activeScene:getName())

local entityName = "TestEntity"
local id = activeScene:addEntity(entityName)
Logger.info("addEntity: ".. entityName)

activeScene:removeEntity(id)
Logger.info("removeEntity: ".. id)