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

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static string Entity_GetName(ulong entityID, out string name);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static string Entity_SetName(ulong entityID, string name);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static string Entity_SetParent(ulong entityID, ulong parentID);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static string Entity_AddChild(ulong entityID, ulong childID);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static int Entity_GetChildCount(ulong entityID);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static ulong Entity_GetChildAt(ulong entityID, int index);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Entity_GetWorldTranslation(ulong entityID, out Vector3 result);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Entity_Destroy(ulong entityID);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Entity_SetActive(ulong entityID, bool active);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool Entity_IsActive(ulong entityID);
		#endregion

		#region TransformComponent
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_GetTranslation(ulong entityID, out Vector3 translation);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_SetTranslation(ulong entityID, Vector3 translation);
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

		#region UITextComponent
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static string UITextComponent_GetText(ulong entityID);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void UITextComponent_SetText(ulong entityID, string text);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void UITextComponent_GetColor(ulong entityID, out Vector4 color);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void UITextComponent_SetColor(ulong entityID, ref Vector4 color);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static float UITextComponent_GetKerning(ulong entityID);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void UITextComponent_SetKerning(ulong entityID, float kerning);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static float UITextComponent_GetLineSpacing(ulong entityID);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void UITextComponent_SetLineSpacing(ulong entityID, float lineSpacing);
		#endregion

		#region Prefab
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static string Prefab_GetFilePath(ulong handle);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Prefab_Instantate(ulong prefabID, out ulong newEntity);
		#endregion

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool Input_IsKeyDown(KeyCode keycode);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Physics_DisableLastContact();


		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool Physics_Contact_IsEnable(ulong contact);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Physics_Contact_SetEnable(ulong contact, bool enable);


		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Game_ChangeScene(string sceneName);

	}
}
