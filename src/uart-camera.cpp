#include <Arduino.h>
#include <SD.h>
#include <SPI.h>
#include <Adafruit_VC0706.h>
#include "wiring_private.h"

#define chipSelect A5
Uart Serial2 (&sercom1, 11, 10, SERCOM_RX_PAD_0, UART_TX_PAD_2);

void SERCOM1_Handler() {
  Serial2.IrqHandler();
}

Adafruit_VC0706 cam = Adafruit_VC0706(&Serial2);

void setup() {
  while(!Serial){}
  Serial2.begin(38400);
  pinPeripheral(10, PIO_SERCOM);
  pinPeripheral(11, PIO_SERCOM);
  pinMode(chipSelect, OUTPUT);
  cam.setImageSize(VC0706_320x240);        // medium
  //use #reset instead of #begin because we've already begun Serial
  if(cam.reset()) {
    Serial.println("Found camera.");
  } else {
    Serial.println("No Camera");
  }
  Serial.begin(9600);
  if(!SD.begin(chipSelect)) {
    Serial.println("Couldn't find SD");
  } else {
    Serial.println("found SD");
  }
  char *reply = cam.getVersion();
  if (reply == 0) {
    Serial.println("Failed to get version");
  } else {
    Serial.println("-----------------");
    Serial.print(reply);
    Serial.println("-----------------");
  }
}

void loop() {
  if (! cam.takePicture())
    Serial.println("Failed to snap!");
  else
    Serial.println("Picture taken!");
  char filename[13];
  strcpy(filename, "IMAGE00.JPG");
  for (int i = 0; i < 100; i++) {
    filename[5] = '0' + i/10;
    filename[6] = '0' + i%10;
    // create if does not exist, do not open existing, write, sync after write
    if (! SD.exists(filename)) {
      break;
    }
  }

  File imgFile = SD.open(filename, FILE_WRITE);

  uint16_t jpglen = cam.frameLength();
  Serial.print(jpglen, DEC);
  Serial.println(" byte image");

  Serial.print("Writing image to "); Serial.print(filename);

  while (jpglen > 0) {
    // read 32 bytes at a time;
    uint8_t *buffer;
    uint8_t bytesToRead = min(64, jpglen); // change 32 to 64 for a speedup but may not work with all setups!
    buffer = cam.readPicture(bytesToRead);
    imgFile.write(buffer, bytesToRead);

    //Serial.print("Read ");  Serial.print(bytesToRead, DEC); Serial.println(" bytes");

    jpglen -= bytesToRead;
  }
  imgFile.close();
  Serial.println("DONE");
  cam.reset();
  delay(1000);
}
