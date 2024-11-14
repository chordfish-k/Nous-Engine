using System;
using System.Runtime.CompilerServices;

namespace Nous
{
	public class Entity
	{
		protected Entity() { ID = 0; }

		public Entity(ulong id)
		{
			ID = id;
		}

		public readonly ulong ID;

		public Vector3 Translation
        {
			get
            {
				InternalCalls.TransformComponent_GetTranslation(ID, out Vector3 translation);
				return translation;
            }

			set
            {
				InternalCalls.TransformComponent_SetTranslation(ID, value);
			}
        }

		public string Name
        {
			get
			{
				InternalCalls.Entity_GetName(ID, out string name);
				return name;
			}
        }

		public bool HasComponent<T>() where T : Component, new()
        {
			Type componentType = typeof(T);
			return InternalCalls.Entity_HasComponent(ID, componentType);
        }

		public T GetComponent<T>() where T : Component, new()
        {
			if (!HasComponent<T>())
				return null;

			T component = new T() { Entity = this };
			return component;
        }

		public Entity FindEntityByName(string name)
        {
			ulong entityID = InternalCalls.Entity_FindEntityByName(name);
			if (entityID == 0)
				return null;

			return new Entity(entityID);
        }

		public T As<T>() where T : Entity, new()
        {
			object instance = InternalCalls.Entity_GetScriptInstance(ID);
			return instance as T;
        }

		public void SetParent(Entity parent)
        {
			InternalCalls.Entity_SetParent(ID, parent.ID);
        }

		public void AddChild(Entity child)
		{
			InternalCalls.Entity_AddChild(ID, child.ID);
		}

		public int GetChildCount()
		{
			return InternalCalls.Entity_GetChildCount(ID);
		}

		public Entity GetChildAt(int index)
		{
			if (index >= 0)
            {
				ulong child = InternalCalls.Entity_GetChildAt(ID, index);
				if (child == 0)
					return null;
				return new Entity(child);
            }
			return null;
		}

		public Vector3 GetWorldTranslation()
        {
			InternalCalls.Entity_GetWorldTranslation(ID, out Vector3 result);
			return result;
        }
	}
}
