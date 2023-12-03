﻿#pragma once

#include "Scene.h"

namespace Hazel
{
	class SceneSerializer
	{
	public :
		SceneSerializer(const Ref<Scene>& scene);

		void SerializeText(const std::string& filePath);
		// runtime
		void SerializeBinary(const std::string& filePath);

		void DeserializeText();
		bool DeserializeBinary(const std::string& filePath);
	protected :
		bool deserializeText(const std::string& filePath);
		Ref<Scene> m_Scene;
	};
}


