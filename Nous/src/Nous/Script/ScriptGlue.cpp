#include "pch.h"
#include "ScriptGlue.h"
#include "ScriptEngine.h"

#include "Nous/Core/UUID.h"
#include "Nous/Scene/Scene.h"
#include "Nous/Scene/Entity.h"

#include "Nous/Core/KeyCodes.h"
#include "Nous/Core/Input.h"

#include "mono/metadata/object.h"

namespace Nous
{
#define NS_ADD_INTERNAL_CALL(Name) mono_add_internal_call("Nous.InternalCalls::" #Name, Name)

	static void NativeLog(MonoString* text, int parameter)
	{
		char* cStr = mono_string_to_utf8(text);
		std::string str(cStr);
		mono_free(cStr);
		std::cout << str << ", " << parameter << std::endl;
	}

	static void NativeLog_Vector(glm::vec3* parameter, glm::vec3* outResult)
	{
		NS_CORE_WARN("Value: {0}", *parameter);
		*outResult = glm::cross(*parameter, glm::vec3(parameter->x, parameter->y, -parameter->z));
	}

	static void Entity_GetTranslation(UUID entityID, glm::vec3* outTranslation)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NS_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		NS_CORE_ASSERT(entity);

		*outTranslation = entity.GetComponent<CTransform>().Translation;
	}

	static void Entity_SetTranslation(UUID entityID, glm::vec3* translation)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		NS_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		NS_CORE_ASSERT(entity);

		entity.GetComponent<CTransform>().Translation = *translation;
	}

	static bool Input_IsKeyDown(KeyCode keycode)
	{
		return Input::IsKeyPressed(keycode);
	}

	void ScriptGlue::RegisterFunctions()
	{
		NS_ADD_INTERNAL_CALL(NativeLog);
		NS_ADD_INTERNAL_CALL(NativeLog_Vector);

		NS_ADD_INTERNAL_CALL(Entity_GetTranslation);
		NS_ADD_INTERNAL_CALL(Entity_SetTranslation);

		NS_ADD_INTERNAL_CALL(Input_IsKeyDown);
	}
}
