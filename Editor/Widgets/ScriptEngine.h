#pragma once
#include "../Backend/Widget.h"

/*
	The toolbar is to be ticked together with the Menu Bar.
*/

namespace Aurora
{
	class Scripting;
}

class ScriptEngine : public Widget
{
public:
	ScriptEngine(Editor* editorContext, Aurora::EngineContext* engineContext);

	void OnTickVisible() override;

private:
	Aurora::Scripting* m_ScriptingSubsystem;
};