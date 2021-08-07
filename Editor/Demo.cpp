#include "Backend/Editor.h"
#include "Engine.h"
#include <objbase.h> // Temporary to save us pain and suffering from CoInitialize spamming.
#include "../Utilities/Console.h"
#include <iostream>

int main(int argc, int argv[])
{
    HRESULT hr = CoInitialize(NULL);
    srand(time(0)); // Seed generator.

    Console::Shade::SetColor(Console::Color::Color_LightAqua);
    std::cout << "Hello" << "\n";
    Console::Shade::Reset();

    Editor editor;
    
    editor.Tick();

    CoUninitialize();
}