#include "tm4c_sdram.h"

#include "inc/hw_epi.h"
#include "inc/hw_gpio.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"

#include "driverlib/epi.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"

/* Piny EPI0 (alternativní funkce 15 v PCTL). */
typedef struct
{
    uint32_t periph;
    uint32_t base;
    uint8_t pins;
} epi_port_t;

static const epi_port_t s_epi_ports[] =
{
    { SYSCTL_PERIPH_GPIOA, GPIO_PORTA_BASE, GPIO_PIN_7 | GPIO_PIN_6 },
    { SYSCTL_PERIPH_GPIOB, GPIO_PORTB_BASE, GPIO_PIN_3 },
    { SYSCTL_PERIPH_GPIOC, GPIO_PORTC_BASE, GPIO_PIN_7 | GPIO_PIN_6 | GPIO_PIN_5 | GPIO_PIN_4 },
    { SYSCTL_PERIPH_GPIOG, GPIO_PORTG_BASE, GPIO_PIN_1 | GPIO_PIN_0 },
    { SYSCTL_PERIPH_GPIOK, GPIO_PORTK_BASE, GPIO_PIN_5 | GPIO_PIN_3 | GPIO_PIN_2 | GPIO_PIN_1 | GPIO_PIN_0 },
    { SYSCTL_PERIPH_GPIOL, GPIO_PORTL_BASE, GPIO_PIN_3 | GPIO_PIN_2 | GPIO_PIN_1 | GPIO_PIN_0 },
    { SYSCTL_PERIPH_GPIOM, GPIO_PORTM_BASE, GPIO_PIN_3 | GPIO_PIN_2 | GPIO_PIN_1 | GPIO_PIN_0 },
    { SYSCTL_PERIPH_GPION, GPIO_PORTN_BASE, GPIO_PIN_3 | GPIO_PIN_2 },
};

#define EPI_PORT_COUNT (sizeof(s_epi_ports) / sizeof(s_epi_ports[0]))
#define EPI_PERIPH_TIMEOUT 1000000U
#define EPI_INIT_TIMEOUT   10000000U

static bool PinsInit(void)
{
    for (uint32_t i = 0U; i < EPI_PORT_COUNT; ++i)
    {
        SysCtlPeripheralEnable(s_epi_ports[i].periph);
    }

    for (uint32_t i = 0U; i < EPI_PORT_COUNT; ++i)
    {
        const epi_port_t *port = &s_epi_ports[i];
        uint32_t timeout = EPI_PERIPH_TIMEOUT;
        uint32_t pctl;

        while (!SysCtlPeripheralReady(port->periph))
        {
            if (--timeout == 0U)
            {
                return false;
            }
        }

        pctl = HWREG(port->base + GPIO_O_PCTL);
        for (uint32_t bit = 0U; bit < 8U; ++bit)
        {
            if ((port->pins & (1U << bit)) != 0U)
            {
                pctl |= 0xFUL << (4U * bit);
            }
        }
        HWREG(port->base + GPIO_O_PCTL) = pctl;

        GPIOPinTypeEPI(port->base, port->pins);
        /* Silnější výstupy pro stabilní SDRAM sběrnici. */
        GPIOPadConfigSet(port->base, port->pins, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD);
    }

    return true;
}

static uint32_t CoreFreqFlag(uint32_t epi_clock_hz)
{
    if (epi_clock_hz > 50000000UL)
    {
        return EPI_SDRAM_CORE_FREQ_50_100;
    }
    if (epi_clock_hz > 30000000UL)
    {
        return EPI_SDRAM_CORE_FREQ_30_50;
    }
    if (epi_clock_hz > 15000000UL)
    {
        return EPI_SDRAM_CORE_FREQ_15_30;
    }
    return EPI_SDRAM_CORE_FREQ_0_15;
}

bool Tm4cSdramInit(uint32_t sysclk_hz, uint16_t refresh_clocks)
{
    uint32_t epi_clock_hz = sysclk_hz;

    SysCtlPeripheralEnable(SYSCTL_PERIPH_EPI0);
    for (uint32_t timeout = EPI_PERIPH_TIMEOUT; !SysCtlPeripheralReady(SYSCTL_PERIPH_EPI0); --timeout)
    {
        if (timeout == 0U)
        {
            return false;
        }
    }

    if (!PinsInit())
    {
        return false;
    }

    /* Nad 60 MHz běží EPI na polovině systémových hodin. */
    if (sysclk_hz > 60000000UL)
    {
        EPIDividerSet(EPI0_BASE, 1U);
        epi_clock_hz = sysclk_hz / 2U;
    }
    else
    {
        EPIDividerSet(EPI0_BASE, 0U);
    }

    EPIModeSet(EPI0_BASE, EPI_MODE_SDRAM);
    EPIConfigSDRAMSet(EPI0_BASE,
                      CoreFreqFlag(epi_clock_hz) | EPI_SDRAM_FULL_POWER | EPI_SDRAM_SIZE_256MBIT,
                      refresh_clocks);
    EPIAddressMapSet(EPI0_BASE, EPI_ADDR_RAM_SIZE_256MB | EPI_ADDR_RAM_BASE_6);

    for (uint32_t timeout = EPI_INIT_TIMEOUT; (HWREG(EPI0_BASE + EPI_O_STAT) & EPI_STAT_INITSEQ) != 0U; --timeout)
    {
        if (timeout == 0U)
        {
            return false;
        }
    }

    return true;
}

bool Tm4cSdramTest(volatile uint16_t *base, uint32_t words)
{
    for (uint32_t i = 0U; i < words; ++i)
    {
        base[i] = (uint16_t)(i ^ (i >> 16) ^ 0xA5A5U);
    }
    for (uint32_t i = 0U; i < words; ++i)
    {
        if (base[i] != (uint16_t)(i ^ (i >> 16) ^ 0xA5A5U))
        {
            return false;
        }
    }
    return true;
}
