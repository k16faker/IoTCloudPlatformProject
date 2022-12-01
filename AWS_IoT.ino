/*
  AWS IoT WiFi

  This sketch securely connects to an AWS IoT using MQTT over WiFi.
  It uses a private key stored in the ATECC508A and a public
  certificate for SSL/TLS authetication.

  It publishes a message every 5 seconds to arduino/outgoing
  topic and subscribes to messages on the arduino/incoming
  topic.

  The circuit:
  - Arduino MKR WiFi 1010 or MKR1000

  The following tutorial on Arduino Project Hub can be used
  to setup your AWS account and the MKR board:

  https://create.arduino.cc/projecthub/132016/securely-connecting-an-arduino-mkr-wifi-1010-to-aws-iot-core-a9f365

  This example code is in the public domain.
*/

// Co2 -> A1
// 팬 핀 -> 7 , 6
// 온습도센서 -> 2
// rfid -> D9 ~ D13

#include <ArduinoBearSSL.h>
#include <ArduinoECCX08.h>
#include <ArduinoMqttClient.h>
#include <WiFiNINA.h> // change to #include <WiFi101.h> for MKR1000
#include "arduino_secrets.h"

// 온습도 센서를 쓰기 위한 핀 설정
#include "DHT.h"
#define DHTPIN 2     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11   // DHT 11
DHT dht(DHTPIN, DHTTYPE);

// RFID를 쓰기 위한 핀 설정
#include <SPI.h> //RFID를 사용하기 위한 라이브러리
#include <MFRC522.h> //RFID를 사용하기 위한 라이브러리
#define SS_PIN 10
#define RST_PIN 9 
MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class
MFRC522::MIFARE_Key key; 
// Init array that will store new NUID 
byte nuidPICC[4];

// 아두이노 호환 팬을 사용하기 위한 팬 핀 설정
int fan_pin1 = 7;
int fan_pin2 = 6;
const char* fan_state = "OFF"; // 팬 작동을 제어하기 위한 변수 생성 (평상시에는 꺼져 있어야 하므로 기본값을 OFF로 선언)

int RFID_count = 0; // 열람실의 현재 인원을 받기 위한 변수 생성
#include <ArduinoJson.h>

/////// Enter your sensitive data in arduino_secrets.h
const char ssid[]        = SECRET_SSID;
const char pass[]        = SECRET_PASS;
const char broker[]      = SECRET_BROKER;
const char* certificate  = SECRET_CERTIFICATE;

WiFiClient    wifiClient;            // Used for the TCP socket connection
BearSSLClient sslClient(wifiClient); // Used for SSL/TLS connection, integrates with ECC508
MqttClient    mqttClient(sslClient);

unsigned long lastMillis = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  dht.begin(); // 온습도 모듈
  
  // 팬 모듈
  pinMode(fan_pin1,OUTPUT);
  pinMode(fan_pin2,OUTPUT);
  
  // RFID 모듈
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522
  
  if (!ECCX08.begin()) {
    Serial.println("No ECCX08 present!");
    while (1);
  }

  // Set a callback to get the current time
  // used to validate the servers certificate
  ArduinoBearSSL.onGetTime(getTime);

  // Set the ECCX08 slot to use for the private key
  // and the accompanying public certificate for it
  sslClient.setEccSlot(0, certificate);

  // Optional, set the client id used for MQTT,
  // each device that is connected to the broker
  // must have a unique client id. The MQTTClient will generate
  // a client id for you based on the millis() value if not set
  //
  // mqttClient.setId("clientId");

  // Set the message callback, this function is
  // called when the MQTTClient receives a message
  mqttClient.onMessage(onMessageReceived);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  if (!mqttClient.connected()) {
    // MQTT client is disconnected, connect
    connectMQTT();
  }

  // poll for new MQTT messages and send keep alives
  mqttClient.poll();

  // publish a message roughly every 5 seconds.
  if (millis() - lastMillis > 5000) {
    lastMillis = millis();
    char payload[512];
    RFID_start(); // RFID로 현재 열람실 인원수를 파악
    getDeviceStatus(payload); // 아두이노를 통해서 열람실의 현재 공기 상태를 파악
    sendMessage(payload); // 현재 상태를 Eclipse에 전송
  }
}

unsigned long getTime() {
  // get the current time from the WiFi module  
  return WiFi.getTime();
}

// WiFI 연결 함수
void connectWiFi() {
  Serial.print("Attempting to connect to SSID: ");
  Serial.print(ssid);
  Serial.print(" ");

  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    // failed, retry
    Serial.print(".");
    delay(5000);
  }
  Serial.println();

  Serial.println("You're connected to the network");
  Serial.println();
}

// MQTT 연결 함수
void connectMQTT() {
  Serial.print("Attempting to MQTT broker: ");
  Serial.print(broker);
  Serial.println(" ");

  while (!mqttClient.connect(broker, 8883)) {
    // failed, retry
    Serial.print(".");
    delay(5000);
  }
  Serial.println();

  Serial.println("You're connected to the MQTT broker");
  Serial.println();

  // 사물 finalexam에 대한 주제 구독
  mqttClient.subscribe("$aws/things/finalexam/shadow/update/delta"); 
}

