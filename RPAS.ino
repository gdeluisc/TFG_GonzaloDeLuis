#include <WebSockets.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ArduinoWebsockets.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

const char *ssid = "MiRed";   //wifi ssid
const char *password ="ContrasenaTrabajoFinDeGrado";   //wifi password
char *websockets_server_host = "192.168.1.53"; // ip servidor
const uint16_t websockets_server_port = 8080; //puerto del servidor

int reto;
int respuesta;
int xorKey;
using namespace websockets;


boolean enviarNodo = true;

WebsocketsClient client;


void setup() {
 Serial.begin(115200);
 delay(35);
  
  //inicio de la conexión wifi
  WiFi.begin(ssid, password);

  //tiempo de espera hasta que se realice la conexión WiFi. Se mostrará en pantalla un "." cada segundo de espera (delay)
  for (int i = 0; i < 10 && WiFi.status() != WL_CONNECTED; i++){
    Serial.print(".");
    delay(1000);
  }
  //si no se ha podido conectar, se mostrará en pantalla un mensaje de fallo
  if(WiFi.status() != WL_CONNECTED){
    Serial.println("La conexión ha fallado");
    return;
  }
  
  Serial.print("Conexión establecida con: ");
  Serial.print(ssid);
  Serial.println("");
  Serial.println("Conectandose al servidor...");

  //conexión con servidor websocket

  bool connected = client.connect(websockets_server_host, websockets_server_port, "/");
  if(connected){
    Serial.println("Conexion con el servidor establecida");    
  }else{
    Serial.println("No se ha podido establecer la conexion con el servidor");
  }

  //callback cuando se reciben mensajes
  client.onMessage([&](WebsocketsMessage message) {
    
    if(message.data().toInt() >= 1 && message.data().toInt() <= 10){
      reto = message.data().toInt();
      Serial.print("Reto: ");
      Serial.println(reto);
      respuesta = puf(reto);
      Serial.print("Respuesta: ");
      Serial.println(respuesta);
      xorKey = trifork(respuesta);
      if(xorKey < 0)
        xorKey = abs(xorKey);
      Serial.print("Clave: ");
      Serial.println(xorKey);

      if(enviarNodo){
      String mensaje = "Este es un mensaje enviado desde el RPAS";
      int longmensaje = mensaje.length() + 1;
      char mensaje_a_enviar [longmensaje];
      mensaje.toCharArray(mensaje_a_enviar, longmensaje);
      encryptDecrypt(mensaje_a_enviar, xorKey, 1);
      } 
    }
    else{
      String receivedString = message.data();           
      unsigned int strlength = message.data().length() + 1;           
      Serial.println("Texto cifrado: ");
      Serial.println(receivedString);
      char inputString [strlength];
              
      receivedString.toCharArray(inputString, strlength);
      Serial.println("Texto descifrado: ");    
      encryptDecrypt(inputString, xorKey, 0);
    }
  });
}
void loop() {
  //Permite al cliente de websocket revisar si hay nuevos mensajes entrantes
  if (client.available()){
    client.poll();
  }
  delay(500);
}


//definicion de funciones


//Funcion para simular un módulo de comunicaciones
int puf (int challenge){
  int output;
  switch(challenge){
    case 3: 
      output = challenge + 2;
      break;
    case 5: 
      output = challenge + 3;
      break;
    case 9:
      output = challenge + 1;
      break;
    default:
      output = 3;
      break;   
  }
  return output;
}

//Generador trifork

int trifork(int input){

  int xn, yn, zn, wn, xn_, yn_, zn_;
  
  //valores para x0,y0 y z0
  xn = input;
  yn = input;
  zn = input;

  int m = 4;
  int d = 5;
  
  int r1 = 4;
  int r2 = 7;
  int r3 = 2;
  
  int s1 = 5;
  int s2 = 8;
  int s3 = 6;

  xn_ =((xn-r1 + xn-s1) % m) >> d;
  yn_ =((yn-r2 + yn-s2) % m) >> d;
  zn_ =((zn-r3 + zn-s3) % m) >> d;
  
  xn = ((xn-r1 + xn-s1) %m) ^ zn_;
  yn = ((yn-r2 + xn-s2) %m) ^ xn_;
  zn = ((xn-r3 + xn-s3) %m) ^ yn_;

  wn = xn ^ zn;

  return wn; 
} 



// La misma función será utilizada para encriptar y desencriptar el mensaje recibido
void encryptDecrypt(char inpString[], int key, int enviar_nodo) 
{ 
    String aux = String(key);        
    char xorKey = aux.charAt(0);   

    int len = strlen(inpString); 
  
    
    for (int i = 0; i < len; i++){ 
        inpString[i] = inpString[i] ^ xorKey; 
        Serial.print(inpString[i]);
    }
    if(enviar_nodo)
      client.send(inpString);    
} 
