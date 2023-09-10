#include "hzpch.h"
#include "SceneCamera.h"
#include <glm/gtc/matrix_transform.hpp>

Hazel::SceneCamera::SceneCamera()
{
}

Hazel::SceneCamera::~SceneCamera()
{
}

void Hazel::SceneCamera::SetOrthographic(float size, float nearDis, float farDis)
{
	m_OrthographicSize = size;
	m_OrthographicNear = nearDis;
	m_OrthographicFar = farDis;
}

void Hazel::SceneCamera::SetViewportSize(uint32_t width, uint32_t height)
{
	float aspectRatio = (float)width / (float)height;
	float orthoLeft		= m_OrthographicSize * aspectRatio * -0.5f;
	float orthoRight		= m_OrthographicSize * aspectRatio * 0.5f;
	float orthoTop			= m_OrthographicSize * 0.5f;
	float orthoBottom	= m_OrthographicSize * -0.5f;

	m_ProjectionMatrix = glm::ortho(orthoLeft, orthoRight, orthoTop, orthoBottom, m_OrthographicNear, m_OrthographicFar);


}
