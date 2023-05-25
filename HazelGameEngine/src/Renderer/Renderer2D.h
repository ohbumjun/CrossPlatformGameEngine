#pragma once

#include "OrthographicCamera.h"

namespace Hazel
{
	// 역할 : Scene 등 데이터를 받아서, 그에 따라 GPU 측에 그려달라고 요청하는 것
	// - 해당 Class 는 super static 이 될 것이다. 즉, 아무런 data storage 도
	// - 들고 있게 하지 않을 것이라는 의미이다.
	class Renderer2D
	{
	public :
		static void Init();
		static void Shutdown();
		static void BeginScene(const OrthographicCamera& camera);
		static void EndScene();

		// Primitives
		static void DrawQuad(const glm::vec2& pos, const glm::vec2& size, const glm::vec4& color);
		static void DrawQuad(const glm::vec3& pos, const glm::vec2& size, const glm::vec4& color);
	};
}

