#pragma once

#include "../pch.h"

class GameTimer final
{
public:
    GameTimer() noexcept :
        m_secondsPerCount(SecondsPerCount())
    {
    }

    // in seconds
    double TotalTime() { return m_currentCount * m_secondsPerCount; }
    double TimeDelta() { return m_deltaFrameCount * m_secondsPerCount; }

    __int64 Frame(double targetFrameRate = 144) { return TotalTime() * targetFrameRate; }

    void Tick()
    {
        if (m_stopCount > 0.0)
        {
            m_deltaFrameCount = 0.0;
            m_previousCount = 0.0;
            return;
        }

        if (m_baseCount == 0)
            m_baseCount = CurrentCounts();

        m_currentCount = CurrentCounts() - m_baseCount - m_totalStopCount;
        m_deltaFrameCount = m_currentCount - m_previousCount;
        m_previousCount = m_currentCount;

        if (m_deltaFrameCount < 0.0)
            m_deltaFrameCount = 0.0;
    }

    void Resume()
    {
        if (m_stopCount == 0)
            return;

        m_totalStopCount += CurrentCounts() - m_stopCount;
        m_stopCount = 0.0;
    }

    void Stop()
    {
        if (m_stopCount > 0)
            return;

        m_stopCount = CurrentCounts();
    }

private:
    const double m_secondsPerCount;

    __int64 m_baseCount = 0;

    __int64 m_totalStopCount = 0;
    __int64 m_stopCount = 0;

    __int64 m_currentCount = 0;
    __int64 m_previousCount = 0;
    __int64 m_deltaFrameCount = 0;

    __int64 CurrentCounts()
    {
        LARGE_INTEGER output;
        auto success = QueryPerformanceCounter(&output);
        assert(success);
        return output.QuadPart;
    };

    double SecondsPerCount()
    {
        LARGE_INTEGER output;
        auto success = QueryPerformanceFrequency(&output);
        assert(success);
        return 1.0 / (double)output.QuadPart;
    };
};