using System;
using System.Runtime.CompilerServices;

namespace Nous
{
	public class Entity
	{
		protected Entity() { ID = 0; }

		internal Entity(ulong id)
		{
			ID = id;
		}

		public readonly ulong ID;

		public Vector3 Translation
        {
			get
            {
				InternalCalls.Entity_GetTranslation(ID, out Vector3 translation);
				return translation;
            }

			set
            {
				InternalCalls.Entity_SetTranslation(ID, ref value);
			}
        }
	}
}
