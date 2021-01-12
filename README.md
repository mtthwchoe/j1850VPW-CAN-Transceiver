# j1850VPW-CAN-Transceiver

![schematic](schematic.png)

  #include <SPI.h>
  #include <mcp2515.h>

  struct can_frame rpm;
  struct can_frame temp;
  MCP2515 mcp2515(10);

  void setup() {
    rpm.can_id  = 0x201; //rpm
    rpm.can_dlc = 8;

    temp.can_id = 0x420;
    temp.can_dlc = 1;
    temp.data[0] = 0xFF;

    delay(4000);
    Serial.begin(9600);
    Serial.println("ATST00");
    clearBuffer();
    mcp2515.reset();
    mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ);
    mcp2515.setNormalMode();
    delay(2000);
  }

  void loop() {
    char data;

    Serial.println("010c"); //coolant temp 1 byte
    delay(100);
    Serial.println("0105"); //rpm 2 bytes
    delay(100);
    Serial.println("010D"); //speed 1 byte
    delay(100);
    while(Serial.available() > 0) {
      data = Serial.read();
      if(data == '4' && Serial.read() == '1') {
        clearSpace();
        data = Serial.read();
        if(data == '0') {
          data = Serial.read();
          switch(data) {
            case '5':
              getTemp(&temp);
              break;
            case 'C':
              getRpm(&rpm);
              break;
            case 'D':
              getSpeed(&rpm);
              break;
          }
        }
      }
    }
    /*
    Serial.print('\n');
    Serial.print(temp.data[0]);
    Serial.print('\n');
    Serial.print('\n');
    Serial.print(rpm.data[0]);
    Serial.print('\n');
    Serial.print('\n');
    Serial.print(rpm.data[1]);
    Serial.print('\n');
    Serial.print('\n');
    Serial.print(rpm.data[4]);
    Serial.print('\n');
    Serial.print('\n');
    Serial.print(rpm.data[5]);
    Serial.print('\n');
    delay(500);
    */
    mcp2515.sendMessage(&rpm);
    mcp2515.sendMessage(&temp);
    //mcp2515.sendMessage(&oilTemp);
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

  void getTemp(can_frame * temp) {

    clearSpace();
    char a = Serial.read();
    char b = Serial.read();
    temp->data[0] = (hexToInt(a,b) - 40);
  }

  void getRpm(can_frame * rpm) {
    clearSpace();
    char a = Serial.read();
    char b = Serial.read();
    clearSpace();
    char c = Serial.read();
    char d = Serial.read();

    unsigned int A = hexToInt(a,b);
    unsigned int B = hexToInt(c,d);

    int RPM = ((256 * A) + B) / 4;
    int lower = 0b0000000011111111;
    int upper = 0b1111111100000000;
    rpm->data[0] = (RPM | upper) >> 8;
    rpm->data[1] = RPM | lower;
  }

  void getSpeed(can_frame * rpm) {
    clearSpace();
    char a = Serial.read();
    char b = Serial.read();
    int sped = hexToInt(a,b);
    int lower = 0b0000000011111111;
    int upper = 0b1111111100000000;
    rpm->data[4] = (sped | upper) >> 8;
    rpm->data[5] = sped | lower;  
  }
