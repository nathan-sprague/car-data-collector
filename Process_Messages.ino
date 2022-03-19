/*
  go through the queue and attempt to compress the data. This is done on the second core

  Depending on the user's settings, it can:
  - filter for certain messages (work in progress)

*/

#include <ESP32CAN.h>
#include <CAN_config.h>


#include <TinyGPSPlus.h>
#include "TimeLib.h"


// GPS-related variables
TinyGPSPlus gps;
// A sample NMEA stream.
const char *gpsStream =
  "$GPRMC,045103.000,A,3014.1984,N,09749.2872,W,0.67,161.46,030913,,,A*7C\r\n"
  "$GPGGA,045104.000,3014.1985,N,09749.2873,W,1,09,1.2,211.6,M,-22.5,M,,0000*62\r\n"
  "$GPRMC,045200.000,A,3014.3820,N,09748.9514,W,36.88,65.02,030913,,,A*77\r\n"
  "$GPGGA,045201.000,3014.3864,N,09748.9411,W,1,10,1.2,200.8,M,-22.5,M,,0000*6C\r\n"
  "$GPRMC,045251.000,A,3014.4275,N,09749.0626,W,0.51,217.94,030913,,,A*7D\r\n"
  "$GPGGA,045252.000,3014.4273,N,09749.0628,W,1,09,1.3,206.9,M,-22.5,M,,0000*6F\r\n";
bool definedTime = false;
uint32_t lastLong = 0;
uint32_t lastLat = 0;
uint32_t lastReadTimeGps = 0; // tenths of a second after last read time (1/10ths of a second or millis()/100)



// CAN-related variables
CAN_device_t CAN_cfg;

// store repeated messages
uint16_t numKnownIds = 0;
uint16_t knownIds[256];
uint8_t lastMsgs[256][8];


// time-related variables
// read time for CAN
uint32_t lastReadTime = 0; // tenths of a second after last read time (1/10ths of a second or millis()/10)



bool setupCan() {
  /*
    Configures the CAN Bus
  */

  // initialize CAN-related parameters
  CAN_cfg.tx_pin_id = GPIO_NUM_5;
  CAN_cfg.rx_pin_id = GPIO_NUM_4;

  // Set up CAN bit rate
  CAN_cfg.speed = CAN_SPEED_500KBPS;

  CAN_cfg.rx_queue = xQueueCreate(30, sizeof(CAN_frame_t));

  //initialize CAN Module
  ESP32Can.CANInit();

  Serial.println("CAN initialized");
  return true;
}




void readCanLive() {
  CAN_frame_t rx_frame;

  //receive next CAN frame from queue
  if (xQueueReceive(CAN_cfg.rx_queue, &rx_frame, 3 * portTICK_PERIOD_MS) == pdTRUE) {


    //    if (rx_frame.FIR.B.FF == CAN_frame_std){ // standard frame (usually enters here)
    //
    //    } else {
    //
    //    }

    if (rx_frame.FIR.B.RTR == CAN_RTR) { // doesn't usually enter here
      //      Serial.println("entered an unusual spot");
      //      printf(" RTR from 0x%08x, DLC %d\r\n", rx_frame.MsgID,  rx_frame.FIR.B.DLC);

    } else {
    }
    long msgId = rx_frame.MsgID;

    liveData += String(msgId) + ",";
    for (int i = 0; i < 8; i++) {
      liveData += String(uint8_t(rx_frame.data.u8[i])) + ",";
    }

  }
}


