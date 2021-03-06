//*****************************************************************************
// Universidad del Valle de Guatemala
// BE3015: Electrónica Digital 2
// Sofía Salguero - 19236
// Ingeniería en Tecnología de Audio
//*****************************************************************************

//*****************************************************************************
//Librerias
//*****************************************************************************
#include <Arduino.h>         //libreria de arduino
#include "esp_adc_cal.h"     //libreria para ADC
#include "Display7Seg.h"     //libreria para display 7 segmentos
#include "AdafruitIO_WiFi.h" //libreria para Adafruit

//*****************************************************************************
//Definicion etiquetas
//*****************************************************************************
#define LM35 33   //toma de datos en sensor
#define boton1 27 //Botón para tomar temperatura

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
#define freqPWMLedR 500037
#define resolutionPWMLedR 8
#define pinPWMLedR 25

//Display 7 segmentos
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

//Prescaler
#define prescaler 80

//*****************************************************************************
//Adafruit
//*****************************************************************************

/************************ Adafruit IO Config *******************************/
#define IO_USERNAME  "sal19236"
#define IO_KEY       "aio_atXN96TScOUPONoV9fOITVkMpbi5"

/******************************* WIFI **************************************/
#define WIFI_SSID "Familia Salguero"
#define WIFI_PASS "Salguero2019"

AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PASS);

/******************************* Feeds **************************************/
AdafruitIO_Feed *temp = io.feed("tempc");

//*****************************************************************************
//Varibles globales
//*****************************************************************************

//variables para la temperatura
float tempC = 0.0;
int decenas = 0;
int unidades = 0;
int decimal = 0;

boolean presionado = 0; //botón ha sido presionado

//Variables para filtro Medio Móvil Exponencial
float adcRaw = 0.0;        //Valor Crudo
double adcFiltradoEMA = 0; // S(0) = Y(0)
double alpha = 0.09;       // Factor de suavizado
float voltage = 0.0;       //Valor de voltaje filtrado

//Temporizadores
hw_timer_t *timer = NULL;
hw_timer_t *timer1 = NULL;
int contadorTimer = 0;
int contadorTimer1 = 0;

//*****************************************************************************
//Prototipos de funcion
//*****************************************************************************
//Leds
void configurarPWMLedR(void);
void configurarPWMLedA(void);
void configurarPWMLedV(void);

//Botón
void configurarBoton1(void);

//Servo
void configurarPWMServo(void);
void servoLeds(void);

//Temperatura
float readVoltage(void);
void temperatura(void);
void emaADC(void);
void convertirTemp(void);

//Displays
void display7Seg(int contadorTimer);

//Timers
void configurarTimer(void);
void configurarTimer1(void);

//interrupciones
void IRAM_ATTR ISRTimer0();
void IRAM_ATTR ISRTimer1();
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

void IRAM_ATTR ISRTimer0() //interrupción para timer de displays
{
  contadorTimer++; //aumenta el contador de timer

  if (contadorTimer > 2) //si es mayor a 2 regresa a cero
  {
    contadorTimer = 0;
  }
}

void IRAM_ATTR ISRTimer1() //interrupción para timer de Adafruit
{
  contadorTimer1 = 1;

  if (contadorTimer1 > 1)
  {
    contadorTimer1 = 0;
  }
}

//*****************************************************************************
//Configuración
//*****************************************************************************
void setup()
{
  //Setup de adafruit
  Serial.begin(115200);
  Serial.print("Connecting to Adafruit IO");
  io.connect();
  while (io.status() < AIO_CONNECTED)
  {
    //Espero la conexión
  }
  Serial.println();
  Serial.println(io.statusText());

  //Temporizadores
  configurarTimer();
  configurarTimer1();

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
  emaADC();                   //Tomar temperatura y filtrarla
  temperatura();              //Tomar temperatura para mostrarla en displays
  servoLeds();                //Mover servo y encender leds según el valor
  convertirTemp();            //Convertir valores de temperatura para displays
  display7Seg(contadorTimer); //Mostrar la temperatura en los displays

  while (contadorTimer1 == 1) //actualiza el valor de la temperatura a Adafruit
  {
    io.run();
    temp->save(tempC);
    contadorTimer1 = 0;
  }
}

