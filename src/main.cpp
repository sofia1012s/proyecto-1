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
#define LM35 35   //toma de datos en sensor
#define boton1 23 //Botón para parte 1

//*****************************************************************************
//Varibles globales
//*****************************************************************************
int raw_LM35 = 0;    //valor tomado del sensor
float voltage = 0.0; //voltaje del sensor
float tempC = 0.0;   //temperatura en °C

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
  }
  ultimo_tiempo_interrupcion = tiempo_interrupcion; //actualiza el valor del tiempo de la interrupción
}

//*****************************************************************************
//Prototipos de funcion
//*****************************************************************************
void configurarBoton1(void);
void temperatura(void);

//*****************************************************************************
//configuracion
//*****************************************************************************
void setup()
{
  pinMode(boton1, INPUT_PULLUP);
  Serial.begin(115200);
  configurarBoton1();
}

//*****************************************************************************
// Loop principal
//*****************************************************************************
void loop()
{
  temperatura();
  Serial.print("Raw Value = ");
  Serial.println(raw_LM35);
  Serial.print("Voltaje = ");
  Serial.println(voltage);
  Serial.print("Grados = ");
  Serial.println(tempC);
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
//Función para convertir el valor de la temperatura
//*****************************************************************************
void temperatura(void)
{
  voltage = raw_LM35 * 3.3 / 4095.0;
  tempC = voltage / 0.010;
}