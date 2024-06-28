/*
 Name:		TEST_Single_CAN_HW_184_NANO.ino
 Created:	2/13/2022 7:54:03 AM
 Author:	Micro
*/

// CAN Receive Example
// Uses the MCP_CAN_lib-master-1.5.0 library
// https://github.com/coryjfowler/MCP_CAN_lib
// This one is set for Arduino Nano

#include <mcp_can_dfs.h>
#include <mcp_can.h>
#include <SPI.h>

unsigned long upTimer = millis();
unsigned long dtaCounter = 0;
uint16_t clutchValue = 18543;
bool clutchDirection = 0;

#define CAN0_INT 7                              // Set INT to pin 2
MCP_CAN CAN0(17);                               // Set CS to pin 10

// Comment out to enter receive mode
#define send

// the setup function runs once when you press reset or power the board
void setup() {
  // Serial needs to be at least this speed otherwise line will be dropped
  Serial.begin(250000);
  // while (!Serial);   // COmment out if Serial will not be used
  delay(100);
  Serial.println("Single Can Bus Sender/Logger, 500kb/s...");

  // Initialize MCP2515 running at 8MHz with a baudrate of 500kb/s and the masks and filters disabled.
  if (CAN0.begin(MCP_ANY, CAN_500KBPS, MCP_16MHZ) == CAN_OK)
    Serial.println("CAN0: MCP2515 Initialized Successfully!");
  else
    Serial.println("CAN0: Error Initializing MCP2515...");
  CAN0.setMode(MCP_NORMAL);                              // Set operation mode to normal so the MCP2515 sends acks to received data.
  pinMode(CAN0_INT, INPUT);                            // Configuring pin for /INT input

  pinMode(23, OUTPUT);
  digitalWrite(23, HIGH);
}

// the loop function runs over and over again until power down or reset
void loop() {
#ifndef send
  if (!digitalRead(CAN0_INT))                         // If CAN0_INT pin is low, read receive buffer
  {
    readCan(CAN0, 0);
  }
#else
  sendCan(CAN0, 0);
#endif
}

void readCan(MCP_CAN myCan, uint8_t MCP2515number) {
  //Serial.println(micros() - timer1);

  long unsigned int rxId;
  unsigned char len = 0;
  unsigned char rxBuf[8];
  char msgString[128];                            // Array to store serial string
  myCan.readMsgBuf(&rxId, &len, rxBuf);      // Read data: rxId = Frame ID, len = data length, buf = data byte(s)

  if ((rxId & 0x80000000) == 0x80000000)     // Determine if ID is standard (11 bits) or extended (29 bits)
    sprintf(msgString, "CAN%1d: Extended ID: 0x%.8lX  DLC: %1d  Data:", MCP2515number, (rxId & 0x1FFFFFFF), len);
  else
    //sprintf(msgString, "CAN%1d: Standard ID: 0x%.3lX       DLC: %1d  Data:", MCP2515number, rxId, len);
    sprintf(msgString, "%.8d  %1d  %.3lX  %1d  ", millis() - upTimer, MCP2515number, rxId, len);

  Serial.print(msgString);

  if ((rxId & 0x40000000) == 0x40000000) {    // Determine if message is a remote request frame.
    sprintf(msgString, " REMOTE REQUEST FRAME");
    Serial.print(msgString);
  }
  else {
    for (byte i = 0; i < len; i++) {
      sprintf(msgString, " %.2X", rxBuf[i]);
      Serial.print(msgString);
    }
  }


  Serial.println();
  digitalWrite(23, !digitalRead(23));

  //timer1 = micros();
}

void sendCan(MCP_CAN myCan, uint8_t MCP2515number) {
  unsigned char dta[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

  // Create loop timer
  unsigned long timer = millis();

  // This sends a data counter on ID 0x147 simply so I can see the Nano and HS CAN has active data
  
  // Increase the counter
  dtaCounter++;
  if (dtaCounter == 1099511627775) dtaCounter = 0;

  //Parse that dtaCounter into values
  dta[7] = dtaCounter;
  dta[6] = dtaCounter >> 8;
  dta[5] = dtaCounter >> 16;
  dta[4] = dtaCounter >> 24;
  dta[3] = dtaCounter >> 32;
  //dta[2] = dtaCounter >> 40;
  //dta[1] = dtaCounter >> 48;
  //dta[0] = dtaCounter >> 56;


  //can.send(0x0145, 0, 0, 8, dta);   // SEND TO ID:0X55
  myCan.sendMsgBuf(0x147, 0, 8, dta);
  //Serial.print("Sending: ");
  //Serial.print(0x0145, HEX);
  //Serial.print(" ");
  //Serial.print("00 00 08 ");
  //for (int i = 0; i <8; i++) {
  //  Serial.print(dta[i],HEX); Serial.print(" ");
  //}
  //Serial.println("");

  
  // Send Clutch Pedal % = HS_0x117  (D2 (bits 0 & 1) * 256 + D3) / 1024 * 100 - updated ~52ms
  // Clutch Pedal % = clutchValue

  //Serial.print("clutchValue = "); Serial.println(clutchValue);
  //Serial.print("clutchValue >> 8 = "); Serial.println(clutchValue >> 8);
  //Serial.print("clutchValue & 0xFF = "); Serial.println(clutchValue >> clutchValue & 0xFF);

  
  dta[7] = 0;
  dta[6] = 0;
  dta[5] = 0;
  dta[4] = 0;
  dta[3] = clutchValue & 0xFF;
  dta[2] = clutchValue >> 8;
  dta[1] = 0;
  dta[0] = 0;

  //Serial.print("Values "); Serial.print(dta[3],HEX); Serial.print(", "); Serial.println(dta[2], HEX);

  //int reverseClutchValue = (((dta[3] & 0x03) * 256 + dta[2]));
  //Serial.print("reverseClutchValue = "); Serial.println(reverseClutchValue);

  //int reversedat3 = dta[3] & 0x03;
  //Serial.print("reversedat3 = "); Serial.println(reversedat3);

  //int reversedat2 = dta[2];
  //Serial.print("reversedat2 = "); Serial.println(reversedat2);
  //
  //
  //int reverseClutchPercentage = (((float)reversedat3 * 256 + (float)reversedat2) / 1024 * 100);
  //Serial.print("reverseClutchPercentage = "); Serial.println(reverseClutchPercentage);
  //Serial.println();

  myCan.sendMsgBuf(0x117, 0, 8, dta);
  Serial.print("Sending: ");
  Serial.print(0x0117, HEX);
  Serial.print(" ");
  Serial.print("00 00 08 ");
  for (int i = 0; i <8; i++) {
    Serial.print(dta[i], HEX); Serial.print(" ");
  }
  Serial.println("");

  if (clutchDirection == 0) {
    clutchValue++;
  }
  else {
    clutchValue--;
  }

  if (clutchValue >= 19319) {
    clutchValue = 19318;
    clutchDirection = 1;
  }
  if (clutchValue <= 18543) {
    clutchValue = 18544;
    clutchDirection = 0;
  }
  
  
  // Delay loop
  // It should take around 222uS to send a packet, the Nano can not run fast enough
  while (millis() - timer < 5) {}

  // report loop time - if its more than 4000uS then the nano took longer to send the data
  unsigned long loopTime = micros() - timer;
  dta[2] = loopTime >> 0;
  dta[1] = loopTime >> 8;
  dta[0] = loopTime >> 16;
  if (dtaCounter % 100 == 0) digitalWrite(23, !digitalRead(23));
}
