#include "Backend/Editor.h"
#include "Engine.h"
#include <objbase.h> // Temporary to save us pain and suffering from CoInitialize spamming.

int main(int argc, int argv[])
{
    HRESULT hr = CoInitialize(NULL);
    srand(time(0)); // Seed generator.

    Editor editor;
    
    editor.Tick();

    CoUninitialize();
}