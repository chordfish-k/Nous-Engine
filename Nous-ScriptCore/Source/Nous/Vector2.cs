using System;

namespace Nous
{
	public struct Vector2
	{
		public float X, Y;

		public static Vector2 Zero = new Vector2(0.0f);

		public Vector2(float scalar)
		{
			X = scalar;
			Y = scalar;
		}

		public Vector2(float x, float y)
		{
			X = x;
			Y = y;
		}

		public static bool operator ==(Vector2 a, Vector2 b)
		{
			return a.X.Equals(b.X) && a.Y.Equals(b.Y);
		}

		public static bool operator !=(Vector2 a, Vector2 b)
		{
			return a.X.Equals(b.X) && a.Y.Equals(b.Y);
		}

		public static Vector2 operator +(Vector2 a, Vector2 b)
		{
			return new Vector2(a.X + b.X, a.Y + b.Y);
		}

		public static Vector2 operator *(Vector2 vector, float scalar)
		{
			return new Vector2(vector.X * scalar, vector.Y * scalar);
		}

		public static Vector2 Lerp(Vector2 source, Vector2 target, float dt)
        {
			return new Vector2((target.X - source.X) * dt + source.X, (target.Y - source.Y) * dt + source.Y);
        }

		public float LengthSquared()
        {
			return X * X + Y * Y;
        }

		public float Length()
        {
			return (float)Math.Sqrt(LengthSquared());
        }

	}
}
