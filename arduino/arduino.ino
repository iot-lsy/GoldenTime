#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>
#include <TimeLib.h>


// Set these to run example.
#define FIREBASE_HOST "arduino-541d0-default-rtdb.asia-southeast1.firebasedatabase.app"
#define FIREBASE_AUTH "R3RGvY2LloKhe5XfIjWksTr7w7ZovVmCIMZu643P"
#define WIFI_SSID "U+Net1D1B"
#define WIFI_PASSWORD "0192023420"

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
#define led_IN 5 //D1 - GPIO5
#define button_OUT 4 //D2 - GPIO4
#define pir_OUT 0 //D3 - GPIO0
#define buzzer_IN 2 //D4 - GPIO2
#define voice_RX 14 //D5 - GPIO14
#define voice_TX 12 //D6 - GPIO12


void inout_check(); // 출입감지 함수
void send_signal(); // 파이어베이스 데이터 전송 함수
void button_check(); // 버튼 센싱 함수
void voice_check(); // 음성 센싱 함수
void buzzer_out(int); // 부저 울리는 함수 (매개변수 0 : 취소알림소리, 1 : 응급상황알림소리, 2 : 입장알림소리)
void get_signal(); // 센서 데이터 가져오는 함수


int entered = 0; // 표준 0, 들어감 1, 나감 2
int count = 0; // 활동감지 횟수
int button_emergency = 0; // 버튼호출 표준 0, 호출 1
int voice_emergency = 0; // 음성호출 표준 0, 호출 1
int cancel_signal = 0; // 표준 0, 응급호출 신호 취소 1 
int pir_delay_time = 0; // 입장 시간
int delay_time = 0; // 와이파이 접속 끊어진 시간

int pir_value = 0; // pir 센서값
int button_value = 0; // 버튼 센서값
int button_count = 0; // 버튼 클릭 횟수

void setup() {
  
  // 센서 디지털 핀 모드 설정
  pinMode(pir_OUT, INPUT);
  pinMode(button_OUT, INPUT);
  pinMode(led_IN, OUTPUT);
  pinMode(buzzer_IN, OUTPUT);
  //음성인식 softwareserial 추가
  
  Serial.begin(115200);


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
  Serial.println("");
  Serial.println("파이어베이스 연결 성공!");
  
}


void loop() {

  //정상상태
  if(WiFi.status() == WL_CONNECTED && !Firebase.failed()){

    get_signal();
    inout_check();
    button_check();
    send_signal();

  //접속불량
  }else{

    if(WiFi.status() != WL_CONNECTED){
      WiFi.begin(WIFI_SSID, WIFI_PASSWORD);  

      Serial.print("와이파이 재연결 중");
    
      while(WiFi.status() != WL_CONNECTED){
        delay_time++;
        Serial.print("."); 
        get_signal();
      }
      
      Serial.println("와이파이 재연결 성공!");
    }
    
    if(Firbase.failed()){
      Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
    
      Serial.println("파이어베이스 재연결 중");
      
      while(Firebase.failed()){
        delay_time++;
        Serial.print(".");
        get_signal();
      }
      
      Serial.println("파이어베이스 재연결 성공!");
    }


    Serial.print("지연시간 : ");
    Serial.print(delay_time);
    Serial.println("(초)");
    pir_delay_time += delay_time;

  }

}


void inout_check(){

  if(pir_value == HIGH && count == 0){ // 들어오는 경우
    
    Serial.println("들어옴");
    pir_delay_time = 0;
    entered = 1;
    //처음 입장시간 저장
    //부저 함수 추가
    
  }else if(pir_value == HIGH && count != 0){ // 안에서 움직이는 경우
    
    Serial.println("내부활동");
    count = 1;
    pir_delay_time = 0;
    
  }else if(pir_delay_time >= 30){ // 나가는 경우 (내부활동 30초 이상 없을경우)
    
    Serial.println("나감");
    pir_delay_time = 0;
    count = 0;
    entered = 2;
    //입장시간 초기화

  }  

}


void button_check(){

  
  if(button_count <= 25){ // 일반적 응급호출 (최대 0.5초 까지 응급호출로 인식)
    
    button_emergency = 1;
    //부저 함수 추가
    
  }else{ // 모든 응급호출 취소 버튼 (버튼 0.5초 이상 누른경우)

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

  button_count = 0;
  
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

  
}

void get_signal(){

  for(int i=0;i<50;i++){
    if(pir_value != 1){
      pir_value = digitalRead(pir_OUT);
    }

    if(button_value != 1){
      button_value = digitalRead(button_OUT);
    }

    if(button_value){
      button_count++;
    }
    
    delay(20);
  }

  
  
}
