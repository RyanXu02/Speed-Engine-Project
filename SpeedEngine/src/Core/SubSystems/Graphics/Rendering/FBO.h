#pragma once

namespace SE
{
	class FBO
	{
    public:
        FBO(uint32_t width, uint32_t height, bool includeDepth = true);
        ~FBO();

        // Delete copy, allow move
        FBO(const FBO&) = delete;
        FBO& operator=(const FBO&) = delete;
        FBO(FBO&& other) noexcept;
        FBO& operator=(FBO&& other) noexcept;

        void bind();
        void unbind();
        void resize(uint32_t width, uint32_t height);

        uint32_t getColorTexture() const { return m_colorTexture; }
        uint32_t getDepthTexture() const { return m_depthTexture; }
        uint32_t getWidth() const { return m_width; }
        uint32_t getHeight() const { return m_height; }

    private:
        void _create();
        void _destroy();

        uint32_t m_fboID = 0;
        uint32_t m_colorTexture = 0;
        uint32_t m_depthTexture = 0;
        uint32_t m_width = 0;
        uint32_t m_height = 0;
        bool m_includeDepth = true;
	};
}

