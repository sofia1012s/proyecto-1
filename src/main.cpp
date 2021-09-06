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
#include "AdafruitIO_WiFi.h" //libreria para Adafruit

//*****************************************************************************
//Definicion etiquetas
//*****************************************************************************
#define LM35 34   //toma de datos en sensor
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
#define IO_USERNAME "sal19236"
#define IO_KEY ""

/******************************* WIFI **************************************/
#define WIFI_SSID "Familia Salguero"
#define WIFI_PASS "Salguero2019"

AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PASS);

/******************************* Feeds **************************************/
AdafruitIO_Feed *temp = io.feed("tempc");
AdafruitIO_Feed *ledV = io.feed("pinpwmledv");
AdafruitIO_Feed *ledA = io.feed("pinpwmleda");
AdafruitIO_Feed *ledR = io.feed("pinpwmledr");

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

//Temporizador
hw_timer_t *timer = NULL;
int contadorTimer = 0;

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
void emaADC(void);
void convertirTemp(void);
void servoLeds(void);
void display7Seg(int contadorTimer);
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

void IRAM_ATTR ISRTimer0() //interrupción para timer
{
  contadorTimer++; //aumenta el contador de timer

  if (contadorTimer > 2) //si es mayor a 2 regresa a cero
  {
    contadorTimer = 0;
  }
}

//*****************************************************************************
//configuracion
//*****************************************************************************
void setup()
{
  //Temporizador
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
  emaADC();                   //Tomar temperatura y filtrarla
  temperatura();              //Tomar temperatura para mostrarla en displays
  convertirTemp();            //Convertir valores de temperatura para displays
  servoLeds();                //Mover servo y encender leds según el valor
  display7Seg(contadorTimer); //Mostrar la temperatura en los displays
}

//******************************************************************************
// Configuración Timer
//******************************************************************************
void configurarTimer(void)
{
  //Fosc = 80MHz = 80,000,000 Hz
  //Fosc / Prescaler = 80,000,000 / 80 = 1,000,000
  //Tosc = 1/Fosc = 1uS

  //Paso 2: Seleccionar Timer
  //Timer 0, prescaler = 80, flanco de subida
  timer = timerBegin(0, prescaler, true);

  //paso 3: Asignar el handler de la interrupción
  timerAttachInterrupt(timer, &ISRTimer0, true);

  //Paso 4: Programar alarma
  //Tic = 1uS
  timerAlarmWrite(timer, 1000, true);

  //Paso 5: Iniciar la alarma
  timerAlarmEnable(timer);
}

//*****************************************************************************
//Configuración interrupción en botón 1
//*****************************************************************************
void configurarBoton1(void)
{
  //me coloca una interrupción en el botón 1 (durante el cambio de alto a bajo)
  attachInterrupt(digitalPinToInterrupt(boton1), ISRBoton1, FALLING);
}

//*****************************************************************************
//Configuración módulo PWM Led Rojo
//*****************************************************************************
void configurarPWMLedR(void)
{
  //Paso 1: Configurar el modulo PWM
  ledcSetup(pwmChannelLedR, freqPWMLedR, resolutionPWMLedR);

  //Paso 2: seleccionar en que GPIO tendremos nuestra señal PWM
  ledcAttachPin(pinPWMLedR, pwmChannelLedR);
}

//*****************************************************************************
//Configuración módulo PWM Led Verde
//*****************************************************************************
void configurarPWMLedV(void)
{
  //Paso 1: Configurar el modulo PWM
  ledcSetup(pwmChannelLedV, freqPWMLedV, resolutionPWMLedV);

  //Paso 2: seleccionar en que GPIO tendremos nuestra señal PWM
  ledcAttachPin(pinPWMLedV, pwmChannelLedV);
}

//*****************************************************************************
//Configuración módulo PWM Led Amarillo
//*****************************************************************************
void configurarPWMLedA(void)
{
  //Paso 1: Configurar el modulo PWM
  ledcSetup(pwmChannelLedA, freqPWMLedA, resolutionPWMLedA);

  //Paso 2: seleccionar en que GPIO tendremos nuestra señal PWM
  ledcAttachPin(pinPWMLedA, pwmChannelLedA);
}

//*****************************************************************************
//Configuración módulo PWM de Servo
//*****************************************************************************
void configurarPWMServo(void)
{
  //Paso 1: Configurar el modulo PWM
  ledcSetup(pwmChannelServo, freqPWMServo, resolutionPWMServo);

  //Paso 2: seleccionar en que GPIO tendremos nuestra señal PWM
  ledcAttachPin(pinPWMServo, pwmChannelServo);
}

//****************************************************************
// Filtro media Móvil exponencial EMA
//****************************************************************
void emaADC(void)
{
  adcRaw = analogReadMilliVolts(LM35);
  adcFiltradoEMA = (alpha * adcRaw) + ((1.0 - alpha) * adcFiltradoEMA);
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