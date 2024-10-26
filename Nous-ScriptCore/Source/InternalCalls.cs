using System;
using System.Runtime.CompilerServices;

namespace Nous
{
	public static class InternalCalls
	{
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void NativeLog(string text, int parameter);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void NativeLog_Vector(ref Vector3 parameter, out Vector3 result);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Entity_GetTranslation(ulong entityID, out Vector3 translation);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Entity_SetTranslation(ulong entityID, ref Vector3 translation);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool Input_IsKeyDown(KeyCode keycode);
	}
}
