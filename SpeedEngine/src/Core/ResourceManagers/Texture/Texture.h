#pragma once
#include "../Resource.h"

namespace SE
{
	class Shader;

	class Texture : public Resource
	{
	public:
		Texture(uint32_t id, std::string name);
		~Texture();

		void init(Shader& shader);
		void bind(unsigned int slot) const;
		void unbind() const;
		void destroy();
	private:
		uint32_t m_resourceId;
		unsigned int m_textureId;

	};
}

