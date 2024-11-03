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
        private CAnimPlayer m_AnimPlayer = null;

        private TheText m_Text = null;

        public float Speed = 1.0f;
        public float JumpSpeed = 1.0f;

        void OnCreate()
        {
            NousConsole.Log($"Player.OnCreate = {ID}");

            m_Transform = GetComponent<CTransform>();
            m_Rigidbody = GetComponent<CRigidbody2D>();
            m_AnimPlayer = GetComponent<CAnimPlayer>();
        }

        void OnStart()
        {
            m_Text = FindEntityByName("Text")?.As<TheText>();
        }

        void OnUpdate(float dt)
        {
            Vector2 velocity = m_Rigidbody.LinearVelocity;

/*            if (Input.IsKeyDown(KeyCode.W))
                velocity.Y = 1.0f;
            else if (Input.IsKeyDown(KeyCode.S))
                velocity.Y = -1.0f;*/

            if (Input.IsKeyDown(KeyCode.A))
                velocity.X = -1.0f * Speed;
            else if (Input.IsKeyDown(KeyCode.D))
                velocity.X = 1.0f * Speed;

            if (Input.IsKeyDown(KeyCode.Space))
                velocity.Y = JumpSpeed;
                //m_Rigidbody.ApplyLinearImpulse(new Vector2(0, 500), true);

            //if (m_Text != null)
            m_Text.Text = "" + dt;

            m_Rigidbody.LinearVelocity = velocity;

            m_AnimPlayer.SetFloat("x", velocity.X);
            m_AnimPlayer.SetFloat("y", velocity.Y);

            NousConsole.Log($"x: {velocity.X}, y: {velocity.Y}");
        }
    }
}
