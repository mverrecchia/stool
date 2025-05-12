#pragma once

#include <cmath>

class Timer {
public:
    Timer() : m_elapsed(0.0f), m_running(false) {}

    void start() { m_running = true; m_elapsed = 0.0f; }
    void stop()  { m_running = false; }
    void reset() { m_elapsed = 0.0f; }
    
    void update(float deltaTime)
    {
        if (m_running)
        {
            m_elapsed += deltaTime;
        }
    }
    
    bool isRunning() const               { return m_running; }
    bool hasElapsed(float seconds) const { return m_elapsed >= seconds; }
    float getElapsed() const             { return m_elapsed; }

private:
    float m_elapsed;
    bool m_running;
};