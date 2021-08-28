#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>

// Set these to run example.
#define FIREBASE_HOST ""
#define FIREBASE_AUTH ""
#define WIFI_SSID ""
#define WIFI_PASSWORD ""

#define led_IN 4
#define button_OUT 5
#define pir_OUT 6
#define buzzer_IN 7
#define voice_TX 8
#define voice_RX 9

#define voice_emergency 111 // 음성 응급호출 111 전송
#define button_emergency 112 // 버튼 응급호출 112 전송
#define button_emergency_cancel 113 // 응급호출 취소 113 전송
#define person_in 121 // pir센서 들어오는 경우 121 전송
#define person_out 122 // pir센서 나가는 경우 122 전송

void inout_check(); // 출입감지 함수
void filtering_signal(); // 센싱 데이터 필터링 함수
void send_signal(); // 파이어베이스 데이터 전송 함수
void button_check(); // 버튼 센싱 함수

int entered = 0; // 들어감 1, 나감 0
int count = 0; // 활동감지 횟수
int button_push_time = 0; // 버튼 눌러지는 시간
int button_emergency = 0; // 버튼호출 표준 0 호출 1
int voice_emergency = 0; // 음성호출 표준 0 호출 1


void setup() {
  Serial.begin(9600);

  // 센서 디지털 핀 모드 설정
  pinMode(button_OUT, INPUT);
  pinMode(pir_OUT, INPUT);
  pinMode(led_IN, OUTPUT);
  pinMode(buzzer_IN, OUTPUT);

  //와이파이 연결
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("와이파이 연결 중");
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  
  Serial.println();
  Serial.println("와이파이 연결 성공!");
  Serial.print("SSID : ");
  Serial.println(WiFi.localIP());

  //파이어베이스 연결
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Serial.print("파이어베이스 연결 중");
  
  while(Firebase.failed()){
    Serial.print(".");
    delay(500);
  }

  Serial.println("파이어베이스 연결 성공!");
  
}


void loop() {

  inout_check();
  
  
  
  
}

void inout_check(){

  int value = digitalRead(pir_OUT);


  if(value == HIGH && count == 0){ // 들어오는 경우
    
    Serial.println("들어옴");
    count = 1;
    entered = 1;
    //처음 입장시간 저장
    
  }else if(value == HIGH && count != 0){ // 안에서 움직이는 경우
    
    Serial.println("내부활동");
    
  }else if(time_delayed >= 30){ // 나가는 경우 (내부활동 30초 이상 없을경우)
    
    Serial.println("나감");
    count = 0;
    entered = 0;
    //입장시간 초기화
    
  }
  
  
}


void button_check(){

  int value = digitalRead(button_OUT);

  if(value == HIGH){
    while(value != LOW){
      button_pushed_time++;
      value = digitalRead(button_OUT);
      delay(1000);
    }
  }

  if(button_pushed_time<=2){ // 일반적 응급호출 (최대 2.9초 까지 응급호출로 인식)
    
    button_emergency = 1;
    
  }else{ // 모든 응급호출 취소 버튼 (버튼 3초 이상 누른경우)

    if(button_emergency){
      
      Serial.println("버튼 응급호출 취소");
      button_emergency = 0;
      
    }else if(voice_emergency){
      
      Serial.println("음성 응급호출 취소");
      voice_emergency = 0;
      
    }else{
      
      Serial.println("취소할 응급호출 신호가 존재하지 않습니다");
      
    }
    
  }

  
  
}

void filtering_siganl(){
  
}

void send_signal(){
  /*
  Firebase.setFloat("number", 42.0);
  // handle error
  if (Firebase.failed()) {
      Serial.print("setting /number failed:");
      Serial.println(Firebase.error());  
      return;
  }
  delay(1000);
  
  // update value
  Firebase.setFloat("number", 43.0);
  // handle error
  if (Firebase.failed()) {
      Serial.print("setting /number failed:");
      Serial.println(Firebase.error());  
      return;
  }
  delay(1000);
  // get value 
  Serial.print("number: ");
  Serial.println(Firebase.getFloat("number"));
  delay(1000);

  // remove value
  Firebase.remove("number");
  delay(1000);

  // set string value
  Firebase.setString("message", "hello world");
  // handle error
  if (Firebase.failed()) {
      Serial.print("setting /message failed:");
      Serial.println(Firebase.error());  
      return;
  }
  delay(1000);
  
  // set bool value
  Firebase.setBool("truth", false);
  // handle error
  if (Firebase.failed()) {
      Serial.print("setting /truth failed:");
      Serial.println(Firebase.error());  
      return;
  }
  delay(1000);

  // append a new value to /logs
  String name = Firebase.pushInt("logs", n++);
  // handle error
  if (Firebase.failed()) {
      Serial.print("pushing /logs failed:");
      Serial.println(Firebase.error());  
      return;
  }
  Serial.print("pushed: /logs/");
  Serial.println(name);
  delay(1000);
  */
  
}