void readCanMessages() {
  /*
    Check the queue continuously to see the messages available. If there enough, set a flag to compress and save
  */
  CAN_frame_t rx_frame;

  //receive next CAN frame from queue
  if (xQueueReceive(CAN_cfg.rx_queue, &rx_frame, 3 * portTICK_PERIOD_MS) == pdTRUE) {
    Serial.println("got message");

    //    if (rx_frame.FIR.B.FF == CAN_frame_std){ // standard frame (usually enters here)
    //
    //    } else {
    //
    //    }

    if (rx_frame.FIR.B.RTR == CAN_RTR) { // doesn't usually enter here
      //      Serial.println("entered an unusual spot");
      //      printf(" RTR from 0x%08x, DLC %d\r\n", rx_frame.MsgID,  rx_frame.FIR.B.DLC);

    } else { // usually enters here
      //      printf(" from 0x%08x, DLC %d\n", rx_frame.MsgID,  rx_frame.FIR.B.DLC);
    }
    // go through all the known ids
    // not particularly optimized, please fix!
    int idNum = 0;
    while (idNum < numKnownIds) {
      if (knownIds[idNum++] ==  rx_frame.MsgID) {
        idNum--;
        break;
      }
    }

    if (idNum == numKnownIds) { // the id is not known. Add it to the array of known ids
      idNum = numKnownIds;
      knownIds[numKnownIds] = rx_frame.MsgID;
      numKnownIds++;

      // Write the new id to the SD card

      // convert the long into 4 bytes. CHECK IF THIS IS ACTUALLY NECESSARY
      uint8_t newId[4];
      newId[0] = (rx_frame.MsgID);
      newId[1] = (rx_frame.MsgID / 256) % 256;
      newId[2] = (rx_frame.MsgID / 65536) % 256;
      newId[3] = (rx_frame.MsgID / 16777216) % 256;

#if USE_SD
      saveData(0, newId, 4);
#endif
    }


    // COMPRESS
    /*
      This compresses the data and calls the FS_Manager's function to save the data.
      This compression will let more be saved to the SD card and it won't be as bottlenecked

      compress the message
      check if the id is known
      if unknown, add to list of ids
      compress with the following format:

      timestamp
      1/10s of a second since last reading
      #255 is reserved, for > 2.54 seconds.
      following byte is second.
      if following byte is #255, following 4 bytes is a unix timestamp

      arbitration id
      #255 is reserved for repeating messages.
      If 255, the following byte would be the arbitration id index
      #254 is reserved for situations where there are more than 253 different kinds of arbitration ids
      if 254, the arbitration id would be 254 + following byte
      if there are more than 508 types of ids, repeat this

      location of non-repeated values.

      ex:
      assuming the last message from said id was 00 00 00 00 00 00 00 00:

      00 00 00 00 00 00 00 00 would be 0000000 in binary, or 0
      00 00 00 00 00 00 00 ff would be 0000001 in binary, or 1
      ff 00 00 00 00 00 00 00 would be 1000000 in binary, or 128
      ff ff ff ff ff ff ff ff would be 1111111 in binary, or 255

      values
      they are already byte form
    */

    uint8_t bufferSize = 0;
    uint8_t canMsg[15];

    // write timestamp


    uint32_t msgTimestamp =  millis() / 100 - lastReadTime;
    lastReadTime = millis() / 100;

    if (msgTimestamp > 255) { // writing the entire unix timestamp
      // write 255, followed by the unix timestamp
      canMsg[bufferSize++] = 255;

      // convert uint32 to 4 bytes
      canMsg[bufferSize++] = (msgTimestamp);
      canMsg[bufferSize++] = (msgTimestamp / 256) % 256;
      canMsg[bufferSize++] = (msgTimestamp / 65536) % 256;
      canMsg[bufferSize++] = (msgTimestamp / 16777216) % 256;
      Serial.println("writing full timestamp: " + String(msgTimestamp));

    } else { // just write the time since last reading
      // just write time change
      canMsg[bufferSize++] = msgTimestamp;
    }



    // write the id index
    canMsg[bufferSize++] = idNum;


    // skip a number to write number of different values later
    int dataLocationIndicatorIndex = bufferSize;
    bufferSize++;

    // location of repeated messages
    uint8_t dataLocationIndicator = 0;

    // write the different values
    for (int i = 0; i < 8; i++) {
      if (lastMsgs[idNum][i] != rx_frame.data.u8[i]) { // different from what was written the last time
        // write the different value
        canMsg[bufferSize++] = rx_frame.data.u8[i];

        // write location of different value
        dataLocationIndicator += pow(2, i);

        // update new value to last messages
        lastMsgs[idNum][i] = rx_frame.data.u8[i];
      }
    }
    // now write the number of different values
    canMsg[dataLocationIndicatorIndex] = dataLocationIndicator;





#if USE_SD
    // save the CAN data to the SD card
    saveData(1, canMsg, bufferSize);
#else
    unsigned int blah = rx_frame.MsgID;
    Serial.print("real message: ");
    Serial.print(blah);
    printf(" from 0x%08x, DLC %d\n", rx_frame.MsgID,  rx_frame.FIR.B.DLC);
    for (int i = 0; i < 8; i++) {
      uint8_t b = rx_frame.data.u8[i];
      Serial.print(" " + String(b));
    }
    Serial.println("");
#endif

  }
}



