#pragma once
#include "../Log/Log.h"
#include <string>

namespace Aurora::ScriptBindings
{
    // Debug.Log
    static void LogString(MonoString* logMessage, LogType logType) { Log::WriteLog(std::string(__FUNCTION__) + "^" + std::string(mono_string_to_utf8(logMessage)), logType); }
    static void LogFloat(float value, LogType logType) { Log::WriteLog(std::string(__FUNCTION__) + "^" + std::to_string(value), logType); }
    
    // static void LogWarning(const std::string& logMessage);
    // static void LogError(const std::string& logMessage);   

    static void RegisterMonoCallbacks()
    {
        mono_add_internal_call("Aurora.Debug::Log(single,Aurora.DebugType)", &Aurora::ScriptBindings::LogFloat);
        mono_add_internal_call("Aurora.Debug::Log(string,Aurora.DebugType)", &Aurora::ScriptBindings::LogString);
    }
}