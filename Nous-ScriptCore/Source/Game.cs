using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Nous
{
    public static class Game
    {
        public static void ChangeScene(string sceneName)
        {
            InternalCalls.Game_ChangeScene(sceneName);
        }
    }
}
