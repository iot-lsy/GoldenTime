#include <SoftwareSerial.h>

#define voice_recogn_RX 2
#define voice_recogn_TX 3
#define bluetooth_RX 6
#define bluetooth_TX 7

#define button_IN 12
#define pir_IN 10

#define voice_emergency 111 // 음성 응급호출 111 전송
#define button_emergency 112 // 버튼 응급호출 112 전송
#define person_in 121 // pir센서 들어오는 경우 121 전송
#define person_out 122 // pir센서 나가는 경우 122 전송

SoftwareSerial mySerial_voice(voice_recogn_RX, voice_recogn_TX);
SoftwareSerial mySerial_BT(bluetooth_RX, bluetooth_TX);

int voice_input_hex = 0;
int bt_connected = 0; // 블루투스 연결O 1, 연결X 0 (!!!!!!!!!!!! 앱에서 받아와야함 !!!!!!!!!!!!!!!)
int entered = 0; // 화장실 들어옴 1, 나감 0

byte buffer[128];

void start_voice_recogn(); // 음성인식모듈 활성화
void voice_recogn(); // 음성인식
void button_pushed(); // 버튼
void pir_recogn(); // 사람 인식 (들어올때 1, 나갈때 0)

void setup() {
  
  pinMode(button_IN, INPUT);
  pinMode(pir_IN, INPUT);
  Serial.begin(9600);
  mySerial_BT.begin(9600);
  mySerial_voice.begin(9600);
  set_voice_recogn();
  delay(1000);
  bt_connected = mySerial_BT.read(); // 앱에서 블루투스 연결상태 전송받아야함
  delay(1000);
  
}

void loop() { // 주기 : 1초

  if(bt_connected){ // 블루투스 연결O

    pir_recogn();

    if(entered){ // 들어올때
      voice_recogn();
      button_pushed();
    }else{ // 나갈때
      
    }
    
  }else{ // 블루투스 연결X

    bt_connected = mySerial_BT.read(); // 연결 끊기면 연결상태 계속 점검
    
  }

  delay(1000);
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
  while(mySerial_voice.available()){
    Serial.println("음성감지");
    voice_input_hex = mySerial_voice.read();
    Serial.print("감지된 음성 16진수:");
    Serial.println(voice_input_hex);
    
    switch(voice_input_hex){
      case 0x11:
      case 0x12:
      case 0x13:
      case 0x14:
      case 0x15:
        Serial.println("음성 응급호출");
        mySerial_BT.write(buffer, voice_emergency);
        break;
    }
  }
}

void button_pushed(){ // 응급호출 버튼
  int value = digitalRead(button_IN);

  if(value == HIGH){
    Serial.println("버튼 응급호출");
    mySerial_BT.write(buffer, button_emergency);
  }
  
}

int pir_recogn(){ // (!!!! 수정 필요<계속 감지되는 경우에 대한 예외처리 해야함> !!!!)
  int value = digitalRead(pir_IN);
  int count = 0;

  if(value == HIGH && count == 0){ // 들어오는 경우
    Serial.println("들어옴");
    mySerial_BT.write(buffer, person_in);
    count = 1;
    entered = 1;
  }else if(value == HIGH && count != 0){ // 안에서 움직이는 경우
    Serial.println("내부활동");
    count++;
  }

  
}
