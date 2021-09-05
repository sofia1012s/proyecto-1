//*****************************************************************************
// Universidad del Valle de Guatemala
// BE3015: Electrónica Digital 2
// Sofía Salguero - 19236
// Ingeniería en Tecnología de Audio
//*****************************************************************************

//*****************************************************************************
//Librerias
//*****************************************************************************
#include <Arduino.h>     //libreria de arduino
#include "esp_adc_cal.h" //libreria para ADC
#include "Display7Seg.h" //libreria para display 7 segmentos

//*****************************************************************************
//Definicion etiquetas
//*****************************************************************************
#define LM35 32   //toma de datos en sensor
#define boton1 27 //Botón para parte 1

//Parámetro PWM servo motor
#define pwmChannelServo 5
#define freqPWMServo 50
#define resolutionPWMServo 16
#define pinPWMServo 13

//Parámetro PWM led Verde
#define pwmChannelLedV 1
#define freqPWMLedV 5000
#define resolutionPWMLedV 8
#define pinPWMLedV 12

//Parámetro PWM led Amarillo
#define pwmChannelLedA 3
#define freqPWMLedA 5000
#define resolutionPWMLedA 8
#define pinPWMLedA 26

//Parámetro PWM led Rojo
#define pwmChannelLedR 2
#define freqPWMLedR 5000
#define resolutionPWMLedR 8
#define pinPWMLedR 25

//7 segmentos
#define a 4
#define b 16
#define c 5
#define d 19
#define e 18
#define f 2
#define g 15
#define dP 17

#define display1 3
#define display2 23
#define display3 22

//*****************************************************************************
//Varibles globales
//*****************************************************************************
int raw_LM35 = 0;    //valor tomado del sensor
float voltage = 0.0; //voltaje del sensor

//variables para la temperatura
float tempC = 0.0;
int decenas = 0;
int unidades = 0;
int decimal = 0;

int period = 500;
unsigned long time_now = 0;

boolean presionado = 0; //botón ha sido presionado

//Temporizador
hw_timer_t *timer = NULL;
int bandera = 0;

//*****************************************************************************
//Prototipos de funcion
//*****************************************************************************
void configurarPWMLedR(void);
void configurarPWMLedA(void);
void configurarPWMLedV(void);
void configurarBoton1(void);
void configurarPWMServo(void);
float readVoltage(void);
void temperatura(void);
void convertirTemp(void);
void servoLeds(void);
void display7Seg(void);
void configurarTimer(void);
void IRAM_ATTR ISRTimer0();
void IRAM_ATTR ISRBoton1();

//*****************************************************************************
//ISR: interrupciones
//*****************************************************************************
void IRAM_ATTR ISRBoton1() //interrupción para botón 1
{
  static unsigned long ultimo_tiempo_interrupcion = 0; //último tiempo de la interrupción
  unsigned long tiempo_interrupcion = millis();        //tiempo actual de la interrupción

  //Si la interrupcion dura menos de 200ms, asumir que es un rebote e ignorar
  if (tiempo_interrupcion - ultimo_tiempo_interrupcion > 200)
  {
    presionado = 1;
  }
  ultimo_tiempo_interrupcion = tiempo_interrupcion; //actualiza el valor del tiempo de la interrupción
}

void IRAM_ATTR ISRTimer0() //temporizador
{
  switch (bandera)
  {
  case 0:
    bandera = 1;
    break;
  case 1:
    bandera = 2;
    break;

  case 2:
    bandera = 3;
    break;

  case 3:
    bandera = 0;
    break;

  default:
    bandera = 0;
    break;
  }
}

//*****************************************************************************
//configuracion
//*****************************************************************************
void setup()
{
  Serial.begin(115200);
  configurarTimer();

  //Botón
  pinMode(boton1, INPUT_PULLUP);
  configurarBoton1();

  //Señales PWM
  configurarPWMServo();
  configurarPWMLedR();
  configurarPWMLedA();
  configurarPWMLedV();

  //Display 7 Segmentos
  configurarDisplay(a, b, c, d, e, f, g, dP);
  pinMode(display1, OUTPUT);
  pinMode(display2, OUTPUT);
  pinMode(display3, OUTPUT);

  digitalWrite(display1, LOW);
  digitalWrite(display2, LOW);
  digitalWrite(display3, LOW);

  desplegar7Seg(0);
}

//*****************************************************************************
// Loop principal
//*****************************************************************************
void loop()
{
  temperatura();
  convertirTemp();
  servoLeds();
  display7Seg();
  Serial.print("Raw Value = ");
  Serial.println(raw_LM35);
  Serial.print("Voltaje = ");
  Serial.println(voltage);
  Serial.print("Grados = ");
  Serial.println(tempC);
  Serial.print("decenas = ");
  Serial.println(decenas);
  Serial.print("unidades = ");
  Serial.println(unidades);
  Serial.print("decimal = ");
  Serial.println(decimal);
}

