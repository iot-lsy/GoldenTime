#include <Arduino.h>
#include <Stream.h>
#include <ESP8266WiFi.h>
#include <TimeLib.h>

//AWS
#include "sha256.h"
#include "Utils.h"
#include "AWSClient.h"

//WEBSockets
#include <WebSocketsClient.h>

//MQTT PAHO
#include <SPI.h>
#include <IPStack.h>
#include <Countdown.h>
#include <MQTTClient.h>

//AWS MQTT Websocket
#include "Client.h"
#include "AWSWebSocketClient.h"
#include "CircularByteBuffer.h"


//AWS IOT config, change these:



//MQTT config
const int maxMQTTpackageSize = 512;
const int maxMQTTMessageHandlers = 1;

WiFiClientSecure espclient;

AWSWebSocketClient awsWSclient(1000);

IPStack ipstack(awsWSclient);

MQTT::Client<IPStack, Countdown, maxMQTTpackageSize, maxMQTTMessageHandlers> *client = NULL;

//# of connections
long connection = 0;

//WeMos D1 R2
// GPIO16 - D0
// GPIO5 - D1
// GPIO4 - D2
// GPIO0 - D3
// GPIO2 - D4
// GPIO14 - D5
// GPIO12 - D6
// GPIO13 - D7
// GPIO15 - D8
// A0 - A0

//D2~8 사용가능
#define led_IN D1 //D1 - GPIO5
#define button_OUT D2 //D2 - GPIO4
#define pir_OUT D3 //D3 - GPIO0

void inout_check(); // 출입감지 함수
void send_signal(); // AWS IoT Core 데이터 전송 함수
void get_button(); // 버튼 데이터 가져오는 함수
void button_check(); // 버튼 센싱 함수
bool connect();
void subscribe();
void sendmessage();
void setup_wifi();

int entered = 0; // 표준 0, 들어감 1, 나감 2
int count = 0; // 활동감지 횟수
int button_emergency = 0; // 버튼호출 표준 0, 호출 1
int cancel_signal = 0; // 표준 0, 응급호출 신호 취소 1 
int pir_delay_time = 0; // 퇴장 판단용 시간 (30초 이상 퇴장)
int delay_time = 0; // 와이파이 접속 끊어진 시간

int pir_value = 0; // pir 센서값
int button_value = 0; // 버튼 센서값
int button_count = 0; // 버튼 클릭됐는지 확인
int button_push_time = 0; // 버튼 눌린 시간

int emergency_time_start = 0; // 응급 상황 발생
int emergency_time_now = 0;
int emergency_state = 0; // 응급상황 데이터 전송 이후 1로 초기화, 이후 최초 움직임 감지 시 0 

int enter_time = 0; // 입장 시간
int in_time = 0;
int during_time = 0; // 입장 이후 경과 시간

//generate random mqtt clientID
char* generateClientID() {
    char* cID = new char[23]();
    for (int i = 0; i<22; i += 1)
        cID[i] = (char)random(1, 256);
    return cID;
}

//count messages arrived
int arrivedcount = 0;


//callback to handle mqtt messages
void messageArrived(MQTT::MessageData& md)
{
    MQTT::Message &message = md.message;

    Serial.print("Message ");
    Serial.print(++arrivedcount);
    Serial.print(" arrived: qos ");
    Serial.print(message.qos);
    Serial.print(", retained ");
    Serial.print(message.retained);
    Serial.print(", dup ");
    Serial.print(message.dup);
    Serial.print(", packetid ");
    Serial.println(message.id);
    Serial.print("Payload ");
    char* msg = new char[message.payloadlen + 1]();
    memcpy(msg, message.payload, message.payloadlen);
    Serial.println(msg);
    delete msg;
}


void setup() {
  
  // 센서 디지털 핀 모드 설정
  pinMode(pir_OUT, INPUT);
  pinMode(button_OUT, INPUT);
  pinMode(led_IN, OUTPUT);

  Serial.begin(115200);

  digitalWrite(led_IN, LOW);
 
  //와이파이 연결
  setup_wifi();
  Serial.setDebugOutput(false);

  //AWS parameters   
  awsWSclient.setAWSRegion(aws_region);
  awsWSclient.setAWSDomain(aws_endpoint);
  awsWSclient.setAWSKeyID(aws_key);
  awsWSclient.setAWSSecretKey(aws_secret);

  // AWS 연결
  if (connect()) {
      subscribe();
      sendmessage();
  }

  //센서초기화
  pir_value = 0; // pir 센서값
  button_value = 0; // 버튼 센서값
  
}


