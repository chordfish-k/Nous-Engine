#include "pch.h"
#include "ScriptGlue.h"

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

	void ScriptGlue::RegisterFunctions()
	{
		NS_ADD_INTERNAL_CALL(NativeLog);
		NS_ADD_INTERNAL_CALL(NativeLog_Vector);
	}
}
