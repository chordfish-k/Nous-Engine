using System;
using System.Runtime.CompilerServices;

namespace Nous
{
	public static class InternalCalls
	{
		#region NousConsole
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool NousConsole_Log(string log, string filepath, int lineNumber);
		#endregion

		#region Entity
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool Entity_HasComponent(ulong entityID, Type componentType);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static ulong Entity_FindEntityByName(string name);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static object Entity_GetScriptInstance(ulong entityID);
        #endregion

        #region TransformComponent
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_GetTranslation(ulong entityID, out Vector3 translation);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_SetTranslation(ulong entityID, ref Vector3 translation);
		#endregion

        #region Rigidbody2DComponent
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool Rigidbody2DComponent_ApplyLinearImpulse(ulong entityID, ref Vector2 impulse, ref Vector2 point, bool wake);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool Rigidbody2DComponent_ApplyLinearImpulseToCenter(ulong entityID, ref Vector2 impluse, bool wake);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Rigidbody2DComponent_GetLinearVelocity(ulong entityID, out Vector2 linearVelocity);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Rigidbody2DComponent_SetLinearVelocity(ulong entityID, ref Vector2 linearVelocity);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static CRigidbody2D.BodyType Rigidbody2DComponent_GetType(ulong entityID);
		
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Rigidbody2DComponent_SetType(ulong entityID, CRigidbody2D.BodyType type);
        #endregion

        #region TextComponent
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static string TextRendererComponent_GetText(ulong entityID);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void TextRendererComponent_SetText(ulong entityID, string text);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void TextRendererComponent_GetColor(ulong entityID, out Vector4 color);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void TextRendererComponent_SetColor(ulong entityID, ref Vector4 color);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static float TextRendererComponent_GetKerning(ulong entityID);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void TextRendererComponent_SetKerning(ulong entityID, float kerning);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static float TextRendererComponent_GetLineSpacing(ulong entityID);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void TextRendererComponent_SetLineSpacing(ulong entityID, float lineSpacing);
		#endregion

		#region AnimPlayerComponent
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void AnimPlayerComponent_SetFloat(ulong entityID, string key, float value);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void AnimPlayerComponent_SetBool(ulong entityID, string key, bool value);
		#endregion AnimPlayerComponent

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool Input_IsKeyDown(KeyCode keycode);
	}
}
