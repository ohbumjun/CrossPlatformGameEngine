#pragma once

#include "Hazel/Scene/Component/Component.h"
#include "Hazel/Scene/SceneCamera.h"
#include "Panel/NativePanel.h"

namespace Hazel
{
class EditorCamera;
}

namespace HazelEditor
{
class CameraPanel : public NativePanel
{
public:
    /*
		주의 사항 
		- HazelEditor::DrawComponent<CameraComponent>("Camera", entity, &HazelEditor::CameraPanel::DrawCameraComponent);
		  와 같이 DrawComponent 함수의 인자용으로 사용한다.
		*/
    static void DrawCameraComponent(Hazel::Component &inputComponent);
    static void DrawEditorCamera(Hazel::EditorCamera *editorCamera);

	inline Hazel::TypeId GetType() const override
    {
        return Hazel::Reflection::GetTypeID<CameraPanel>();
    }
};
} // namespace HazelEditor
