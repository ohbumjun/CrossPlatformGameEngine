﻿#pragma once

#include "Hazel/Core/Application/Window.h"

#include <GLFW/glfw3.h>

#include "Renderer/GraphicsContext.h"

namespace Hazel
{
class WindowsWindow : public Window
{
public:
    WindowsWindow(const WindowProps &props);
    virtual ~WindowsWindow();

    // @brief Event
    virtual void PeekEvent();


    inline unsigned int GetWidth() const override
    {
        return m_Data.Width;
    }
    inline unsigned int GetHeight() const override
    {
        return m_Data.Height;
    }

    // Window Attribute
    virtual void SetEventCallback(const EventCallbackFn &callback) override
    {
        m_Data.EventCallback = callback;
    }
    void SetVSync(bool enabled);
    bool IsVSync() const;

    inline virtual void *GetNativeWindow() const
    {
        return m_Window;
    }

private:
    // @brief Frame 을 정리해주는 함수
    void onEndFrame() override;
    virtual void init(const WindowProps &props);
    virtual void shutdown();

    // Handle To Window
    GLFWwindow *m_Window;

    GraphicsContext *m_Context;

    // 아래 struct 는 GLFW 에 넘겨주기 위한 데이터이다.
    struct WindowData
    {
        std::string Title;
        unsigned int Width, Height;
        bool VSync;

        // Application.h 의 OnEvent 가 여기에 세팅될 것이다.
        EventCallbackFn EventCallback;
    };

    WindowData m_Data;

    HINSTANCE m_hInstance;
};

} // namespace Hazel
