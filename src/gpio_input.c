#include <stddef.h>
#include <stdbool.h>

#include "config.h"
#include "gpio_input.h"

#include "doomkeys.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"

#define ARRAY_LEN(x) (sizeof(x) / sizeof((x)[0]))

typedef struct
{
    uint32_t periph;
    uint32_t port;
    uint8_t pin;
    unsigned char doom_key;
} tm4c_button_t;

static const tm4c_button_t s_buttons[] = {
    { TM4C_BTN_UP_PERIPH,    TM4C_BTN_UP_PORT,    TM4C_BTN_UP_PIN,    KEY_UPARROW },
    { TM4C_BTN_DOWN_PERIPH,  TM4C_BTN_DOWN_PORT,  TM4C_BTN_DOWN_PIN,  KEY_DOWNARROW },
    { TM4C_BTN_LEFT_PERIPH,  TM4C_BTN_LEFT_PORT,  TM4C_BTN_LEFT_PIN,  KEY_LEFTARROW },
    { TM4C_BTN_RIGHT_PERIPH, TM4C_BTN_RIGHT_PORT, TM4C_BTN_RIGHT_PIN, KEY_RIGHTARROW },
    { TM4C_BTN_FIRE_PERIPH,  TM4C_BTN_FIRE_PORT,  TM4C_BTN_FIRE_PIN,  KEY_FIRE },
    { TM4C_BTN_USE_PERIPH,   TM4C_BTN_USE_PORT,   TM4C_BTN_USE_PIN,   KEY_USE },
    { TM4C_BTN_MENU_PERIPH,  TM4C_BTN_MENU_PORT,  TM4C_BTN_MENU_PIN,  KEY_ESCAPE },
};

static uint32_t s_previous_state;
static uint32_t s_pending_changes;
static uint32_t s_pending_state;

static bool ButtonPressed(const tm4c_button_t *button)
{
    uint8_t value = (uint8_t)GPIOPinRead(button->port, button->pin);
    return value == TM4C_INPUT_ACTIVE_LEVEL;
}

static uint32_t SnapshotButtons(void)
{
    uint32_t state = 0U;

    for (size_t i = 0; i < ARRAY_LEN(s_buttons); ++i)
    {
        if (ButtonPressed(&s_buttons[i]))
        {
            state |= (1UL << i);
        }
    }

    return state;
}

void GPIO_InputInit(void)
{
    uint32_t enabled = 0U;

    for (size_t i = 0; i < ARRAY_LEN(s_buttons); ++i)
    {
        if ((enabled & s_buttons[i].periph) == 0U)
        {
            SysCtlPeripheralEnable(s_buttons[i].periph);
            while (!SysCtlPeripheralReady(s_buttons[i].periph))
            {
            }
            enabled |= s_buttons[i].periph;
        }

        GPIOPinTypeGPIOInput(s_buttons[i].port, s_buttons[i].pin);
        GPIOPadConfigSet(s_buttons[i].port,
                         s_buttons[i].pin,
                         GPIO_STRENGTH_2MA,
                         GPIO_PIN_TYPE_STD_WPU);
    }

    s_previous_state = SnapshotButtons();
    s_pending_changes = 0U;
    s_pending_state = s_previous_state;
}

int GPIO_InputPollEvent(int *pressed, unsigned char *key)
{
    if (s_pending_changes == 0U)
    {
        s_pending_state = SnapshotButtons();
        s_pending_changes = s_pending_state ^ s_previous_state;
        s_previous_state = s_pending_state;
    }

    if (s_pending_changes == 0U)
    {
        return 0;
    }

    for (size_t i = 0; i < ARRAY_LEN(s_buttons); ++i)
    {
        uint32_t mask = (1UL << i);

        if ((s_pending_changes & mask) != 0U)
        {
            s_pending_changes &= ~mask;
            *pressed = ((s_pending_state & mask) != 0U) ? 1 : 0;
            *key = s_buttons[i].doom_key;
            return 1;
        }
    }

    return 0;
}
