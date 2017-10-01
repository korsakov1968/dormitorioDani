#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>


IPAddress server(192, 168, 1, 51); // IP de la raspberry Pi
const char* ssid     = "La_red_de_mi_casa"; // Your ssid
const char* pass = "primeraclave1234segundaclave5678"; // Your Password
const char* host = "DormDaniSonoff1"; // nombre del entorno

int rele = 12; //pin del rele en la ESP8266
int led = 13;  //pin del led en la ESP8266
int pulsador = 0;
boolean estado_pulsador ;
boolean estado_luz = 0;
#define BUFFER_SIZE 100
int contador=0;
boolean conectado=0;

WiFiClient wclient;
PubSubClient client(wclient, server);

void callback(const MQTT::Publish& pub) {
  Serial.println (pub.payload_string());
    if(pub.payload_string() == "on")
    {
      digitalWrite(rele, HIGH); // en caso de que el modulo rele funcione al reves, cambiarl LOW por HIGH
      digitalWrite(led, LOW);
      estado_luz=1;
      Serial.println("Switch On");
          
    }
    if(pub.payload_string() == "off")
    {
      digitalWrite(rele, LOW); // en caso de que el modulo rele funcione al reves, cambiarl HIGH por LOW
      digitalWrite(led, HIGH);
      estado_luz=0;
      Serial.println("Switch Off");
    }

    if (estado_luz != EEPROM.read(0)) EEPROM.write(0,estado_luz);
    EEPROM.commit();
}

void setup() 
{
  pinMode(rele,OUTPUT);
  pinMode(led,OUTPUT);
  pinMode(pulsador,INPUT);
  delay(1000);
  digitalWrite(led, LOW);
  Serial.begin(115200);
  EEPROM.begin(512);
  delay(10);
  Serial.println();
  Serial.println();
  client.set_callback(callback);
  if (WiFi.status() != WL_CONNECTED) 
  {
    Serial.print("Connecting to ");
    Serial.print(ssid);
    Serial.println("...");
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, pass);
    
    if (WiFi.waitForConnectResult() != WL_CONNECTED)
    {
    Serial.println("WiFi not connected");
    conectado=0;
    }
    else
    {
    Serial.println("WiFi connected");
    for (int i=0; i<10; i++)
    {
    digitalWrite(led, LOW);
    delay(150);
    digitalWrite(led, HIGH);
    delay(150);
    conectado=1;
    }                         
    }
  }
  estado_luz=EEPROM.read(0);
  digitalWrite(rele,EEPROM.read(0));
  digitalWrite(led,!EEPROM.read(0));
  delay(200);
}

void(* Resetea) (void) = 0;

void loop() {


  estado_pulsador=digitalRead(pulsador);
  if (estado_pulsador==LOW)
  {
    if (estado_luz==0)
    {
      estado_luz=1;
      client.publish("DormDaniSonoff1","SwitchedOn");
      Serial.println("Switch On");
      digitalWrite(rele,HIGH);  // en caso de que el modulo rele funcione al reves, cambiarl HIGH por LOW
      digitalWrite(led, LOW);     
    }
    else
    {
      estado_luz=0; 
      client.publish("DormDaniSonoff1","SwitchedOff");
      Serial.println("Switch Off");
      digitalWrite(rele,LOW); // en caso de que el modulo rele funcione al reves, cambiarl LOW por HIGH
      digitalWrite(led, HIGH);          
    }

    delay(1000);
    EEPROM.write(0,estado_luz);    
  }

  while (estado_pulsador==LOW)
  {
    estado_pulsador=digitalRead(pulsador);
    Serial.println(contador);
    contador++;
    delay(10);
  }
  if (contador>=250)
  {
        for (int i=0; i<4; i++)
    {
      digitalWrite(rele,HIGH);  // en caso de que el modulo rele funcione al reves, cambiarl HIGH por LOW
      digitalWrite(led, LOW);  
      delay(500);
      digitalWrite(rele,LOW); // en caso de que el modulo rele funcione al reves, cambiarl LOW por HIGH
      digitalWrite(led, HIGH);    
      delay(500); 
    }
    EEPROM.commit();                  
    Resetea();
  }
  contador=0;
  
  if (conectado==1)
  {
    if (WiFi.status() == WL_CONNECTED) {
    if (!client.connected()) {
      if (client.connect("ESP8266: DormDaniSonoff1")) {
        client.publish("outTopic",(String)"hello world, I'm "+host);
        client.subscribe(host+(String)"/#");
      }
    }
    if (client.connected())

      client.loop();
  }
  else
  {
    if (estado_luz==0)
    {
       digitalWrite(rele,HIGH); // en caso de que el modulo rele funcione al reves, cambiarl LOW por HIGH
       digitalWrite(led, LOW);
    }
    else
    {
      digitalWrite(rele,LOW);  // en caso de que el modulo rele funcione al reves, cambiarl HIGH por LOW
      digitalWrite(led, HIGH);
    if (estado_luz != EEPROM.read(0)) EEPROM.write(0,estado_luz);
    }
    EEPROM.commit();
    delay(1000);
  }
  }
  else
  {
    EEPROM.commit(); 
  }
}



