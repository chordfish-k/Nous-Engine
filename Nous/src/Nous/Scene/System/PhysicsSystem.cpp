#include "pch.h"
#include "PhysicsSystem.h"

#include "Nous/Scene/Component.h"
#include "Nous/Scene/Entity.h"

// Box2D
#include "box2d/b2_world.h"
#include "box2d/b2_body.h"
#include "box2d/b2_fixture.h"
#include "box2d/b2_polygon_shape.h"
#include "box2d/b2_circle_shape.h"

namespace Nous
{
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
    }

	static Scene* s_Scene = nullptr;
    static b2World* s_PhysicsWorld = nullptr;

	void PhysicsSystem::Start(Scene* scene)
	{
		s_Scene = scene;

        s_PhysicsWorld = new b2World({ 0.0f, -9.8f });
        auto view = s_Scene->GetAllEntitiesWith<CRigidbody2D>();
        for (auto e : view)
        {
            Entity entity = { e, s_Scene };
            auto& transform = entity.GetComponent<CTransform>();
            auto& rb2d = entity.GetComponent<CRigidbody2D>();

            b2BodyDef bodyDef;
            bodyDef.type = Utils::Rigidbody2DTypeToBox2DBody(rb2d.Type);
            bodyDef.position.Set(transform.Translation.x, transform.Translation.y);
            bodyDef.angle = transform.Rotation.z;

            b2Body* body = s_PhysicsWorld->CreateBody(&bodyDef);
            body->SetFixedRotation(rb2d.FixedRotation);
            rb2d.RuntimeBody = body;

            if (entity.HasComponent<CBoxCollider2D>())
            {
                auto& bc2d = entity.GetComponent<CBoxCollider2D>();

                b2PolygonShape boxShape;
                boxShape.SetAsBox(bc2d.Size.x * transform.Scale.x, bc2d.Size.y * transform.Scale.y);

                b2FixtureDef fixtureDef;
                fixtureDef.shape = &boxShape;
                fixtureDef.density = bc2d.Density;
                fixtureDef.friction = bc2d.Friction;
                fixtureDef.restitution = bc2d.Restitution;
                fixtureDef.restitutionThreshold = bc2d.RestitutionThreshold;
                body->CreateFixture(&fixtureDef);
            }

            if (entity.HasComponent<CCircleCollider2D>())
            {
                auto& cc2d = entity.GetComponent<CCircleCollider2D>();

                b2CircleShape circleShape;
                circleShape.m_p.Set(cc2d.Offset.x, cc2d.Offset.y);
                circleShape.m_radius = transform.Scale.x * cc2d.Radius;

                b2FixtureDef fixtureDef;
                fixtureDef.shape = &circleShape;
                fixtureDef.density = cc2d.Density;
                fixtureDef.friction = cc2d.Friction;
                fixtureDef.restitution = cc2d.Restitution;
                fixtureDef.restitutionThreshold = cc2d.RestitutionThreshold;
                body->CreateFixture(&fixtureDef);
            }
        }
	}

	void PhysicsSystem::Update(Timestep dt)
	{
        // 物理更新
        if (s_PhysicsWorld)
        {
            // 控制物理模拟的迭代次数
            const int32_t velocityIterations = 6;
            const int32_t positionIterations = 2;
            s_PhysicsWorld->Step(dt, velocityIterations, positionIterations);

            // 从Box2D中取出transform数据
            auto view = s_Scene->GetAllEntitiesWith<CRigidbody2D>();
            for (auto e : view)
            {
                Entity entity = { e, s_Scene };
                auto& transform = entity.GetComponent<CTransform>();
                auto& rb2d = entity.GetComponent<CRigidbody2D>();

                b2Body* body = (b2Body*)rb2d.RuntimeBody;
                const auto& position = body->GetPosition();
                transform.Translation.x = position.x;
                transform.Translation.y = position.y;
                transform.Rotation.z = body->GetAngle();
            }
        }
	}

	void PhysicsSystem::Stop()
	{
        delete s_PhysicsWorld;
        s_PhysicsWorld = nullptr;
        //s_Scene = nullptr;
	}
}