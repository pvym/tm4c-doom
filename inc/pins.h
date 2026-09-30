//*****************************************************************************
//
// pins.h - Maps logical pin names to physical device pins.
//
// Copyright (c) 2011-2019 Pavel Vymetálek, vym.cz
//
//*****************************************************************************

#ifndef __PINS_H__
#define __PINS_H__


//*****************************************************************************
//
// \defgroup pins_api Definitions
// @{
//
//*****************************************************************************

//*****************************************************************************
// UART - port
#define PIN_UART0RX_PORT        GPIO_PORTA_BASE
#define PIN_UART0TX_PORT        GPIO_PORTA_BASE

// UART - pins
#define PIN_UART0RX_PIN         GPIO_PIN_0
#define PIN_UART0TX_PIN         GPIO_PIN_1



//*****************************************************************************
// vstupy pro hlidani napajeni a central stopu
// BAT24 - U24 - CSI - CSI(IN1 -


#define PIN_IN123_PORT			GPIO_PORTP_BASE
#define PIN_IN1_CHECK_VOLTAGE	GPIO_PIN_6			/// IN1 - CSI - vstupni napeti z central stopky z panelu
#define PIN_IN2_CENTRAL_STOP	GPIO_PIN_7			/// IN2 - CSO - vystup stopky displeje pro napajeni FPF
#define PIN_IN3_MAIN_POWER		GPIO_PIN_1			/// IN3 - D24 napajeni displeje

// ovladani podsviceni displeje
#define PIN_LCD_BACKLIGHT_PORT	GPIO_PORTF_BASE
#define PIN_LCD_BACKLIGHT		GPIO_PIN_4

// ovladani napajeni motoru
// #define PIN_MOTOR_VOLTAGE_PORT	GPIO_PORTG_BASE
// #define PIN_MOTOR_VOLTAGE		GPIO_PIN_5

// klavesnice

#define PIN_KEY_COLUMN_A_PORT	GPIO_PORTD_BASE
#define PIN_KEY_COLUMN_B_PORT	GPIO_PORTD_BASE
#define PIN_KEY_COLUMN_C_PORT	GPIO_PORTD_BASE
#define PIN_KEY_COLUMN_D_PORT	GPIO_PORTE_BASE

#define PIN_KEY_COLUMN_A		GPIO_PIN_1
#define PIN_KEY_COLUMN_B		GPIO_PIN_2
#define PIN_KEY_COLUMN_C		GPIO_PIN_3
#define PIN_KEY_COLUMN_D		GPIO_PIN_2

#define PIN_KEY_ROW_1_PORT		GPIO_PORTE_BASE
#define PIN_KEY_ROW_2_PORT		GPIO_PORTD_BASE
#define PIN_KEY_ROW_3_PORT		GPIO_PORTD_BASE
#define PIN_KEY_ROW_4_PORT		GPIO_PORTD_BASE
#define PIN_KEY_ROW_5_PORT		GPIO_PORTD_BASE
#define PIN_KEY_ROW_6_PORT		GPIO_PORTE_BASE

#define PIN_KEY_ROW_1			GPIO_PIN_3
#define PIN_KEY_ROW_2			GPIO_PIN_4
#define PIN_KEY_ROW_3			GPIO_PIN_5
#define PIN_KEY_ROW_4			GPIO_PIN_6
#define PIN_KEY_ROW_5			GPIO_PIN_7
#define PIN_KEY_ROW_6			GPIO_PIN_7


// Podsviceni
// PH
#define LCD_PIN_BACKLIGHT		GPIO_PIN_5
#define LCD_PIN_BACKLIGHT_PORT	GPIO_PORTH_BASE

// LCD - SPI interface
#define LCD_SSI_PERIPH          SYSCTL_PERIPH_SSI0
#define LCD_SSI_BASE            SSI0_BASE

#define LCD_SSI_GPIO_PERIPH     SYSCTL_PERIPH_GPIOA
#define LCD_SSI_GPIO_BASE       GPIO_PORTA_BASE

#define LCD_SSI_CLK_CFG         GPIO_PA2_SSI0CLK
#define LCD_SSI_FSS_CFG 		GPIO_PA3_SSI0FSS
#define LCD_SSI_TX_CFG          GPIO_PA4_SSI0XDAT0
#define LCD_SSI_RX_CFG          GPIO_PA5_SSI0XDAT1

// LCD
//*****************************************************************************
//
// Defines for the pins that are used to communicate with the LCD
//
//*****************************************************************************
#define LCD_RST_PERIPH			SYSCTL_PERIPH_GPIOF
#define LCD_RST_BASE			GPIO_PORTF_BASE
#define LCD_RST_PIN				GPIO_PIN_6