//******************************************************************************
// Configuración Timer
//******************************************************************************
void configurarTimer(void)
{
  // Fosc = 80 MHz -> 80,000,000 Hz
  // significa que se ejecuta 80,000,000 ciclos por segundo
  // Si seleccionamos un prescaler de 80
  // entonces 80,000,000 / 80 = 1,000,000 Hz
  // Si obtenemos el período T = 1 / F
  // T = 1 / 1,000,000 = 1 uSeg

  // Paso 2: Seleccionamos el Timer 0, usamos prescaler 80, flanco de subida
  timer = timerBegin(0, 80, true);

  // Paso 3: le asignamos el handler de interrupción
  timerAttachInterrupt(timer, &ISRTimer0, true);

  // Paso 4: programamos la alarma para que se de cada 250mS
  timerAlarmWrite(timer, 250000, true);

  // Paso 5: Iniciamos la alarma
  timerAlarmEnable(timer);
}

//*****************************************************************************
//Función para configurar interrupción en botón 1
//*****************************************************************************
void configurarBoton1(void)
{
  //me coloca una interrupción en el botón 1 (durante el cambio de alto a bajo)
  attachInterrupt(digitalPinToInterrupt(boton1), ISRBoton1, RISING);
}

//*****************************************************************************
//Función para configurar módulo PWM Led Rojo
//*****************************************************************************
void configurarPWMLedR(void)
{
  //Paso 1: Configurar el modulo PWM
  ledcSetup(pwmChannelLedR, freqPWMLedR, resolutionPWMLedR);

  //Paso 2: seleccionar en que GPIO tendremos nuestra señal PWM
  ledcAttachPin(pinPWMLedR, pwmChannelLedR);
}

//*****************************************************************************
//Función para configurar módulo PWM Led Verde
//*****************************************************************************
void configurarPWMLedV(void)
{
  //Paso 1: Configurar el modulo PWM
  ledcSetup(pwmChannelLedV, freqPWMLedV, resolutionPWMLedV);

  //Paso 2: seleccionar en que GPIO tendremos nuestra señal PWM
  ledcAttachPin(pinPWMLedV, pwmChannelLedV);
}

//*****************************************************************************
//Función para configurar módulo PWM Led Amarillo
//*****************************************************************************
void configurarPWMLedA(void)
{
  //Paso 1: Configurar el modulo PWM
  ledcSetup(pwmChannelLedA, freqPWMLedA, resolutionPWMLedA);

  //Paso 2: seleccionar en que GPIO tendremos nuestra señal PWM
  ledcAttachPin(pinPWMLedA, pwmChannelLedA);
}

//*****************************************************************************
//Función para configurar módulo PWM de Servo
//*****************************************************************************
void configurarPWMServo(void)
{
  //Paso 1: Configurar el modulo PWM
  ledcSetup(pwmChannelServo, freqPWMServo, resolutionPWMServo);

  //Paso 2: seleccionar en que GPIO tendremos nuestra señal PWM
  ledcAttachPin(pinPWMServo, pwmChannelServo);
}

//*****************************************************************************
//Función para tomar el valor de la temperatura
//*****************************************************************************
void temperatura(void)
{
  if (presionado == 1)
  {
    voltage = analogReadMilliVolts(LM35);
    presionado = 0;
  }
}
//*****************************************************************************
//Función para convertir el valor de la temperatura
//*****************************************************************************
void convertirTemp(void)
{
  tempC = voltage / 10;
  int temp = tempC * 10; //variable temporal
  decenas = temp / 100;
  temp = temp - (decenas * 100);
  unidades = temp / 10;
  temp = temp - (unidades * 10);
  decimal = temp;
}
//*****************************************************************************
//Función para encender leds y mover servo
//*****************************************************************************
void servoLeds(void)
{
  if (tempC <= 37.0)
  {
    ledcWrite(pwmChannelLedV, 255);
    ledcWrite(pwmChannelLedA, 0);
    ledcWrite(pwmChannelLedR, 0);
    ledcWrite(pwmChannelServo, 6730);
  }

  else if (tempC > 37.0 && tempC <= 37.5)
  {
    ledcWrite(pwmChannelLedV, 0);
    ledcWrite(pwmChannelLedA, 255);
    ledcWrite(pwmChannelLedR, 0);
    ledcWrite(pwmChannelServo, 4546);
  }

  else if (tempC > 37.5)
  {
    ledcWrite(pwmChannelLedV, 0);
    ledcWrite(pwmChannelLedA, 0);
    ledcWrite(pwmChannelLedR, 255);
    ledcWrite(pwmChannelServo, 1362);
  }
}

//*****************************************************************************
//Función para encender displays
//*****************************************************************************
void display7Seg(void)
{
  digitalWrite(display1, HIGH);
  digitalWrite(display2, LOW);
  digitalWrite(display3, LOW);
  desplegarPunto(0);
  desplegar7Seg(decenas);
  delay(500);

  //Desplegar unidades
  digitalWrite(display1, LOW);
  digitalWrite(display2, HIGH);
  digitalWrite(display3, LOW);
  desplegarPunto(1);
  desplegar7Seg(unidades);
  delay(500);

  //Desplegar decimal
  digitalWrite(display1, LOW);
  digitalWrite(display2, LOW);
  digitalWrite(display3, HIGH);
  desplegarPunto(0);
  desplegar7Seg(decimal);
  delay(500);  
}