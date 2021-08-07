#pragma once
#include <Windows.h>
#include <map>
#include <string>

/* == Console ==

    Allows us to add color and formatting to our console output. Automates all forms of setting and resetting of the console after usage. There are 16 single colors and thus 256 combinations
    (text + background) that is supported by the Windows console.

    There are two primary namespaces here:

    - Dye: Allows us to manipulate certain words in an object oriented manner.
    - Shade: Sets the state to the specified color and resets as requested by the user.

    Color Information: https://docs.microsoft.com/en-us/windows-server/administration/windows-commands/color
*/

namespace Console
{
    constexpr int CONSOLE_DEFAULT_COLOR = 7; // White

    enum Color
    {
        Color_Black = 0,
        Color_Blue = 1,
        Color_Green = 2,
        Color_Aqua = 3,
        Color_Red = 4,
        Color_Purple = 5,
        Color_Yellow = 6,
        Color_White = 7,
        Color_Grey = 8,
        Color_LightBlue = 9,
        Color_LightGreen = 10,
        Color_LightAqua = 11,
        Color_LightRed = 12,
        Color_LightPurple = 13,
        Color_LightYellow = 14,
        Color_BrightWhite = 15,

        Colors_Count
    };

    namespace Dye
    {

    }

    namespace Shade
    {
        inline bool IsColorSupported(int colorCode)
        {
            return 0 <= colorCode && colorCode < 256;
        }

        inline void SetColor(int colorCode)
        {
            if (IsColorSupported(colorCode))
            {
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), colorCode);
            }
        }

        inline void SetColor(Color color)
        {
            SetColor(static_cast<int>(color));
        }

        inline void Reset()
        {
            SetColor(CONSOLE_DEFAULT_COLOR);
        }
    }
}