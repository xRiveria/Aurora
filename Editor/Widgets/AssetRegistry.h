#pragma once
#include "../Backend/Widget.h"
#include "../Backend/Utilities/Extensions.h"

namespace Aurora
{
	class ResourceCache;
}

using namespace EditorExtensions;
class AssetRegistry : public Widget
{
public:
	AssetRegistry(Editor* editorContext, Aurora::EngineContext* engineContext);

	void OnTickVisible() override;

private:
	ImGuiTextFilter m_RegistryFilter;

private:
	Aurora::ResourceCache* m_ResourceCache;
};
