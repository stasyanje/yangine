#include "../pch.h"

class GameTimer final
{
public:
    GameTimer() noexcept :
        m_secondsPerCount(SecondsPerCount())
    {
    }

    bool Running()
    {
        return !m_stopped;
    }

    __int64 Frame()
    {
        return Time() / m_secondsPerFrame;
    }

    float DeltaTime()
    {
        return m_deltaTime;
    }

    double Time() // in seconds
    {
        return m_currentTime * m_secondsPerCount;
    }

    void Tick()
    {
        if (m_stopped)
        {
            m_deltaTime = 0.0;
            return;
        }

        m_currentTime = CurrentCounts();
        m_deltaTime = m_currentTime - m_prevTime;
        m_prevTime = m_currentTime;

        if (m_deltaTime < 0.0)
            m_deltaTime = 0.0;
    }

    void Resume()
    {
        if (!m_stopped)
            return;

        m_stopped = false;
    }

    void Stop()
    {
        if (m_stopped)
            return;

        m_stopped = true;
    }

private:
    const double m_secondsPerCount;
    const double m_secondsPerFrame = 1 / 60.0;

    __int64 m_currentTime = 0;
    __int64 m_prevTime = 0;

    double m_deltaTime = 0.0;

    bool m_stopped = false;

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