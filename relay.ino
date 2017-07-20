#include <EEPROM.h>

#define N 8
#define START_PIN 2
#define DELAY 1000
#define WAIT_TIME 60
#define SERIAL_TIMEOUT 5000
#define SEP " / "
#define ulong unsigned long

#define INTERVAL_FACTOR 86400
#define INTERVAL_NAME "days"

#define TIME_FACTOR 3600
#define TIME_NAME "hours"

ulong epoch = 0;
ulong PIN[N];
ulong INTERVAL[N];
ulong TIME[N];

ulong fromByte(byte a, byte b){
  ulong r = sizeof(byte)*a;
  r += b;
  return r;
}

void pinConfig(){
  for(int i = 0; i < N; i++){
    PIN[i] = i + START_PIN;
    pinMode(i + START_PIN, OUTPUT);
  }
}

void intervalConfig(){
  byte a, b;
  ulong tmp;

  for(int i = 0; i < N; i++){
    a = EEPROM.read(2*i);
    b = EEPROM.read(2*i+1);

    tmp = fromByte(a,b);
    INTERVAL[i] = tmp;
  }
}

void timeConfig(){
  byte a, b;
  ulong tmp;

  for(int i = N; i < 2*N; i++){
    a = EEPROM.read(2*i);
    b = EEPROM.read(2*i+1);

    tmp = fromByte(a,b);
    TIME[i] = tmp;
  }
}

void epochToDetail(ulong v){
  Serial.print(v/86400);
  Serial.print("d ");
  Serial.print((v%86400)/3600);
  Serial.print("h ");
  Serial.print((v%3600)/60);
  Serial.print("m ");
  Serial.print(v%60);
  Serial.print("s");
}

void showConfig(){
  if(epoch > 0){
    Serial.print("Epoch: ");
    Serial.print(epoch);
    Serial.print(" (");
    epochToDetail(epoch);
    Serial.println(") up."); 
  }
  
  for(int i = 0; i < N; i++){
    Serial.print(i);
    Serial.print(":\t");

    Serial.print("Interval: ");
    epochToDetail(INTERVAL[i]);
    Serial.print("\tTimeFrame: ");
    epochToDetail(TIME[i]);

    Serial.println();
  }
}



void setConfig(){
  ulong tmp;
  for(int i = 0; i < N; i++){
    Serial.println(i+1);

    Serial.print("Interval (in ");
    Serial.print(INTERVAL_NAME);
    Serial.println(")");
    tmp = readSerial()*INTERVAL_FACTOR;
    if(tmp > 0) INTERVAL[i] = tmp; 

    Serial.print("TimeFrame (in ");
    Serial.print(TIME_NAME);
    Serial.println(")");
    tmp = readSerial()*60;
    if(tmp > 0) TIME[i] =  tmp;

    Serial.println();
  }

  Serial.println("New Config:");
  showConfig();
}

ulong readSerial(){
  ulong tmp = 0;
  byte d = 0;
  Serial.println("Write the number. Press Enter to continue or ESC to cancel");
  do{
    if(Serial.available()){
      d = Serial.read();
      
      if(d == 27){
        tmp = 0;
        d = 13; 
      }else if(d > 47 && d < 58){
        Serial.write(d);
        tmp *= 10;
        tmp += (int)d - 48;
      }
    }
  }while(d != 13);

  
  Serial.println();
  return tmp;
}

void writeConfig(){
  
}

void checkSerial(){
  if(Serial.available() > 0){
    char a = Serial.read();

    if(a == 's'){
      showConfig();
    }else if(a == 'c'){
      setConfig();
    }else if(a == 'w'){
      writeConfig();
    }else{
      Serial.println("?");
    }
  }
}

void setRelays(){
  ulong interval, time, delta;
  bool r;
  for(int i = 0; i < N; i++){
    interval = INTERVAL[i];
    time = TIME[i];
    delta = epoch%interval;
    r = delta > interval-time;
    digitalWrite(PIN[i], r ? HIGH : LOW);
  }
}

void setup() {
  Serial.begin(9600);
  Serial.setTimeout(SERIAL_TIMEOUT);
  
  pinConfig();
  intervalConfig();
  timeConfig();
  
  showConfig();
}

void loop() {
  checkSerial();
  setRelays();
  
  epoch++;
  delay(DELAY);
}