void readGpsMessages() {
  if (*gpsStream) {
    if (gps.encode(*gpsStream++)) {


      // get unix timestamp
      setTime(gps.time.hour(), gps.time.minute(), gps.time.second(), gps.date.day(), gps.date.month(), gps.date.year());

      if (definedTime) {
        uint8_t dataToSend[8];
        unsigned long accurateTime = now();

        dataToSend[0] = (accurateTime);
        dataToSend[1] = (accurateTime / 256) % 256;
        dataToSend[2] = (accurateTime / 65536) % 256;
        dataToSend[3] = (accurateTime / 16777216) % 256;

        dataToSend[4] = (millis() / 1000);
        dataToSend[5] = (millis() / 1000 / 256) % 256;
        dataToSend[6] = (millis() / 1000 / 65536) % 256;
        dataToSend[7] = (millis() / 1000 / 16777216) % 256;

        saveData(2, dataToSend, 8);
      }

      int bufferSize = 0;
      uint8_t gpsMsg[20];

      uint32_t msgTimestamp =  millis() / 1000 - lastReadTimeGps;
      lastReadTimeGps = millis() / 1000;

      if (msgTimestamp > 255) { // writing the entire unix timestamp
        // write 255, followed by the unix timestamp
        gpsMsg[bufferSize++] = 255;

        // convert uint32 to 4 bytes
        gpsMsg[bufferSize++] = (msgTimestamp);
        gpsMsg[bufferSize++] = (msgTimestamp / 256) % 256;
        gpsMsg[bufferSize++] = (msgTimestamp / 65536) % 256;
        gpsMsg[bufferSize++] = (msgTimestamp / 16777216) % 256;
        Serial.println("writing full timestamp: " + String(msgTimestamp));

      } else { // just write the time since last reading
        // just write time change
        gpsMsg[bufferSize++] = msgTimestamp;
      }
      return;


      uint32_t lattitude = gps.location.lat() * 1000000 + 2147483647; // turn a signed float into an unsigned long
      uint32_t longitude = gps.location.lng() * 1000000 + 2147483647;

      uint32_t latDif = lattitude - lastLat;
      uint32_t longDif = longitude - lastLong;

      int infoIndex = bufferSize++; // increment buffer size to put in message length later
      int i = 0;
      while (latDif > 0) {
        gpsMsg[bufferSize++] = latDif % 256;
        latDif /= 256;
        i += 1;
      }
      gpsMsg[infoIndex] = i;

      i = 0;
      infoIndex = bufferSize++; // increment buffer size to put in message length later
      while (longDif > 0) {
        gpsMsg[bufferSize++] = longDif % 256;
        longDif /= 256;
        i += 1;
      }
      gpsMsg[infoIndex] = i;


      saveData(2, gpsMsg, bufferSize);


    }
  }

}
