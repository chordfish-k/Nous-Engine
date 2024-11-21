#include "pch.h"
#include "ScriptEngine.h"

#include "ScriptGlue.h"

#include <mono/jit/jit.h>
#include <mono/metadata/attrdefs.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/mono-debug.h>
#include <mono/metadata/threads.h>
#include <mono/metadata/image.h>

#include <FileWatch.h>

#include "Nous/Core/Application.h"
#include "Nous/Core/Timer.h"
#include "Nous/Core/Buffer.h"
#include "Nous/Core/Console.h"
#include "Nous/Core/FileSystem.h"
#include "Nous/Project/Project.h"
#include <glm/gtc/type_ptr.hpp>
#include <set>

namespace Nous
{
	static std::unordered_map<std::string, ScriptFieldType> s_ScriptFieldTypeMap =
	{
		{"System.Single",	ScriptFieldType::Float	},
		{"System.Double",	ScriptFieldType::Double	},
		{"System.Boolean",	ScriptFieldType::Bool	},
		{"System.Char",		ScriptFieldType::Float	},
		{"System.Int16",	ScriptFieldType::Short	},
		{"System.Int32",	ScriptFieldType::Int	},
		{"System.Int64",	ScriptFieldType::Long	},
		{"System.Byte",		ScriptFieldType::Byte	},

		{"System.UByte",	ScriptFieldType::UByte	},
		{"System.UInt16",	ScriptFieldType::UShort	},
		{"System.UInt32",	ScriptFieldType::UInt	},
		{"System.UInt64",	ScriptFieldType::ULong	},

		{"Nous.Entity",		ScriptFieldType::Entity	},
		{"Nous.Prefab",		ScriptFieldType::Prefab	},
		{"Nous.Vector2",	ScriptFieldType::Vector2},
		{"Nous.Vector3",	ScriptFieldType::Vector3},
	};

	namespace Utils
	{
		static MonoAssembly* LoadMonoAssembly(const std::filesystem::path& assemblyPath, bool loadPDB = false)
		{
			ScopedBuffer fileData = FileSystem::ReadFileBinary(assemblyPath);

			// NOTE: We can't use this image for anything other than loading the assembly because this image doesn't have a reference to the assembly
			MonoImageOpenStatus status;
			MonoImage* image = mono_image_open_from_data_full(fileData.As<char>(), fileData.Size(), 1, &status, 0);

			if (status != MONO_IMAGE_OK)
			{
				const char* errorMessage = mono_image_strerror(status);
				// Log some error message using the errorMessage data
				return nullptr;
			}

			if (loadPDB)
			{
				std::filesystem::path pdbPath = assemblyPath;
				pdbPath.replace_extension(".pdb"); // 添加.pdb后缀

				if (std::filesystem::exists(pdbPath))
				{
					ScopedBuffer pdbFileData = FileSystem::ReadFileBinary(pdbPath);
					mono_debug_open_image_from_memory(image, pdbFileData.As<const mono_byte>(), pdbFileData.Size());
					NS_CORE_INFO("Load PDB {}", pdbPath);
				}
			}

			MonoAssembly* assembly = mono_assembly_load_from_full(image, assemblyPath.string().c_str(), &status, 0);
			mono_image_close(image);

			return assembly;
		}

		void PrintAssemblyTypes(MonoAssembly* assembly)
		{
			MonoImage* image = mono_assembly_get_image(assembly);
			const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
			int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);

			for (int32_t i = 0; i < numTypes; i++)
			{
				uint32_t cols[MONO_TYPEDEF_SIZE];
				mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

				const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
				const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);

