
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoOTA.h>

// lecteur rfid porte 001
#define RST_PIN D1  // SDA-PIN for RC522 - RFID - SPI
#define SS_PIN D2   // SDA-PIN for RC522 - RFID - SPI

// lecteur rfid porte 002
#define RST_PIN_2 D1  // RST-PIN for RC522 - RFID - SPI
#define SS_PIN_2 D3   // SDA-PIN for RC522 - RFID - SPI

String Identification = "";  //uid lue par les lecteurs rfid
boolean habilitation = 0;

MFRC522 rfid_in(SS_PIN, RST_PIN);       // Create MFRC522 instance
MFRC522 rfid_out(SS_PIN_2, RST_PIN_2);  // Create MFRC522 instance

// information de connexion
const char *SSID = "HUAWEI-E5330-8104";
const char *PASSWORD = "2003111223guels";
const char *device_token = "9e2395a4b3426c92";

//gestionnaire d'evenement WiFi:lorsque on est connecte
void onConnected(const WiFiEventStationModeConnected event);

//gestionnaire d'evenement WiFi:lorsque on a eu l'adresse IP
void onGotIP(const WiFiEventStationModeGotIP event);

//gestionnaire d'evenement WiFi:lorsque on est deconnecte
void onDisconnected(const WiFiEventStationModeDisconnected event);

void SendCardID(String Identification);

// information de connexion pour le site
String URL = "http://192.168.43.142/rfidattendance/getdata.php";  //192.168.238.138 ==> adresse ip  du pc
String getData, Link;

// definir les entrees et sorties
#define PIN_LED_ROUGE D0

// SoftwareSerial Nom_Instance (RX,TX);
SoftwareSerial Arduino_Serial(D9, D10);  //6 == Tx de l'arduino ety 7 ==  de l'arduino

//bouton de sellection de badge par classe
const int buttonPin = D4;
int buttonPushCounter = 0;
int buttonState = 0;
int lastButtonState = 0;

//fonction qui enregistre l'identification lue par les deux lecteurs rfid
String dump_byte_array(byte *buffer, byte bufferSize) {

  String identification = "";
  for (byte i = 0; i < bufferSize; i++) {
    identification.concat(String(buffer[i] < 0x10 ? " 0" : ""));
    identification.concat(String(buffer[i], HEX));
  }
  identification.toUpperCase();
  return (identification);
}

// fonction qui sellectionne la classe ou les uids doivent etre sellectionees
void bouton_sellect_classe() {

  buttonState = digitalRead(buttonPin);

  if ((buttonState != lastButtonState) && (buttonState == LOW)) {
    buttonPushCounter++;
  }
  lastButtonState = buttonState;
  delay(50);

  if (buttonPushCounter > 3) {
    buttonPushCounter = 0;
  }
  // Serial.println(buttonPushCounter);
}

// fonction qui sellectionne les uids qui doivent etre lues en fonction de la classe
void sellect_uid() {
  switch (buttonPushCounter) {
    case 1:
      if (Identification == "DE6FBE57") {  // les uids pour la classe de la 2HT ELO
        habilitation = 1;
        SendCardID(Identification);
      } else {
        habilitation = 0;
      }
      break;
    case 2:
      if (Identification == "3A4D19B3") {  // les uids pour la classe de la 3HT ELO
        habilitation = 1;
        SendCardID(Identification);
      } else {
        habilitation = 0;
      }
      break;
    case 3:
      if (Identification == "03733E 0F") {  // les uids pour la classe de la 4HT ELO
        habilitation = 1;
        SendCardID(Identification);
      } else {
        habilitation = 0;
      }
      break;
    default:
      habilitation = 0;
      break;
  }
}

//fonction qui lue le deuxieme rfid
void read_second_rfid() {
  if (!rfid_out.PICC_ReadCardSerial()) {
    delay(50);
    return;
  }

  Identification = dump_byte_array(rfid_out.uid.uidByte, rfid_out.uid.size);
  sellect_uid();
  Serial.print(F("DEPART ==>> MESSAGE ENVOYE : "));

  // Type ou unite de cmd; UID; habilitation; depart ou arriver; ID de l'unite cmd;
  Serial.print(F("LECTEUR-RFID_"));
  Serial.print(Identification);
  Serial.print(F("_"));

  Arduino_Serial.print(F("LECTEUR-RFID_"));
  Arduino_Serial.print(Identification);
  Arduino_Serial.print(F("_"));

  if (habilitation == 1) {
    // Envoiot ds informations avec autorisation d'acces
    Serial.print(habilitation);
    Serial.print(F("_"));
    Serial.print(buttonPushCounter);
    Serial.print(F("_DEPART_PORTE002 \n"));

    Arduino_Serial.print(habilitation);
    Arduino_Serial.print(F("_"));
    Arduino_Serial.print(buttonPushCounter);
    Arduino_Serial.print(F("_DEPART_PORTE002 \n"));

    habilitation = 0;
  } else {
    // Envoiot ds informations avec refus d'acces
    Serial.print(habilitation);
    Serial.print(F("_"));
    Serial.print(buttonPushCounter);
    
    Serial.print(F("_DEPART_PORTE002 \n"));

    Arduino_Serial.print(habilitation);
    Arduino_Serial.print(F("_"));
    Arduino_Serial.print(buttonPushCounter);
    Arduino_Serial.print(F("_DEPART_PORTE002 \n"));
  }

  delay(1000);
}

void detect_second_rfid() {
  if (!rfid_out.PICC_IsNewCardPresent()) {
    return;
  }
  read_second_rfid();
}

