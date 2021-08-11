#pragma once
#include "../Backend/Widget.h"
#include "../Backend/Utilities/Extensions.h"

namespace Aurora
{
	class ResourceCache;
}

using namespace EditorExtensions;
class AssetManager : public Widget
{
public:
	AssetManager(Editor* editorContext, Aurora::EngineContext* engineContext);

	void OnTickVisible() override;

private:
	Aurora::ResourceCache* m_ResourceCache;
};
