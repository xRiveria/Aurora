using System;
using AuroraEngine;

namespace AuroraEngine
{
    // Main Class
    public class Initializer
    {
        public float m_Pikas = 5.0f;
        public float m_Pika = 5.0f;
        public float m_Derpy = 4.0f;
        public float m_Trains = 6.0f;
        public float m_Pokemon = 5.0f;
        public float m_Derp = 4.0f;
        public float m_ILikeTrains = 5.0f;
        public float m_MovementSpeed = 5.0f;
        public bool m_IsEnabled = false;
        public string m_PlayerName;

        // Main Function
        public void OnStart()
        {
            Debug.Log("I Like Butts Too");
        }

        public void OnUpdate(float deltaTime)
        {
            // Debug.Log(m_MovementSpeed);
        }
    }
}

