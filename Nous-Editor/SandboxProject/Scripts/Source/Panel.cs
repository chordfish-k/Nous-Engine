using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Nous;

namespace Sandbox
{
    public class Panel : Entity
    {
        public float CheckDirectionThreshold = 0.0f; // 接触面法向量y的阈值，控制什么样的角度认为是可以被穿透的方向
        public bool CollisionOnBottom = false; // 表示当前碰撞条件(上穿还是下穿)
        public bool IsDisable = false; // 表示当前是否可以被透过

        public void SetOnWayCollision(bool isup)
        {
            CollisionOnBottom = isup;
        }

        void OnUpdate(float dt)
        {
        }

        void OnCollisionPreSolve(CollisionContact contact, ulong entityId, Vector2 normal)
        {
            if (CollisionOnBottom)
            {
                if (normal.Y >= CheckDirectionThreshold)
                {
                    NousConsole.Log($"{normal.Y}");
                    IsDisable = true;
                }
            }
            else
            {
                
                if (normal.Y <= -CheckDirectionThreshold)
                {
                    NousConsole.Log($"{normal.Y}");
                    IsDisable = true;
                }
            }



            if (IsDisable)
                contact.Enable = false;
        }

        void OnCollisionPostSolve(CollisionContact contact, ulong entityId, Vector2 normal)
        {

        }

        void OnCollisionExit(CollisionContact contact, ulong entityId)
        {
            IsDisable = false;
        }
    }

}
