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
int delayed_time = 0; // 블루투스 연결이 끊겨 긴급호출 지연된 시간
int 

byte buffer[128];

void start_voice_recogn();
void voice_recogn();
void button_pushed();
void pir_recogn();
void start_bluetooth();

void setup() {
  
  pinMode(button_IN, INPUT);
  pinMode(pir_IN, INPUT);
  Serial.begin(9600);
  mySerial_voice.begin(9600);
  set_voice_recogn();
  
}

void loop() {



  if(pir_recogn() == 1){  
    if(mySerail_BT.available()){ 
    voice_recogn();
    button_pushed();
  }    
  }

  
  delay(1000);
}

int bt_connected(){
  mySerial_BT.begin(9600);  
  if(mySerial_BT.available){
    return 1; 
  }else{
    return 0;
  }
}

void set_voice_recogn(){ // 음성인식 모듈 활성화
  Serial.println("wait settings are in progress");
  delay(1000);
  mySerial_voice.write(0xAA); // compact mode
  mySerial_voice.write(0x37);
  delay(1000);
  mySerial_voice.write(0xAA);
  mySerial_voice.write(0x21);

  Serial.println("The settings are complete");
}

void voice_recogn(){ // 음성인식 
  while(mySerial_voice.available()){
    Serial.println("Voice input");
    voice_input_hex = mySerial_voice.read();
    Serial.print("voice_recogn:");
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
    Serial.println("Emergency Button Pushed");
    mySerial_BT.write(buffer, button_emergency);
  }
}

int pir_recogn(){ // (!!!! 수정 필요<계속 감지되는 경우에 대한 예외처리 해야함> !!!!)
  int value = digitalRead(pir_IN);
  int count = 0;

  if(value == HIGH && count==0){ // 들어오는 경우
    Serial.println("Movement recognition");
    mySerial_BT.write(buffer, person_in);
    count++;
    return 1;
  }else if(value == HIGH && count!=0){ // 나가는 경우
    Serial.println("Movement recognition");
    mySerial_BT.write(buffer, person_out);
    count = 0;
    return 0;
  }
}
