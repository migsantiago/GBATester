#include <array>
#include <gba_console.h>
#include <gba_video.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_input.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>

enum class ButtonsT : u8
{
    KEY_A,
    KEY_B,
    KEY_SELECT,
    KEY_START,
    KEY_RIGHT,
    KEY_LEFT,
    KEY_UP,
    KEY_DOWN,
    KEY_R,
    KEY_L,
    KEY_MAX,
};

std::array<const char*, static_cast<size_t>(ButtonsT::KEY_MAX)> buttonText =
{{
    "A     ",
    "B     ",
    "Select",
    "Start ",
    "Right ",
    "Left  ",
    "Up    ",
    "Down  ",
    "R     ",
    "L     ",
}};

template <typename type>
static constexpr int toInt(type value)
{
    return static_cast<int>(value);
}

void print(const std::string& msg, const s16 x, const s16 y)
{
    iprintf((std::string("\x1b[")
            + std::to_string(y)
            + ";"
            + std::to_string(x)
            + "H"
            + msg).c_str());
}

//---------------------------------------------------------------------------------
// Program entry point
//---------------------------------------------------------------------------------
int main(void)
{
    // the vblank interrupt must be enabled for VBlankIntrWait() to work
    // since the default dispatcher handles the bios flags no vblank handler
    // is required
    irqInit();
    irqEnable(IRQ_VBLANK);

    consoleDemoInit();

    print("GBA Tester", 0, 0);

//    int keysPressed = 0;
//    int keysReleased = 0;

    while (1)
    {
        VBlankIntrWait();

        // Read buttons
        scanKeys();
//        keysPressed = keysDown();
//        keysReleased = keysUp();

//        (void)keysReleased;

        std::array<bool, static_cast<size_t>(ButtonsT::KEY_MAX)> buttons = {{}};

        s16 y = 2;
        for (int currentBtn = toInt(ButtonsT::KEY_A); currentBtn < toInt(ButtonsT::KEY_MAX); ++currentBtn)
        {
            if ((REG_KEYINPUT & (1 << currentBtn)) != 0)
            {
                buttons[currentBtn] = true;
            }

            print(
                    std::string(buttonText[currentBtn])
                    + " "
                    + (buttons[currentBtn] ? "PRESSED " : "RELEASED"),
                    0,
                    y);

            y += 1;
        }
    }
}