#define LCD_BACKLIGHT_PERIPH	SYSCTL_PERIPH_GPIOH
#define LCD_BACKLIGHT_BASE		GPIO_PORTH_BASE
#define LCD_BACKLIGHT_PIN		GPIO_PIN_5

#define LCD_ENABLE_PERIPH		SYSCTL_PERIPH_GPIOJ
#define LCD_ENABLE_BASE			GPIO_PORTJ_BASE
#define LCD_ENABLE_PIN			GPIO_PIN_6

//***************************************************************************

// PA
#define LCD_PIN_DCLK			GPIO_PIN_2		//	PA2 - DWRX
#define LCD_PIN_DCSX			GPIO_PIN_3		//	PA3 - /CS
#define LCD_PIN_DSDA			GPIO_PIN_4		//	PA4 - XDAT0
#define LCD_PIN_DSDO			GPIO_PIN_5		//	PA5 - XDAT0

//PF
#define LCD_PIN_DRDX			GPIO_PIN_4		// PF4
#define LCD_PIN_DRESET			GPIO_PIN_6		// PF6
#define LCD_PIN_DD2				GPIO_PIN_7		// PF7

//PR
#define LCD_PIN_DOTCLK			GPIO_PIN_0		// PR0
#define LCD_PIN_VSYNC			GPIO_PIN_1		// PR1
#define LCD_PIN_HSYNC			GPIO_PIN_2		// PR2
#define LCD_PIN_DD3				GPIO_PIN_3		// PR3
#define LCD_PIN_DD0				GPIO_PIN_4		// PR4
#define LCD_PIN_DD1				GPIO_PIN_5		// PR5
#define LCD_PIN_DD4				GPIO_PIN_6		// PR6
#define LCD_PIN_DD5				GPIO_PIN_7		// PR7

//PS
#define LCD_PIN_DD6				GPIO_PIN_4		// PS4
#define LCD_PIN_DD7				GPIO_PIN_5		// PS5
#define LCD_PIN_DD8				GPIO_PIN_6		// PS6
#define LCD_PIN_DD9				GPIO_PIN_7		// PS7

// PT
#define LCD_PIN_DD10			GPIO_PIN_0		// PT0
#define LCD_PIN_DD11			GPIO_PIN_1		// PT1

// PN
#define LCD_PIN_DD12			GPIO_PIN_7		// PN7
#define LCD_PIN_DD13			GPIO_PIN_6		// PN6

// PJ
#define LCD_PIN_DD14			GPIO_PIN_2		// PJ2
#define LCD_PIN_DD15			GPIO_PIN_3		// PJ3
#define LCD_PIN_LCDAC			GPIO_PIN_6		// PJ6

// 27 pinu k LCD



//*****************************************************************************
//
// The GPIO port on which the phase A low side pin resides.
//
//*****************************************************************************
#define PIN_PHASEA_LOW_PORT     GPIO_PORTF_BASE

//*****************************************************************************
//
// The GPIO pin on which the phase A low side pin resides.
//
//*****************************************************************************
#define PIN_PHASEA_LOW_PIN      GPIO_PIN_1

//*****************************************************************************
//
// The PWM channel on which the phase A low side resides.
//
//*****************************************************************************
#define PWM_PHASEA_LOW          (1 << 1)

//*****************************************************************************
//
// The GPIO port on which the phase A high side pin resides.
//
//*****************************************************************************
#define PIN_PHASEA_HIGH_PORT    GPIO_PORTF_BASE

//*****************************************************************************
//
// The GPIO pin on which the phase A high side pin resides.
//
//*****************************************************************************
#define PIN_PHASEA_HIGH_PIN     GPIO_PIN_0

//*****************************************************************************
//
// The PWM channel on which the phase A high side resides.
//
//*****************************************************************************
#define PWM_PHASEA_HIGH         (1 << 0)

//*****************************************************************************
//
// The GPIO port on which the quadrature encoder index pin resides.
//
//*****************************************************************************
#define PIN_INDEX_PORT          GPIO_PORTB_BASE

//*****************************************************************************
//
// The GPIO pin on which the quadrature encoder index pin resides.
//
//*****************************************************************************
#define PIN_INDEX_PIN           GPIO_PIN_2

//*****************************************************************************
//
// The GPIO port on which the user push button resides.
//
//*****************************************************************************
#define PIN_SWITCH_PORT         GPIO_PORTB_BASE

//*****************************************************************************
//
// The GPIO pin on which the user push button resides.
//
//*****************************************************************************
#define PIN_SWITCH_PIN          GPIO_PIN_3

//*****************************************************************************
//
// The bit lane of the GPIO pin on which the user push button resides.
//
//*****************************************************************************
#define PIN_SWITCH_PIN_BIT      3

