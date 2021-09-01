//*****************************************************************************
// Universidad del Valle de Guatemala
// BE3015: Electrónica Digital 2
// Sofía Salguero - 19236
// Ingeniería en Tecnología de Audio
//*****************************************************************************

//*****************************************************************************
//Librerias
//*****************************************************************************
#include <Arduino.h> //libreria de arduino

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

//*****************************************************************************
//Varibles globales
//*****************************************************************************
int raw_LM35 = 0;    //valor tomado del sensor
float voltage = 0.0; //voltaje del sensor
float tempC = 0.0;   //temperatura en °C
int contador = 160;

//*****************************************************************************
//ISR: interrupciones
//*****************************************************************************
void IRAM_ATTR ISRBoton1() //interrupción para botón 1 (Derecha)
{
  static unsigned long ultimo_tiempo_interrupcion = 0; //último tiempo de la interrupción
  unsigned long tiempo_interrupcion = millis();        //tiempo actual de la interrupción

  //Si la interrupcion dura menos de 200ms, asumir que es un rebote e ignorar
  if (tiempo_interrupcion - ultimo_tiempo_interrupcion > 200)
  {
    raw_LM35 = analogRead(LM35); //tomar el dato del sensor y actualizarlo
    contador -= 20;              //Disminuye 20 al contador de botón

    if (contador == 180) //el valor no puede ser mayor a 180
    {
      contador = 180;
    }

    else if (contador == 0) //el valor no puede ser menor a 0
    {
      contador = 0;
    }
  }
  ultimo_tiempo_interrupcion = tiempo_interrupcion; //actualiza el valor del tiempo de la interrupción
}

//*****************************************************************************
//Prototipos de funcion
//*****************************************************************************
void configurarPWMLedR(void);
void configurarPWMLedA(void);
void configurarPWMLedV(void);
void configurarBoton1(void);
void configurarPWMServo(void);
void temperatura(void);
void encenderLeds(void);
void moverServo();

//*****************************************************************************
//configuracion
//*****************************************************************************
void setup()
{
  pinMode(boton1, INPUT_PULLUP);
  Serial.begin(115200);
  configurarBoton1();

  //Señales PWM
  configurarPWMServo();
  configurarPWMLedR();
  configurarPWMLedA();
  configurarPWMLedV();
}

//*****************************************************************************
// Loop principal
//*****************************************************************************
void loop()
{

  temperatura();
  encenderLeds();
  moverServo(); //función para mover servo
  Serial.print("Raw Value = ");
  Serial.println(raw_LM35);
  Serial.print("Voltaje = ");
  Serial.println(voltage);
  Serial.print("Grados = ");
  Serial.println(tempC);
  Serial.print("Contador = ");
  Serial.println(contador);
  Serial.print("Angulo = ");
  Serial.println(contador);
  delay(1000);
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
//Función para convertir el valor de la temperatura
//*****************************************************************************
void temperatura(void)
{
  voltage = raw_LM35 * 3.3 / 4095.0;
  tempC = voltage / 0.010;
}

//*****************************************************************************
//Función para encender leds
//*****************************************************************************
void encenderLeds(void)
{
  if (tempC <= 37.0)
  {
    ledcWrite(pwmChannelLedV, 255);
    ledcWrite(pwmChannelLedA, 0);
    ledcWrite(pwmChannelLedR, 0);
  }

  else if (tempC > 37.0 && tempC <= 37.5)
  {
    ledcWrite(pwmChannelLedV, 0);
    ledcWrite(pwmChannelLedA, 255);
    ledcWrite(pwmChannelLedR, 0);
    ledcWrite(pwmChannelServo, 100);
  }

  else if (tempC > 37.5)
  {
    ledcWrite(pwmChannelLedV, 0);
    ledcWrite(pwmChannelLedA, 0);
    ledcWrite(pwmChannelLedR, 255);
    ledcWrite(pwmChannelServo, 200);
  }
}

//*****************************************************************************
//Función para mover servo con el contador
//*****************************************************************************
void moverServo(void)
{
  /*if (tempC <= 37.0)
  {
    ledcWrite(pwmChannelServo, 0);
  }

  else if (tempC > 37.0 && tempC <= 37.5)
  {
    ledcWrite(pwmChannelServo, 32768);
  }

  else if (tempC > 37.5)
  {
    ledcWrite(pwmChannelServo, 65000);
  }*/
  int ang = (((contador / 180.0) * 2000) / 20000.0 * 65536.0) + 1634;
  ledcWrite(pwmChannelServo, ang);
  delay(15);
}
