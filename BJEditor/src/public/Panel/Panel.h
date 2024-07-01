#pragma once

#include "Hazel/Core/Reflection/TypeId.h"
#include "Hazel/Core/Reflection/Reflection.h"

// Editor 에 있는 코드이지만, Scene 에 접근할 수 있게 하기 위해
// 예외적으로 하나만 HazelEditor 가 아니라
// Hazel Editor Namespace 로 설정한다.
namespace HazelEditor
{
class Panel
{
    friend class PanelController;

public:
    Panel();
    virtual ~Panel();
    // @brief 창 생성
    template <typename T>
    static void Open()
    {
        Open(Hazel::Reflection::GetTypeId<T>());
    }
    static void Open(Hazel::TypeId type);

    void Initialize();

    void Open(bool fullscreen = false); 

    // @brief 창 끄기
    void Close()
    {
        _isOpen = false;
    }

    void Destroy()
    {
        _destroying = true;
    }
    const char *GetName() const
    {
        return _name.c_str();
    }

    bool IsOpen() const
    {
        return _isOpen;
    }

    bool IsDestroying() const
    {
        return _destroying;
    }

    uint64 GetID()
    {
        return _id;
    }

    /**
	 * \brief 최대로 열 수 있는 창 갯수 (0인 경우 무제한)
	 */
    virtual size_t GetMaxCount() const
    {
        return 1;
    }

    virtual Hazel::TypeId GetType() const = 0;


protected:

    void setUniqueName(const char *format, ...);
    virtual void onInitialize()
    {
    }
    virtual void onOpen()
    {
    }
    virtual void onDestroy()
    {
    }
    virtual void onGUI()
    {
    }
    bool _isOpen = false;
    /**
	 * \brief 현재 프레임에서 띄어쓰기 1개의 폰트 크기
	 */
    ImVec2 unitSize;

private:
    uint32 _type = 0;
    uint64 _id = 0;
    bool _destroying = false;
    std::string _name;
};
} // namespace Hazel