				NS_CORE_TRACE("{}.{}", nameSpace, name);
			}
		}

		ScriptFieldType MonoTypeToScriptFieldType(MonoType* monoType)
		{
			std::string typeName = mono_type_get_name(monoType);

			auto it = s_ScriptFieldTypeMap.find(typeName);
			if (it == s_ScriptFieldTypeMap.end())
			{
				NS_CORE_ERROR("未知类型: {}", typeName);
				return ScriptFieldType::None;
			}

			return it->second;
		}
	}

	struct ScriptEngineData
	{
		MonoDomain* RootDomain = nullptr;
		MonoDomain* AppDomain = nullptr;

		MonoAssembly* CoreAssembly = nullptr;
		MonoImage* CoreAssemblyImage = nullptr;

		MonoAssembly* AppAssembly = nullptr;
		MonoImage* AppAssemblyImage = nullptr;

		std::filesystem::path CoreAssemblyFilePath;
		std::filesystem::path AppAssemblyFilePath;

		ScriptClass EntityClass;
		ScriptClass CollisionContactClass;
		
		MonoObject* ConsoleTextWriterInstance = nullptr;

		std::unordered_map<std::string, Ref<ScriptClass>> InternalClasses;

		std::unordered_map<std::string, Ref<ScriptClass>> EntityClasses;
		std::unordered_map<UUID, Ref<ScriptInstance>> EntityInstances;
		std::unordered_map<UUID, ScriptFieldMap> EntityScriptFields;

		Scope<filewatch::FileWatch<std::string>> AppAssemblyFileWatcher;
		bool AssemblyReloadPending = false;

		
#if NS_RELEASE
		bool EnableDebugging = false;
#endif
#if NS_DEBUG
		bool EnableDebugging = true;
#endif
		// Runtime
		Scene* SceneContext = nullptr;
	};

	static ScriptEngineData* s_Data = nullptr;

	static void OnAppAssemblyFileSystemEvent(const std::string& path, const filewatch::Event changeType)
	{
		// 如果脚本dll被修改，则重载
		if (!s_Data->AssemblyReloadPending && changeType == filewatch::Event::modified)
		{
			s_Data->AssemblyReloadPending = true;

			// 发送一个待处理事件给主线程执行（此时处于filewatch的线程）
			Application::Get().SubmitToMainThread([]()
			{
				s_Data->AppAssemblyFileWatcher.reset();
				ScriptEngine::ReloadAssembly();
			});
		}
	}

	void ScriptEngine::Init()
	{
		s_Data = new ScriptEngineData();
		InitMono();
	}

	void ScriptEngine::Shutdown()
	{
		ShutdownMono();
		delete s_Data;
	}
	
	void ScriptEngine::InitMono()
	{
		//mono_set_assemblies_path("mono/lib");
		// 如果有中文(unicode)路径
		auto path = std::filesystem::current_path() / "mono/lib";
		_putenv_s("MONO_PATH", path.generic_string().c_str());

		// debug
		if (s_Data->EnableDebugging)
		{
			const char* argv[2] = {
				"--debugger-agent=transport=dt_socket,address=127.0.0.1:56000,server=y,suspend=n,loglevel=3,logfile=MonoDebugger.log",
				"--soft-breakpoints"
			};

			mono_jit_parse_options(2, (char**)argv);
			mono_debug_init(MONO_DEBUG_FORMAT_MONO);
		}
		MonoDomain* rootDomain = mono_jit_init("NousJITRuntime");

		NS_CORE_ASSERT(rootDomain);

		// 存储这个 root domain 指针
		s_Data->RootDomain = rootDomain;

		// debug
		if (s_Data->EnableDebugging)
			mono_debug_domain_create(s_Data->RootDomain);

		mono_thread_set_main(mono_thread_current());
	}

	void ScriptEngine::ShutdownMono()
	{
		if (!s_Data)
			return;

		mono_domain_set(mono_get_root_domain(), false);

		if (s_Data->AppDomain)
			mono_domain_unload(s_Data->AppDomain);
		s_Data->AppDomain = nullptr;

		if (s_Data->RootDomain)
			mono_jit_cleanup(s_Data->RootDomain);
		s_Data->RootDomain = nullptr;
	}

	void ScriptEngine::InitApp()
	{
		if (!s_Data || !s_Data->RootDomain)
			Init();

		mono_domain_set(mono_get_root_domain(), false);


		if (s_Data && s_Data->AppDomain)
		{
			mono_domain_unload(s_Data->AppDomain);
			s_Data->AppDomain = nullptr;
		}
		
		bool status = LoadAssembly("resources/Scripts/Nous-ScriptCore.dll");
		if (!status)
		{
			NS_CORE_ERROR("[ScriptEngine] 无法加载 Nous-ScriptCore.dll");
			return;
		}

		auto scriptModulePath = Project::GetActiveAssetDirectory() / Project::GetActive()->GetConfig().ScriptModulePath;
		status = LoadAppAssembly(scriptModulePath);
		if (!status)
		{
			NS_CORE_ERROR("[ScriptEngine] 无法加载脚本二进制文件");
			return;
		}

		LoadAppAssemblyClasses();

		s_Data->EntityClass = ScriptClass("Nous", "Entity", true);
		s_Data->CollisionContactClass = ScriptClass("Nous", "CollisionContact", true);

		s_Data->InternalClasses["Vector2"] = CreateRef<ScriptClass>("Nous", "Vector2", true);

		ScriptGlue::RegisterFunctions();
		ScriptGlue::RegisterComponents();
	}

	bool ScriptEngine::LoadAssembly(const std::filesystem::path& filepath)
	{
		NS_CORE_TRACE("加载dll: {}", filepath.string());
		// 创建一个 app domain
		s_Data->AppDomain = mono_domain_create_appdomain("NousScriptDomain", nullptr);
		mono_domain_set(s_Data->AppDomain, true);

		s_Data->CoreAssemblyFilePath = filepath;
		s_Data->CoreAssembly = Utils::LoadMonoAssembly(filepath, s_Data->EnableDebugging);
		if (s_Data->CoreAssembly == nullptr)
			return false;

		s_Data->CoreAssemblyImage = mono_assembly_get_image(s_Data->CoreAssembly);
		return true;
	}

	bool ScriptEngine::LoadAppAssembly(const std::filesystem::path& filepath)
	{
		if (std::filesystem::exists(filepath))
		{
			NS_CORE_TRACE("加载dll: {}", filepath.string());
			s_Data->AppAssemblyFilePath = filepath;
			s_Data->AppAssembly = Utils::LoadMonoAssembly(filepath, s_Data->EnableDebugging);
			s_Data->AppAssemblyImage = mono_assembly_get_image(s_Data->AppAssembly);
			if (s_Data->CoreAssembly == nullptr)
				return false;

			// 监视文件变动并重载
			s_Data->AppAssemblyFileWatcher = CreateScope<filewatch::FileWatch<std::string>>(filepath.string(), OnAppAssemblyFileSystemEvent);
			s_Data->AssemblyReloadPending = false;
			return true;
		}
		return false;
	}

	std::vector<std::string> ScriptEngine::FindDependencies()
	{
		static std::vector<std::string> res;
		res.clear();
		MonoFunc func = [](void* assembly, void* user_data) {
			MonoImage* image = mono_assembly_get_image((MonoAssembly*)assembly);
			const char* name = mono_image_get_name(image);
			res.emplace_back(name);
		};
		mono_assembly_foreach(func, nullptr);
		return res;
	}

	void ScriptEngine::ReloadAssembly()
	{
		mono_domain_set(mono_get_root_domain(), false);

		mono_domain_unload(s_Data->AppDomain);

		LoadAssembly(s_Data->CoreAssemblyFilePath);
		LoadAppAssembly(s_Data->AppAssemblyFilePath);
		LoadAppAssemblyClasses();

		ScriptGlue::RegisterComponents();

		s_Data->EntityClass = ScriptClass("Nous", "Entity", true);
	}

	MonoImage* ScriptEngine::GetCoreAssmblyImage()
	{
		return s_Data->CoreAssemblyImage;
	}

	MonoObject* ScriptEngine::GetManagedInstance(UUID uuid)
	{
		if(s_Data->EntityInstances.find(uuid) != s_Data->EntityInstances.end())
			return s_Data->EntityInstances.at(uuid)->GetManagedObject();
		return nullptr;
	}

	MonoString* ScriptEngine::CreateString(const char* string)
	{
		return mono_string_new(s_Data->AppDomain, string);
	}

	void ScriptEngine::OnRuntimeStart(Scene* scene)
	{
		s_Data->SceneContext = scene;
	}

	void ScriptEngine::OnRuntimeStop()
	{
		s_Data->SceneContext = nullptr;

		s_Data->EntityInstances.clear();
	}

	Ref<ScriptInstance> ScriptEngine::GetEntityScriptInstance(UUID entityID)
	{
		auto it = s_Data->EntityInstances.find(entityID);
		if (it == s_Data->EntityInstances.end())
			return nullptr;
		return it->second;
	}

	bool ScriptEngine::EntityClassExists(const std::string& fullClassName)
	{
		return s_Data->EntityClasses.find(fullClassName) != s_Data->EntityClasses.end();
	}

	void ScriptEngine::InvokeInstanceMethod(Entity entity, const std::string& methodName)
	{
		Ref<ScriptInstance> instance = ScriptEngine::GetEntityScriptInstance(entity.GetUUID());
		if (instance)
		{
			const auto method = instance->GetScriptClass()->GetMethod(methodName, 0);
			if (method)
			{
				instance->GetScriptClass()->InvokeMethod(instance->GetManagedObject(), method, nullptr);
			}
			else
			{
				NS_CORE_ERROR("找不到实体 {0} 的 {1} 方法", entity.GetName(), methodName);
			}
		}
		else
		{
			NS_CORE_ERROR("找不到实体 {0}({1}) 的脚本实例", entity.GetName(), entity.GetUUID());
		}
	}

	void ScriptEngine::OnCreateEntity(Entity entity)
	{
		const auto& sc = entity.GetComponent<CMonoScript>();
		if (ScriptEngine::EntityClassExists(sc.ClassName))
		{
			UUID entityID = entity.GetUUID();

			Ref<ScriptInstance> instance = CreateRef<ScriptInstance>(s_Data->EntityClasses[sc.ClassName], entity);
			s_Data->EntityInstances[entity.GetUUID()] = instance;
			
			// 复制缓存的字段到脚本实例
			if (s_Data->EntityScriptFields.find(entityID) != s_Data->EntityScriptFields.end())
			{
				const ScriptFieldMap& fieldMap = s_Data->EntityScriptFields.at(entityID);
				for (const auto& [name, fieldInstance] : fieldMap)
					instance->SetFieldValueInternal(name, fieldInstance.m_Buffer);
			}

			instance->InvokeOnCreate();
		}
	}

	void ScriptEngine::OnDestoryEntity(Entity entity)
	{
		UUID entityUUID = entity.GetUUID();
		if (s_Data->EntityInstances.find(entityUUID) != s_Data->EntityInstances.end())
		{
			// TODO InvokeOnDestroy
			//Ref<ScriptInstance> instance = s_Data->EntityInstances[entityUUID];
			//instance->InvokeOnStart();
			s_Data->EntityInstances.erase(entityUUID);
		}
		else
		{
			NS_CORE_ERROR("[ScriptEngine] 找不到 {0}(id={1}) 的 ScriptInstance", entity.GetName(), entityUUID);
		}
	}

	void ScriptEngine::OnStartEntity(Entity entity)
	{
		UUID entityUUID = entity.GetUUID();
		if (s_Data->EntityInstances.find(entityUUID) != s_Data->EntityInstances.end())
		{
			Ref<ScriptInstance> instance = s_Data->EntityInstances[entityUUID];
			instance->InvokeOnStart();
		}
		else
		{
			NS_CORE_ERROR("[ScriptEngine] 找不到 {0}(id={1}) 的 ScriptInstance", entity.GetName(), entityUUID);
		}
	}

	void ScriptEngine::OnUpdateEntity(Entity entity, Timestep dt)
	{
		UUID entityUUID = entity.GetUUID();
		if (s_Data->EntityInstances.find(entityUUID) != s_Data->EntityInstances.end())
		{
			// 非active，阻止调用
			if (!entity.GetTransform().Active)
				return;

			Ref<ScriptInstance> instance = s_Data->EntityInstances[entityUUID];
			instance->InvokeOnUpdate((float)dt);
		}
		else
		{
			NS_CORE_ERROR("[ScriptEngine] 找不到 {0}(id={1}) 的 ScriptInstance", entity.GetName(), entityUUID);
		}
	}

	void ScriptEngine::OnUpdatePhysicsEntity(Entity entity, Timestep dt)
	{
		UUID entityUUID = entity.GetUUID();
		if (s_Data->EntityInstances.find(entityUUID) != s_Data->EntityInstances.end())
		{
			// 非active，阻止调用
			if (!entity.GetTransform().Active)
				return;

			Ref<ScriptInstance> instance = s_Data->EntityInstances[entityUUID];
			instance->InvokeOnUpdatePhysics((float)dt);
		}
		else
		{
			NS_CORE_ERROR("[ScriptEngine] 找不到 {0}(id={1}) 的 ScriptInstance", entity.GetName(), entityUUID);
		}
	}

	void ScriptEngine::OnCollisionPreSolve(void* contactPtr, Entity entity, UUID otherID, glm::vec2& normal)
	{
		UUID entityUUID = entity.GetUUID();
		if (s_Data->EntityInstances.find(entityUUID) != s_Data->EntityInstances.end())
		{
			// 非active，阻止调用
			if (!entity.GetTransform().Active)
				return;

			Ref<ScriptInstance> instance = s_Data->EntityInstances[entityUUID];
			instance->InvokeOnCollisionPreSolve(contactPtr, otherID, normal);
		}
	}

	void ScriptEngine::OnCollisionPostSolve(void* contactPtr, Entity entity, UUID otherID, glm::vec2& normal)
	{
		UUID entityUUID = entity.GetUUID();
		if (s_Data->EntityInstances.find(entityUUID) != s_Data->EntityInstances.end())
		{
			// 非active，阻止调用
			if (!entity.GetTransform().Active)
				return;

			Ref<ScriptInstance> instance = s_Data->EntityInstances[entityUUID];
			instance->InvokeOnCollisionPostSolve(contactPtr, otherID, normal);
		}
	}

	void ScriptEngine::OnCollisionEnter(void* contactPtr, Entity entity, UUID otherID, glm::vec2& normal)
	{
		UUID entityUUID = entity.GetUUID();
		if (s_Data->EntityInstances.find(entityUUID) != s_Data->EntityInstances.end())
		{
			// 非active，阻止调用
			if (!entity.GetTransform().Active)
				return;

			Ref<ScriptInstance> instance = s_Data->EntityInstances[entityUUID];
			instance->InvokeOnCollisionEnter(contactPtr, otherID, normal);
		}
	}

	void ScriptEngine::OnCollisionExit(void* contactPtr, Entity entity, UUID otherID)
	{
		UUID entityUUID = entity.GetUUID();
		if (s_Data->EntityInstances.find(entityUUID) != s_Data->EntityInstances.end())
		{
			if (!entity.GetTransform().Active)
				return;

			Ref<ScriptInstance> instance = s_Data->EntityInstances[entityUUID];
			instance->InvokeOnCollisionExit(contactPtr, otherID);
		}
	}

	Scene* ScriptEngine::GetSceneContext()
	{
		return s_Data->SceneContext;
	}

	std::unordered_map<std::string, Ref<ScriptClass>>& ScriptEngine::GetEntityClasses()
	{
		return s_Data->EntityClasses;
	}

	Ref<ScriptClass> ScriptEngine::GetEntityClass(const std::string className)
	{
		if (s_Data->EntityClasses.find(className) == s_Data->EntityClasses.end())
			return nullptr;
		return s_Data->EntityClasses.at(className);
	}

	ScriptFieldMap& ScriptEngine::GetScriptFieldMap(Entity entity)
	{
		NS_CORE_ASSERT(entity);

		UUID entityID = entity.GetUUID();
		return s_Data->EntityScriptFields[entityID]; // 如果不存在这个key，会自动创建
	}

	void ScriptEngine::LoadAppAssemblyClasses()
	{
		s_Data->EntityClasses.clear();

		const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(s_Data->AppAssemblyImage, MONO_TABLE_TYPEDEF);
		int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);
		MonoClass* entityClass = mono_class_from_name(s_Data->CoreAssemblyImage, "Nous", "Entity");

		for (int32_t i = 0; i < numTypes; i++)
		{
			uint32_t cols[MONO_TYPEDEF_SIZE];
			mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

			const char* nameSpace = mono_metadata_string_heap(s_Data->AppAssemblyImage, cols[MONO_TYPEDEF_NAMESPACE]);
			const char* className = mono_metadata_string_heap(s_Data->AppAssemblyImage, cols[MONO_TYPEDEF_NAME]);
			std::string fullName;
			if (strlen(nameSpace) != 0)
				fullName = fmt::format("{}.{}", nameSpace, className);
			else
				fullName = className;

			MonoClass* monoClass = mono_class_from_name(s_Data->AppAssemblyImage, nameSpace, className);

			if (monoClass == entityClass)
				continue;

			bool isEntity = mono_class_is_subclass_of(monoClass, entityClass, false);
			if (!isEntity)
				continue;

			Ref<ScriptClass> scriptClass = CreateRef<ScriptClass>(nameSpace, className);
			s_Data->EntityClasses[fullName] = scriptClass;


			int fieldCount = mono_class_num_fields(monoClass);
			NS_CORE_WARN("{} has {} fields:", className, fieldCount);
			void* iterator = nullptr;
			while (MonoClassField* field = mono_class_get_fields(monoClass, &iterator))
			{
				const char* fieldName = mono_field_get_name(field);
				uint32_t flags = mono_field_get_flags(field);
				
				if (flags & MONO_FIELD_ATTR_PUBLIC)
				{
					MonoType* type = mono_field_get_type(field);
					//mono_class_get_type
					ScriptFieldType fieldType = Utils::MonoTypeToScriptFieldType(type);
					NS_CORE_WARN("  {} ({})", fieldName, Utils::ScriptFieldTypeToString(fieldType));

					scriptClass->m_Fields[fieldName] = { fieldType, fieldName, field };
				}
			}
		}
	}

	MonoObject* ScriptEngine::InstantiateClass(MonoClass* monoClass)
	{
		MonoObject* instance = mono_object_new(s_Data->AppDomain, monoClass);
		mono_runtime_object_init(instance);
		return instance;
	}

	ScriptClass::ScriptClass(const std::string& classNamespace, const std::string& className, bool isCore)
		: m_ClassNamespace(classNamespace), m_ClassName(className)
	{
		m_MonoClass = mono_class_from_name(isCore ? s_Data->CoreAssemblyImage : s_Data->AppAssemblyImage, classNamespace.c_str(), className.c_str());
	}

	MonoObject* ScriptClass::Instantiate()
	{
		return ScriptEngine::InstantiateClass(m_MonoClass);
	}

	MonoMethod* ScriptClass::GetMethod(const std::string& name, int parameterCount)
	{
		return mono_class_get_method_from_name(m_MonoClass, name.c_str(), parameterCount);
	}

	MonoObject* ScriptClass::InvokeMethod(MonoObject* instance, MonoMethod* method, void** params)
	{
		MonoObject* execption = nullptr;
		return mono_runtime_invoke(method, instance, params, &execption);
	}

	MonoType* ScriptClass::GetType()
	{
		return mono_class_get_type(m_MonoClass);
	}

	// 获取钩子函数
	ScriptInstance::ScriptInstance(Ref<ScriptClass> scriptClass, Entity entity)
		: m_ScriptClass(scriptClass)
	{
		m_Instance = scriptClass->Instantiate();

		m_Constructor = s_Data->EntityClass.GetMethod(".ctor", 1);
		m_OnCreateMethod = scriptClass->GetMethod("OnCreate", 0);
		m_OnStartMethod = scriptClass->GetMethod("OnStart", 0);
		m_OnUpdateMethod = scriptClass->GetMethod("OnUpdate", 1);
		m_OnUpdatePhysicsMethod = scriptClass->GetMethod("OnUpdatePhysics", 1);
		m_OnCollisionPreSolveMethod = scriptClass->GetMethod("OnCollisionPreSolve", 3);
		m_OnCollisionPostSolveMethod = scriptClass->GetMethod("OnCollisionPostSolve", 3);
		m_OnCollisionEnterMethod = scriptClass->GetMethod("OnCollisionEnter", 3);
		m_OnCollisionExitMethod = scriptClass->GetMethod("OnCollisionExit", 2);

		{
			UUID entityID = entity.GetUUID();
			void* param = &entityID;
			m_ScriptClass->InvokeMethod(m_Instance, m_Constructor, &param);
		}
	}

	void ScriptInstance::InvokeOnCreate()
	{
		if (m_OnCreateMethod)
			m_ScriptClass->InvokeMethod(m_Instance, m_OnCreateMethod, nullptr);
	}

	void ScriptInstance::InvokeOnStart()
	{
		if (m_OnStartMethod)
			m_ScriptClass->InvokeMethod(m_Instance, m_OnStartMethod, nullptr);
	}

	void ScriptInstance::InvokeOnUpdate(float dt)
	{
		if (m_OnUpdateMethod)
		{
			void* param = &dt;
			m_ScriptClass->InvokeMethod(m_Instance, m_OnUpdateMethod, &param);
		}
	}

	void ScriptInstance::InvokeOnUpdatePhysics(float dt)
	{
		if (m_OnUpdatePhysicsMethod)
		{
			void* param = &dt;
			m_ScriptClass->InvokeMethod(m_Instance, m_OnUpdatePhysicsMethod, &param);
		}
	}

	void ScriptInstance::InvokeOnCollisionPreSolve(void* contactPtr, UUID otherID, glm::vec2& normal)
	{
		if (!m_OnCollisionPreSolveMethod)
			return;

		ClassWrapper contact{ contactPtr };

		void* param[3] =
		{
			&contact,
			&otherID,
			&normal
		};
		m_ScriptClass->InvokeMethod(m_Instance, m_OnCollisionPreSolveMethod, param);
	}

	void ScriptInstance::InvokeOnCollisionPostSolve(void* contactPtr, UUID otherID, glm::vec2& normal)
	{
		if (!m_OnCollisionPostSolveMethod)
			return;

		ClassWrapper contact{ contactPtr };

		void* param[3] =
		{
			&contact,
			&otherID,
			&normal
		};
		m_ScriptClass->InvokeMethod(m_Instance, m_OnCollisionPostSolveMethod, param);
	}

	void ScriptInstance::InvokeOnCollisionEnter(void* contactPtr, UUID otherID, glm::vec2& normal)
	{
		if (!m_OnCollisionEnterMethod)
			return;

		ClassWrapper contact{ contactPtr };

		void* param[3] =
		{
			&contact,
			&otherID,
			&normal
		};
		m_ScriptClass->InvokeMethod(m_Instance, m_OnCollisionEnterMethod, param);
	}

	void ScriptInstance::InvokeOnCollisionExit(void* contactPtr, UUID otherID)
	{
		if (!m_OnCollisionExitMethod)
			return;

		ClassWrapper contact{ contactPtr };

		void* param[2] =
		{
			&contact,
			&otherID
		};
		m_ScriptClass->InvokeMethod(m_Instance, m_OnCollisionExitMethod, param);
	}

	bool ScriptInstance::GetFieldValueInternal(const std::string& name, void* buffer)
	{
		const auto& fields = m_ScriptClass->GetFields();
		auto it = fields.find(name);
		if (it == fields.end())
			return false;

		const ScriptField& field = it->second;
		mono_field_get_value(m_Instance, field.ClassField, buffer);
		return true;
	}

	bool ScriptInstance::SetFieldValueInternal(const std::string& name, const void* value)
	{
		const auto& fields = m_ScriptClass->GetFields();
		auto it = fields.find(name);
		if (it == fields.end())
			return false;

		const ScriptField& field = it->second;
		mono_field_set_value(m_Instance, field.ClassField, (void*)value);
		return true;
	}
}