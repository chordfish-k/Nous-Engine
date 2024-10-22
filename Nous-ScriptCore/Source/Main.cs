using System;
using System.Runtime.CompilerServices;

namespace Nous
{
	public struct Vector3
    {
		public float X, Y, Z;

		public Vector3(float x, float y, float z)
        {
			X = x; 
			Y = y; 
			Z = z; 
        }
    }

	public static class InternalCalls
    {
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void NativeLog(string text, int parameter);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void NativeLog_Vector(ref Vector3 parameter, out Vector3 result);
	}

	public class Entity
	{
		public float FloatVar { get; set; }

		public Entity()
		{
			Console.WriteLine("Main constructor!");
			Log("TestLog", 114514);

			Vector3 pos = new Vector3(1, 1, 4);
			Vector3 result = Log(ref pos);
			Console.WriteLine($"{result.X}, {result.Y}, {result.Z}");
		}

		public void PrintMessage()
		{
			Console.WriteLine("Hello World from C#!");
		}

		public void PrintInt(int value)
		{
			Console.WriteLine($"C# says: {value}");
		}

		public void PrintInts(int value1, int value2)
		{
			Console.WriteLine($"C# says: {value1} and {value2}");
		}

		public void PrintCustomMessage(string message)
		{
			Console.WriteLine($"C# says: {message}");
		}

		private void Log(string text, int parameter)
        {
			InternalCalls.NativeLog(text, parameter);
        }

		private Vector3 Log(ref Vector3 parameter)
		{
			InternalCalls.NativeLog_Vector(ref parameter, out Vector3 result);
			return result;
		}

		
	}
}
