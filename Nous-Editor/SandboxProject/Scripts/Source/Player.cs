using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Nous;

namespace Sandbox
{
    public enum Direction
    {
        None,
        Left,
        Right
    }

    public class Player : Entity
    {
        
        private CTransform m_Transform = null;
        private CRigidbody2D m_Rigidbody = null;
        private CAnimPlayer m_AnimPlayer = null;

        private TheText m_Text = null;

        public float Speed = 1.0f;
        public float SpeedOnAirMul = 0.6f;
        public float AccelFalling = 0.2f;
        public float JumpSpeed = 1.0f;
        private Vector2 velocity;

        private bool m_IsJumpKeyPress = false;
        private bool s_OnGround = false;
        private Direction s_OnWall = Direction.None;

        private Dictionary<ulong, Vector2> m_Collisions;

        private bool m_PassingFromBottom = false;
        private bool m_PassingFromTop = false;
        private ulong m_LastPassingEntityID = 0;

        void OnCreate()
        {
            NousConsole.Log($"Player.OnCreate = {ID}");

            m_Transform = GetComponent<CTransform>();
            m_Rigidbody = GetComponent<CRigidbody2D>();
            m_AnimPlayer = GetComponent<CAnimPlayer>();

            m_Collisions = new Dictionary<ulong, Vector2>();
        }

        void OnStart()
        {
            m_Text = FindEntityByName("Text")?.As<TheText>();
        }

        void OnUpdate(float dt)
        {
            velocity = m_Rigidbody.LinearVelocity;

            if (Input.IsKeyDown(KeyCode.A))
            {
                if (s_OnGround)
                {
                    velocity.X = -1.0f * Speed;
                }
                else 
                {
                    if (velocity.X >= 0.0f)
                        velocity.X = -1.0f * Speed * SpeedOnAirMul * SpeedOnAirMul;
                    else
                        velocity.X = -1.0f * Speed * SpeedOnAirMul;
                }
            }
            else if (Input.IsKeyDown(KeyCode.D))
            {
                if (s_OnGround)
                {
                    velocity.X = 1.0f * Speed;
                }
                else 
                {
                    if (velocity.X <= 0.0f)
                        velocity.X = 1.0f * Speed * SpeedOnAirMul * SpeedOnAirMul;
                    else
                        velocity.X = 1.0f * Speed * SpeedOnAirMul;
                }
            }

            if (Input.IsKeyDown(KeyCode.Space))
            {
                if (s_OnGround && !m_IsJumpKeyPress)
                {
                    velocity.Y = JumpSpeed;
                    s_OnGround = false;
                }
                m_IsJumpKeyPress = true;
            }
            else
            {
                m_IsJumpKeyPress = false;
            }

            if (!s_OnGround && velocity.Y < 0)
            {
                // 加速下落
                velocity.Y -= AccelFalling;
            }

            if (s_OnWall == Direction.Left && velocity.X < 0 || s_OnWall == Direction.Right && velocity.X > 0)
                velocity.X = 0;

            m_Text.Text = "" + dt;
            m_Rigidbody.LinearVelocity = velocity;

            m_AnimPlayer.SetFloat("x", velocity.X);
            m_AnimPlayer.SetFloat("y", velocity.Y);
            m_AnimPlayer.SetBool("onGround", s_OnGround);

            //NousConsole.Log($"x: {velocity.X}, y: {velocity.Y}, onGround: {s_OnGround}, onWall: {s_OnWall}");
        }

        void OnPreCollision(CollisionContact contact, ulong otherID, Vector2 normal)
        {
            Entity otherEntity = new Entity(otherID);
            if (otherEntity.Name == "Panel")
            {
                if (normal.Y == 1.0f)
                {
                    // 碰到顶
                    m_PassingFromBottom = true;
                    m_PassingFromTop = false;
                    m_LastPassingEntityID = otherID;
                }
                if (normal.Y == -1.0f && Input.IsKeyDown(KeyCode.S))
                {
                    // 下落触底
                    m_PassingFromTop = true;
                    m_PassingFromBottom = false;
                    m_LastPassingEntityID = otherID;
                }

                if (m_LastPassingEntityID == otherID)
                {
                    if (m_PassingFromBottom)
                    {
                        if (velocity.Y > 0.0f)
                            contact.Enable = false;
                        else
                            m_PassingFromBottom = false;
                    }
                    if (m_PassingFromTop)
                    {
                        if (velocity.Y < 0.0f)
                            contact.Enable = false;
                        else
                            m_PassingFromTop = false;
                    }
                } 
            }
            m_LastPassingEntityID = otherID;
        }

        void OnCollision(ulong otherID, Vector2 normal, bool enter)
        {
            Entity otherEntity = new Entity(otherID);
            
            // 离开碰撞的话获取先前碰撞的法向量，好判断情况，后面移动到Nous
            if (enter)
            {
                m_Collisions[otherID] = normal;
            }
            else
            {
                if (m_Collisions.ContainsKey(otherID))
                {
                    normal = m_Collisions[otherID];
                    m_Collisions.Remove(otherID);
                }
                s_OnGround = false;
            }

            // 检查是否在地面上
            if (normal.Y < 0)
            {
                s_OnGround = enter;
                NousConsole.Log($"ground: {enter}");
            }
            // 检查是否在天花板上
            else if (normal.Y > 0)
            {
                //OnCeil = true;
            }
            // 检查是否在墙上
            if (Math.Abs(normal.X) > 0)
            {
                if (enter)
                {
                    s_OnWall = normal.X < 0 ? Direction.Left : Direction.Right;
                    NousConsole.Log($"wall: {enter}");
                }
                else
                {
                    s_OnWall = Direction.None;
                }
            }
            
            NousConsole.Log($"OnCollision = {ID} -> ({normal.X},{normal.Y})");
        }
    }
}
