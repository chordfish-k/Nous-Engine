using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Nous;

namespace Sandbox
{
    public class Kid : Entity
    {
        public Prefab BulletPrefab;

        public float ShootCD = 0.3f;
        private float m_ShootCDCount = 0;
        private float m_Speed = 10.0f;

        void OnUpdate(float dt)
        {
            Vector3 translation = Translation;
            if (Input.IsKeyDown(KeyCode.A))
                translation.X -= m_Speed * dt;
            if (Input.IsKeyDown(KeyCode.D))
                translation.X += m_Speed * dt;
            if (Input.IsKeyDown(KeyCode.W))
                translation.Y += m_Speed * dt;
            if (Input.IsKeyDown(KeyCode.S))
                translation.Y -= m_Speed * dt;
            Translation = translation;

            if (Input.IsKeyDown(KeyCode.J))
            {
                if (m_ShootCDCount <= 0)
                {
                    m_ShootCDCount = ShootCD;

                    if (BulletPrefab.IsValid())
                    {
                        Entity entity = BulletPrefab.Instantate();
                        entity.As<Bullet>().Shoot(new Vector3(Translation.X, Translation.Y, 0), new Vector2(10, 0) );
                    }
                }
            }
            
            if (m_ShootCDCount > 0)
                m_ShootCDCount -= dt;
        }
    }
}
