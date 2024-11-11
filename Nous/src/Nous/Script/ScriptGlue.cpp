#include "pch.h"
#include "ScriptGlue.h"
#include "ScriptEngine.h"

#include "Nous/Core/UUID.h"
#include "Nous/Core/Console.h"
#include "Nous/Scene/Scene.h"
#include "Nous/Scene/Entity.h"
#include "Nous/Scene/SceneSerializer.h"
#include "Nous/Core/KeyCodes.h"
#include "Nous/Core/Input.h"

#include "Nous/Physics/Physics2D.h"

#include "Nous/Scene/System/PhysicsSystem.h"
#include "Nous/Scene/System/TransformSystem.h"

#include "Nous/Asset/AssetManager.h"
#include "Nous/Anim/AnimMachine.h"


#include "mono/metadata/object.h"
#include "mono/metadata/reflection.h"

#include <box2d/b2_body.h>
#include <box2d/b2_contact.h>

namespace Nous
{
	namespace Utils
	{
		std::string MonnoStringToString(MonoString* string)
		{
			char* cStr = mono_string_to_utf8(string);
			std::string str(cStr);
			mono_free(cStr);
			return str;
		}
	}

	static std::unordered_map<MonoType*, std::function<bool(Entity)>> s_EntityHasComponentFuncs;

#define NS_ADD_INTERNAL_CALL(Name) mono_add_internal_call("Nous.InternalCalls::" #Name, Name)

#define NS_CORE_ASSERT_ENTITYID(entityID)				\
	Scene* scene = ScriptEngine::GetSceneContext();		\
	NS_CORE_ASSERT(scene);								\
	Entity entity = scene->GetEntityByUUID(entityID);	\
	NS_CORE_ASSERT(entity)

#define NS_CORE_ASSERT_COMPONENT(componentClass) NS_CORE_ASSERT(entity.HasComponent<componentClass>())

	// Log
	static void NousConsole_Log(MonoString* text, MonoString* filepath, int lineNumber)
	{
		std::string cText = Utils::MonnoStringToString(text);
		std::string cFilepath = Utils::MonnoStringToString(filepath);
		
		if (Console::IsInited())
			NS_TRACE("{0} {{{1},{2}}}", cText, cFilepath, lineNumber);
		else
			NS_TRACE(cText);
	}

	// 获取脚本实例
	static MonoObject* Entity_GetScriptInstance(UUID entityID)
	{
		return ScriptEngine::GetManagedInstance(entityID);
	}

