#pragma once

#include "../pch.h"
#include "StepTimer.h"

namespace timer
{
struct Tick
{
    double totalTime{0.0};
    double deltaTime{0.0};
    uint64_t frameCount{0};
};
}; // namespace timer

class GameTimer final
{
public:
    GameTimer() noexcept :
        m_stepTimer(DX::StepTimer())
    {
        m_stepTimer.SetFixedTimeStep(TRUE);
        m_stepTimer.SetTargetElapsedSeconds(1 / 144.0);
    }

    // in seconds

    timer::Tick Tick()
    {
        if (m_pauseTime > 0) {
            return m_currentTick;
        }

        m_stepTimer.Tick([&] {
            m_currentTick.deltaTime = m_stepTimer.GetElapsedSeconds();
            m_currentTick.totalTime = m_stepTimer.GetTotalSeconds() - m_pausedTotalTime;
            m_currentTick.frameCount = m_stepTimer.GetFrameCount();
        });

        return m_currentTick;
    }

    void Resume()
    {
        if (m_pauseTime == 0)
            return;

        m_stepTimer.ResetElapsedTime();
        m_pausedTotalTime += m_stepTimer.GetTotalSeconds() - m_pauseTime;
        m_pauseTime = 0;
    }

    void Pause()
    {
        if (m_pauseTime > 0)
            return;

        m_currentTick.deltaTime = 0;
        m_pauseTime = m_stepTimer.GetTotalSeconds();
    }

private:
    DX::StepTimer m_stepTimer;

    timer::Tick m_currentTick{};

    double m_pausedTotalTime = 0.0;
    double m_pauseTime = 0;
};