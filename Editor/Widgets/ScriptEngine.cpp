#include "ScriptEngine.h"
#include "../Scripting/Scripting.h"
#include <mono/jit/jit.h>

ScriptEngine::ScriptEngine(Editor* editorContext, Aurora::EngineContext* engineContext) : Widget(editorContext, engineContext)
{
    m_ScriptingSubsystem = m_EngineContext->GetSubsystem<Aurora::Scripting>();
    m_WidgetName = "Script Engine";
}

void ScriptEngine::OnTickVisible()
{
    for (int i = 0; i < m_ScriptingSubsystem->m_ScriptLibrary.size(); i++)
    {
        Aurora::ScriptInstance* script = &m_ScriptingSubsystem->m_ScriptLibrary[i];
        if (ImGui::CollapsingHeader(mono_class_get_name(script->m_MonoClass)))
        {
            MonoClassField* classField = NULL;
            void* iterator = NULL;

            while ((classField = mono_class_get_fields(script->m_MonoClass, &iterator)))
            {
                const char* fieldName = mono_field_get_name(classField);
                MonoType* fieldType = mono_field_get_type(classField);
                int type = mono_type_get_type(fieldType);

                switch (type)
                {
                    case MONO_TYPE_BOOLEAN:
                    {
                        bool boolValue = script->ReadFieldValue<bool>(fieldName);
                        ImGui::Checkbox(fieldName, &boolValue);
                    }
                    break;

                    case MONO_TYPE_R4:
                    {
                        float floatValue = script->ReadFieldValue<float>(fieldName);
                        if (ImGui::DragFloat(fieldName, &floatValue))
                        {
                            script->SetFieldValue<float>(floatValue, fieldName);
                        }
                    }
                    break;

                    case MONO_TYPE_STRING:
                    {
                        char stringBuffer[256];
                        memset(stringBuffer, 0, sizeof(stringBuffer));
                        strcpy_s(stringBuffer, script->ReadFieldString(fieldName).c_str());
                        ImGui::InputText(fieldName, stringBuffer, sizeof(stringBuffer));
                    }
                    break;

                    default:
                        AURORA_ERROR(Aurora::LogLayer::Scripting, "Value type not found for C# field name: %s", fieldName);
                        break;
                }
            }
        }
    }
}
