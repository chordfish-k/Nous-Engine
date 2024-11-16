#include "pch.h"
#include "PhysicsSystem.h"

#include "Nous/Scene/Component.h"
#include "Nous/Scene/Entity.h"

#include "ScriptSystem.h"
#include "TransformSystem.h"

#include "Nous/Renderer/Renderer2D.h"


// Box2D
#include "box2d/box2d.h"
#include "box2d/b2_world.h"
#include "box2d/b2_body.h"
#include "box2d/b2_fixture.h"
#include "box2d/b2_polygon_shape.h"
#include "box2d/b2_circle_shape.h"
#include "box2d/b2_collision.h"
#include "box2d/b2_contact.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/euler_angles.hpp>


namespace Nous
{
    class ContactListener : public b2ContactListener
    {
    public:
        virtual void BeginContact(b2Contact* contact) override;
        virtual void EndContact(b2Contact* contact) override;
        virtual void PreSolve(b2Contact* contact, const b2Manifold* oldManifold) override;
        virtual void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse) override;
    };

    static Scene* s_Scene = nullptr;
    static b2World* s_PhysicsWorld = nullptr;
    static ContactListener* s_ContactListener = nullptr;
    static b2Contact* s_LastContact = nullptr;
    static bool s_EnableDebugDraw = false;

    namespace Utils
    {
        static b2BodyType Rigidbody2DTypeToBox2DBody(CRigidbody2D::BodyType bodyType)
        {
            switch (bodyType)
            {
            case CRigidbody2D::BodyType::Static:    return b2_staticBody;
            case CRigidbody2D::BodyType::Dynamic:   return b2_dynamicBody;
            case CRigidbody2D::BodyType::Kinematic: return b2_kinematicBody;
            }

            NS_CORE_ASSERT(false, "未知的 body type");
            return b2_staticBody;
        }


        class DebugDraw : public b2Draw 
        {
        public:
            // 设置绘图标志（如绘制形状、关节、AABB等）
            DebugDraw() {
                SetFlags(b2Draw::e_shapeBit | b2Draw::e_jointBit | b2Draw::e_aabbBit);
            }

            // 重写 DrawPolygon 方法
            void DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) override {
                glm::vec4 glmColor = { color.r, color.g, color.b, color.a };
                for (int32 i = 0; i < vertexCount; ++i) {
                    glm::vec3 p0 = { vertices[i].x, vertices[i].y, 0.0f };
                    glm::vec3 p1 = { vertices[(i + 1) % vertexCount].x, vertices[(i + 1) % vertexCount].y, 0.0f };
                    DrawLine(p0, p1, glmColor);
                }
            }

            // 重写 DrawSolidPolygon 方法
            void DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) override {
                glm::vec4 glmColor = { color.r, color.g, color.b, color.a };
                for (int32 i = 0; i < vertexCount; ++i) {
                    glm::vec3 p0 = { vertices[i].x, vertices[i].y, 0.0f };
                    glm::vec3 p1 = { vertices[(i + 1) % vertexCount].x, vertices[(i + 1) % vertexCount].y, 0.0f };
                    DrawLine(p0, p1, glmColor);
                }
            }

            void DrawSolidCircle(const b2Vec2& center, float radius, const b2Vec2& axis, const b2Color& color) override {
                DrawCircle(center, radius, color); // 直接调用 DrawCircle 以绘制边缘
            }

            // 重写 DrawCircle 方法
            void DrawCircle(const b2Vec2& center, float radius, const b2Color& color) override {
                constexpr const int segments = 16;
                constexpr float angleStep = 2.0f * glm::pi<float>() / segments;
                glm::vec4 glmColor = { color.r, color.g, color.b, color.a };

                for (int i = 0; i < segments; ++i) {
                    float angle0 = i * angleStep;
                    float angle1 = (i + 1) * angleStep;

                    glm::vec3 p0 = { center.x + radius * cos(angle0), center.y + radius * sin(angle0), 0.0f };
                    glm::vec3 p1 = { center.x + radius * cos(angle1), center.y + radius * sin(angle1), 0.0f };

                    DrawLine(p0, p1, glmColor);
                }
            }

            // 重写 DrawSegment 方法
            void DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color) override {
                glm::vec4 glmColor = { color.r, color.g, color.b, color.a };
                glm::vec3 start = { p1.x, p1.y, 0.0f };
                glm::vec3 end = { p2.x, p2.y, 0.0f };
                DrawLine(start, end, glmColor);
            }

            // 重写 DrawTransform 方法
            void DrawTransform(const b2Transform& xf) override {
                const float axisLength = 0.5f; // 长度可根据需要调整
                glm::vec3 origin = { xf.p.x, xf.p.y, 0.0f };

                // x 轴（红色）
                glm::vec3 xAxis = { xf.p.x + axisLength * xf.q.GetXAxis().x, xf.p.y + axisLength * xf.q.GetXAxis().y, 0.0f };
                DrawLine(origin, xAxis, glm::vec4(1, 0, 0, 1));

                // y 轴（绿色）
                glm::vec3 yAxis = { xf.p.x + axisLength * xf.q.GetYAxis().x, xf.p.y + axisLength * xf.q.GetYAxis().y, 0.0f };
                DrawLine(origin, yAxis, glm::vec4(0, 1, 0, 1));
            }

            // 重写 DrawPoint 方法（可选）
            void DrawPoint(const b2Vec2& p, float size, const b2Color& color) override {
                glm::vec4 glmColor = { color.r, color.g, color.b, color.a };
                glm::vec3 point = { p.x, p.y, 0.0f };

                // 画一个小圆圈表示点，或者用线段画一个十字
                float halfSize = size / 2.0f;
                DrawLine({ point.x - halfSize, point.y, 0.0f }, { point.x + halfSize, point.y, 0.0f }, glmColor);
                DrawLine({ point.x, point.y - halfSize, 0.0f }, { point.x, point.y + halfSize, 0.0f }, glmColor);
            }

        private:
            void DrawLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color)
            {
                Renderer2D::DrawLine(p0, p1, color);
            }
        };
        static DebugDraw debugDraw;
    }

    static float s_TotalDt = 0;

	void PhysicsSystem::Start(Scene* scene)
	{
		s_Scene = scene;

        s_PhysicsWorld = new b2World({ 0.0f, -9.8f });
        // 1.从每个包含CRigidbody2D的节点往下找，找到叶子节点或者找到又一个CRigidbody2D，一个CRigidbody2D一个body
        // 2.找出范围内所有Collider2D, 将其形状都作为fixture添加到根的body
        for (auto& [uuid, e] : s_Scene->GetRootEntities())
        {
            SetupRigidbody(e);
        }

        s_ContactListener = new ContactListener();
        s_PhysicsWorld->SetContactListener(s_ContactListener);
        Utils::debugDraw.SetFlags(b2Draw::e_shapeBit | b2Draw::e_centerOfMassBit);
        s_PhysicsWorld->SetDebugDraw(&Utils::debugDraw);

        s_TotalDt = 0.0f;
	}

	void PhysicsSystem::Update(Timestep dt)
	{
        NS_PROFILE_FUNCTION();

        // 物理更新
        if (s_PhysicsWorld)
        {
            auto view = s_Scene->GetAllEntitiesWith<CRigidbody2D>();
            {
                NS_PROFILE_SCOPE("Physics Pre-Update");
                for (auto e : view)
                {
                    Entity entity = { e, s_Scene };
                    auto& transform = entity.GetComponent<CTransform>();

                    auto& rb2d = entity.GetComponent<CRigidbody2D>();

                    b2Body* body = (b2Body*)rb2d.RuntimeBody;
                    if (!body)
                    {
                        SetupRigidbody(e);
                        body = (b2Body*)rb2d.RuntimeBody;
                    }

                    auto tr = transform.ParentTransform * transform.GetTransform();
                    // 世界坐标系矩阵结算
                    glm::vec3 scale, rotation, translation, _;
                    glm::vec4 __;
                    glm::quat orientation;
                    glm::decompose(tr, scale, orientation, translation, _, __);
                    rotation = glm::eulerAngles(orientation);

                    body->SetTransform({ translation.x, translation.y }, rotation.z);
                    body->SetEnabled(transform.Active);
                }
            }

            // 控制物理模拟的迭代次数
            constexpr int32_t velocityIterations = 6;
            constexpr int32_t positionIterations = 2;
#if 0          
            const float physicsTimestep = 1.0 / 60.0f;

            s_TotalDt += dt;
            if (s_TotalDt >= physicsTimestep)
            {
                s_PhysicsWorld->Step(physicsTimestep, velocityIterations, positionIterations);
                //while (s_TotalDt >= physicsTimestep) 
                s_TotalDt -= physicsTimestep;
            }
#else
            {
                NS_PROFILE_SCOPE("Box2d Step");
                s_PhysicsWorld->Step(dt, velocityIterations, positionIterations);
            }
#endif

            {
                NS_PROFILE_SCOPE("Physics Update");
                for (auto e : view)
                {
                    Entity entity = { e, s_Scene };
                    auto& transform = entity.GetComponent<CTransform>();

                    auto& rb2d = entity.GetComponent<CRigidbody2D>();

                    b2Body* body = (b2Body*)rb2d.RuntimeBody;
                    if (body == nullptr)
                    {
                        SetupRigidbody(e);
                        body = (b2Body*)rb2d.RuntimeBody;

                        if (body == nullptr)
                            continue;
                    }

                    if (!body->IsAwake())
                        continue;

                    const auto& position = body->GetPosition();

                    glm::mat4 tr = glm::inverse(transform.ParentTransform)
                        * glm::translate(glm::mat4(1.0f), { position.x, position.y, 0 })
                        * glm::toMat4(glm::quat({ 0, 0, body->GetAngle() }));

                    // 世界坐标系矩阵结算
                    glm::vec3 scale, rotation, translation, _;
                    glm::vec4 __;
                    glm::quat orientation;
                    glm::decompose(tr, scale, orientation, translation, _, __);
                    rotation = glm::eulerAngles(orientation);

                    transform.Translation.x = translation.x;
                    transform.Translation.y = translation.y;
                    transform.Rotation.z = rotation.z;

                    TransformSystem::SetSubtreeDirty(s_Scene, e);
                }
            }
        }
        if (s_EnableDebugDraw)
        {
            Entity camera = s_Scene->GetPrimaryCameraEntity();
            Renderer2D::BeginScene(camera.GetComponent<CCamera>().Camera, camera.GetComponent<CTransform>().GetTransform());
            Renderer2D::SetLineWidth(2.0f);
            s_PhysicsWorld->DebugDraw();
            Renderer2D::EndScene();
        }
	}

	void PhysicsSystem::Stop()
	{
        delete s_ContactListener;
        s_ContactListener = nullptr;

        delete s_PhysicsWorld;
        s_PhysicsWorld = nullptr;
	}

    void PhysicsSystem::EnableDebugDraw(bool enable)
    {
        s_EnableDebugDraw = enable;
    }

    void PhysicsSystem::DisableLastContact()
    {
        if (s_LastContact)
            s_LastContact->SetEnabled(false);
    }

    void PhysicsSystem::SetupRigidbody(entt::entity e, b2Body* rootBody)
    {
        Entity entity{ e, s_Scene };
        auto& transformC = entity.GetComponent<CTransform>();

        if (!transformC.Active)
            return;

        auto transform = transformC.ParentTransform * transformC.GetTransform();

        // 世界坐标系矩阵结算
        glm::vec3 scale, rotation, translation, _;
        glm::vec4 __;
        glm::quat orientation;
        glm::decompose(transform, scale, orientation, translation, _, __);
        rotation = glm::eulerAngles(orientation);  // 将弧度转换为度数

        bool hasBody = false;
        if (entity.HasComponent<CRigidbody2D>())
        {
            hasBody = true;

            auto& rb2d = entity.GetComponent<CRigidbody2D>();

            b2BodyDef bodyDef;
            bodyDef.type = Utils::Rigidbody2DTypeToBox2DBody(rb2d.Type);

            bodyDef.position.Set(translation.x, translation.y);
            bodyDef.angle = rotation.z;

            // 将uuid存储在body中
            b2BodyUserData userData;
            userData.pointer = entity.GetUUID();
            bodyDef.userData = userData;

            rootBody = s_PhysicsWorld->CreateBody(&bodyDef);
            rootBody->SetFixedRotation(rb2d.FixedRotation);
            rb2d.RuntimeBody = rootBody;

            transformC.HasRigidBody = true;
        }


        if (entity.HasComponent<CBoxCollider2D>() && rootBody)
        {
            auto& bc2d = entity.GetComponent<CBoxCollider2D>();
            auto offset = bc2d.Offset;
            b2PolygonShape boxShape;

            float hx = bc2d.Size.x;
            float hy = bc2d.Size.y;
            b2Vec2 points[4] =
            {
                {-hx + offset.x, -hy + offset.y},
                { hx + offset.x, -hy + offset.y},
                { hx + offset.x,  hy + offset.y},
                {-hx + offset.x,  hy + offset.y}
            };

            for (auto& v : points)
            {
                glm::vec4 vec = { v.x, v.y, 0, 1 };
                vec = transform * vec;
                auto p = rootBody->GetLocalPoint({ vec.x, vec.y });
                v.x = p.x;
                v.y = p.y;
            }


            boxShape.Set(points, 4);

            b2FixtureDef fixtureDef;
            fixtureDef.shape = &boxShape;
            fixtureDef.density = bc2d.Density;
            fixtureDef.friction = bc2d.Friction;
            fixtureDef.restitution = bc2d.Restitution;
            fixtureDef.restitutionThreshold = bc2d.RestitutionThreshold;

            bc2d.RuntimeFixture = rootBody->CreateFixture(&fixtureDef);
        }

        if (entity.HasComponent<CCircleCollider2D>() && rootBody)
        {
            auto& cc2d = entity.GetComponent<CCircleCollider2D>();

            b2CircleShape circleShape;
            circleShape.m_p.Set(cc2d.Offset.x, cc2d.Offset.y);
            circleShape.m_radius = scale.x * cc2d.Radius;

            b2FixtureDef fixtureDef;
            fixtureDef.shape = &circleShape;
            fixtureDef.density = cc2d.Density;
            fixtureDef.friction = cc2d.Friction;
            fixtureDef.restitution = cc2d.Restitution;
            fixtureDef.restitutionThreshold = cc2d.RestitutionThreshold;
            cc2d.RuntimeFixture = rootBody->CreateFixture(&fixtureDef);
        }

        for (auto& uid : transformC.Children)
        {
            Entity childNode = s_Scene->GetEntityByUUID(uid);
            SetupRigidbody((entt::entity)childNode, rootBody);
        }
    }

    void PhysicsSystem::DeleteRigidbody(entt::entity e)
    {
        if (!s_Scene)
            return;

        Entity entity{ e, s_Scene };
        if (entity.HasComponent<CRigidbody2D>())
            s_PhysicsWorld->DestroyBody((b2Body*)entity.GetComponent<CRigidbody2D>().RuntimeBody);
    }

    void PhysicsSystem::SetEnableRigidbody(entt::entity e, bool flag)
    {
        if (!s_Scene)
            return;

        Entity entity{ e, s_Scene };
        if (entity.HasComponent<CRigidbody2D>())
        {
            auto body = (b2Body*)entity.GetComponent<CRigidbody2D>().RuntimeBody;
            body->SetEnabled(flag);
        }
    }

    void ContactListener::BeginContact(b2Contact* contact)
    {
        UUID idA = contact->GetFixtureA()->GetBody()->GetUserData().pointer;
        UUID idB = contact->GetFixtureB()->GetBody()->GetUserData().pointer;

        // 需要修改
        b2WorldManifold manifold;
        contact->GetWorldManifold(&manifold);
        glm::vec2 normal = { manifold.normal.x, manifold.normal.y };

        ScriptSystem::OnCollisionEnter(contact, idA, idB, normal);
    }

    void ContactListener::EndContact(b2Contact* contact)
    {
        UUID idA = contact->GetFixtureA()->GetBody()->GetUserData().pointer;
        UUID idB = contact->GetFixtureB()->GetBody()->GetUserData().pointer;

        // 需要修改
        b2WorldManifold manifold;
        contact->GetWorldManifold(&manifold);
        glm::vec2 normal = { manifold.normal.x, manifold.normal.y };

        ScriptSystem::OnCollisionExit(contact, idA, idB);
    }

    void ContactListener::PreSolve(b2Contact* contact, const b2Manifold* oldManifold)
    {
        UUID idA = contact->GetFixtureA()->GetBody()->GetUserData().pointer;
        UUID idB = contact->GetFixtureB()->GetBody()->GetUserData().pointer;

        // 需要修改
        b2WorldManifold manifold;
        contact->GetWorldManifold(&manifold);
        glm::vec2 normal = { manifold.normal.x, manifold.normal.y };

        ScriptSystem::OnCollisionPreSolve(contact, idA, idB, normal);
    }

    void ContactListener::PostSolve(b2Contact* contact, const b2ContactImpulse* impulse)
    {
        UUID idA = contact->GetFixtureA()->GetBody()->GetUserData().pointer;
        UUID idB = contact->GetFixtureB()->GetBody()->GetUserData().pointer;

        // 需要修改
        b2WorldManifold manifold;
        contact->GetWorldManifold(&manifold);
        glm::vec2 normal = { manifold.normal.x, manifold.normal.y };

        ScriptSystem::OnCollisionPostSolve(contact, idA, idB, normal);
    }
}