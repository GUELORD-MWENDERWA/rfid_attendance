#include <Arduino.h>

char nodemcuSerialMsg[160];
unsigned int position = 0;
char NodemcuMsgType[5][40];

// Reinitialisation du message une fois qu'il y a overload (depassement)
void resetNodemcu_SerialMsg() {
  memset(nodemcuSerialMsg, 0, sizeof(nodemcuSerialMsg));
  position = 0;
}

void serialDataInTraitement(char caractereRecu) {

  // Type ou unite de cmd; UID; habilitation; depart ou arriver; ID de l'unite cmd;
  const char separateurDataIn[] = "_";

  // jdfgjshfdigfnjdf_dfsfghjjjmmmnns_sdfsdf_sfsf_shdmjxjdhfdkdf_jfd
  char* NodemcuMsg;

  // Reception du message
  nodemcuSerialMsg[position++] = caractereRecu;

  // Traitement du overload
  if (position >= sizeof(nodemcuSerialMsg)) {
    resetNodemcu_SerialMsg();
  }
  if (caractereRecu == '\n') {


    // Separation du mesage selon son type
    NodemcuMsg = strtok(nodemcuSerialMsg, separateurDataIn);
    //copie : strncpy(destination,messageAcopier, tailleDestinateur);
    strncpy(NodemcuMsgType[0], NodemcuMsg, sizeof(NodemcuMsgType[0]));  // Premier message
    for (byte i = 1; i < 5; i++) {
      NodemcuMsg = strtok(NULL, separateurDataIn);
      // NULL : parcours des informations contenue dans le meme message
      strncpy(NodemcuMsgType[i], NodemcuMsg, sizeof(NodemcuMsgType[i]));  //le reste du message
    }
    // Type ou unite de cmd; UID; habilitation; depart ou arriver; ID de l'unite cmd;
    // ENVOIT DU MSSAGE VERS LE SITE


    // Preparation de la variable nodeMCU_SerialMsg pour une nouvelle reception
    resetNodemcu_SerialMsg();
  }
}


void setup() {
  Serial.begin(57600);
  delay(1000);
}

void loop() {
  // put your main code here, to run repeatedly:

  
  Serial.print(F("UID DE LA CARTE : "));
  Serial.println(NodemcuMsgType[1]);
  Serial.print(F("HABILITATION DE LA CARTE : "));
  Serial.println(NodemcuMsgType[2]);
  Serial.print(F("ID DE LA PORTE : "));
  Serial.println(NodemcuMsgType[4]);
  delay(150);
  

  while (Serial.available()) {
    serialDataInTraitement(Serial.read());
  }

  //resetNodemcu_SerialMsg();
}
