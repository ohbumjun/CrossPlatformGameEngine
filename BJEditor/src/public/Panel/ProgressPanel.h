#pragma once

#include "Panel/MessagePanel.h"
#include "Hazel/Core/Reflection/TypeId.h"
#include "Hazel/Core/Reflection/Reflection.h"

// Editor 에 있는 코드이지만, Scene 에 접근할 수 있게 하기 위해
// 예외적으로 하나만 HazelEditor 가 아니라
// Hazel Editor Namespace 로 설정한다.
namespace HazelEditor
{
class ProgressPanel : public MessagePanel
{
public:
    ProgressPanel() = default;

    inline Hazel::TypeId GetType() const override
    {
        return Hazel::Reflection::GetTypeID<ProgressPanel>();
    }

    void Open(const char *title, const char *message, float progress);

protected:
    void onMessageGUI() override;

private:
    float _progress = 0.0f;
};
} // namespace Hazel
