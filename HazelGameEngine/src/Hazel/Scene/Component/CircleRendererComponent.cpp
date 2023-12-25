#include "hzpch.h"
#include "CircleRendererComponent.h"
#include "Hazel/Core/Serialization/Serializer.h"

Hazel::CircleRendererComponent::CircleRendererComponent()
{
	Reflection::RegistType<CircleRendererComponent>();
}

Hazel::CircleRendererComponent::CircleRendererComponent(const CircleRendererComponent&)
{
	Reflection::RegistType<CircleRendererComponent>();
}

void Hazel::CircleRendererComponent::Serialize(Serializer* serializer)
{
	serializer->BeginSaveMap(Reflection::GetTypeID<CircleRendererComponent>(), this);

	Reflection::TypeInfo* compTypeInfo = Reflection::GetTypeInfo(GetType());

	serializer->Save("compName", compTypeInfo->m_Name.c_str());

	serializer->Save("color", m_Color);
	serializer->Save("Thickness", m_Thickness);
	serializer->Save("Fade", m_Fade);

	serializer->EndSaveMap();
}

void Hazel::CircleRendererComponent::Deserialize(Serializer* serializer)
{
	serializer->BeginLoadMap(Reflection::GetTypeID<CircleRendererComponent>(), this);

	std::string compName;
	serializer->Load("compName", compName);

	serializer->Load("color", m_Color);
	serializer->Load("Thickness", m_Thickness);
	serializer->Load("Fade", m_Fade);

	serializer->EndLoadMap();
}

const TypeId Hazel::CircleRendererComponent::GetType() const
{
	return Reflection::GetTypeID<CircleRendererComponent>();
}
