#pragma once

#include "Nous/Scene/Component.h"

#include <box2d/b2_body.h>

namespace Nous
{
	namespace Utils
	{
        inline static b2BodyType Rigidbody2DTypeToBox2DBody(CRigidbody2D::BodyType bodyType)
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

		inline CRigidbody2D::BodyType Rigidbody2DTypeFromBox2DBody(b2BodyType bodyType)
		{
			switch (bodyType)
			{
			case b2_staticBody:    return CRigidbody2D::BodyType::Static;
			case b2_dynamicBody:   return CRigidbody2D::BodyType::Dynamic;
			case b2_kinematicBody: return CRigidbody2D::BodyType::Kinematic;
			}

			NS_CORE_ASSERT(false, "未知的 body type");
			return CRigidbody2D::BodyType::Static;
		}

	}
}