void loop() {
  
  //keep the mqtt up and running
  if (awsWSclient.connected() && WiFi.status() == WL_CONNECTED) {
    
      Serial.println("===============================================");
      client->yield();
      get_button();
      inout_check();
      get_button();
      button_check();
      send_signal();
      Serial.print("entered_time : ");
      Serial.println(enter_time);
      Serial.print("during_time : ");
      Serial.println(during_time);
      Serial.print("pir_delay_time : ");
      Serial.println(pir_delay_time);
      Serial.println("===============================================");
      
  }
  else {
      //handle reconnection

      int time1 = (hour() * 3600) + (minute() * 60) + second();
      
      if (connect()) {
          subscribe();
          Serial.println("AWS IoT Core 재연결");
      }
      
      else if(WiFi.status() != WL_CONNECTED){
      WiFi.begin(ssid, password);  

      Serial.print("와이파이 재연결 중");
    
      while(WiFi.status() != WL_CONNECTED){
        Serial.print("."); 
        }
        Serial.println("");
        Serial.println("와이파이 재연결 성공!");
      }

    int time2 = (hour() * 3600) + (minute() * 60) + second();

    delay_time = time2 - time1;

    Serial.print("지연시간 : ");
    Serial.print(delay_time);
    Serial.println("(초)");
    pir_delay_time += delay_time;
    
  }
}


void inout_check(){ // (수정 예정)부저 추가하기

  pir_value = digitalRead(pir_OUT);
  pir_delay_time = ((hour() * 3600) + (minute() * 60) + second()) - in_time; // 현재 - 입장시간 = 경과 시간

  get_button();

  if(emergency_state == 0){ // 평상시
    if(pir_value == 1 && count == 0){ // 들어오는 경우

      Serial.println("들어옴");
      entered = 1;
      pir_delay_time = 0;
      enter_time = (hour() * 3600) + (minute() * 60) + second();
      in_time = enter_time;
    
    }else if(pir_value == 1 && count != 0){ // 안에서 움직이는 경우
    
      Serial.println("내부활동");
      count = 1;
      in_time = ((hour() * 3600) + (minute() * 60) + second()); // 입장 시간 내부 활동시간으로 초기화 
    
    }else if(entered && pir_delay_time >= 30){ // 나가는 경우 (내부활동 30초 이상 없을경우)
    
      Serial.println("나감");
      entered = 2;

    }  
  }else{ // 응급상황 데이터 전송 이후
    if(pir_value == 1){
      emergency_state = 0;
    }else if(during_time-emergency_time_start >= 1200){ // 응급상황 발생 후 20분 지난 상황
      // 추가적인 알림?
    }
  }
  
  get_button();
  if(count == 1){
        Serial.print("during_time in inout_check() : ");
        Serial.println(during_time);
        during_time = ((hour() * 3600) + (minute() * 60) + second()) - enter_time;
  }

}


void button_check(){
  
  if(button_count == 1 && button_push_time < 3){ // 일반적 응급호출 (최대 2.9초 까지 응급호출로 인식)
    
    button_emergency = 1;
    emergency_time_start = (hour()*3600) + (minute()*60) + second();
    Serial.println("버튼 응급호출");
    digitalWrite(led_IN, HIGH);
    
  }else if(button_count == 1 && button_push_time >= 3){ // 모든 응급호출 취소 버튼 (버튼 3초 이상 누른경우)

    if(button_emergency){
      
      Serial.println("버튼 응급호출 취소");
      digitalWrite(led_IN, LOW);
      button_emergency = 0;
      cancel_signal = 1;
      
    }else{
      
      Serial.println("취소할 응급호출 신호가 존재하지 않습니다");
      
    }
  }

  button_count = 0;
  button_push_time = 0;
  
}


