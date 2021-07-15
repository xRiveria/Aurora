#pragma once
#include "../Backend/Widget.h"

/*
	The toolbar is to be ticked together with the Menu Bar.
*/

class Toolbar : public Widget
{
public:
	Toolbar(Editor* editorContext, Aurora::EngineContext* engineContext);

	void OnTickAlways() override;

private:
	float m_ButtonSize = 20.0f;
};