//*****************************************************************************
//
// The GPIO port on which the run LED resides.
//
//*****************************************************************************
#define PIN_LEDRUN_PORT         GPIO_PORTG_BASE

//*****************************************************************************
//
// The GPIO pin on which the run LED resides.
//
//*****************************************************************************
#define PIN_LEDRUN_PIN          GPIO_PIN_0

//*****************************************************************************
//
// The GPIO port on which the fault LED resides.
//
//*****************************************************************************
#define PIN_LEDFAULT_PORT       GPIO_PORTC_BASE

//*****************************************************************************
//
// The GPIO pin on which the fault LED resides.
//
//*****************************************************************************
#define PIN_LEDFAULT_PIN        GPIO_PIN_5

//*****************************************************************************
//
// The GPIO port on which the quadrature encoder channel A pin resides.
//
//*****************************************************************************
#define PIN_ENCA_PORT           GPIO_PORTC_BASE

//*****************************************************************************
//
// The GPIO pin on which the quadrature encoder channel A pin resides.
//
//*****************************************************************************
#define PIN_ENCA_PIN            GPIO_PIN_4

//*****************************************************************************
//
// The GPIO port on which the quadrature encoder channel B pin resides.
//
//*****************************************************************************
#define PIN_ENCB_PORT           GPIO_PORTC_BASE

//*****************************************************************************
//
// The GPIO pin on which the quadrature encoder channel B pin resides.
//
//*****************************************************************************
#define PIN_ENCB_PIN            GPIO_PIN_7

//*****************************************************************************
//
// The GPIO port on which the brake pin resides.
//
//*****************************************************************************
#define PIN_BRAKE_PORT          GPIO_PORTC_BASE

//*****************************************************************************
//
// The GPIO pin on which the brake pin resides.
//
//*****************************************************************************
#define PIN_BRAKE_PIN           GPIO_PIN_6

//*****************************************************************************
//
// The GPIO port on which the phase B low side pin resides.
//
//*****************************************************************************
#define PIN_PHASEB_LOW_PORT     GPIO_PORTD_BASE

//*****************************************************************************
//
// The GPIO pin on which the phase B low side pin resides.
//
//*****************************************************************************
#define PIN_PHASEB_LOW_PIN      GPIO_PIN_3

//*****************************************************************************
//
// The PWM channel on which the phase B low side resides.
//
//*****************************************************************************
#define PWM_PHASEB_LOW          (1 << 3)

//*****************************************************************************
//
// The GPIO port on which the phase B high side pin resides.
//
//*****************************************************************************
#define PIN_PHASEB_HIGH_PORT    GPIO_PORTD_BASE

//*****************************************************************************
//
// The GPIO pin on which the phase B high side pin resides.
//
//*****************************************************************************
#define PIN_PHASEB_HIGH_PIN     GPIO_PIN_2

//*****************************************************************************
//
// The PWM channel on which the phase B high side resides.
//
//*****************************************************************************
#define PWM_PHASEB_HIGH         (1 << 2)

//*****************************************************************************
//
// The GPIO port on which the Ethernet status zero LED resides.
//
//*****************************************************************************
#define PIN_LEDSTATUS0_PORT     GPIO_PORTF_BASE

//*****************************************************************************
//
// The GPIO pin on which the Ethernet status zero LED resides.
//
//*****************************************************************************
#define PIN_LEDSTATUS0_PIN      GPIO_PIN_3

//*****************************************************************************
//
// The GPIO port on which the Ethernet status one LED resides.
//
//*****************************************************************************
#define PIN_LEDSTATUS1_PORT     GPIO_PORTF_BASE

//*****************************************************************************
//
// The GPIO pin on which the Ethernet status one LED resides.
//
//*****************************************************************************
#define PIN_LEDSTATUS1_PIN      GPIO_PIN_2

//*****************************************************************************
//
// The GPIO port on which the phase C low side pin resides.
//
//*****************************************************************************
#define PIN_PHASEC_LOW_PORT     GPIO_PORTE_BASE

//*****************************************************************************
//
// The GPIO pin on which the phase C low side pin resides.
//
//*****************************************************************************
#define PIN_PHASEC_LOW_PIN      GPIO_PIN_1

//*****************************************************************************
//
// The PWM channel on which the phase C low side resides.
//
//*****************************************************************************
#define PWM_PHASEC_LOW          (1 << 5)

//*****************************************************************************
//
// The GPIO port on which the phase C high side pin resides.
//
//*****************************************************************************
#define PIN_PHASEC_HIGH_PORT    GPIO_PORTE_BASE

//*****************************************************************************
//
// The GPIO pin on which the phase C high side pin resides.
//
//*****************************************************************************
#define PIN_PHASEC_HIGH_PIN     GPIO_PIN_0

