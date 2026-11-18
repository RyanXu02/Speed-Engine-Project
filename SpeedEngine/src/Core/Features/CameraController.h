#pragma once

namespace SE
{
	struct CameraFrustum;
	class InputSystem;

	class CameraController
	{
	public:
		void update(CameraFrustum& camera, const InputSystem& input, double deltaTime);

		float& getCameraSensitivity() { return m_sensitivity; }
		float& getCameraMoveSpeed() { return m_cameraMoveSpeed; }
		float& getCameraDollySpeed() { return m_cameraDollySpeed; }

	private:
		float m_sensitivity = 0.005f;
		float m_cameraMoveSpeed = 6.0f;
		float m_cameraDollySpeed = 10.0f;
	};
} // namespace SE
