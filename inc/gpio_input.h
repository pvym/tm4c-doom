#ifndef TM4C_GPIO_INPUT_H
#define TM4C_GPIO_INPUT_H

#include <stdint.h>

void GPIO_InputInit(void);
int GPIO_InputPollEvent(int *pressed, unsigned char *key);

#endif
