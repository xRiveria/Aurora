using System;
using AuroraEngine;

// Main Class
public class Initializer
{
    public float m_MyMumLOL = 6.0f;
    public float m_MyMum = 6.0f;
    public float m_Derpssss = 5.0f;
    public float m_Trains = 6.0f;
    public float m_Pokemon = 5.0f;
    public float m_Derp = 4.0f;
    public float m_ILikeTrains = 5.0f;
    public float m_MovementSpeed = 5.0f;
    public bool m_IsEnabled = false;
    public string m_PlayerName;

    // Main Function
    public void Start()
    {
        m_Derp = 6.0f;
        m_IsEnabled = true;
        m_PlayerName = "Bob";
        m_MovementSpeed = 5.0f;
        Debug.Log("I Like Trains");
        Debug.Log("I Like Doka");
        Debug.Log("I Like Doka");
        Debug.Log("I Like Doka");
    }

    public void Update(float deltaTime)
    {
        Debug.Log(m_MovementSpeed);
    }
}

