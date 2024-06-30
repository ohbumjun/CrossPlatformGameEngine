﻿#pragma once

#include <string>
#include <hzpch.h>
#include "Hazel/Core/Thread/ThreadVars.h"

namespace HazelEditor
{
class Project
{
    enum class PlayState : uint8
    {
        PLAY,
        PAUSE,
        STOP,
    };

    enum class PlayMode : uint8
    {
        EDITOR = 0,
        ENGINE = 1,
    };

    enum class LibraryType : uint8
    {
        EDITOR,
        APPLICATION,
        MAX,
    };

public:
	// @param projectPath 프로젝트 절대경로
    Project(const char *projectPath);
    ~Project();
    
	// @brief 현재 프로젝트 로드여부 반환
    static bool IsLoaded();

    
	 // @brief 기존 프로젝트 열기
	 // @param projectAbsolutePath 대상 프로젝트 절대경로
    static void Open(const char *projectAbsolutePath);

	// @brief 현재 프로젝트 닫기
    static void Close();

	/**
	* @brief 빌트인 에셋 로드
	*/
    void LoadBuiltIn(std::string &state) const;

	void Play();
    void Pause();
    void Stop();
    void NextFrame();
    void SaveSettings();
    void LoadSettings();

	inline PlayState GetPlayState() const
    {
        return _playState;
    }

    inline PlayMode GetPlayMode() const
    {
        return _playMode;
    }

    inline void SetPlayMode(PlayMode mode)
    {
        if (_playMode != mode)
        {
            _playMode = mode;
        }
    }

    
	Hazel::CRIC_SECT *_loopMutex = nullptr;
    Hazel::ConditionVariable *_loopCondition = nullptr;
    Hazel::Atomic _loopAtomic;

    PlayState _playState = PlayState::STOP;
    PlayMode _playMode = PlayMode::EDITOR;
};
} // namespace HazelEditor
