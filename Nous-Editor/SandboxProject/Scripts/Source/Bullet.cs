using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Nous;

namespace Sandbox
{
    public class Bullet : Entity
    {
        void OnStart()
        {
        }
        void OnUpdate(float dt)
        {
        }

        public void Shoot(Vector3 pos, Vector2 speed)
        {
            Translation = pos;

            CRigidbody2D cRigidbody2D = GetComponent<CRigidbody2D>();
            cRigidbody2D.LinearVelocity = speed;
        }

    }
}
