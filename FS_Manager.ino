#include "FS.h"



// file currently writing to. Keep the file open, but close it before opening the next file.
// 3 kinds of files: 0=can ids, 1=can messages, 2=gps data
int fileNum = -1;
String filenames[3];
File currentFile;



void readSettings() {
  /*
    Set up flash to check settings
  */

  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Mount Failed");
    return;
  } else {
    Serial.println("SPIFF Mount Successful");
  }

  File file = SPIFFS.open("/settings.txt");
  if (!file || file.isDirectory()) {
    Serial.println("No settings found. Using default settings");
    return;
  }

  int settingNum = 0;
  String val = "";
  while (file.available()) {
    char c = file.read();

    if (c == ',' && val.length() > 0 && val[val.length() - 1] == '\\') { // special character
      val[val.length() - 1] = ',';
    } else if (c == ',') {
      if (c == 0) {
        apName = val;
      } else if (c == 1) {
        apPwd = val;
      } else if (c == 2) {
        bitRate = val.toInt();
      }

      c++;
      val = "";
    }

    val += c;

  }


  return;
}

void saveSettings() {
  /*
    Saves the settings to FLASH, not the SD card
    The following settings are saved:
      1. ap name
      2. ap password
      3. Bit rate (not implemented yet)
      4. should whitelist or blacklist messages (0,1) (not implemented yet)
      5. Whitelisted/blacklisted messages (not implemented yet)
  */

  File file = SPIFFS.open("/settings.txt", FILE_WRITE);
  Serial.println("Saving settings file");
  String msg = String(apName) + "," + String(apPwd);
  file.print(msg);
  file.close();
}




bool setupSD() {


  //    if (!MAIN_FS.begin()) {
  //      Serial.println("SD Card Mount Failed");
  //      return false;
  //    }
  //    uint8_t cardType = MAIN_FS.cardType();
  //    if (cardType == CARD_NONE) {
  //      Serial.println("No SD Card attached");
  //      return false;
  //    }
  //    Serial.println("SD Card Mount Successful");
  //  } else {
  //
  //  }
  Serial.println("Spiffs already mounted (hopefully)");


  int prefix = findFilenamePrefix();
  filenames[0] = "/" + String(prefix) + "_canIDs";
  filenames[1] = "/" + String(prefix) + "_canMsgs";
  filenames[2] = "/" + String(prefix) + "_gps";
  Serial.println("writing files with prefix: " + String(prefix));


  return true;

}


void saveData(uint8_t fileToOpen, uint8_t writeData[], uint16_t bufferSize) {
  /*
    Write the data to the SD card. Verify the file is open and write to it
  */
  if (fileNum == -1) {
    currentFile = MAIN_FS.open(filenames[fileToOpen], FILE_APPEND);
  } else if (fileNum != fileToOpen) { // file is different from what is already opened
    currentFile.close();
    currentFile = MAIN_FS.open(filenames[fileToOpen], FILE_APPEND);
  }
  fileNum = fileToOpen;

  Serial.print("writing to : " + filenames[fileToOpen]);
  for (int i = 0; i < bufferSize; i++) {
    Serial.print(writeData[i]);
    Serial.print(", ");
  }
  Serial.println("");

  currentFile.write(writeData, bufferSize);

}

void closeFiles() {
  if (fileNum != -1) {
    currentFile.close();
  }
  fileNum = -1;
}




void deleteFile( const char * path) {
  Serial.printf("Deleting file: %s\n", path);
  if (MAIN_FS.remove(path)) {
    Serial.println("File deleted");
  } else {
    Serial.println("Delete failed");
  }
}


int findFilenamePrefix() {
  int prefix = 0;
  File root = MAIN_FS.open("/");
  if (!root) {
    Serial.println("Failed to open any files. Assuming prefix is 0.");
    return 0;
  }
  if (!root.isDirectory()) {
    Serial.println("Not a directory");
    return 0;
  }

  File file = root.openNextFile();
  while (file) {

    String n = String(file.name());
    Serial.println("checking file: " + n);
    int i = 1;
    String filePrefix = "";
    while (i < n.length() && n[i] != '_') {
      filePrefix += n[i++];
    }
    Serial.println("file prefix: " + filePrefix);
    if (filePrefix.toInt() >= prefix) {

      prefix = filePrefix.toInt() + 1;
    }
    file = root.openNextFile();
  }

  return prefix;
}


void listSpiffsDir() {
  Serial.println("Listing directory:");

  File root = SPIFFS.open("/");
  if (!root) {
    Serial.println("- failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println(" - not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {

    Serial.print("  FILE: ");
    Serial.print(file.name());
    Serial.print("\tSIZE: ");
    Serial.println(file.size());
    file = root.openNextFile();
  }
}




String listDir() {
  String res = "[ ";
  File root = MAIN_FS.open("/");

  if (!root) {
    Serial.println("Failed to open directory");
    return "[]";
  }
  if (!root.isDirectory()) {
    Serial.println("Not a directory");
    return "[]";
  }

  File file = root.openNextFile();
  while (file) {
    String n = String(file.name());
    Serial.println("checking file: " + n);
    int i = 1;
    String filePrefix = "";
    while (i < n.length() && n[i] != '_' && isDigit(n[i])) {
      filePrefix += n[i++];
    }
    if (i < n.length() && n[i] == '_'){
      res += "\"" + String(file.name()) + "\", " + String(file.size()) + ",";
    }
    
    file = root.openNextFile();
  }

  res[res.length() - 1] = ']';
  Serial.println("named files: " + res);
  return res;
}
