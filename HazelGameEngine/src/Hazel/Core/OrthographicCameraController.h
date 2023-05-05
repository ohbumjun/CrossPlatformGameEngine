#pragma once

#include "Renderer/OrthographicCamera.h"
#include "Hazel/Core/TimeStep.h"

#include "Hazel/Event/ApplicationEvent.h"
#include "Hazel/Event/MouseEvent.h"

namespace Hazel
{
	class OrthographicCameraController
	{
	public :
		OrthographicCameraController(float aspectRatio, float rotation = false); // aspect ratio * 2 units

		void OnUpdate(Timestep ts);
		void OnEvent(Event& e);

		const OrthographicCamera& GetCamera() const { return m_Camera; }
		OrthographicCamera& GetCamera() { return m_Camera; }
	private :
		bool OnMouseScrolledEvent(MouseScrolledEvent& e);
		bool OnWindowResizeEvent(WindowResizeEvent& e);
	private :
		float m_AspectRatio;
		float m_ZoomLevel = 1.f;

		OrthographicCamera m_Camera;

		bool m_Rotation;
		float m_CameraRot = 0.f;
		glm::vec3 m_CameraPos = { 0.f, 0.f, 0.f };
		float m_CameraRotSpeed = 10.f;
		float m_CameraMoveSpeed = 1.0f;
	};

}


