using System;
using AuroraEngine;

// Main Class
public class Initializer
{
    public float m_MovementSpeed = 5.0f;
    public bool m_IsEnabled = false;
    public string m_PlayerName;

    // Main Function
    public void Start()
    {
        m_IsEnabled = true;
        m_PlayerName = "Bob";
        m_MovementSpeed = 5.0f;

        Debug.Log("I Like Trains");
        Debug.Log(500.0f, DebugType.Warning);
    }

    public void Update(float deltaTime)
    {
        Debug.Log(m_MovementSpeed);
    }
}