void send_signal(){  // 중요 - 모든 응급신호 10초간 대기하고 10초 안에 취소 버튼 누를경우 응급신호 보내지 않음

  get_button();
  if(entered == 1 && count == 0){
    
    Serial.println("입장 데이터 전송");
    count++;

    //AWS IoT Core
    MQTT::Message message;
    char buf[100];
    strcpy(buf, "{\"state\":{\"reported\":{\"enter\":true},\"desired\":{\"enter\":true}}}");
    message.qos = MQTT::QOS0;
    message.retained = false;
    message.dup = false;
    message.payload = (void*)buf;
    message.payloadlen = strlen(buf) + 1;
    int rc = client->publish(aws_topic, message);
    
    Serial.println("입장 데이터 전송");
    
    pir_value = 0;
    
  }
  
  get_button();
  if(entered == 2){

    //AWS IoT Core
    
    sendmessage();
    sendmessage();

    Serial.println("퇴장 데이터 전송");

    // 퇴장과 동시에 출입센서 변수 초기화
    entered = 0;
    count = 0;
    pir_delay_time = 0;
    enter_time = 0;
    during_time = 0;
    emergency_time_now = 0;
    emergency_time_start = 0;
    digitalWrite(led_IN, LOW);
    
  }

  //버튼 전송 조건문 
  emergency_time_now = (hour()*3600) + (minute()*60) + second();

  get_button();
  if(emergency_time_now - emergency_time_start >= 5){ // 5초 이상 취소버튼 없을경우
    
    if(button_emergency == 1){

      //AWS IoT Core
      MQTT::Message message;
      char buf[100];
      strcpy(buf, "{\"state\":{\"reported\":{\"button\": true},\"desired\":{\"button\":true}}}");
      message.qos = MQTT::QOS0;
      message.retained = false;
      message.dup = false;
      message.payload = (void*)buf;
      message.payloadlen = strlen(buf) + 1;
      int rc = client->publish(aws_topic, message);

      
      Serial.println("응급호출버튼 데이터 전송");

      emergency_state = 1; //퇴장시간 20분으로 연장
      
      button_emergency = 0;
      
    }
    
  }

  get_button();
  if(entered && (during_time==1800 || during_time==1799 || during_time==1801)){ //30분

      //AWS IoT Core
      MQTT::Message message;
      char buf[100];
      strcpy(buf, "{\"state\":{\"reported\":{\"thirty_mins\":true},\"desired\":{\"thirty_mins\":true}}}");
      message.qos = MQTT::QOS0;
      message.retained = false;
      message.dup = false;
      message.payload = (void*)buf;
      message.payloadlen = strlen(buf) + 1;
      int rc = client->publish(aws_topic, message);

  }

  get_button();
  if(entered && (during_time==2700 || during_time==2699 || during_time==2701)){ //45분

      //AWS IoT Core
      MQTT::Message message;
      char buf[100];
      strcpy(buf, "{\"state\":{\"reported\":{\"forty_mins\":true},\"desired\":{\"forty_mins\":true}}}");
      message.qos = MQTT::QOS0;
      message.retained = false;
      message.dup = false;
      message.payload = (void*)buf;
      message.payloadlen = strlen(buf) + 1;
      int rc = client->publish(aws_topic, message);

  }

  get_button();
  if(entered && (during_time==3600 || during_time==3599 || during_time==3601)){ //60분

      //AWS IoT Core
      MQTT::Message message;
      char buf[100];
      strcpy(buf, "{\"state\":{\"reported\":{\"sixty_mins\":true},\"desired\":{\"sixty_mins\":true}}}");
      message.qos = MQTT::QOS0;
      message.retained = false;
      message.dup = false;
      message.payload = (void*)buf;
      message.payloadlen = strlen(buf) + 1;
      int rc = client->publish(aws_topic, message);

  }

  get_button();
  if(entered && (during_time==5400 || during_time==5399 || during_time==5401)){ // 90분

      //AWS IoT Core
      MQTT::Message message;
      char buf[100];
      strcpy(buf, "{\"state\":{\"reported\":{\"ninety_mins\":true},\"desired\":{\"ninety_mins\":true}}}");
      message.qos = MQTT::QOS0;
      message.retained = false;
      message.dup = false;
      message.payload = (void*)buf;
      message.payloadlen = strlen(buf) + 1;
      int rc = client->publish(aws_topic, message);
   
  }

  get_button();
  if(entered && (during_time==7200 || during_time==7199 || during_time==7201)){ // 120분

      //AWS IoT Core
      MQTT::Message message;
      char buf[100];
      strcpy(buf, "{\"state\":{\"reported\":{\"hundred_mins\":true},\"desired\":{\"hundred_mins\":true}}}");
      message.qos = MQTT::QOS0;
      message.retained = false;
      message.dup = false;
      message.payload = (void*)buf;
      message.payloadlen = strlen(buf) + 1;
      int rc = client->publish(aws_topic, message);
   
  }
  
}


