#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>



// Set these to run example.
#define FIREBASE_HOST "arduino-541d0-default-rtdb.asia-southeast1.firebasedatabase.app"
#define FIREBASE_AUTH "R3RGvY2LloKhe5XfIjWksTr7w7ZovVmCIMZu643P"
#define WIFI_SSID "U+Net1D1B"
#define WIFI_PASSWORD "0192023420"

//D6~11 플래시메모리용으로 사용중
//D0,2,15 부팅모드 설정용 사용시 주의
//D1,3 시리얼 통신용
//D16 Wake용

#define led_IN 4
#define button_OUT 7
#define buzzer_IN 5
#define voice_TX 8
#define voice_RX 9

const int GPIO_1 = 0;
const int led = 0;

uint8_t pir_OUT = 6;

#define wifi_RX 2
#define wifi_TX 3

SoftwareSerial mywifi(wifi_RX, wifi_TX);

//#define voice_emergency 111 // 음성 응급호출 111 전송
//#define button_emergency 112 // 버튼 응급호출 112 전송
//#define button_emergency_cancel 113 // 응급호출 취소 113 전송
//#define person_in 121 // pir센서 들어오는 경우 121 전송
//#define person_out 122 // pir센서 나가는 경우 122 전송

void inout_check(); // 출입감지 함수
void send_signal(); // 파이어베이스 데이터 전송 함수
void button_check(); // 버튼 센싱 함수
void voice_check(); // 음성 센싱 함수
void buzzer_out(int); // 부저 울리는 함수 (매개변수 0 : 취소알림소리, 1 : 응급상황알림소리, 2 : 입장알림소리)

int entered = 0; // 표준 0, 들어감 1, 나감 2
int count = 0; // 활동감지 횟수
int button_push_time = 0; // 버튼 눌러지는 시간
int button_emergency = 0; // 버튼호출 표준 0, 호출 1
int voice_emergency = 0; // 음성호출 표준 0, 호출 1
int cancel_signal = 0; // 표준 0, 응급호출 신호 취소 1 
int pir_delayed_time = 0; 
int delayed_time = 0; // 와이파이 접속 끊어진 시간


void setup() {
  
  // 센서 디지털 핀 모드 설정
  pinMode(pir_OUT, INPUT);
  pinMode(button_OUT, INPUT);
  pinMode(led_IN, OUTPUT);
  pinMode(buzzer_IN, OUTPUT);
  
  Serial.begin(9600);
  mywifi.begin(9600);


  //와이파이 연결
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  
  Serial.println();
  Serial.println("connected!");
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

  if(WiFi.status() == WL_CONNECTED && !Firebase.failed()){
    
    inout_check();
    button_check();
    send_signal();
    Serial.println("loop()");
  
  }else{
    
    Serial.print("와이파이 재연결 중");
    
    while(WiFi.status() != WL_CONNECTED){
      delayed_time++;
      Serial.print("."); 
      delay(1000);
    }
    Serial.println("와이파이 재연결 성공!");
    
    Serial.println("파이어베이스 재연결 중");
    while(Firebase.failed()){
      delayed_time++;
      Serial.print(".");
      delay(1000);
      
    }
    Serial.println("파이어베이스 재연결 성공!");
    Serial.print("지연시간 : ");
    Serial.print(delayed_time);
    Serial.println("(초)");
    pir_delayed_time += delayed_time;
  }

}


void inout_check(){

  int value = digitalRead(pir_OUT);
  Serial.print("value : ");
  Serial.println(value);
  if(value == HIGH && count == 0){ // 들어오는 경우
    
    Serial.println("들어옴");
    pir_delayed_time = 0;
    entered = 1;
    //처음 입장시간 저장
    //부저 함수 추가
    
  }else if(value == HIGH && count != 0){ // 안에서 움직이는 경우
    
    Serial.println("내부활동");
    count = 1;
    pir_delayed_time = 0;
    
  }else if(pir_delayed_time >= 30){ // 나가는 경우 (내부활동 30초 이상 없을경우)
    
    Serial.println("나감");
    pir_delayed_time = 0;
    count = 0;
    entered = 2;
    //입장시간 초기화

  }  

  pir_delayed_time++;
  delay(1000);  
}


void button_check(){

  int value = digitalRead(button_OUT);

  if(value == HIGH){
    while(value != LOW || button_push_time != 3){
      button_push_time++;
      pir_delayed_time++;
      value = digitalRead(button_OUT);
      delay(1000);
    }
  }

  if(button_push_time <= 2){ // 일반적 응급호출 (최대 2.9초 까지 응급호출로 인식)
    
    button_emergency = 1;
    
  }else{ // 모든 응급호출 취소 버튼 (버튼 3초 이상 누른경우)

    if(button_emergency){
      
      Serial.println("버튼 응급호출 취소");
      button_emergency = 0;
      cancel_signal = 1;
      //부저 함수 추가
      
    }else if(voice_emergency){
      
      Serial.println("음성 응급호출 취소");
      voice_emergency = 0;
      cancel_signal = 1;
      //부저 함수 추가
      
    }else{
      
      Serial.println("취소할 응급호출 신호가 존재하지 않습니다");
      
    }
    
  }

  button_push_time = 0;
  
}

void send_signal(){  // 중요 - 모든 응급신호 10초간 대기하고 10초 안에 취소 버튼 누를경우 응급신호 보내지 않음

  if(entered == 1 && count == 0){
    Firebase.setString("enter", "true");
    Serial.println("파이어베이스 입장 데이터 전송");
    count++;
    // handle error
    if (Firebase.failed()) {
      
        Serial.print("출입상태 전송 오류 : ");
        Serial.println(Firebase.error());  
        count = 0;
        
    }
  }
  

  if(entered == 2){
    
    Firebase.setString("enter", "false");
        Serial.println("파이어베이스 퇴장 데이터 전송");
    // handle error
    if (Firebase.failed()) {
      
        Serial.print("setting /number failed:");
        Serial.println(Firebase.error());
        
    }else{ // 퇴장과 동시에 출입센서 초기화
      entered = 0;
      count = 0;
    }
  }

  //버튼 전송 조건문 


  /*
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
