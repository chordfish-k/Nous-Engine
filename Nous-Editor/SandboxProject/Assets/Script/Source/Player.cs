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


        void OnCreate()
        {
            Console.WriteLine($"Player.OnCreate = {ID}");

            m_Transform = GetComponent<CTransform>();
            m_Rigidbody = GetComponent<CRigidbody2D>();
        }

        void OnUpdate(float dt)
        {
            Console.WriteLine($"Player.OnUpdate: {dt}");

            float speed = 1.0f;
            Vector3 velocity = Vector3.Zero;

            if (Input.IsKeyDown(KeyCode.W))
                velocity.Y = 1.0f;
            else if (Input.IsKeyDown(KeyCode.S))
                velocity.Y = -1.0f;

            if (Input.IsKeyDown(KeyCode.A))
                velocity.X = -1.0f;
            else if (Input.IsKeyDown(KeyCode.D))
                velocity.X = 1.0f;

            Entity cameraEntity = FindEntityByName("Camera");
            if (cameraEntity != null)
            {
                Camera camera = cameraEntity.As<Camera>();

                if (Input.IsKeyDown(KeyCode.Q))
                    camera.DistanceFromPlayer += speed * 2.0f * dt;
                else if (Input.IsKeyDown(KeyCode.E))
                    camera.DistanceFromPlayer -= speed * 2.0f * dt;
            }

            velocity *= speed;

            m_Rigidbody?.ApplyLinearImpulse(velocity.XY, true);
        }
    }
}