//*****************************************************************************
//
// The PWM channel on which the phase C high side resides.
//
//*****************************************************************************
#define PWM_PHASEC_HIGH         (1 << 4)

//*****************************************************************************
//
// The GPIO port on which the CAN0 Rx pin resides.
//
//*****************************************************************************
#define PIN_CAN0RX_PORT         GPIO_PORTD_BASE

//*****************************************************************************
//
// The GPIO pin on which the CAN0 Rx pin resides.
//
//*****************************************************************************
#define PIN_CAN0RX_PIN          GPIO_PIN_0

//*****************************************************************************
//
// The GPIO port on which the CAN0 Tx pin resides.
//
//*****************************************************************************
#define PIN_CAN0TX_PORT         GPIO_PORTD_BASE

//*****************************************************************************
//
// The GPIO pin on which the CAN0 Tx pin resides.
//
//*****************************************************************************
#define PIN_CAN0TX_PIN          GPIO_PIN_1

//*****************************************************************************
//
// The GPIO port on which the CFG0 pin resides.
//
//*****************************************************************************
#define PIN_CFG0_PORT           GPIO_PORTA_BASE

//*****************************************************************************
//
// The GPIO pin on which the CFG0 pin resides.
//
//*****************************************************************************
#define PIN_CFG0_PIN            GPIO_PIN_2

//*****************************************************************************
//
// The GPIO port on which the CFG1 pin resides.
//
//*****************************************************************************
#define PIN_CFG1_PORT           GPIO_PORTA_BASE

//*****************************************************************************
//
// The GPIO pin on which the CFG1 pin resides.
//
//*****************************************************************************
#define PIN_CFG1_PIN            GPIO_PIN_3

//*****************************************************************************
//
// The GPIO port on which the CFG2 pin resides.
//
//*****************************************************************************
#define PIN_CFG2_PORT           GPIO_PORTA_BASE

//*****************************************************************************
//
// The GPIO pin on which the CFG2 pin resides.
//
//*****************************************************************************
#define PIN_CFG2_PIN            GPIO_PIN_4

//*****************************************************************************
//
// The GPIO port on which the HALL A pin resides.
//
//*****************************************************************************
#define PIN_HALLA_PORT          GPIO_PORTB_BASE

//*****************************************************************************
//
// The GPIO pin on which the HALL A pin resides.
//
//*****************************************************************************
#define PIN_HALLA_PIN           GPIO_PIN_4

//*****************************************************************************
//
// The GPIO port on which the HALL B pin resides.
//
//*****************************************************************************
#define PIN_HALLB_PORT          GPIO_PORTB_BASE

//*****************************************************************************
//
// The GPIO pin on which the HALL B pin resides.
//
//*****************************************************************************
#define PIN_HALLB_PIN           GPIO_PIN_5

//*****************************************************************************
//
// The GPIO port on which the HALL C pin resides.
//
//*****************************************************************************
#define PIN_HALLC_PORT          GPIO_PORTB_BASE

//*****************************************************************************
//
// The GPIO pin on which the HALL C pin resides.
//
//*****************************************************************************
#define PIN_HALLC_PIN           GPIO_PIN_6

//*****************************************************************************
//
// The ADC channel on which the DC bus voltage sense resides.
//
//*****************************************************************************
#define PIN_VSENSE              ADC_CTL_CH0

//*****************************************************************************
//
// The ADC channel on which the DC Back EMF (Phase A) voltage sense resides.
//
//*****************************************************************************
#define PIN_VBEMFA              ADC_CTL_CH1

//*****************************************************************************
//
// The ADC channel on which the DC Back EMF (Phase B) voltage sense resides.
//
//*****************************************************************************
#define PIN_VBEMFB              ADC_CTL_CH2

//*****************************************************************************
//
// The ADC channel on which the DC Back EMF (Phase C) voltage sense resides.
//
//*****************************************************************************
#define PIN_VBEMFC              ADC_CTL_CH3

//*****************************************************************************
//
// The ADC channel on which the phase A current sense resides.
//
//*****************************************************************************
#define PIN_IPHASEA             ADC_CTL_CH4

//*****************************************************************************
//
// The ADC channel on which the phase B current sense resides.
//
//*****************************************************************************
#define PIN_IPHASEB             ADC_CTL_CH5

//*****************************************************************************
//
// The ADC channel on which the phase C current sense resides.
//
//*****************************************************************************
#define PIN_IPHASEC             ADC_CTL_CH6

//*****************************************************************************
//
// The ADC channel on which the Analog Input voltage sense resides.
//
//*****************************************************************************
#define PIN_VANALOG             ADC_CTL_CH7

//*****************************************************************************
//
// Close the Doxygen group.
// @}
//
//*****************************************************************************

#endif // __PINS_H__
