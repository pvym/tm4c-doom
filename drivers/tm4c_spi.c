#include "tm4c_spi.h"

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"

#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/ssi.h"
#include "driverlib/sysctl.h"

#define TM4C_SPI_READY_TIMEOUT 1000000U
#define TM4C_SPI_TX_FIFO_DEPTH 8U

static bool PeripheralsReady(const tm4c_spi_t *spi)
{
    for (uint32_t timeout = TM4C_SPI_READY_TIMEOUT; timeout > 0U; --timeout)
    {
        bool ready = SysCtlPeripheralReady(spi->ssi_periph) &&
                     SysCtlPeripheralReady(spi->clk.gpio_periph) &&
                     SysCtlPeripheralReady(spi->mosi.gpio_periph) &&
                     SysCtlPeripheralReady(spi->miso.gpio_periph);

        if (ready && (spi->fss.pin != 0U))
        {
            ready = SysCtlPeripheralReady(spi->fss.gpio_periph);
        }
        if (ready)
        {
            return true;
        }
    }

    return false;
}

static void DrainRx(const tm4c_spi_t *spi)
{
    uint32_t discard;

    while (SSIBusy(spi->ssi_base))
    {
    }
    while (SSIDataGetNonBlocking(spi->ssi_base, &discard))
    {
    }
}

bool Tm4cSpiInit(const tm4c_spi_t *spi, uint32_t sysclk_hz, uint32_t bitrate_hz)
{
    SysCtlPeripheralEnable(spi->clk.gpio_periph);
    SysCtlPeripheralEnable(spi->mosi.gpio_periph);
    SysCtlPeripheralEnable(spi->miso.gpio_periph);
    if (spi->fss.pin != 0U)
    {
        SysCtlPeripheralEnable(spi->fss.gpio_periph);
    }
    SysCtlPeripheralEnable(spi->ssi_periph);

    if (!PeripheralsReady(spi))
    {
        return false;
    }

    GPIOPinConfigure(spi->clk.pin_config);
    GPIOPinConfigure(spi->mosi.pin_config);
    GPIOPinConfigure(spi->miso.pin_config);
    if (spi->fss.pin != 0U)
    {
        GPIOPinConfigure(spi->fss.pin_config);
        GPIOPinTypeSSI(spi->fss.gpio_base, spi->fss.pin);
    }

    GPIOPinTypeSSI(spi->clk.gpio_base, spi->clk.pin);
    GPIOPinTypeSSI(spi->mosi.gpio_base, spi->mosi.pin);
    GPIOPinTypeSSI(spi->miso.gpio_base, spi->miso.pin);
    GPIOPadConfigSet(spi->clk.gpio_base, spi->clk.pin,
                     GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD);
    GPIOPadConfigSet(spi->mosi.gpio_base, spi->mosi.pin,
                     GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD);
    GPIOPadConfigSet(spi->miso.gpio_base, spi->miso.pin,
                     GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

    Tm4cSpiSetBitrate(spi, sysclk_hz, bitrate_hz);
    return true;
}

void Tm4cSpiSetBitrate(const tm4c_spi_t *spi, uint32_t sysclk_hz, uint32_t bitrate_hz)
{
    SSIDisable(spi->ssi_base);
    SSIConfigSetExpClk(spi->ssi_base, sysclk_hz, spi->protocol,
                       SSI_MODE_MASTER, bitrate_hz, spi->data_width);
    SSIEnable(spi->ssi_base);
    DrainRx(spi);
}

uint32_t Tm4cSpiTransfer(const tm4c_spi_t *spi, uint32_t value)
{
    uint32_t received = 0U;

    SSIDataPut(spi->ssi_base, value);
    while (SSIBusy(spi->ssi_base))
    {
    }
    SSIDataGet(spi->ssi_base, &received);

    return received;
}

void Tm4cSpiReadBlock(const tm4c_spi_t *spi, uint8_t *buffer, uint32_t length, uint8_t fill)
{
    uint32_t sent = (length < TM4C_SPI_TX_FIFO_DEPTH) ? length : TM4C_SPI_TX_FIFO_DEPTH;
    uint32_t received;

    for (uint32_t i = 0U; i < sent; ++i)
    {
        SSIDataPut(spi->ssi_base, fill);
    }

    for (uint32_t i = 0U; i < length; ++i)
    {
        SSIDataGet(spi->ssi_base, &received);
        buffer[i] = (uint8_t)received;

        if (sent < length)
        {
            SSIDataPut(spi->ssi_base, fill);
            ++sent;
        }
    }
}
