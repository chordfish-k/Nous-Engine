#pragma once

#include "Nous/Scene/Entity.h"
#include "Nous/Scene/Scene.h"

#include <map>

#define MAX_FIELD_DATA_SIZE 16

// 向前声明
extern "C" {
	typedef struct _MonoClass MonoClass;
	typedef struct _MonoObject MonoObject;
	typedef struct _MonoMethod MonoMethod;
	typedef struct _MonoAssembly MonoAssembly;
	typedef struct _MonoImage MonoImage;
	typedef struct _MonoClassField MonoClassField;
	typedef struct _MonoString MonoString;
	typedef struct _MonoType MonoType;
}

namespace Nous
{
	enum class ScriptFieldType
	{
		None = 0,
		Float, Double,
		Bool, Char, Byte, Short, Int, Long,
		UByte, UShort, UInt, ULong,
		Vector2, Vector3, Vector4,
		Entity, Prefab
	};

	struct ScriptField
	{
		ScriptFieldType Type;
		std::string Name;

		MonoClassField* ClassField;
	};

	// 存储字段数据
	struct ScriptFieldInstance
	{
		ScriptField Field;

		ScriptFieldInstance()
		{
			memset(m_Buffer, 0, sizeof(m_Buffer));
		}

		template<typename T>
		T GetValue()
		{
			static_assert(sizeof(T) <= MAX_FIELD_DATA_SIZE, "Type too large!");
			return *(T*)m_Buffer;
		}

		template<typename T>
		void SetValue(T value)
		{
			static_assert(sizeof(T) <= MAX_FIELD_DATA_SIZE, "Type too large!");
			memcpy(m_Buffer, &value, sizeof(T));
		}
	private:
		uint8_t m_Buffer[MAX_FIELD_DATA_SIZE]; // 最大存储MAX_FIELD_DATA_SIZE个字节 

		friend class ScriptEngine;
		friend class ScriptInstance;
	};

	using ScriptFieldMap = std::unordered_map<std::string, ScriptFieldInstance>;

	class ScriptClass
	{
	public:
		ScriptClass() = default;
		ScriptClass(const std::string& classNamespace, const std::string& className, bool isCore = false);
	
		MonoObject* Instantiate();
		MonoMethod* GetMethod(const std::string& name, int parameterCount);
		MonoObject* InvokeMethod(MonoObject* instance, MonoMethod* method, void** params);

		MonoType* GetType();
	
		const std::map<std::string, ScriptField>& GetFields() const { return m_Fields;  }
	private:
		std::string m_ClassNamespace;
		std::string m_ClassName;

		std::map<std::string, ScriptField> m_Fields;

		MonoClass* m_MonoClass = nullptr;

		friend class ScriptEngine;
	};

	class ScriptInstance
	{
	public:
		ScriptInstance(Ref<ScriptClass> scriptClass, Entity entity);

		void InvokeOnCreate();
		void InvokeOnStart();
		void InvokeOnUpdate(float dt);
		void InvokeOnUpdatePhysics(float dt);
		void InvokeOnCollisionPreSolve(void* contactPtr, UUID otherID, glm::vec2& normal);
		void InvokeOnCollisionPostSolve(void* contactPtr, UUID otherID, glm::vec2& normal);
		void InvokeOnCollisionEnter(void* contactPtr, UUID otherID, glm::vec2& normal);
		void InvokeOnCollisionExit(void* contactPtr, UUID otherID);

		Ref<ScriptClass> GetScriptClass() { return m_ScriptClass; }

		template<typename T>
		T GetFieldValue(const std::string& name)
		{
			static_assert(sizeof(T) <= MAX_FIELD_DATA_SIZE, "Type too large!");

			bool success = GetFieldValueInternal(name, s_FieldValueBuffer);
			if (!success)
				return T();
			return *(T*)s_FieldValueBuffer;
		}

		template<typename T>
		void SetFieldValue(const std::string& name, const T& value)
		{
			static_assert(sizeof(T) <= MAX_FIELD_DATA_SIZE, "Type too large!");

			SetFieldValueInternal(name, &value);
		}

		MonoObject* GetManagedObject() { return m_Instance; }
	private:
		bool GetFieldValueInternal(const std::string& name, void* buffer);
		bool SetFieldValueInternal(const std::string& name, const void* value);
	private:
		Ref<ScriptClass> m_ScriptClass;

		MonoObject* m_Instance = nullptr;
		MonoMethod* m_Constructor = nullptr;
		MonoMethod* m_OnCreateMethod = nullptr;
		MonoMethod* m_OnStartMethod = nullptr;
		MonoMethod* m_OnUpdateMethod = nullptr;
		MonoMethod* m_OnUpdatePhysicsMethod = nullptr;
		MonoMethod* m_OnCollisionPreSolveMethod = nullptr;
		MonoMethod* m_OnCollisionPostSolveMethod = nullptr;
		MonoMethod* m_OnCollisionEnterMethod = nullptr;
		MonoMethod* m_OnCollisionExitMethod = nullptr;

