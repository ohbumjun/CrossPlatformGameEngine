#pragma once
#include "GuiProcessors/GuiProcessor.h"
#include "Hazel/Core/DataStructure/RingBuffer.h"
#include "Hazel/Core/Reflection/Reflection.h"
#include "Hazel/Core/Reflection/TypeId.h"
#include "Hazel/Core/Thread/ThreadVars.h"
#include "Panel/Panel.h"
#include "Panel/PanelController.h"
#include "Panel/ProgressPanel.h"
#include <queue>

namespace HazelEditor
{
class EditorGuiProcessr : public GuiProcessor
{
public:
    static EditorGuiProcessr *BJ_GetEditorGuiProcessr();

    EditorGuiProcessr();

    template <typename T>
    inline T *CreatePanel()
    {
        return (T *)CreatePanel(Hazel::Reflection::GetTypeID<T>());
    }

    Panel *CreatePanel(const Hazel::TypeId &type);

    template <typename T>
    inline std::vector<T *> FindPanels()
    {
        std::vector<T *> result;
        std::vector<Panel *> finds =
            FindPanels(Hazel::Reflection::GetTypeID<T>());
        for (uint64 i = 0, max = finds.size(); i < max; i++)
        {
            result.push_back((T *)finds[i]);
        }
        return result;
    }
    std::vector<Panel *> FindPanels(const Hazel::TypeId &type) const;
    Panel *FindPanel(const char *name) const;

    void OpenMessagePopup(const std::string &title, const std::string &message);

    void DisplayProgressBar(float progress,
                            const char *title,
                            const char *message,
                            const std::string &key = "");

    void ClearProgressBar(const std::string &key = "");

private:
    struct ProgressData
    {
        std::string message = "";
        std::string title = "";
        float progress = 0.f;
        bool show = false;

        ProgressData() = default;
        ProgressData(float progress, const char *title, const char *message)
            : progress(progress), title(title), message(message), show(true)
        {
        }

        bool operator==(const ProgressData &other) const
        {
            return progress == other.progress && title == other.title &&
                   message == other.message && show == show;
        }

        bool operator!=(const ProgressData &other) const
        {
            return !operator==(other);
        }

        static const ProgressData Show(float progress,
                                       const char *title,
                                       const char *message)
        {
            return ProgressData(progress, title, message);
        }

        static const ProgressData &Close()
        {
            const static ProgressData close;
            return close;
        }
    };

    void onInit() override;

    void onUpdate(float deltatime) override;

    void onOpen() override;

    void onClose() override;

    void onPrepareGUI() override;

    void onFinishGUI() override;

    void onGUI() override;

    void onNextFrame() override;

    void updateProgressBar(const ProgressData &data);

    PanelController *_panelController = nullptr;

    ProgressPanel *_progress = nullptr;

    Hazel::CRIC_SECT *_mutex = nullptr;

    Hazel::RingBuffer<ProgressData> _progressQueue;
    std::queue<float> _seconds;

    bool _presentable = true;

    ProgressData _lastProgressData;
};

}; // namespace HazelEditor
