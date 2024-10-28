#include "pch.h"
#include "ScriptGlue.h"
#include "ScriptEngine.h"

#include "Nous/Core/UUID.h"
#include "Nous/Scene/Scene.h"
#include "Nous/Scene/Entity.h"

#include "Nous/Core/KeyCodes.h"
#include "Nous/Core/Input.h"

#include "Nous/Physics/Physics2D.h"

#include "mono/metadata/object.h"
#include "mono/metadata/reflection.h"

#include <box2d/b2_body.h>

namespace Nous
{
	static std::unordered_map<MonoType*, std::function<bool(Entity)>> s_EntityHasComponentFuncs;

#define NS_ADD_INTERNAL_CALL(Name) mono_add_internal_call("Nous.InternalCalls::" #Name, Name)

	// Log
	static void NativeLog(MonoString* text, int parameter)
	{
		char* cStr = mono_string_to_utf8(text);
		std::string str(cStr);
		mono_free(cStr);
		std::cout << str << ", " << parameter << std::endl;
	}

	// Log
	static void NativeLog_Vector(glm::vec3* parameter, glm::vec3* outResult)
	{
		NS_CORE_WARN("Value: {0}", *parameter);
		*outResult = glm::cross(*parameter, glm::vec3(parameter->x, parameter->y, -parameter->z));
	}

	// 获取脚本实例
	static MonoObject* GetScriptInstance(UUID entityID)
	{
		return ScriptEngine::GetManagedInstance(entityID);
	}

	// 实体：是否持有组件
	static bool Entity_HasComponent(UUID entityID, MonoReflectionType* componentType)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NS_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		NS_CORE_ASSERT(entity);

		MonoType* managedType = mono_reflection_type_get_type(componentType);
		NS_CORE_ASSERT(s_EntityHasComponentFuncs.find(managedType) != s_EntityHasComponentFuncs.end());
		return s_EntityHasComponentFuncs.at(managedType)(entity);
	}

	// 实体：根据名称找实体
	static uint64_t Entity_FindEntityByName(MonoString* name)
	{
		char* nameCStr = mono_string_to_utf8(name);

		Scene* scene = ScriptEngine::GetSceneContext();
		NS_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByName(nameCStr);
		mono_free(nameCStr);

		if (!entity)
			return 0;

		return entity.GetUUID();
	}

	// Transform：获取位移
	static void TransformComponent_GetTranslation(UUID entityID, glm::vec3* outTranslation)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NS_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		NS_CORE_ASSERT(entity);

		*outTranslation = entity.GetComponent<CTransform>().Translation;
	}

	// Transform：设置位移
	static void TransformComponent_SetTranslation(UUID entityID, glm::vec3* translation)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NS_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		NS_CORE_ASSERT(entity);

		entity.GetComponent<CTransform>().Translation = *translation;
	}

	// Rigidbody2D：应用线性冲量（施加力）
	static void CRigidbody2D_ApplyLinearImpulse(UUID entityID, glm::vec2* impluse, glm::vec2* point, bool wake)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NS_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		NS_CORE_ASSERT(entity);

		auto& rb2d = entity.GetComponent<CRigidbody2D>();
		b2Body * body = (b2Body*)rb2d.RuntimeBody;
		body->ApplyLinearImpulse(b2Vec2(impluse->x, impluse->y), b2Vec2(point->x, point->y), wake);
	}

	// Rigidbody2D：应用线性冲量（施加力）
	static void CRigidbody2D_ApplyLinearImpulseToCenter(UUID entityID, glm::vec2* impluse, bool wake)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NS_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		NS_CORE_ASSERT(entity);

		auto& rb2d = entity.GetComponent<CRigidbody2D>();
		b2Body* body = (b2Body*)rb2d.RuntimeBody;
		body->ApplyLinearImpulseToCenter(b2Vec2(impluse->x, impluse->y), wake);
	}

	static void CRigidbody2D_GetLinearVelocity(UUID entityID, glm::vec2* outLinearVelocity)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NS_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		NS_CORE_ASSERT(entity);

		auto& rb2d = entity.GetComponent<CRigidbody2D>();
		b2Body* body = (b2Body*)rb2d.RuntimeBody;
		const b2Vec2& linearVelocity = body->GetLinearVelocity();
		*outLinearVelocity = glm::vec2(linearVelocity.x, linearVelocity.y);
	}

	static CRigidbody2D::BodyType CRigidbody2D_GetType(UUID entityID)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NS_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		NS_CORE_ASSERT(entity);

		auto& rb2d = entity.GetComponent<CRigidbody2D>();
		b2Body* body = (b2Body*)rb2d.RuntimeBody;
		return Utils::Rigidbody2DTypeFromBox2DBody(body->GetType());
	}

	static void CRigidbody2D_SetType(UUID entityID, CRigidbody2D::BodyType bodyType)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NS_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		NS_CORE_ASSERT(entity);

		auto& rb2d = entity.GetComponent<CRigidbody2D>();
		b2Body* body = (b2Body*)rb2d.RuntimeBody;
		body->SetType(Utils::Rigidbody2DTypeToBox2DBody(bodyType));
	}

	// 输入：键盘按键按下
	static bool Input_IsKeyDown(KeyCode keycode)
	{
		return Input::IsKeyPressed(keycode);
	}

	// 注册组件
	template<typename... Component>
	static void RegisterComponent()
	{
		([]()
		{
			std::string_view typeName = typeid(Component).name(); // xxx:Cxxxx
			size_t pos = typeName.find_last_of(':');
			std::string_view structName = typeName.substr(pos + 1);
			std::string managedTypeName = fmt::format("Nous.{}", structName);

			MonoType* managedType = mono_reflection_type_from_name(managedTypeName.data(), ScriptEngine::GetCoreAssmblyImage());
			if (!managedType)
			{
				NS_CORE_ERROR("无法找到组件{}", managedTypeName);
				return;
			}
			s_EntityHasComponentFuncs[managedType] = [](Entity entity) { return entity.HasComponent<Component>(); };
		}(), ...);
	}

	template<typename... Component>
	static void RegisterComponent(ComponentGroup<Component...>)
	{
		RegisterComponent<Component...>();
	}

	void ScriptGlue::RegisterComponents()
	{
		s_EntityHasComponentFuncs.clear();
		RegisterComponent(AllComponents{});
	}

	void ScriptGlue::RegisterFunctions()
	{
		NS_ADD_INTERNAL_CALL(NativeLog);
		NS_ADD_INTERNAL_CALL(NativeLog_Vector);

		NS_ADD_INTERNAL_CALL(GetScriptInstance);

		NS_ADD_INTERNAL_CALL(Entity_HasComponent);
		NS_ADD_INTERNAL_CALL(Entity_FindEntityByName);

		NS_ADD_INTERNAL_CALL(TransformComponent_GetTranslation);
		NS_ADD_INTERNAL_CALL(TransformComponent_SetTranslation);

		NS_ADD_INTERNAL_CALL(CRigidbody2D_ApplyLinearImpulse);
		NS_ADD_INTERNAL_CALL(CRigidbody2D_ApplyLinearImpulseToCenter);

		NS_ADD_INTERNAL_CALL(Input_IsKeyDown);
	}
}
