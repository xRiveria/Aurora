using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace AuroraEngine
{
    public enum DebugType
    {
        Info    = 0,
        Warning = 1,
        Error   = 2
    }

    public class Debug
    {
        [MethodImpl(MethodImplOptions.InternalCall)] // Internal to our C++ code that is exposing it.
        public extern static void Log(string logMessage, DebugType debugType = DebugType.Info); // Mark as extern to tell Mono that it is not in our implementation, rather in C++.

        [MethodImpl(MethodImplOptions.InternalCall)] 
        public extern static void Log(float value, DebugType debugType = DebugType.Info);
    }
}
