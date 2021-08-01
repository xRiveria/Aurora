#pragma once
#include <unordered_map>
#include "ISubsystem.h"
#include "WindowUtilities.h"

/* Personal Notes:

    - If we are to expand our engine to include a project selector (Unity Hub), perhaps an enum based map will make more sense?
    - The issue with this is that as our engine grows, the windows we create can easily outgrow the enum count. We will take a further look down the road regarding this. 
      For now, we will assume that our map's 0 key is our render window.
    - This class is to be kept as abstract as possible. This is to allow for future API swaps so as to not harm user code.

    - Store Window Width/Height?
*/

namespace Aurora
{
    class WindowContext : public ISubsystem
    {
    public:
        WindowContext(EngineContext* engineContext);

        virtual bool Initialize() override;
        virtual void Shutdown() override;
        void Tick(float deltaTime) override;
        bool IsWindowRunning() const { return m_IsRunning; }
        
        void* Create(const WindowDescription& description);

        void SetCurrentContext(int windowID);
        void SetCurrentContext(void* window);
        void SetCurrentContextTitle_Scene(const std::string& inputSceneName);
        
        bool WindowExistsInMapping(void* window) const;
        bool IsInitialized() const { return m_IsInitialized; }

        float GetWindowWidth(int windowID);
        float GetWindowWidth(void* window);

        float GetWindowHeight(int windowID);
        float GetWindowHeight(void* window);

        float GetWindowDPI(int windowID);
        float GetWindowDPI(void* window);

        HWND GetWindowHWND(int windowID);
        HWND GetWindowHWND(void* window);
        void* GetRenderWindow();

    private:
        std::unordered_map<uint8_t, void*> m_Windows;
        bool m_IsRunning = true;
        bool m_IsInitialized = false;
    };
}