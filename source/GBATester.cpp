#include <array>
#include <gba.h>
#include <gba_console.h>
#include <gba_video.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_input.h>
#include <maxmod.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "soundbank.h"
#include "soundbank_bin.h"

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

struct Color
{
    unsigned x : 1;
    unsigned red : 5;
    unsigned blue : 5;
    unsigned green : 5;
};

template <typename type>
static constexpr int toInt(type value)
{
    return static_cast<int>(value);
}

void print(const std::string& msg, const s16 x, const s16 y)
{
    const char* index[] =
    {
        "0", "1", "2", "3", "4", "5", "6", "7", "8", "9",
        "10", "11", "12", "13", "14", "15", "16", "17", "18", "19",
        "20", "21", "22", "23", "24", "25", "26", "27", "28", "29",
    };

    iprintf("\x1b[%s;%sH%s", index[y], index[x], msg.c_str());
}

//---------------------------------------------------------------------------------
// Program entry point
//---------------------------------------------------------------------------------
int main(void)
{
    irqInit();
    // Maxmod requires the vblank interrupt to reset sound DMA.
    // Link the VBlank interrupt to mmVBlank, and enable it.
    irqSet( IRQ_VBLANK, mmVBlank );
    irqEnable(IRQ_VBLANK);

    consoleDemoInit();

    print("GBA Tester", 0, 0);

    // initialise maxmod with soundbank and 8 channels
    mmInitDefault( (mm_addr)soundbank_bin, 8 );

    // Start playing module
    mmStart( MOD_FLATOUTLIES, MM_PLAY_LOOP );

    mm_sound_effect ambulance = {
        { SFX_AMBULANCE } ,         // id
        (int)(1.0f * (1<<10)),  // rate
        0,      // handle
        255,    // volume
        255,      // panning
    };
    mm_sfxhand ambHandle = 0;

    mm_sound_effect boom = {
        { SFX_BOOM } ,          // id
        (int)(1.0f * (1<<10)),  // rate
        0,      // handle
        255,    // volume
        255,    // panning
    };
    mm_sfxhand boomHandle = 0;

    print("Press A to play jump", 0, 1);
    print("Press B to play explosion", 0, 2);
    s16 y = 4;
    for (int currentBtn = toInt(ButtonsT::KEY_A); currentBtn < toInt(ButtonsT::KEY_MAX); ++currentBtn)
    {
        print(std::string(buttonText[currentBtn]), 0, y);
        ++y;
    }

    bool ambulancePlaying = false;
    bool explosionPlaying = false;

    while (1)
    {
        VBlankIntrWait();
        mmFrame();

        // Read buttons
        scanKeys();

        static std::array<bool, static_cast<size_t>(ButtonsT::KEY_MAX)> previousButtons =
            {{true, true, true, true, true, true, true, true, true, true}};

        std::array<bool, static_cast<size_t>(ButtonsT::KEY_MAX)> buttons =
            {{true, true, true, true, true, true, true, true, true, true}};

        y = 4;
        for (int currentBtn = toInt(ButtonsT::KEY_A); currentBtn < toInt(ButtonsT::KEY_MAX); ++currentBtn)
        {
            if ((REG_KEYINPUT & (1 << currentBtn)) != 0)
            {
                buttons[currentBtn] = false; // inverted logic
            }

            if (previousButtons[currentBtn] != buttons[currentBtn])
            {
                print((buttons[currentBtn] ? "Pressed " : "Released"), 7, y);
                previousButtons[currentBtn] = buttons[currentBtn];
            }

            y += 1;
        }

        if (buttons[toInt(ButtonsT::KEY_A)] != ambulancePlaying)
        {
            if (buttons[toInt(ButtonsT::KEY_A)])
            {
                ambHandle = mmEffectEx(&ambulance);
            }
            else
            {
                mmEffectCancel(ambHandle);
            }
            ambulancePlaying = buttons[toInt(ButtonsT::KEY_A)];
        }

        if (buttons[toInt(ButtonsT::KEY_B)] != explosionPlaying)
        {
            if (buttons[toInt(ButtonsT::KEY_B)])
            {
                mmEffectEx(&boom);
            }
            else
            {
                mmEffectCancel(boomHandle);
            }
            explosionPlaying = buttons[toInt(ButtonsT::KEY_B)];
        }
    }
}
