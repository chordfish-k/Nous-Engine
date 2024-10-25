using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Nous;

namespace Sandbox
{
    public class Player : Entity
    {
        void OnCreate()
        {
            Console.WriteLine("Player.OnCreate");
        }

        void OnUpdate(float dt)
        {
            Console.WriteLine($"Player.OnUpdate: {dt}");
        }
    }
}
