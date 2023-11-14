/*************************************************************
  *Projeto de irrigação doméstica com IoT
  *Grupo de PI_V doa alunos da Univesp, turma 2020
  *
  *Automação para monitoramento de umidade do solo
  *e irrigação de hortas pequenas para apartamentos
  *e afins
  *
  *12/11/2023
  *
  *nodeMCU V3
  *Sensor DHT11
  *Sensor Sonda de Umidade de Solo
  *Chave Relé
  *Alimentação HW-131
 *************************************************************/

/* Informações do Projeto Blynk */
#define BLYNK_TEMPLATE_ID " "
#define BLYNK_TEMPLATE_NAME " "
#define BLYNK_AUTH_TOKEN " "

/* Comente isso para desabilitar impressões e economizar espaço */
#define BLYNK_PRINT Serial

const unsigned long trintaMin = 1800000ul; //Intervalo de 30mim
unsigned long lastWakeUp;


#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include "DHTesp.h"
#include "ThingSpeak.h"

// Credenciaias do WiFi.
char ssid[] = " ";
char pass[] = " ";

//Conexão com ThingSpeak
unsigned long myChannelNumber =  ; //Número do canal
const char * myWriteAPIKey = " ";

WiFiClient  client;
//unsigned long last_connection_time;
//last_connection_time = millis();

//Sensor DHT11
DHTesp dht;

//Sensor de Solo
const int pinSolo = A0;

//Bomba Dágua
const int pinBomba = D1;

BlynkTimer timer;

void sendSensor()
{
  float h = dht.getHumidity();
  float t = dht.getTemperature(); // or dht.readTemperature(true) for Fahrenheit
  float us = analogRead(pinSolo);
  float usPer = map(us, 560, 420, 0, 100); //Transforma leitura do sensor de solo em %
  

  if (isnan(h) || isnan(t) || isnan(us)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  // You can send any value at any time.
  // Please don't send more that 10 values per second.
  Blynk.virtualWrite(V5, h);
  Blynk.virtualWrite(V6, t);
  Blynk.virtualWrite(V4, usPer);
}

void setup(){
  dht.setup(2, DHTesp::DHT11);
  pinMode(pinBomba, OUTPUT);

  //last_connection_time = 0;
  // Debug console
  Serial.begin(115200);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  // You can also specify server:
  //Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass, "blynk.cloud", 80);
  //Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass, IPAddress(192,168,1,100), 8080);

  //dht.begin();

  // Setup a function to be called every second
  timer.setInterval(1000L, sendSensor);
  ThingSpeak.begin(client);
}
BLYNK_WRITE(V3) // Executes when the value of virtual pin 0 changes
{
  if(param.asInt() == 1)
  {
    // execute este código se o widget de troca estiver LIGADO
    digitalWrite(pinBomba,HIGH);  
    Blynk.virtualWrite(V3,1);
    delay(4000);
    digitalWrite(pinBomba,LOW);     
    Blynk.virtualWrite(V3,0);
  }
}
void loop()
{
  Blynk.run();
  timer.run();
  float u = dht.getHumidity();
  float t = dht.getTemperature();
  float us = analogRead(pinSolo);
  float usPer = map(us, 560, 420, 0, 100);

  //Enviando dados para ThingSpeak.
  if((millis() - lastWakeUp) >= trintaMin){
    ThingSpeak.setField(1, t);
    ThingSpeak.setField(2, u);
    ThingSpeak.setField(3, usPer);
    ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
    lastWakeUp = millis();
  }
  //Escrevendo no Monitor Serial.
  Serial.print("umidade (%): ");
  Serial.print(u, 1);
  Serial.print("\t");
  Serial.print("Temperatura (C): ");
  Serial.print(t, 1);
  Serial.print("\t");
  Serial.print("Umidade de solo (%): ");
  Serial.print(us, 1);
  Serial.print("\t");
  Serial.println();
  delay(15000);
}
