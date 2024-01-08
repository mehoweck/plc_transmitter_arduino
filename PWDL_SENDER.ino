
//
// this sketch sends the dataArray over the pin OUTPUT_PIN every time 
// when BUTTON_PIN is pressed (there is a logic low on this pin)
//

#define BTN_4_PIN D4
#define BTN_3_PIN D5
#define BTN_2_PIN D6
#define BTN_1_PIN D7
#define OUTPUT_PIN D0

#define BYTE_PIN D1
#define TICK_PIN D2

#define ADDRESS_LENGTH 4
#define COMMAND_LENGTH 3
#define PARAM1_LENGTH 3
#define PARAM2_LENGTH 3
#define PARAM3_LENGTH 3
//#define FRAME_LENGTH 16

#define ADDRESS_BROADCAST 0b1011 // match any of the receiver addresses

#define readButtonInterval 50000  //25ms
#define minCommandInterval 300000 //600ms 

#define LED_ON digitalWrite(LED_BUILTIN, LOW)
#define LED_OFF digitalWrite(LED_BUILTIN, HIGH)

typedef union {
  struct {
    unsigned int  address : ADDRESS_LENGTH;  // 4 bits for address
    unsigned int  command : COMMAND_LENGTH;  // 3 bits for command
    unsigned int  param1 : PARAM1_LENGTH;   // 3 bits for param1
    unsigned int  param2 : PARAM2_LENGTH;   // 3 bits for param2
    unsigned int  param3 : PARAM3_LENGTH;   // 3 bits for param3
  } fields;
  uint16_t rawWord;
} CommandData;

typedef union {
  struct {
    unsigned int  address : 4;  // 4 bits for address
    unsigned int  command : 4;  // 3 bits for command
  } fields;
  uint16_t rawWord;
} TestData;

enum CommandCode : uint8_t {
  CMD_LED_OFF  = 1, // turn LED off
  CMD_LED_ON   = 2, // turn LED on (param1 - light level 0-7) 
  CMD_LED_UP   = 3, // smoothly raise the LED brightness to the level given by param1 for time in param2 
  CMD_LED_DOWN = 4, // smoothly dimm to the level given by param1
  CMD_LED_AUTO = 5, // set the LED light level automaticaly
  CMD_LED_RGB  = 6, // turn the RBG color given by params[1-3] 
  CMD_FUTURE2  = 7, // future use
  CMD_FUTURE3  = 0  // future use
} command_; 

volatile CommandData commandData;

uint8_t commandCode;
uint32_t timeSinceLastCommand;

void setup() {
  Serial.begin(9600);
  Serial.println("Start....");

  pinMode(BTN_1_PIN, INPUT_PULLUP);
  pinMode(BTN_2_PIN, INPUT_PULLUP);
  pinMode(BTN_3_PIN, INPUT_PULLUP);
  pinMode(BTN_4_PIN, INPUT_PULLUP);
  
  pinMode(OUTPUT_PIN, OUTPUT);
  digitalWrite(OUTPUT_PIN, HIGH);

  pinMode(BYTE_PIN, OUTPUT);
  pinMode(TICK_PIN, OUTPUT);

  digitalWrite(BYTE_PIN, HIGH);
  digitalWrite(TICK_PIN, HIGH);

  pinMode(LED_BUILTIN, OUTPUT);
  LED_OFF;
}

/**
*  main loop ================================
*/
void loop() {
  LED_OFF;
  commandCode |= readButton();
  
  if (timeSinceLastCommand >= minCommandInterval) {
    LED_ON;

    if (commandCode > 0) {
      Serial.print("\ncommand: ");
      Serial.println(commandCode);

      switch(commandCode) {
        case CMD_LED_OFF : sendCommand(ADDRESS_BROADCAST, CMD_LED_OFF, 0, 0, 0); break;
        case CMD_LED_ON  : sendCommand(ADDRESS_BROADCAST, CMD_LED_ON, 5, 2, 1); break; 
        case CMD_LED_UP  : sendCommand(ADDRESS_BROADCAST, CMD_LED_UP,   2, 7, 2); break;
        case CMD_LED_DOWN: sendCommand(ADDRESS_BROADCAST, CMD_LED_DOWN, 0, 0, 0); break;
        case CMD_LED_AUTO: sendCommand(ADDRESS_BROADCAST, CMD_LED_AUTO, 0, 0, 0); break;
        case CMD_LED_RGB : sendCommand(ADDRESS_BROADCAST, CMD_LED_RGB,  0, 0, 0); break;     
        case CMD_FUTURE2 : sendCommand(ADDRESS_BROADCAST, CMD_FUTURE2,  0, 0, 0); break;   
        case CMD_FUTURE3 : sendCommand(ADDRESS_BROADCAST, CMD_FUTURE3,  0, 0, 0); break;
      }
      delayMicroseconds(minCommandInterval);
    }

    commandCode = 0;
    timeSinceLastCommand = 0;
  } 
  else {
    timeSinceLastCommand += readButtonInterval;
    delayMicroseconds(readButtonInterval);
  }

}

uint8_t readButton() {
  uint8_t command = 0;
  if (digitalRead(BTN_3_PIN) == LOW) command |= 4;
  if (digitalRead(BTN_2_PIN) == LOW) command |= 2;
  if (digitalRead(BTN_1_PIN) == LOW) command |= 1;
  return command;
}

void sendCommand(byte address, CommandCode command, byte param1, byte param2, byte param3) {
  commandData.fields.address = address;
  commandData.fields.command = command;
  commandData.fields.param1  = param1;
  commandData.fields.param2  = param2;
  commandData.fields.param3  = param3;

  sendData(commandData.rawWord);  
  printData(commandData.rawWord);
}

void printData(volatile uint16_t dataWord) {
  Serial.print("sendData:");
  Serial.println(dataWord, HEX);

  for (int i = 15; i >= 0; i--) {
    Serial.print(bitRead(dataWord, i)); 
  }

  Serial.println("");
}

void sendData(volatile uint16_t dataWord) {
  digitalWrite(OUTPUT_PIN, LOW);
  delayMicroseconds(50);
  
  for (int i = 15; i >= 0; i--) {
    if (1 == bitRead(dataWord, i)) {
      sendHigh();
    } else { 
      sendLow();
    }  
  }

  digitalWrite(OUTPUT_PIN, HIGH);
}

inline void sendHigh() {
  digitalWrite(OUTPUT_PIN, HIGH);
  delayMicroseconds(70);
  digitalWrite(OUTPUT_PIN, LOW);
  delayMicroseconds(28);
}

inline void sendLow() {
  digitalWrite(OUTPUT_PIN, HIGH);
  delayMicroseconds(30);
  digitalWrite(OUTPUT_PIN, LOW);
  delayMicroseconds(68);
}

inline void tick(uint8_t tick_pin) {
  digitalWrite(tick_pin, LOW);
  digitalWrite(tick_pin, HIGH);
}

String byte2Bin(byte value) {
  String binaryString = "";
  for (int i = 0; i <= 7; i++) {
    binaryString += char('0' + bitRead(value, i));
  }
  return binaryString;
}