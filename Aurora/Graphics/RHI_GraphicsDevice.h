#pragma once

/* Personal Notes

    Our graphics APIs will inherit from this abstract API class to create their own respective resources. All forms of rendering from the Renderer classes will utilize commands that are 
    implemented by the underlying APIs.

    For now, since we only have DX11, we will implement DX11 directly.
*/

#include "DX11/DX11_GraphicsDevice.h"