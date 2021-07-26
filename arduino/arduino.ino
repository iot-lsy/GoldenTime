#include <SoftwareSerial.h>

#define voice_recogn_RX 2
#define voice_recogn_TX 3
#define bluetooth_RX 6
#define bluetooth_TX 7

#define pir_LED 8
#define button_IN 12
#define pir_IN 10

#define emergency_signal 119 // 응급상황 119 값을 블루투스를 통해 앱으로전송

#define person_in 121 // pir센서 들어오는 경우 121 전송
#define person_out 122 // pir센서 나가는 경우 122 전송


SoftwareSerial mySerial_voice(voice_recogn_RX, voice_recogn_TX);
SoftwareSerial mySerial_BT(bluetooth_RX, bluetooth_TX);

int voice_input_hex = 0;
int bt_connected = 1; // 블루투스 연결O 1, 연결X 0 (!!!!!!!!!!!! 앱에서 받아와야함 !!!!!!!!!!!!!!!)
int entered = 0; // 화장실 들어옴 1, 나감 0
int time_delayed = 0; // 적외선 센서 마지막 감지 이후 경과된 시간
int emergency_status = 0; // 응급상황O 1, 응급상황X 0
int pir_value = LOW;
int pir_state = 0;
int button_value = LOW;



byte buffer[128];

void set_voice_recogn(); // 음성인식모듈 활성화
void voice_recogn(); // 음성인식
void button_pushed(); // 버튼
void pir_recogn(); // 사람 인식 (들어올때 1, 나갈때 0)
void send_signal(); // 응급신호 일괄 전송
void get_signal(); // 센서 신호 받아오는 함수

void setup() {
  
  pinMode(button_IN, INPUT);
  pinMode(pir_IN, INPUT);
  pinMode(pir_LED, OUTPUT);
  Serial.begin(9600);
  mySerial_BT.begin(9600);
  mySerial_voice.begin(9600);
  set_voice_recogn();
  delay(1000);
  
  if(mySerial_BT.available()){
  bt_connected = mySerial_BT.read(); // 앱에서 블루투스 연결상태 전송받아야함
  }
  
  delay(1000);
  Serial.println("setup() done");
}

void loop() { // 주기 : 1초
  if(bt_connected){ // 블루투스 연결O
    
    pir_recogn();
    Serial.println("블루투스 연결 성공");
    
    if(entered){ // 들어올때
      send_signal();
      
    }
    
  }else{ // 블루투스 연결X
    //Serial.println("블루투스 연결 끊김");
    //if(mySerial_BT.available()){
    //bt_connected = mySerial_BT.read(); // 연결 끊기면 연결상태 계속 점검   
    }

  Serial.println("done");
  Serial.println("");

  get_signal();
}

void send_signal(){
  
  if(emergency_status){
    mySerial_BT.write(buffer, emergency_signal);
    Serial.println("응급상황 앱으로 전송");
  }

  emergency_status = 0;
}

void get_signal(){

  for(int i=0; i<20; i++){
    button_pushed();
    voice_recogn();
    delay(50);
  }
  
}

void set_voice_recogn(){ // 음성인식 모듈 활성화
  Serial.println("wait settings are in progress");
  delay(500);
  mySerial_voice.write(0xAA); // compact mode
  mySerial_voice.write(0x37);
  mySerial_voice.write(0xAA);
  mySerial_voice.write(0x21);
  delay(500);

  Serial.println("The settings are complete");
}

void voice_recogn(){ // 음성인식 
  if(mySerial_voice.available()){
    //Serial.println("음성감지");
    voice_input_hex = mySerial_voice.read();
    //Serial.print("감지된 음성 16진수:");
    //Serial.println(voice_input_hex);
    
    switch(voice_input_hex){
      case 0x11:
      case 0x12:
      case 0x13:
      case 0x14:
      case 0x15:
        Serial.println("음성 응급호출");      
        emergency_status = 1;
        break;
    }
  }
}

void button_pushed(){ // 응급호출 버튼
  
  button_value = digitalRead(button_IN);
  //Serial.print("버튼 : ");
  //Serial.println(button_value);
  
  if(button_value == HIGH){
    Serial.println("버튼 응급호출");
    emergency_status = 1;
  }
  
}

void pir_recogn(){ // (!!!! 수정 필요<안에서 움직이는 경우에 대한 예외처리 해야함> !!!!)
  
  pir_value = digitalRead(pir_IN);
  Serial.print("pir 값 : ");
  Serial.println(pir_value);
  Serial.print("time_delayed : ");
  Serial.println(time_delayed);
  
  if(pir_value == HIGH && pir_state == 0){ // 들어오는 경우수
    
    Serial.println("들어옴");
    mySerial_BT.write(buffer, person_in);
    entered = 1;
    time_delayed = 0;
    pir_state = 1;
    digitalWrite(pir_LED, HIGH);
    
  } else if(pir_value == HIGH && pir_state == 1){ // 안에서 움직이는 경우
    
    Serial.println("내부활동");
    time_delayed = 0;
    pir_state = 1;
    digitalWrite(pir_LED, LOW);
    
  } else if(time_delayed > 15){ // 나가는 경우
    
    Serial.println("나감");
    mySerial_BT.write(buffer, person_out);
    entered = 0;
    time_delayed = 0;
    pir_state = 0;
    digitalWrite(pir_LED, LOW);
    
  }

  if(entered) time_delayed++;
}
