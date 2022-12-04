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
// 팬 핀 -> D7 , D8
// 온습도센서 -> D2
// rfid -> SDA:
// 증가버튼 -> D4
// 감소버튼 -> D5

#include <ArduinoBearSSL.h>
#include <ArduinoECCX08.h>
#include <ArduinoMqttClient.h>
#include <WiFiNINA.h> // change to #include <WiFi101.h> for MKR1000
#include <Servo.h>
#include "arduino_secrets.h"

// 온습도 센서를 쓰기 위한 핀 설정
#include "DHT.h"
#define DHTPIN 2     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11   // DHT 11
DHT dht(DHTPIN, DHTTYPE);

int fan_pin1 = 7; // 팬을 작동하기 위해 7,8번 핀 사용.
int fan_pin2 = 8;
const char* fan_state = "OFF"; // 팬 작동을 제어하기 위해

int count = 0;
#include <ArduinoJson.h>

int up = 4 ; 
int down = 5 ;

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

  dht.begin();
  pinMode(fan_pin1,OUTPUT);
  pinMode(fan_pin2,OUTPUT);
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
    count_up();
    count_down();
    getDeviceStatus(payload);
    sendMessage(payload);
  }
  if (CO2()>1000){
    fan_play();
  }
}

unsigned long getTime() {
  // get the current time from the WiFi module  
  return WiFi.getTime();
}

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

  // subscribe to a topic
  mqttClient.subscribe("$aws/things/finalexam/shadow/update/delta"); // 사물을 새로 만들었으니 MyMKRWiFi1010 -> finalexam으로 수정
}

void getDeviceStatus(char* payload) {  
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  int c = CO2() ; // co2값을 반디 위한 변수
  int p = count ; // RFID를 통해 인원수를 입력받음
  const char* fan = (digitalRead(fan_pin1)==LOW)? "ON" : "OFF" ; // 팬이 돌아가는지 확인하기 위해
  // make payload for the device update topic ($aws/things/MyMKRWiFi1010/shadow/update)
  // 온습도 , CO2 , RFID으로부터 값을 받고
  // 팬 제어
  sprintf(payload,"{\"state\":{\"reported\":{\"temperature\":\"%0.2f\",\"humidity\":\"%0.2f\",\"CO2\":\"%d\",\"R\":\"%d\",\"FAN\":\"%s\"}}}",t,h,c,p,fan);
}

void sendMessage(char* payload) {
  char TOPIC_NAME[]= "$aws/things/finalexam/shadow/update"; // 여기부분도 사물 새로 만들었음으로 바꿔야 함
  
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
  // 여기 코드부터 액츄에이터 상태를 받음
  JsonObject state = root["state"];
  const char* flag = state["FAN"];
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
  if (strcmp(flag,"ON")==0) {
    fan_play();
    sprintf(payload,"{\"state\":{\"reported\":{\"FAN\":\"%s\"}}}","ON");
    sendMessage(payload);
    
  } 
  else if (strcmp(flag,"OFF")==0) {
    fan_stop();
    sprintf(payload,"{\"state\":{\"reported\":{\"FAN\":\"%s\"}}}","OFF");
    sendMessage(payload);
  }
 
}

int CO2(){ //공기 중에 이산화탄소 농도 측정하는 함수. 디지털 핀은 안써도 됨
  int value = 0;
  for (int i=0;i<10;i++){
    value += analogRead(A1); // co2 값 받기 위해서 A1을 사용
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

// 열람실 인원수를 받기 위해 버튼을 사용하는 함수
void count_up(){
  if(digitalRead(up)==HIGH) count++;
}
void count_down(){
  if(digitalRead(down)==HIGH) count--;
}
