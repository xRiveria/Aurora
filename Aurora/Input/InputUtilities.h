#pragma once
#include <string>

/* Personal Notes:

    The numbers line up with ASCII code. This means that we can convert the numbers to a char and we can print the character to the console.
    Refer to: https://www.glfw.org/docs/3.3/group__input.html.

*/

// Mouse Buttons

#define AURORA_MOUSE_BUTTON_1         0
#define AURORA_MOUSE_BUTTON_2         1
#define AURORA_MOUSE_BUTTON_3         2
#define AURORA_MOUSE_BUTTON_4         3
#define AURORA_MOUSE_BUTTON_5         4
#define AURORA_MOUSE_BUTTON_6         5
#define AURORA_MOUSE_BUTTON_7         6
#define AURORA_MOUSE_BUTTON_8         7
#define AURORA_MOUSE_BUTTON_LAST      AURORA_MOUSE_BUTTON_8
#define AURORA_MOUSE_BUTTON_LEFT      AURORA_MOUSE_BUTTON_1
#define AURORA_MOUSE_BUTTON_RIGHT     AURORA_MOUSE_BUTTON_2
#define AURORA_MOUSE_BUTTON_MIDDLE    AURORA_MOUSE_BUTTON_3

// Key Buttons

#define AURORA_KEY_SPACE              32
#define AURORA_KEY_APOSTROPHE         39  /* ' */
#define AURORA_KEY_COMMA              44  /* , */
#define AURORA_KEY_MINUS              45  /* - */
#define AURORA_KEY_PERIOD             46  /* . */
#define AURORA_KEY_SLASH              47  /* / */
#define AURORA_KEY_0                  48
#define AURORA_KEY_1                  49
#define AURORA_KEY_2                  50
#define AURORA_KEY_3                  51
#define AURORA_KEY_4                  52
#define AURORA_KEY_5                  53
#define AURORA_KEY_6                  54
#define AURORA_KEY_7                  55
#define AURORA_KEY_8                  56
#define AURORA_KEY_9                  57
#define AURORA_KEY_SEMICOLON          59  /* ; */
#define AURORA_KEY_EQUAL              61  /* = */
#define AURORA_KEY_A                  65
#define AURORA_KEY_B                  66
#define AURORA_KEY_C                  67
#define AURORA_KEY_D                  68
#define AURORA_KEY_E                  69
#define AURORA_KEY_F                  70
#define AURORA_KEY_G                  71
#define AURORA_KEY_H                  72
#define AURORA_KEY_I                  73
#define AURORA_KEY_J                  74
#define AURORA_KEY_K                  75
#define AURORA_KEY_L                  76
#define AURORA_KEY_M                  77
#define AURORA_KEY_N                  78
#define AURORA_KEY_O                  79
#define AURORA_KEY_P                  80
#define AURORA_KEY_Q                  81
#define AURORA_KEY_R                  82
#define AURORA_KEY_S                  83
#define AURORA_KEY_T                  84
#define AURORA_KEY_U                  85
#define AURORA_KEY_V                  86
#define AURORA_KEY_W                  87
#define AURORA_KEY_X                  88
#define AURORA_KEY_Y                  89
#define AURORA_KEY_Z                  90
#define AURORA_KEY_LEFT_BRACKET       91  /* [ */
#define AURORA_KEY_BACKSLASH          92  /* \ */
#define AURORA_KEY_RIGHT_BRACKET      93  /* ] */
#define AURORA_KEY_GRAVE_ACCENT       96  /* ` */
#define AURORA_KEY_WORLD_1            161 /* Non-US #1 */
#define AURORA_KEY_WORLD_2            162 /* Non-US #2 */
#define AURORA_KEY_ESCAPE             256
#define AURORA_KEY_ENTER              257
#define AURORA_KEY_TAB                258
#define AURORA_KEY_BACKSPACE          259
#define AURORA_KEY_INSERT             260
#define AURORA_KEY_DELETE             261
#define AURORA_KEY_RIGHT              262
#define AURORA_KEY_LEFT               263
#define AURORA_KEY_DOWN               264
#define AURORA_KEY_UP                 265
#define AURORA_KEY_PAGE_UP            266
#define AURORA_KEY_PAGE_DOWN          267
#define AURORA_KEY_HOME               268
#define AURORA_KEY_END                269
#define AURORA_KEY_CAPS_LOCK          280
#define AURORA_KEY_SCROLL_LOCK        281
#define AURORA_KEY_NUM_LOCK           282
#define AURORA_KEY_PRINT_SCREEN       283
#define AURORA_KEY_PAUSE              284
#define AURORA_KEY_F1                 290
#define AURORA_KEY_F2                 291
#define AURORA_KEY_F3                 292
#define AURORA_KEY_F4                 293
#define AURORA_KEY_F5                 294
#define AURORA_KEY_F6                 295
#define AURORA_KEY_F7                 296
#define AURORA_KEY_F8                 297
#define AURORA_KEY_F9                 298
#define AURORA_KEY_F10                299
#define AURORA_KEY_F11                300
#define AURORA_KEY_F12                301
#define AURORA_KEY_F13                302
#define AURORA_KEY_F14                303
#define AURORA_KEY_F15                304
#define AURORA_KEY_F16                305
#define AURORA_KEY_F17                306
#define AURORA_KEY_F18                307
#define AURORA_KEY_F19                308
#define AURORA_KEY_F20                309
#define AURORA_KEY_F21                310
#define AURORA_KEY_F22                311
#define AURORA_KEY_F23                312
#define AURORA_KEY_F24                313
#define AURORA_KEY_F25                314
#define AURORA_KEY_KP_0               320
#define AURORA_KEY_KP_1               321
#define AURORA_KEY_KP_2               322
#define AURORA_KEY_KP_3               323
#define AURORA_KEY_KP_4               324
#define AURORA_KEY_KP_5               325
#define AURORA_KEY_KP_6               326
#define AURORA_KEY_KP_7               327
#define AURORA_KEY_KP_8               328
#define AURORA_KEY_KP_9               329
#define AURORA_KEY_KP_DECIMAL         330
#define AURORA_KEY_KP_DIVIDE          331
#define AURORA_KEY_KP_MULTIPLY        332
#define AURORA_KEY_KP_SUBTRACT        333
#define AURORA_KEY_KP_ADD             334
#define AURORA_KEY_KP_ENTER           335
#define AURORA_KEY_KP_EQUAL           336
#define AURORA_KEY_LEFT_SHIFT         340
#define AURORA_KEY_LEFT_CONTROL       341
#define AURORA_KEY_LEFT_ALT           342
#define AURORA_KEY_LEFT_SUPER         343
#define AURORA_KEY_RIGHT_SHIFT        344
#define AURORA_KEY_RIGHT_CONTROL      345
#define AURORA_KEY_RIGHT_ALT          346
#define AURORA_KEY_RIGHT_SUPER        347
#define AURORA_KEY_MENU               348