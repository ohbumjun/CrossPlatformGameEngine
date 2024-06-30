#pragma once

#include "Panel/Panel.h"

// Editor 에 있는 코드이지만, Scene 에 접근할 수 있게 하기 위해
// 예외적으로 하나만 HazelEditor 가 아니라
// Hazel Editor Namespace 로 설정한다.
namespace HazelEditor
{

class NativePanel : public Panel
{
    friend class PanelController;
    friend class EditorWindow;
};
} // namespace Hazel
