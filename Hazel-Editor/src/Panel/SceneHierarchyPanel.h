﻿#pragma once

#include "Hazel/Scene/Entity.h"
#include "Hazel/Scene/Scene.h"

#include <glm/gtc/type_ptr.hpp>

// Editor 에 있는 코드이지만, Scene 에 접근할 수 있게 하기 위해
// 예외적으로 하나만 HazelEditor 가 아니라
// Hazel Editor Namespace 로 설정한다.
namespace Hazel
{
	class SceneHierarchyPanel
	{
	public :
		SceneHierarchyPanel() : m_Context(nullptr) {};
		SceneHierarchyPanel(const Ref<Scene>& scene);
		void SetContext(const Ref<Scene>& scene);
		void OnImGuiRender();
	private :
		void drawEntityNode(Entity entity);
		void drawComponents(Entity entity);
		Ref<Scene> m_Context;
		Entity m_SelectedEntity;

		friend class Scene;
	};
}



