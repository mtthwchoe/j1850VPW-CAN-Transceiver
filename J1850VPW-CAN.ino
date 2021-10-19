#include <SPI.h>
#include <mcp2515.h>

void clearSpace();
void clearBuffer();
void printBuffer();
void clearChar();
int hexToInt(char a, char b);
void getTemp(can_frame * b420);
void getRpm(can_frame * b201);
void getSpeed(can_frame * b201);

char dummy;
char timeout = 0;
struct can_frame b201;
struct can_frame b212;
struct can_frame b420;
MCP2515 mcp2515(10);

void setup() {
  b201.can_id  = 0x201;
  b201.can_dlc = 8;
  b201.data[0] = (8000 * 4) / 256;  // high byte rpm
  b201.data[1] = (8000 * 4) % 256;  // low byte rpm
  b201.data[2] = 0;
  b201.data[3] = 0;
  b201.data[4] = 0;                 // high byte speed
  b201.data[5] = 0;                 // Low byte speed
  b201.data[6] = 0;                 // throttle pos
  b201.data[7] = 0;                 //

  b212.can_id  = 0x212;
  b212.can_dlc = 7;
  b212.data[0] = 0;
  b212.data[1] = 0;
  b212.data[2] = 0;
  b212.data[3] = 0;
  b212.data[4] = 0;
  b212.data[5] = 0;
  b212.data[6] = 0;
  
  b420.can_id = 0x420;
  b420.can_dlc = 7;
  b420.data[0] = 170; // coolant temp
  b420.data[1] = 0;   // odometer
  b420.data[2] = 0;
  b420.data[3] = 0;
  b420.data[4] = 1;   // oil pressure
  b420.data[5] = 0;
  b420.data[6] = 0;
  
  delay(3000);
  Serial.begin(9600);
  //Serial.println("ATE0");
  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ);
  mcp2515.setNormalMode();

  for(int i = 0; i < 10; i++) { // start up test
    mcp2515.sendMessage(&b201);
    mcp2515.sendMessage(&b212);
    mcp2515.sendMessage(&b420);
    delay(100);
  }
  clearBuffer();
  delay(1000);
}

void loop() {

  Serial.println("01 05"); // request coolant temp from elm322 mode 01 pid 05
  delay(90);              //wait for reply
  clearChar();
  while(Serial.available() > 0) {
    if(Serial.read() == '4' && Serial.read() == '1' && Serial.read() == ' ' && Serial.read() == '0' && Serial.read() == '5' && Serial.read() == ' ') {
      getTemp(&b420);
      clearBuffer();
    }
    else {
      clearBuffer();
    }
  }
  mcp2515.sendMessage(&b420);

  Serial.println("01 0C"); // request rpm
  delay(90);
  clearChar();
  while(Serial.available() > 0) {
    if(Serial.read() == '4' && Serial.read() == '1' && Serial.read() == ' ' && Serial.read() == '0' && Serial.read() == 'C' && Serial.read() == ' ') {
      getRpm(&b201);
      clearBuffer();
    }
    else {
      //b201.data[0] = (0 * 4) / 256;  // high byte rpm
      //b201.data[1] = (0 * 4) % 256;  // low byte rpm
      clearBuffer();
    }
  }
  mcp2515.sendMessage(&b201);

  Serial.println("01 0D"); // request speed
  delay(90);
  clearChar();
  while(Serial.available() > 0) {
    if(Serial.read() == '4' && Serial.read() == '1' && Serial.read() == ' ' && Serial.read() == '0' && Serial.read() == 'D' && Serial.read() == ' ') {
      getSpeed(&b201);
      clearBuffer();
    }
    else {
      clearBuffer();
    }
  }
  mcp2515.sendMessage(&b201);
  delay(10);
  mcp2515.sendMessage(&b201);
  delay(10);
  mcp2515.sendMessage(&b201);
  delay(10);
  mcp2515.sendMessage(&b201);
  delay(10);
  mcp2515.sendMessage(&b201);
  delay(10);
  mcp2515.sendMessage(&b201);
  delay(10);
  mcp2515.sendMessage(&b212);
  delay(10);
  mcp2515.sendMessage(&b420);
}

void clearSpace() {
  char filler;
  if(Serial.peek() == '\n' || Serial.peek() == '\r' || Serial.peek() == ' ') {
    filler = Serial.read();
    clearSpace();
  }
}

void clearBuffer() {
  char filler;
  while(Serial.available() > 0) {
    filler = Serial.read();
  }
}


void printBuffer() {
  char filler;
  while(Serial.available() > 0) {
    filler = Serial.read();
    Serial.print(filler);
    //Serial.print('\n');
  }
}

void clearChar() {
  while(Serial.peek() != '4') {
    dummy = Serial.read();
    timeout++;
    if(timeout > 20) {
      timeout = 0;
      break;
    }
  }
}

int hexToInt(char a, char b) {
  int num;
  if(a < 58) {
    num = ((a - '0') * 16);
  }
  else {
    num = ((a - '7') * 16);
  }
  if(b < 58) {
    num += (b - '0');
  }
  else {
    num += (b - '7');
  }
  return(num);
}

void getTemp(can_frame * b420) {
  char a = Serial.read();
  char b = Serial.read();
  b420->data[0] = ((hexToInt(a,b) - 40) * 9 / 5) + 32 - 40; // (hex to Int(a,b) - 40) gives ect in celcius, convert to farenheit and subtract 40. coolant temp gauge 50% = 145, engine operating temp = 185
}

void getRpm(can_frame * b201) {
  clearSpace();
  char a = Serial.read();
  char b = Serial.read();
  clearSpace();
  char c = Serial.read();
  char d = Serial.read();

  unsigned int A = hexToInt(a,b);
  unsigned int B = hexToInt(c,d);

  int rpm = ((256 * A) + B) / 4;
  b201->data[0] = (rpm * 4) / 256;
  b201->data[1] = (rpm * 4) % 256;
}

void getSpeed(can_frame * b201) {
  clearSpace();
  char a = Serial.read();
  char b = Serial.read();
  int kmh = hexToInt(a,b);
  b201->data[4] = (kmh * 100 + 10000) / 256;
  b201->data[5] = (kmh * 100 + 10000) % 256; 
}