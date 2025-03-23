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
                InternalCalls.TransformComponent_SetTranslation(Entity.ID, value);
            }
        }
    }

    public class CRigidbody2D : Component
    {
        public enum BodyType { Static = 0, Dynamic, Kinematic }

        public Vector2 LinearVelocity
        {
            get
            {
                InternalCalls.Rigidbody2DComponent_GetLinearVelocity(Entity.ID, out Vector2 velocity);
                return velocity;
            }
            set
            {
                InternalCalls.Rigidbody2DComponent_SetLinearVelocity(Entity.ID, ref value);
            }
        }

        public BodyType Type
        {
            get => InternalCalls.Rigidbody2DComponent_GetType(Entity.ID);
            set => InternalCalls.Rigidbody2DComponent_SetType(Entity.ID, value);
        }

        public void ApplyLinearImpulse(Vector2 impulse, Vector2 worldPosition, bool wake)
        {
            InternalCalls.Rigidbody2DComponent_ApplyLinearImpulse(Entity.ID, ref impulse, ref worldPosition, wake);
        }

        public void ApplyLinearImpulse(Vector2 impulse, bool wake)
        {
            InternalCalls.Rigidbody2DComponent_ApplyLinearImpulseToCenter(Entity.ID, ref impulse, wake);
        }
    }

    public class CTextRenderer : Component
    {

        public string Text
        {
            get => InternalCalls.TextRendererComponent_GetText(Entity.ID);
            set => InternalCalls.TextRendererComponent_SetText(Entity.ID, value);
        }

        public Vector4 Color
        {
            get
            {
                InternalCalls.TextRendererComponent_GetColor(Entity.ID, out Vector4 color);
                return color;
            }

            set
            {
                InternalCalls.TextRendererComponent_SetColor(Entity.ID, ref value);
            }
        }

        public float Kerning
        {
            get => InternalCalls.TextRendererComponent_GetKerning(Entity.ID);
            set => InternalCalls.TextRendererComponent_SetKerning(Entity.ID, value);
        }

        public float LineSpacing
        {
            get => InternalCalls.TextRendererComponent_GetLineSpacing(Entity.ID);
            set => InternalCalls.TextRendererComponent_SetLineSpacing(Entity.ID, value);
        }

    }

    public class CAnimPlayer : Component
    {
        public void SetFloat(string key, float value)
        {
            InternalCalls.AnimPlayerComponent_SetFloat(Entity.ID, key, value);
        }

        public void SetBool(string key, bool value)
        {
            InternalCalls.AnimPlayerComponent_SetBool(Entity.ID, key, value);
        }
    }


    public class CUIText : Component
    {
        public string Text
        {
            get => InternalCalls.UITextComponent_GetText(Entity.ID);
            set => InternalCalls.UITextComponent_SetText(Entity.ID, value);
        }

        public Vector4 Color
        {
            get
            {
                InternalCalls.UITextComponent_GetColor(Entity.ID, out Vector4 color);
                return color;
            }

            set
            {
                InternalCalls.UITextComponent_SetColor(Entity.ID, ref value);
            }
        }

        public float Kerning
        {
            get => InternalCalls.UITextComponent_GetKerning(Entity.ID);
            set => InternalCalls.UITextComponent_SetKerning(Entity.ID, value);
        }

        public float LineSpacing
        {
            get => InternalCalls.UITextComponent_GetLineSpacing(Entity.ID);
            set => InternalCalls.UITextComponent_SetLineSpacing(Entity.ID, value);
        }

    }
}
