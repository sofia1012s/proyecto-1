#ifndef __DISPLAY7SEG_H__
#define __DISPLAY7SEG_H__
#include <Arduino.h>

extern uint8_t pinA, pinB, pinC, pinD, pinE, pinF, pinG, pindP;

//Función para configurar display de 7 segmentos
void configurarDisplay(uint8_t A, uint8_t B, uint8_t C, uint8_t D, uint8_t F, uint8_t E, uint8_t G, uint8_t dP);

//Función para desplegar el digito display de 7 segmentos
void desplegar7Seg(uint8_t digito);

//Funcion para desplegar el punto decimal
void desplegarPunto(boolean punto);

#endif // __DISPLAY7SEG_H__