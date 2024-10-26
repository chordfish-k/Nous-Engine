using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Nous
{
    public abstract class Component
    {
        public Entity Entity { get; internal set; }
    }

    public class CTransform : Component
    {
        public Vector3 Translation
        { 
            get
            {
                InternalCalls.TransformComponent_GetTranslation(Entity.ID, out Vector3 translation);
                return translation;
            }
            set
            {
                InternalCalls.TransformComponent_SetTranslation(Entity.ID, ref value);
            }
        }
    }

    public class CRigidbody2D : Component
    {
        public void ApplyLinearImpulse(Vector2 impluse, Vector2 worldPosition, bool wake)
        {
            InternalCalls.Rigidbody2DComponent_ApplyLinearImpulse(Entity.ID, ref impluse, ref worldPosition, wake);
        }

        public void ApplyLinearImpulse(Vector2 impluse, bool wake)
        {
            InternalCalls.Rigidbody2DComponent_ApplyLinearImpulseToCenter(Entity.ID, ref impluse, wake);
        }
    }
}