void get_button(){

  if(button_count == 0){

    int time1 = 0; // 버튼 누름
    int time2 = 0; // 버튼 뗌
    button_push_time = 0;
    button_count = 0;
    button_value = digitalRead(button_OUT);
  
    if(button_value){
      time1 = (hour()*3600) + (minute()*60) + second();
      button_count = 1;
      while(button_value!=0){
        button_value = digitalRead(button_OUT);
        delay(50);    
      }
      time2 = (hour()*3600) + (minute()*60) + second();
    }

    button_push_time = time2 - time1;

    if(button_count!=0){
      Serial.print("버튼 눌린 시간 : ");
      Serial.println(button_push_time);
    }
  }
}


//connects to websocket layer and mqtt layer
bool connect() {

    if (client == NULL) {
        client = new MQTT::Client<IPStack, Countdown, maxMQTTpackageSize, maxMQTTMessageHandlers>(ipstack);
    }
    else {

        if (client->isConnected()) {
            client->disconnect();
        }
        delete client;
        client = new MQTT::Client<IPStack, Countdown, maxMQTTpackageSize, maxMQTTMessageHandlers>(ipstack);
    }

    delay(1000);
    Serial.print(millis());
    Serial.print(" - conn: ");
    Serial.print(++connection);
    Serial.print(" - (");
    Serial.print(ESP.getFreeHeap());
    Serial.println(")");

    int rc = ipstack.connect(aws_endpoint, port);
    Serial.print(rc);

    if (rc != 1)
    {
        Serial.println("error connection to the websocket server");
        return false;
    }
    else {
        Serial.println("websocket layer connected");
    }

    Serial.println("MQTT connecting");
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.MQTTVersion = 3;
    char* clientID = generateClientID();
    data.clientID.cstring = clientID;
    
    rc = client->connect(data);
    delete[] clientID;
    if (rc != 0)
    {
        Serial.print("error connection to MQTT server");
        Serial.println(rc);
        return false;
    }
    
    Serial.println("MQTT connected");
    return true;
}


//subscribe to a mqtt topic
void subscribe() {
    //subscript to a topic
    int rc = client->subscribe(aws_topic, MQTT::QOS0, messageArrived);
    Serial.println(rc);
    if (rc != 0) {
        Serial.print("rc from MQTT subscribe is ");
        Serial.println(rc);
        return;
    }
    Serial.println("MQTT subscribed");
}


//reset a message to a mqtt topic
void sendmessage() {
    //send a message
    MQTT::Message message;
    
    char buf[512];
    strcpy(buf, "{\"state\":{\"reported\":{\"enter\":false,\"button\":false,\"thirty_mins\":false,\"forty_mins\":false,\"sixty_mins\":false,\"ninety_mins\":false,\"hundred_mins\":false},\"desired\":{\"enter\":false,\"button\":false,\"thirty_mins\":false,\"forty_mins\":false,\"sixty_mins\":false,\"ninety_mins\":false,\"hundred_mins\":false}}}");
    //30분,60분,45분,90분,120
    
    message.qos = MQTT::QOS0;
    message.retained = false;
    message.dup = false;
    message.payload = (void*)buf;
    message.payloadlen = strlen(buf) + 1;
    int rc = client->publish(aws_topic, message);
}


void setup_wifi() {

    delay(10);
    // We start by connecting to a WiFi network
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}