void getDeviceStatus(char* payload) {  
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature(); // 현재 열람실 온도 측정
  float h = dht.readHumidity(); // 현재 열람실 습도 측정
  int c = CO2() ; // 현재 열람실 co2값 측정
  int r = RFID_count ; // RFID를 통해 현재 열람실 인원수를 입력받음
  const char* fan = (digitalRead(fan_pin1)==LOW)? "ON" : "OFF" ; // 팬의 현재 상태 파악 (제어를 하기 위해서도 필요)
  // make payload for the device update topic ($aws/things/MyMKRWiFi1010/shadow/update)
  // 온습도 , CO2 , RFID으로부터 값을 받고
  // 팬 제어
  sprintf(payload,"{\"state\":{\"reported\":{\"temperature\":\"%0.2f\",\"humidity\":\"%0.2f\",\"CO2\":\"%d\",\"R\":\"%d\",\"FAN\":\"%s\"}}}",t,h,c,r,fan);
}

void sendMessage(char* payload) {
  char TOPIC_NAME[]= "$aws/things/finalexam/shadow/update"; // 사물 finalexam에 대한 주제
  
  Serial.print("Publishing send message:");
  Serial.println(payload);
  mqttClient.beginMessage(TOPIC_NAME);
  mqttClient.print(payload);
  mqttClient.endMessage();
}


void onMessageReceived(int messageSize) {
  // we received a message, print out the topic and contents
  Serial.print("Received a message with topic '");
  Serial.print(mqttClient.messageTopic());
  Serial.println();

  // store the message received to the buffer
  char buffer[512] ;
  int count=0;
  while (mqttClient.available()) {
     buffer[count++] = (char)mqttClient.read();
  }
  buffer[count]='\0'; // 버퍼의 마지막에 null 캐릭터 삽입
  Serial.println(buffer);
  Serial.println();

  // JSon 형식의 문자열인 buffer를 파싱하여 필요한 값을 얻어옴.
  // 디바이스가 구독한 토픽이 $aws/things/MyMKRWiFi1010/shadow/update/delta 이므로,
  // JSon 문자열 형식은 다음과 같다.
  // {
  //    "version":391,
  //    "timestamp":1572784097,
  //    "state":{
  //        "LED":"ON"
  //    },
  //    "metadata":{
  //        "LED":{
  //          "timestamp":15727840
  //         }
  //    }
  // }
  //
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, buffer);
  JsonObject root = doc.as<JsonObject>();
  JsonObject state = root["state"]; // 상태 부분을 state 변수에 받음
  const char* flag = state["FAN"]; // 열람실 상태에서 팬의 상태 부분을 변수 flag에 저장
  // {
  //      "state": {
  //          "desired": {
  //              "FAN": "ON"
  //          }
  //   }
  // }
  // 이런식으로 상태 변경 온다면
  // Serial.println(led);
  
  char payload[512];
  Serial.println(flag);
  if (strcmp(flag,"ON")==0) { // 팬 상태가 'ON' 이라면
    fan_play(); // 팬을 동작시킴
    sprintf(payload,"{\"state\":{\"reported\":{\"FAN\":\"%s\"}}}","ON"); // 팬이 현재 작동중인 상태라고 보냄
    sendMessage(payload);
    
  } 
  else if (strcmp(flag,"OFF")==0) { // 팬 상태가 'OFF'라면
    fan_stop(); // 팬을 멈추고
    sprintf(payload,"{\"state\":{\"reported\":{\"FAN\":\"%s\"}}}","OFF"); // 팬이 현재 정지 상태라고 보냄
    sendMessage(payload);
  }
 
}

//공기 중에 이산화탄소 농도 측정하는 함수
int CO2(){
  int value = 0;
  for (int i=0;i<10;i++){
    value += analogRead(A1); // co2 값 받기 위해서 A0를 사용
    delay(20);
  }
  value = value / 10 ;
  return value; 
}

// 팬 관련 함수
void fan_play(){ // 팬을 동작 시키는 함수
  digitalWrite(fan_pin1,LOW);
  digitalWrite(fan_pin2,HIGH);
}
void fan_stop(){ // 팬을 멈추는 함수
  digitalWrite(fan_pin1,HIGH);
  digitalWrite(fan_pin2,HIGH);
}

// RFID를 사용하기 위한 함수
void RFID_start(){
  // 새 카드 접촉이 있을 때만 다음 단계로 넘어감 
  if ( ! rfid.PICC_IsNewCardPresent())
    return;

  // 카드 읽힘이 제대로 되면 다음으로 넘어감
  if ( ! rfid.PICC_ReadCardSerial())
    return;
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  // MIFARE 방식의 카드인지 확인
  if (
    piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&  
    piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
    piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("Your tag is not of type MIFARE Classic."));
    return;
  }
  else RFID_count++;

  // 이전 인식된 카드와 다른 , 혹은 새카드가 인식되면
  if (rfid.uid.uidByte[0] != nuidPICC[0] || 
    rfid.uid.uidByte[1] != nuidPICC[1] || 
    rfid.uid.uidByte[2] != nuidPICC[2] || 
    rfid.uid.uidByte[3] != nuidPICC[3] ) {
    RFID_count++ ; //인원수 추가

    // 고유아이디 값을 저장
    for (byte i = 0; i < 4; i++) {
      nuidPICC[i] = rfid.uid.uidByte[i];
    }
   
   //그 UID값을 10진값으로 출력
    Serial.println(F("The NUID tag is:"));
    Serial.print(F("In dec: "));
    printDec(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();
  }
  //연속으로 동일한 카드를 접촉하면 다른 처리 없이 '이미 인식된 카드'라는 메세지 출력
  else RFID_count--; // 나가는 거니까 인원수 감소
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

void printDec(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], DEC);
  }
}