		inline static char s_FieldValueBuffer[8];

		friend class ScriptEngine;
		friend class ScriptFieldInstance;
	};

	class ScriptEngine
	{
	public:
		static void Init();
		static void InitApp();
		static void Shutdown();

		static bool LoadAssembly(const std::filesystem::path& filepath);
		static bool LoadAppAssembly(const std::filesystem::path& filepath);
		static std::vector<std::string> FindDependencies();

		static void ReloadAssembly();

		static void OnRuntimeStart(Scene* scene);
		static void OnRuntimeStop();

		static bool EntityClassExists(const std::string& fullClassName);

		static void OnCreateEntity(Entity entity);
		static void OnDestoryEntity(Entity entity);
		static void OnStartEntity(Entity entity);
		static void OnUpdateEntity(Entity entity, Timestep dt);
		static void OnUpdatePhysicsEntity(Entity entity, Timestep dt);
		static void OnCollisionPreSolve(void* contactPtr, Entity entity, UUID otherID, glm::vec2& normal);
		static void OnCollisionPostSolve(void* contactPtr, Entity entity, UUID otherID, glm::vec2& normal);
		static void OnCollisionEnter(void* contactPtr, Entity entity, UUID otherID, glm::vec2& normal);
		static void OnCollisionExit(void* contactPtr, Entity entity, UUID otherID);

		static Scene* GetSceneContext();
		static Ref<ScriptInstance> GetEntityScriptInstance(UUID entityID);

		static Ref<ScriptClass> GetEntityClass(const std::string className);
		static std::unordered_map<std::string, Ref<ScriptClass>>& GetEntityClasses();
		static ScriptFieldMap& GetScriptFieldMap(Entity entity);

		static MonoImage* GetCoreAssmblyImage();

		static MonoObject* GetManagedInstance(UUID uuid);

		static MonoString* CreateString(const char* string);
	private:
		static void InitMono();
		static void ShutdownMono();

		static MonoObject* InstantiateClass(MonoClass* monoClass);
		static void LoadAppAssemblyClasses();

		friend class ScriptClass;
		friend class ScriptGlue;
	};

	namespace Utils
	{
		inline const char* ScriptFieldTypeToString(ScriptFieldType type)
		{
			switch (type)
			{
				case ScriptFieldType::Float:	return "Float";
				case ScriptFieldType::Double:	return "Double";
				case ScriptFieldType::Bool:		return "Bool";
				case ScriptFieldType::Char:		return "Char";
				case ScriptFieldType::Byte:		return "Byte";
				case ScriptFieldType::Short:	return "Short";
				case ScriptFieldType::Int:		return "Int";
				case ScriptFieldType::Long:		return "Long";
				case ScriptFieldType::UByte:	return "UByte";
				case ScriptFieldType::UShort:	return "UShort";
				case ScriptFieldType::UInt:		return "UInt";
				case ScriptFieldType::ULong:	return "ULong";
				case ScriptFieldType::Vector2:	return "Vector2";
				case ScriptFieldType::Vector3:	return "Vector3";
				case ScriptFieldType::Vector4:	return "Vector4";
				case ScriptFieldType::Entity:	return "Entity";
				case ScriptFieldType::Prefab:	return "Prefab";
			}
			NS_CORE_ASSERT(false, "未知的脚本字段类型");
			return "<Invaild>";
		}

		inline ScriptFieldType ScriptFieldTypeFromString(std::string_view type)
		{
			if (type == "None")		return ScriptFieldType::None;
			if (type == "Float")	return ScriptFieldType::Float;
			if (type == "Double")	return ScriptFieldType::Double;
			if (type == "Bool")		return ScriptFieldType::Bool;
			if (type == "Char")		return ScriptFieldType::Char;
			if (type == "Byte")		return ScriptFieldType::Byte;
			if (type == "Short")	return ScriptFieldType::Short;
			if (type == "Int")		return ScriptFieldType::Int;
			if (type == "Long")		return ScriptFieldType::Long;
			if (type == "UByte")	return ScriptFieldType::UByte;
			if (type == "UShort")	return ScriptFieldType::UShort;
			if (type == "UInt")		return ScriptFieldType::UInt;
			if (type == "ULong")	return ScriptFieldType::ULong;
			if (type == "Vector2")	return ScriptFieldType::Vector2;
			if (type == "Vector3")	return ScriptFieldType::Vector3;
			if (type == "Vector4")	return ScriptFieldType::Vector4;
			if (type == "Entity")	return ScriptFieldType::Entity;
			if (type == "Prefab")	return ScriptFieldType::Prefab;

			NS_CORE_ASSERT(false, "未知的脚本字段类型");
			return ScriptFieldType::None;
		}
	}
}


