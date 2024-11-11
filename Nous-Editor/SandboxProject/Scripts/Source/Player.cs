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
        Left = -1,
        None,
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
        public float MinJumpSpeed = 1.0f;
        public float JumpBoost = 0.2f;
        public float MaxJumpPressTime = 0.5f;
        public float CoyoteTime = 0.2f; // 离地后允许跳跃的时间
        private float m_NowCoyoteTime = 0.0f;
        private float m_JumpTimer = 0.0f;
        public Vector2 Velocity;

        public Prefab TestPrefab;

        private bool m_IsJumpKeyPress = false;
        private bool m_OnGround = false;
        private bool m_IsJumping = false;

        private Direction m_OnWallDir = Direction.None;
        private Direction m_Facing = Direction.None;

        private Dictionary<ulong, Vector2> m_Collisions;
        private Dictionary<ulong, Panel> m_StandingPanel;

        private float m_TotalDt = 0.0f;
        private int m_CountFrame = 50;
        private int m_FrameCD = 0;
        void OnCreate()
        {
            m_Transform = GetComponent<CTransform>();
            m_Rigidbody = GetComponent<CRigidbody2D>();
            m_AnimPlayer = GetComponent<CAnimPlayer>();

            m_Collisions = new Dictionary<ulong, Vector2>();
            m_StandingPanel = new Dictionary<ulong, Panel>();

            if (TestPrefab.Handle != 0)
            {
                Entity newEntity = TestPrefab.Instantate();
                newEntity.SetParent(this);
                newEntity.Translation = new Vector3(0, 0, -1.0f);
            }
        }

        void OnStart()
        {
            m_Text = FindEntityByName("Text")?.As<TheText>();
        }

        void OnUpdate(float dt)
        {
            Velocity = m_Rigidbody.LinearVelocity;
            float realSpeed = Speed;

            // 处理真实速度
            if (Input.IsKeyDown(KeyCode.A))
            {
                // 1.地面奔跑是原速
                // 2.起跳是第二级速度
                // 3.空中转向是将原本是速度反转
                if (!m_OnGround)
                {
                    if (m_Facing == Direction.Right)
                        realSpeed = Math.Abs(Velocity.X);
                    else
                        realSpeed *= SpeedOnAirMul;
                }
                m_Facing = Direction.Left;

                // 根据方向施加x速度
                Velocity.X = (int)m_Facing * realSpeed;
            }
            else if (Input.IsKeyDown(KeyCode.D))
            {
                if (!m_OnGround)
                {
                    if (m_Facing == Direction.Left)
                        realSpeed = Math.Abs(Velocity.X);
                    else
                        realSpeed *= SpeedOnAirMul;

                }
                m_Facing = Direction.Right;

                // 根据方向施加x速度
                Velocity.X = (int)m_Facing * realSpeed;
            }
            else
            {
                // m_Facing = Direction.None;
            }

            

            // 空格起跳：必须触地
            if (Input.IsKeyDown(KeyCode.Space))
            {
                // 刚按下
                if (!m_IsJumpKeyPress)
                {
                    if (m_OnGround || m_NowCoyoteTime > 0)
                    {
                        m_IsJumping = true;
                        m_JumpTimer = MaxJumpPressTime;
                        Velocity.Y = MinJumpSpeed;
                    }
                }
                else
                {
                    // 持续按下
                    if (m_IsJumping)
                    {
                        if (m_JumpTimer > 0)
                        {
                            Velocity.Y = JumpBoost;
                            m_JumpTimer -= dt;
                        }
                        else
                        {
                            m_IsJumping = false;
                        }
                    }
                }
            }
            else
            {
                // 松开跳跃键
                if (m_IsJumpKeyPress)
                {
                    m_IsJumping = false;
                }
            }
            m_IsJumpKeyPress = Input.IsKeyDown(KeyCode.Space);

            // 郊狼时间计时
            if (m_NowCoyoteTime > 0)
            {
                m_NowCoyoteTime -= dt;
            }

            // 下蹲：穿越平台
            if (Input.IsKeyDown(KeyCode.S))
            {
                if (m_OnGround && m_StandingPanel.Count > 0)
                {
                    foreach (Panel p in m_StandingPanel.Values)
                    {
                        if (p != null)
                        {
                            p.SetOnWayCollision(true);
                        }
                    }
                }
            }

            // 下落过程比上升过程快
            if (!m_OnGround && Velocity.Y < 0)
                Velocity.Y -= AccelFalling * dt;

            // 如果在撞墙：不再施加速度，自由滑落
            if (m_OnWallDir == Direction.Left && Velocity.X < 0 || m_OnWallDir == Direction.Right && Velocity.X > 0)
            {
                Velocity.X = 0;
                NousConsole.Log("YES");
            }


            m_TotalDt += dt;
            m_FrameCD++;
            if (m_FrameCD == m_CountFrame)
            {
                float fps =  m_CountFrame / m_TotalDt;
                m_Text.Text = "" + fps;
                m_FrameCD = 0;
                m_TotalDt = 0;
            }
            m_Rigidbody.LinearVelocity = Velocity;

            // 设置动画状态机
            m_AnimPlayer.SetFloat("x", Velocity.X);
            m_AnimPlayer.SetFloat("y", Velocity.Y);
            m_AnimPlayer.SetBool("onGround", m_OnGround);
        }


        void OnCollisionEnter(CollisionContact contact, ulong otherID, Vector2 normal)
        {
            Entity otherEntity = new Entity(otherID);
            
            m_Collisions[otherID] = normal;

            bool isPanel = otherEntity.Name == "Panel";
            // 检查是否在地面上
            if (normal.Y < 0)
            {
                m_OnGround = true;

                if (isPanel)
                {
                    m_StandingPanel[otherID] = otherEntity.As<Panel>();
                }
            }

            // 检查是否在墙上
            if (Math.Abs(normal.X) > 0 && !isPanel/* || isPanel && Math.Abs(Velocity.Y) < 0.1f*/)
            {
                m_OnWallDir = normal.X < 0 ? Direction.Left : Direction.Right;
                NousConsole.Log($"? {normal.X} {normal.Y}");
            }
        }

        void OnCollisionExit(CollisionContact contact, ulong otherID)
        {
            Entity otherEntity = new Entity(otherID);

            // 根据id取出先前的接触面法向量
            Vector2 normal = Vector2.Zero;
            if (m_Collisions.ContainsKey(otherID))
            {
                normal = m_Collisions[otherID];
                m_Collisions.Remove(otherID);
            }

            if (normal == Vector2.Zero)
                return;

            float lastPanelCount = m_StandingPanel.Count;
            // 检查是否在地面上
            if (normal.Y < 0)
            {
                // 默认认为离地
                m_OnGround = false;

                // 如果离开的是平台：移除这个平台的记录
                if (otherEntity.Name == "Panel")
                {
                    m_StandingPanel[otherID].SetOnWayCollision(false);
                    m_StandingPanel.Remove(otherID);
                }
            }

            if (otherEntity.Name == "Panel")
            {
                if (m_StandingPanel.Count > 0)
                { 
                    // 仍然站在平台上，视为没有离开地面
                    m_OnGround = true;
                }
            }

            // 启动郊狼计时
            if (!m_OnGround)
            {
                m_NowCoyoteTime = CoyoteTime;
            }

            // 脱离撞墙状态
            if (Math.Abs(normal.X) > 0)
            {
                m_OnWallDir = Direction.None;
            }
        }
    }
}
