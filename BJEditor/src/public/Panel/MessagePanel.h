#pragma once

#include "Panel/NativePanel.h"
#include "Hazel/Core/Reflection/TypeId.h"
#include <string>
#include <vector>

// Editor 에 있는 코드이지만, Scene 에 접근할 수 있게 하기 위해
// 예외적으로 하나만 HazelEditor 가 아니라
// Hazel Editor Namespace 로 설정한다.
namespace HazelEditor
{
class MessagePanel : public NativePanel
{
public:
    enum OkCancel
    {
        OK,
        CANCEL,
    };
    
	void Open(const char *title, const char *message, 
        bool canCloase = true);
    void Open(const char *title,
              const char *message,
              std::function<void(OkCancel)> result);
    void Open(const char *title,
              const char *message,
              std::function<void(int)> onClose);

    inline Hazel::TypeId GetType() const override
    {
        return Hazel::Reflection::GetTypeID<MessagePanel>();
    }
    inline size_t GetMaxCount() const override
    {
        return 0;
    }
    inline Hazel::TypeId GetType() const override
    {
        return Hazel::Reflection::GetTypeID<MessagePanel>();
    }

protected:
    void onGUI() override final;

    virtual void onMessageGUI();

    std::string _message;

    std::function<void(uint)> _onClose;
};
} // namespace Hazel
