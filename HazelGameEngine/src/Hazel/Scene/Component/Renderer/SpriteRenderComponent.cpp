﻿#include "hzpch.h"
#include "Hazel/Core/Serialization/Serializer.h"
#include "SpriteRenderComponent.h"

Hazel::SpriteRenderComponent::SpriteRenderComponent()
{
	Reflection::RegistType<SpriteRenderComponent>();
}

Hazel::SpriteRenderComponent::SpriteRenderComponent(const SpriteRenderComponent& other)
: m_Color(other.m_Color),
m_TilingFactor(other.m_TilingFactor)
{
	Reflection::RegistType<SpriteRenderComponent>();
}

void Hazel::SpriteRenderComponent::Serialize(Serializer* serializer)
{
	serializer->BeginSaveMap(Reflection::GetTypeID<SpriteRenderComponent>(), this);

	Reflection::TypeInfo* compTypeInfo = Reflection::GetTypeInfo(GetType());

	serializer->Save("compName", compTypeInfo->m_Name.c_str());

	serializer->Save("texturePath", m_Texture->GetPath());

	serializer->Save("tilingFactor", m_TilingFactor);

	serializer->Save("color", m_Color);

	serializer->EndSaveMap();
}

void Hazel::SpriteRenderComponent::Deserialize(Serializer* serializer)
{
	serializer->BeginLoadMap(Reflection::GetTypeID<SpriteRenderComponent>(), this);
	
	std::string compName;
	serializer->Load("compName", compName);

	std::string texturePath;
	serializer->Load("texturePath", texturePath);
	
	if (texturePath.empty() == false)
	{
		m_Texture = Texture2D::Create(texturePath);
	}

	serializer->Load("tilingFactor", m_TilingFactor);

	serializer->Load("color", m_Color);

	serializer->EndLoadMap();
}

const TypeId Hazel::SpriteRenderComponent::GetType() const
{
	return Reflection::GetTypeID<SpriteRenderComponent>();
}
