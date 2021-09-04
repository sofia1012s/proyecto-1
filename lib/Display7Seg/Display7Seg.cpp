#include "Display7Seg.h"

uint8_t pinA, pinB, pinC, pinD, pinE, pinF, pinG, pindP;

//Función para configurar display de 7 segmentos de ánodo común LOW = encendido, HIGH = apagado
void configurarDisplay(uint8_t A, uint8_t B, uint8_t C, uint8_t D, uint8_t E, uint8_t F, uint8_t G, uint8_t dP)
{
    pinA = A;
    pinB = B;
    pinC = C;
    pinD = D;
    pinE = E;
    pinF = F;
    pinG = G;
    pindP = dP;

    pinMode(pinA, OUTPUT);
    pinMode(pinB, OUTPUT);
    pinMode(pinC, OUTPUT);
    pinMode(pinD, OUTPUT);
    pinMode(pinE, OUTPUT);
    pinMode(pinF, OUTPUT);
    pinMode(pinG, OUTPUT);
    pinMode(pindP, OUTPUT);

    digitalWrite(pinA, HIGH);
    digitalWrite(pinB, HIGH);
    digitalWrite(pinC, HIGH);
    digitalWrite(pinD, HIGH);
    digitalWrite(pinE, HIGH);
    digitalWrite(pinF, HIGH);
    digitalWrite(pinG, HIGH);
    digitalWrite(pindP, HIGH);
}

//Función para desplegar el digito display de 7 segmentos de ánodo común LOW = encendido, HIGH = apagado
void desplegar7Seg(uint8_t digito)
{
    switch (digito)
    {
    case 0:
        digitalWrite(pinA, LOW);
        digitalWrite(pinB, LOW);
        digitalWrite(pinC, LOW);
        digitalWrite(pinD, LOW);
        digitalWrite(pinE, LOW);
        digitalWrite(pinF, LOW);
        digitalWrite(pinG, HIGH);
        break;

    case 1:
        digitalWrite(pinA, HIGH);
        digitalWrite(pinB, LOW);
        digitalWrite(pinC, LOW);
        digitalWrite(pinD, HIGH);
        digitalWrite(pinE, HIGH);
        digitalWrite(pinF, HIGH);
        digitalWrite(pinG, HIGH);
        break;

    case 2:
        digitalWrite(pinA, LOW);
        digitalWrite(pinB, LOW);
        digitalWrite(pinC, HIGH);
        digitalWrite(pinD, LOW);
        digitalWrite(pinE, LOW);
        digitalWrite(pinF, HIGH);
        digitalWrite(pinG, LOW);
        break;

    case 3:
        digitalWrite(pinA, LOW);
        digitalWrite(pinB, LOW);
        digitalWrite(pinC, LOW);
        digitalWrite(pinD, LOW);
        digitalWrite(pinE, HIGH);
        digitalWrite(pinF, HIGH);
        digitalWrite(pinG, LOW);
        break;

    case 4:
        digitalWrite(pinA, HIGH);
        digitalWrite(pinB, LOW);
        digitalWrite(pinC, LOW);
        digitalWrite(pinD, HIGH);
        digitalWrite(pinE, HIGH);
        digitalWrite(pinF, LOW);
        digitalWrite(pinG, LOW);
        break;

    case 5:
        digitalWrite(pinA, LOW);
        digitalWrite(pinB, HIGH);
        digitalWrite(pinC, LOW);
        digitalWrite(pinD, LOW);
        digitalWrite(pinE, HIGH);
        digitalWrite(pinF, LOW);
        digitalWrite(pinG, LOW);
        break;

    case 6:
        digitalWrite(pinA, LOW);
        digitalWrite(pinB, HIGH);
        digitalWrite(pinC, LOW);
        digitalWrite(pinD, LOW);
        digitalWrite(pinE, LOW);
        digitalWrite(pinF, LOW);
        digitalWrite(pinG, LOW);
        break;

    case 7:
        digitalWrite(pinA, LOW);
        digitalWrite(pinB, LOW);
        digitalWrite(pinC, LOW);
        digitalWrite(pinD, HIGH);
        digitalWrite(pinE, HIGH);
        digitalWrite(pinF, HIGH);
        digitalWrite(pinG, HIGH);
        break;

    case 8:
        digitalWrite(pinA, LOW);
        digitalWrite(pinB, LOW);
        digitalWrite(pinC, LOW);
        digitalWrite(pinD, LOW);
        digitalWrite(pinE, LOW);
        digitalWrite(pinF, LOW);
        digitalWrite(pinG, LOW);
        break;

    case 9:
        digitalWrite(pinA, LOW);
        digitalWrite(pinB, LOW);
        digitalWrite(pinC, LOW);
        digitalWrite(pinD, LOW);
        digitalWrite(pinE, HIGH);
        digitalWrite(pinF, LOW);
        digitalWrite(pinG, LOW);
        break;

    default:
        digitalWrite(pinA, LOW);
        digitalWrite(pinB, LOW);
        digitalWrite(pinC, LOW);
        digitalWrite(pinD, LOW);
        digitalWrite(pinE, LOW);
        digitalWrite(pinF, LOW);
        digitalWrite(pinG, HIGH);
        break;
    }
}

//Funcion para desplegar el punto decimal
void desplegarPunto(boolean punto)
{
    if (punto == 1)
    {
        digitalWrite(pindP, LOW);
    }
    else
    {
        digitalWrite(pindP, HIGH);
    }
}