//******************************************************************************
// Configuración Timers
//******************************************************************************
void configurarTimer(void) //Timer para displays
{
  //Fosc = 80MHz = 80,000,000 Hz
  //Fosc / Prescaler = 80,000,000 / 80 = 1,000,000
  //Tosc = 1/Fosc = 1uS

  //Timer 0, prescaler = 80, flanco de subida
  timer = timerBegin(0, prescaler, true);

  //Handler de la interrupción
  timerAttachInterrupt(timer, &ISRTimer0, true);

  //Tic = 1uS     1ms = 1000uS
  timerAlarmWrite(timer, 1000, true);

  //Inicia alarma
  timerAlarmEnable(timer);
}

void configurarTimer1(void) //Timer para setup Adafruit
{
  //Fosc = 80MHz = 80,000,000 Hz
  //Fosc / Prescaler = 80,000,000 / 80 = 1,000,000
  //Tosc = 1/Fosc = 1uS

  //Timer 1, prescaler = 80, flanco de subida
  timer1 = timerBegin(1, prescaler, true);

  //Asignar el handler de la interrupción
  timerAttachInterrupt(timer1, &ISRTimer1, true);

  //Tic = 1uS   3s = 3000000 uS
  timerAlarmWrite(timer1, 3000000, true);

  //Iniciar la alarma
  timerAlarmEnable(timer1);
}
//*****************************************************************************
//Configuración interrupción en botón 1
//*****************************************************************************
void configurarBoton1(void)
{
  //me coloca una interrupción en el botón 1 (durante el cambio de alto a bajo)
  attachInterrupt(digitalPinToInterrupt(boton1), ISRBoton1, RISING);
}

//*****************************************************************************
//Configuración módulo PWM Led Rojo
//*****************************************************************************
void configurarPWMLedR(void)
{
  //Configurar el modulo PWM
  ledcSetup(pwmChannelLedR, freqPWMLedR, resolutionPWMLedR);

  //Seleccionar en qué GPIO tendremos nuestra señal PWM
  ledcAttachPin(pinPWMLedR, pwmChannelLedR);
}

//*****************************************************************************
//Configuración módulo PWM Led Verde
//*****************************************************************************
void configurarPWMLedV(void)
{
  //Configurar el modulo PWM
  ledcSetup(pwmChannelLedV, freqPWMLedV, resolutionPWMLedV);

  //Seleccionar en qué GPIO tendremos nuestra señal PWM
  ledcAttachPin(pinPWMLedV, pwmChannelLedV);
}

//*****************************************************************************
//Configuración módulo PWM Led Amarillo
//*****************************************************************************
void configurarPWMLedA(void)
{
  //Configurar el modulo PWM
  ledcSetup(pwmChannelLedA, freqPWMLedA, resolutionPWMLedA);

  //Seleccionar en qué GPIO tendremos nuestra señal PWM
  ledcAttachPin(pinPWMLedA, pwmChannelLedA);
}

//*****************************************************************************
//Configuración módulo PWM de Servo
//*****************************************************************************
void configurarPWMServo(void)
{
  //Configurar el modulo PWM
  ledcSetup(pwmChannelServo, freqPWMServo, resolutionPWMServo);

  //Seleccionar en qué GPIO tendremos nuestra señal PWM
  ledcAttachPin(pinPWMServo, pwmChannelServo);
}

//****************************************************************
// Filtro media Móvil exponencial EMA
//****************************************************************
void emaADC(void)
{
  adcRaw = analogReadMilliVolts(LM35); //toma valor que está midiendo el sensor
  adcFiltradoEMA = (alpha * adcRaw) + ((1.0 - alpha) * adcFiltradoEMA); //filtra ese valor
}

//*****************************************************************************
//Función para tomar el valor de la temperatura
//*****************************************************************************
void temperatura(void)
{
  if (presionado == 1)
  {
    voltage = adcFiltradoEMA;
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
    ledcWrite(pwmChannelServo, 4000);
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
void display7Seg(int contadorTimer)
{
  switch (contadorTimer)
  {
  case 0:
    //desplegar decenas
    digitalWrite(display1, HIGH);
    digitalWrite(display2, LOW);
    digitalWrite(display3, LOW);
    desplegarPunto(0);
    desplegar7Seg(decenas);
    break;

  case 1:
    //Desplegar unidades
    digitalWrite(display1, LOW);
    digitalWrite(display2, HIGH);
    digitalWrite(display3, LOW);
    desplegarPunto(1);
    desplegar7Seg(unidades);
    break;

  case 2:
    //Desplegar decimal
    digitalWrite(display1, LOW);
    digitalWrite(display2, LOW);
    digitalWrite(display3, HIGH);
    desplegarPunto(0);
    desplegar7Seg(decimal);
    break;

  default:
    break;
  }
}