void setup() {

  Serial.begin(57600);
  Arduino_Serial.begin(57600);
  SPI.begin();

  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(PIN_LED_ROUGE, OUTPUT);
 
  // nom pour l'objet OTA
  ArduinoOTA.setHostname("objet_rfid");

  rfid_in.PCD_Init();   // Init MFRC522 In
  rfid_out.PCD_Init();  // Init MFRC522

  // DELAIS D'INITIALISATION DE LA COMMUNICATION SERIE
  delay(1000);

  //mode de connexion
  WiFi.mode(WIFI_STA);

  //demarage de la connection
  WiFi.begin(SSID, PASSWORD);

  //  gestionnaire d'evennement
  static WiFiEventHandler onConnectedHandler = WiFi.onStationModeConnected(onConnected);
  static WiFiEventHandler onGotIPHandler = WiFi.onStationModeGotIP(onGotIP);
  static WiFiEventHandler onDisconnectedHandler = WiFi.onStationModeDisconnected(onDisconnected);

  Serial.println(F("\n MONITORING PORTE A DISTANCE VIA LE WEB"));
}

void loop() {
  
  // sellection de classe
  bouton_sellect_classe();
  
  if(WiFi.isConnected()){
    ArduinoOTA.begin();
  }

  // arduinoOTA ...
 ArduinoOTA.handle();

  // Look for new cards
  if (!rfid_in.PICC_IsNewCardPresent()) {
    detect_second_rfid();
    return;
  }
  // Select one of the cards
  if (!rfid_in.PICC_ReadCardSerial()) {
    delay(50);
    return;
  }

  Identification = dump_byte_array(rfid_in.uid.uidByte, rfid_in.uid.size);

  sellect_uid();
  Serial.print(F("ARRIVER ==>> MESSAGE ENVOYE : "));

  // Type ou unite de cmd; UID; habilitation; depart ou arriver; ID de l'unite cmd;
  Serial.print(F("LECTEUR-RFID_"));
  Serial.print(Identification);
  Serial.print(F("_"));

  Arduino_Serial.print(F("LECTEUR-RFID_"));
  Arduino_Serial.print(Identification);
  Arduino_Serial.print(F("_"));

  if (habilitation == 1) {
    // Envoiot ds informations avec autorisation d'acces
    //Serial.print(F("_"));
    Serial.print(habilitation);
    Serial.print(F("_"));
    Serial.print(buttonPushCounter);
    Serial.print(F("_ARRIVER_PORTE001 \n"));

    Arduino_Serial.print(habilitation);
    Arduino_Serial.print(F("_"));
    Arduino_Serial.print(buttonPushCounter);
    Arduino_Serial.print(F("_ARRIVER_PORTE001 \n"));

    habilitation = 0;
  } else {
    // Envoiot ds informations avec refus d'acces
    //Serial.print(F("_"));
    Serial.print(habilitation);
    Serial.print(F("_"));
    Serial.print(buttonPushCounter);
    Serial.print(F("_ARRIVER_PORTE001 \n"));

    Arduino_Serial.print(habilitation);
    Arduino_Serial.print(F("_"));
    Arduino_Serial.print(buttonPushCounter);
    Arduino_Serial.print(F("_ARRIVER_PORTE001 \n"));
  }

  delay(1000);

  // Halt PICC
  rfid_in.PICC_HaltA();
  rfid_in.PCD_StopCrypto1();
  // Stop encryption on PCD
  rfid_out.PCD_StopCrypto1();
  rfid_out.PICC_HaltA();
}

void SendCardID(String Identification) {
  Serial.println("IDENTIFICATION DE LA CARTE ENVOYE");
  if (WiFi.isConnected()) {
    HTTPClient http;  //Declare object of class HTTPClient
    //GET Data
    getData = "?card_uid=" + String(Identification) + "&device_token=" + String(device_token);  // Add the Card ID to the GET array in order to send it
    //GET methode
     Link = URL + getData;
    //  http.begin(Link);  //initiate HTTP request   //Specify content-type header

    int httpCode = http.GET();          //Send the request
    String payload = http.getString();  //Get the response payload

    //    Serial.println(Link);   //Print HTTP return code
    Serial.println(httpCode);        //Print HTTP return code
    Serial.println(Identification);  //Print Card ID
    Serial.println(payload);         //Print request response payload

    if (httpCode == 200) {
      if (payload.substring(0, 5) == "login") {
        String user_name = payload.substring(5);
        //  Serial.println(user_name);

      } else if (payload.substring(0, 6) == "logout") {
        String user_name = payload.substring(6);
        //  Serial.println(user_name);

      } else if (payload == "succesful") {
            
      } else if (payload == "available") {
      }
      delay(100);
      http.end();  //Close connection
    }
  }
}

void onConnected(const WiFiEventStationModeConnected event) {
  Serial.println("");
  Serial.println("wifi connecte");
  digitalWrite(PIN_LED_ROUGE, HIGH);

}

void onGotIP(const WiFiEventStationModeGotIP event) {
  Serial.println("Adressse ip : " + WiFi.localIP().toString());
  Serial.println("Passerelle ip : " + WiFi.gatewayIP().toString());
  Serial.println("DNS ip : " + WiFi.dnsIP().toString());
  Serial.print("puissance de reception : ");
  Serial.println(WiFi.RSSI());
}

void onDisconnected(const WiFiEventStationModeDisconnected event) {
  Serial.println("");
  Serial.println("wifi deconnecte");
  digitalWrite(PIN_LED_ROUGE, LOW);
}