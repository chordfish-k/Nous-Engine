using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Nous;

namespace Sandbox
{
    public class Player : Entity
    {
        private CTransform m_Transform = null;
        private CRigidbody2D m_Rigidbody = null;

        private TheText m_Text = null;

        public float Speed = 1.0f;

        void OnCreate()
        {
            Console.WriteLine($"Player.OnCreate = {ID}");

            m_Transform = GetComponent<CTransform>();
            m_Rigidbody = GetComponent<CRigidbody2D>();
        }

        void OnStart()
        {
            m_Text = FindEntityByName("Text")?.As<TheText>();
        }

        void OnUpdate(float dt)
        {
            // Console.WriteLine($"Player.OnUpdate: {dt}");

            Vector2 velocity = Vector2.Zero;

            if (Input.IsKeyDown(KeyCode.W))
                velocity.Y = 1.0f;
            else if (Input.IsKeyDown(KeyCode.S))
                velocity.Y = -1.0f;

            if (Input.IsKeyDown(KeyCode.A))
                velocity.X = -1.0f;
            else if (Input.IsKeyDown(KeyCode.D))
                velocity.X = 1.0f;

            velocity *= Speed;

            //if (m_Text != null)
            m_Text.Text = "" + dt;

            m_Rigidbody.LinearVelocity = velocity;
        }
    }
}
