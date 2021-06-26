#include "Engine.h"
#include <objbase.h> // Temporary to save us pain and suffering from CoInitialize spamming.

int main(int argc, int argv[])
{
    HRESULT hr = CoInitialize(NULL);

    Aurora::Engine engine;

    while (true)
    {
        engine.Tick();
    }

    CoUninitialize();
}