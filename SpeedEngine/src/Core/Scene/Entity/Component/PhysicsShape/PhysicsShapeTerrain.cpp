#include "pch.h"
#include "PhysicsShapeTerrain.h"

#include "Engine.h"

#include "Core/ResourceManagers/Terrain/Terrain.h"
#include "Core/ResourceManagers/ResourceManager.h"

#include <Jolt/Physics/Collision/Shape/HeightFieldShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>

#include <stb_image.h>

namespace SE
{
	void PhysicsShapeTerrain::setTerrainID(uint32_t resourceID)
	{
		if (resourceID == m_terrainID)
			return;

		m_terrainID = resourceID;
		updateShape();
	}

	void PhysicsShapeTerrain::setPath(std::string path) 
	{
		if (path == m_path)
			return;

		m_path = path;
		auto resourceManager = Engine::Instance().getSubSystem<ResourceManager>();
		setTerrainID(resourceManager->loadTerrain({ .path = m_path, .name = m_path }));
	}

	void PhysicsShapeTerrain::createUnscaledShape() 
	{
		if (m_terrainID == 0)
			return;

		Terrain* terrain = static_cast<Terrain*>(Engine::Instance().getSubSystem<ResourceManager>()->getResource(m_terrainID));

		// load heightmap
		stbi_set_flip_vertically_on_load(true);
		int width = 0, height = 0, channels = 0;
		const std::string path(terrain->getHeightMapPath());
		uint16_t* heightmap = stbi_load_16(path.c_str(), &width, &height, &channels, 1);
		if (!heightmap)
			return;
		uint32_t sampleCount = static_cast<uint32_t>(height);

		// Convert heightmap data to float 0-1
		std::vector<float> terrainData(sampleCount*sampleCount);
		for (size_t i = 0; i < terrainData.size(); ++i)
		{
			terrainData[i] = static_cast<float>(heightmap[i]) / 65535.0f;
		}
		stbi_image_free(heightmap);

		const float cellSize = 2.0f / sampleCount;

		float heightScale = terrain->getHeightScale();

		JPH::HeightFieldShapeSettings shapeSettings(
			terrainData.data(),
			JPH::Vec3{ -0.5f * cellSize * sampleCount, 0.0f, -0.5f * cellSize * sampleCount },
			JPH::Vec3{ cellSize, heightScale, cellSize },
			sampleCount
		);

		JPH::Shape::ShapeResult result; // can be used for error checking
		m_unscaledShape = new JPH::HeightFieldShape(shapeSettings, result);
	}

}
