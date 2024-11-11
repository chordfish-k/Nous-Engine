using System;
using System.IO;
using System.Runtime.CompilerServices;
using System.Text;

namespace Nous
{
    public static class NousConsole
    {
        public static void Log(string value, 
                            [CallerFilePath] string filepath = "",
                            [CallerLineNumber] int lineNumber = 0)
        { 
            InternalCalls.NousConsole_Log(value, filepath, lineNumber);
        }
    }
}


