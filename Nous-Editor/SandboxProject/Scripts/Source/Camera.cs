using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Nous;

namespace Sandbox
{
	public class Camera : Entity
	{
		public Entity OtherEntity;

		public float DistanceFromPlayer = 5.0f;

		private Entity m_Player;

		public float Speed = 1.5f;

		void OnCreate()
        {
			m_Player = FindEntityByName("Shikuro");
        }

		void OnUpdate(float ts)
		{
			if (m_Player != null)
				Translation = new Vector3(Vector2.Lerp(Translation.XY, m_Player.Translation.XY, ts * Speed), DistanceFromPlayer);
		}

	}
}