	// 实体：是否持有组件
	static bool Entity_HasComponent(UUID entityID, MonoReflectionType* componentType)
	{
		NS_CORE_ASSERT_ENTITYID(entityID)

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

	static void Entity_GetName(UUID entityID, MonoString** name)
	{
		NS_CORE_ASSERT_ENTITYID(entityID);
		NS_CORE_ASSERT(scene);
		Entity e = { scene->GetEntityByUUID(entityID), scene };
		*name = ScriptEngine::CreateString(e.GetName().c_str());
	}

	static void Entity_Instantate(UUID entityID, AssetHandle prefabID, UUID* newEntity)
	{
		NS_CORE_ASSERT_ENTITYID(entityID);
		NS_CORE_ASSERT(scene);
		if (AssetManager::IsAssetHandleValid(prefabID))
		{
			SceneSerializer serializer(scene);
			UUID outRoot;
			serializer.DeserializeTo(prefabID, entityID, &outRoot);
			Entity e = scene->GetEntityByUUID(outRoot);
			auto& tr = e.GetTransform();
			tr.Translation = glm::vec3(0.0f);
			auto instance = ScriptEngine::GetEntityScriptInstance(outRoot);
			instance->InvokeOnCreate();
			*newEntity = outRoot;
		}
	}

	// Transform：获取位移
	static void TransformComponent_GetTranslation(UUID entityID, glm::vec3* outTranslation)
	{
		NS_CORE_ASSERT_ENTITYID(entityID);

		*outTranslation = entity.GetComponent<CTransform>().Translation;
	}

	// Transform：设置位移
	static void TransformComponent_SetTranslation(UUID entityID, glm::vec3* translation)
	{
		NS_CORE_ASSERT_ENTITYID(entityID);

		auto& tr = entity.GetComponent<CTransform>();
		tr.Translation = *translation;
		tr.Dirty = true;

		TransformSystem::SetSubtreeDirty(scene, entity);
	}

	// Rigidbody2D：应用线性冲量（施加力）
	static void Rigidbody2DComponent_ApplyLinearImpulse(UUID entityID, glm::vec2* impluse, glm::vec2* point, bool wake)
	{
		NS_CORE_ASSERT_ENTITYID(entityID);
		NS_CORE_ASSERT_COMPONENT(CRigidbody2D);

		auto& rb2d = entity.GetComponent<CRigidbody2D>();
		b2Body * body = (b2Body*)rb2d.RuntimeBody;
		body->ApplyLinearImpulse(b2Vec2(impluse->x, impluse->y), b2Vec2(point->x, point->y), wake);
	}

	// Rigidbody2D：应用线性冲量（施加力）
	static void Rigidbody2DComponent_ApplyLinearImpulseToCenter(UUID entityID, glm::vec2* impluse, bool wake)
	{
		NS_CORE_ASSERT_ENTITYID(entityID);
		NS_CORE_ASSERT_COMPONENT(CRigidbody2D);

		auto& rb2d = entity.GetComponent<CRigidbody2D>();
		b2Body* body = (b2Body*)rb2d.RuntimeBody;
		body->ApplyLinearImpulseToCenter(b2Vec2(impluse->x, impluse->y), wake);
	}

	static void Rigidbody2DComponent_GetLinearVelocity(UUID entityID, glm::vec2* outLinearVelocity)
	{
		NS_CORE_ASSERT_ENTITYID(entityID);
		NS_CORE_ASSERT_COMPONENT(CRigidbody2D);

		auto& rb2d = entity.GetComponent<CRigidbody2D>();
		b2Body* body = (b2Body*)rb2d.RuntimeBody;
		const b2Vec2& linearVelocity = body->GetLinearVelocity();
		*outLinearVelocity = glm::vec2(linearVelocity.x, linearVelocity.y);
	}

	static void Rigidbody2DComponent_SetLinearVelocity(UUID entityID, glm::vec2* inLinearVelocity)
	{
		NS_CORE_ASSERT_ENTITYID(entityID);
		NS_CORE_ASSERT_COMPONENT(CRigidbody2D);

		auto& rb2d = entity.GetComponent<CRigidbody2D>();
		b2Body* body = (b2Body*)rb2d.RuntimeBody;
		body->SetLinearVelocity({inLinearVelocity->x, inLinearVelocity->y});
	}

	static CRigidbody2D::BodyType Rigidbody2DComponent_GetType(UUID entityID)
	{
		NS_CORE_ASSERT_ENTITYID(entityID);
		NS_CORE_ASSERT_COMPONENT(CRigidbody2D);

		auto& rb2d = entity.GetComponent<CRigidbody2D>();
		b2Body* body = (b2Body*)rb2d.RuntimeBody;
		return Utils::Rigidbody2DTypeFromBox2DBody(body->GetType());
	}

	static void Rigidbody2DComponent_SetType(UUID entityID, CRigidbody2D::BodyType bodyType)
	{
		NS_CORE_ASSERT_ENTITYID(entityID);
		NS_CORE_ASSERT_COMPONENT(CRigidbody2D);

		auto& rb2d = entity.GetComponent<CRigidbody2D>();
		b2Body* body = (b2Body*)rb2d.RuntimeBody;
		body->SetType(Utils::Rigidbody2DTypeToBox2DBody(bodyType));
	}

	static MonoString* TextRendererComponent_GetText(UUID entityID)
	{
		NS_CORE_ASSERT_ENTITYID(entityID);
		NS_CORE_ASSERT_COMPONENT(CTextRenderer);

		auto& tc = entity.GetComponent<CTextRenderer>();
		return ScriptEngine::CreateString(tc.TextString.c_str());
	}

	static void TextRendererComponent_SetText(UUID entityID, MonoString* textString)
	{
		NS_CORE_ASSERT_ENTITYID(entityID);
		NS_CORE_ASSERT_COMPONENT(CTextRenderer);

		auto& tc = entity.GetComponent<CTextRenderer>();
		tc.TextString = Utils::MonnoStringToString(textString);
	}

	static void TextRendererComponent_GetColor(UUID entityID, glm::vec4* color)
	{
		NS_CORE_ASSERT_ENTITYID(entityID);
		NS_CORE_ASSERT_COMPONENT(CTextRenderer);

		auto& tc = entity.GetComponent<CTextRenderer>();
		*color = tc.Color;
	}

	static void TextRendererComponent_SetColor(UUID entityID, glm::vec4* color)
	{
		NS_CORE_ASSERT_ENTITYID(entityID);
		NS_CORE_ASSERT_COMPONENT(CTextRenderer);

		auto& tc = entity.GetComponent<CTextRenderer>();
		tc.Color = *color;
	}

	static float TextRendererComponent_GetKerning(UUID entityID)
	{
		NS_CORE_ASSERT_ENTITYID(entityID);
		NS_CORE_ASSERT_COMPONENT(CTextRenderer);

		auto& tc = entity.GetComponent<CTextRenderer>();
		return tc.Kerning;
	}

	static void TextRendererComponent_SetKerning(UUID entityID, float kerning)
	{
		NS_CORE_ASSERT_ENTITYID(entityID);
		NS_CORE_ASSERT_COMPONENT(CTextRenderer);

		auto& tc = entity.GetComponent<CTextRenderer>();
		tc.Kerning = kerning;
	}

	static float TextRendererComponent_GetLineSpacing(UUID entityID)
	{
		NS_CORE_ASSERT_ENTITYID(entityID);
		NS_CORE_ASSERT_COMPONENT(CTextRenderer);

		auto& tc = entity.GetComponent<CTextRenderer>();
		return tc.LineSpacing;
	}

	static void TextRendererComponent_SetLineSpacing(UUID entityID, float lineSpacing)
	{
		NS_CORE_ASSERT_ENTITYID(entityID);
		NS_CORE_ASSERT_COMPONENT(CTextRenderer);

		auto& tc = entity.GetComponent<CTextRenderer>();
		tc.LineSpacing = lineSpacing;
	}

	static void AnimPlayerComponent_SetFloat(UUID entityID, MonoString* key, float value)
	{
		NS_CORE_ASSERT_ENTITYID(entityID);
		NS_CORE_ASSERT_COMPONENT(CAnimPlayer);

		auto& c = entity.GetComponent<CAnimPlayer>();
		if (c.Type == AssetType::AnimMachine)
		{
			Ref<AnimMachine> m = AssetManager::GetAsset<AnimMachine>(c.AnimClip);
			m->SetFloat(entityID, Utils::MonnoStringToString(key), value);
		}
	}

	static void AnimPlayerComponent_SetBool(UUID entityID, MonoString* key, bool value)
	{
		NS_CORE_ASSERT_ENTITYID(entityID);
		NS_CORE_ASSERT_COMPONENT(CAnimPlayer);

		auto& c = entity.GetComponent<CAnimPlayer>();
		if (c.Type == AssetType::AnimMachine)
		{
			Ref<AnimMachine> m = AssetManager::GetAsset<AnimMachine>(c.AnimClip);
			m->SetBool(entityID, Utils::MonnoStringToString(key), value);
		}
	}

	static MonoString* Prefab_GetFilePath(AssetHandle handle)
	{
		bool valid = AssetManager::IsAssetHandleValid(handle);
		if (valid)
		{
			std::string path = Project::GetActive()->GetEditorAssetManager()->GetFilePath(handle).generic_string();
			return ScriptEngine::CreateString(path.c_str());
		}
		else
			return ScriptEngine::CreateString("");
	}

	// 输入：键盘按键按下
	static bool Input_IsKeyDown(KeyCode keycode)
	{
		return Input::IsKeyPressed(keycode);
	}

	static void Physics_DisableLastContact()
	{
		PhysicsSystem::DisableLastContact();
	}

	static void Physics_Contact_SetEnable(b2Contact* contact, bool enable)
	{
		contact->SetEnabled(enable);
	}

	static bool Physics_Contact_IsEnable(b2Contact* contact)
	{
		return contact->IsEnabled();
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
		NS_ADD_INTERNAL_CALL(NousConsole_Log);

		NS_ADD_INTERNAL_CALL(Entity_GetScriptInstance);
		NS_ADD_INTERNAL_CALL(Entity_HasComponent);
		NS_ADD_INTERNAL_CALL(Entity_FindEntityByName);
		NS_ADD_INTERNAL_CALL(Entity_GetName);
		NS_ADD_INTERNAL_CALL(Entity_Instantate);

		NS_ADD_INTERNAL_CALL(TransformComponent_GetTranslation);
		NS_ADD_INTERNAL_CALL(TransformComponent_SetTranslation);

		NS_ADD_INTERNAL_CALL(Rigidbody2DComponent_ApplyLinearImpulse);
		NS_ADD_INTERNAL_CALL(Rigidbody2DComponent_ApplyLinearImpulseToCenter);
		NS_ADD_INTERNAL_CALL(Rigidbody2DComponent_GetLinearVelocity);
		NS_ADD_INTERNAL_CALL(Rigidbody2DComponent_SetLinearVelocity);
		NS_ADD_INTERNAL_CALL(Rigidbody2DComponent_GetType);
		NS_ADD_INTERNAL_CALL(Rigidbody2DComponent_SetType);

		NS_ADD_INTERNAL_CALL(TextRendererComponent_GetText);
		NS_ADD_INTERNAL_CALL(TextRendererComponent_SetText);
		NS_ADD_INTERNAL_CALL(TextRendererComponent_GetColor);
		NS_ADD_INTERNAL_CALL(TextRendererComponent_SetColor);
		NS_ADD_INTERNAL_CALL(TextRendererComponent_GetKerning);
		NS_ADD_INTERNAL_CALL(TextRendererComponent_SetKerning);
		NS_ADD_INTERNAL_CALL(TextRendererComponent_GetLineSpacing);
		NS_ADD_INTERNAL_CALL(TextRendererComponent_SetLineSpacing);

		NS_ADD_INTERNAL_CALL(AnimPlayerComponent_SetFloat);
		NS_ADD_INTERNAL_CALL(AnimPlayerComponent_SetBool);

		NS_ADD_INTERNAL_CALL(Prefab_GetFilePath);

		NS_ADD_INTERNAL_CALL(Input_IsKeyDown);

		NS_ADD_INTERNAL_CALL(Physics_DisableLastContact);
		NS_ADD_INTERNAL_CALL(Physics_Contact_IsEnable);
		NS_ADD_INTERNAL_CALL(Physics_Contact_SetEnable);
	}
}
