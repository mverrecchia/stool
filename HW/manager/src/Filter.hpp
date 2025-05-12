// DigitalFilter.h
#pragma once

#include <algorithm>

class DigitalFilter {
public:
    DigitalFilter(int threshold = 3) 
        : m_counter(0)
        , m_threshold(threshold)
        , m_lastStableState(false)
    {}
    
    bool update(bool rawValue)
    {
        if (rawValue)
        {
            m_counter = std::min(m_counter + 1, m_threshold + 5);
        }
        else
        {
            m_counter = std::max(m_counter - 1, 0);
        }
        
        m_currentStableState = (m_counter >= m_threshold);
        
        m_risingEdge = m_currentStableState && !m_lastStableState;
        m_fallingEdge = !m_currentStableState && m_lastStableState;
        m_lastStableState = m_currentStableState;
        
        return m_currentStableState;
    }
    
    bool getState(void) const
    {
        return m_currentStableState;
    }
    
    bool risingEdgeDetected(void) const
    {
        return m_risingEdge;
    }
    
    bool fallingEdgeDetected(void) const
    {
        return m_fallingEdge;
    }
    
    void reset(bool initialState = false)
    {
        m_counter = initialState ? (m_threshold + 5) : 0;
        m_currentStableState = initialState;
        m_lastStableState = initialState;
        m_risingEdge = false;
        m_fallingEdge = false;
    }
    
    void setThreshold(int threshold)
    {
        m_threshold = threshold;
    }
    
private:
    int m_counter;           // Debounce counter
    int m_threshold;         // Threshold for state change
    bool m_lastStableState;  // Previous stable state
    bool m_currentStableState = false; // Current stable state
    bool m_risingEdge = false;  // Rising edge flag
    bool m_fallingEdge = false; // Falling edge flag
};