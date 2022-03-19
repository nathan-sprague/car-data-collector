// use wifi (takes longer to compile)
#define USE_WIFI 1
#define USE_SD 0

#include "SPIFFS.h"
#include "SD_MMC.h"            // SD Card ESP32
#define MAIN_FS SPIFFS

#define USE_CAPTIVE_PORTAL 0

uint32_t lastGpsReadTime = 0;

bool liveStream = false;
String liveData = "[ ";

// Tx: pin 5
// Rx: Pin 4


// setting variables
String apName = "CAN LOGGER";
String apPwd = "";
uint16_t bitRate = 500;
uint16_t gpsReadRate = 1;


#include <DNSServer.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>
#include <WebServer.h>
#include <HTTPClient.h>

#include <ESPmDNS.h> // not needed for ap

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>



DNSServer dnsServer;
AsyncWebServer server(80);

void setupPages() {

  server.on("/", [](AsyncWebServerRequest * request) {
    //    listSpiffsDir();
    Serial.println("closed files");
    request->send(200, "text/html", "<!DOCTYPE html>\n<script>\n\n\n function setup(){\n\n }\n var fileNames = [];\n var settings = [1000,1,\"ssid\",\"pwd\",0]//[0,1,2,3,4,5,6,7,8,9,10,11,12,13]; \n var settingChangeWarning = 0;\n\n\n\n function getSettings(){\n   console.log(\"getting settings\");\n    var xhttp = new XMLHttpRequest();\n   xhttp.onreadystatechange = function(){  \n      if(xhttp.status==200 && xhttp.readyState==4){ \n          settings = stringListConvert(xhttp.responseText);\n         console.log(\"settings: \", settings);\n          console.log(xhttp.responseText);\n          updateSettings();\n         functionToRun+=1;\n         setupFunctions[functionToRun]();\n        \n      } };\n    var date = new Date();  \n    xhttp.open('GET', '/_info', true);   \n   xhttp.send();\n\n }\n\n function saveSettings(){\n    console.log(\"saving settings\");\n   var xhttp = new XMLHttpRequest();\n   xhttp.onreadystatechange = function(){  \n      if(xhttp.status==200 && xhttp.readyState==4){ \n          settings = stringListConvert(xhttp.responseText);\n         console.log(\"settings: \", settings);\n          console.log(xhttp.responseText);\n          updateSettings();\n         functionToRun+=1;\n         setupFunctions[functionToRun]();\n        \n      } };\n\n    xhttp.open('GET', '/_saveSettings', true);   \n   xhttp.send();\n\n }\n\n\n \n</script>\n\n<html>\n<title>CAN BUS LOGGER</title>\n\n<style>\n\n* {\n  box-sizing: border-box;\n}\n\n/* Create two equal columns that floats next to each other */\n.column {\n  float: left;\n  width: 200px;\n /* padding: 10px;\n  /*height: 300px; /* Should be removed. Only for demonstration */\n}\n\n/* Clear floats after the columns */\n.row:after {\n  content: \"\";\n  display: table;\n  clear: both;\n}\n</style>\n</head>\n<body>\n<p>\nCAN DATA <span id=\"recording\"></span> RECORDING<br><br>\n<button onclick=\"window.location.href='/livestream'\" id=\"liveStream\">Live Data Stream</button><br><br>\n<button onclick=\"window.location.href='/downloadData'\" id=\"download\">Download Data</button><br><br>\n<a href=\"../html-link.htm\" target=\"popup\" onclick=\"window.open('../html-link.htm','name','width=600,height=400')\">Open page in new window</a>\n<button onclick=\"\" id=\"saveSettings\">Save Settings</button><br>\n<br>\n<br>\n</p>\n\nLog Unknown Message IDs: <input type=\"checkbox\" id=\"logUnknown\" checked>  <br>\n\n<div class=\"row\">\n  <div class=\"column\">\n    <h2>CAN IDs to Log</h2>\n    <select id=\"canToLog\" size=2 style=\"width:80%;\"></select>\n   \n\n  </div>\n  <div class=\"column\">\n    <p style=\"padding-top:100px; padding-left:20%;\">\n    <button>>>>>>></button><br>\n    <button><<<<<<</button><br>\n</p>\n\n  </div>\n  <div class=\"column\">\n    <h2>CAN IDs NOT Logged</h2>\n    <select id=\"canNotLog\" size=2 style=\"width:80%;\"></select>\n  </div>\n</div>\n\n</body>\n</html>\n\n\n</body>\n</html> \n"
                 );
  } );

  server.on("/downloadData", [](AsyncWebServerRequest * request) {
    Serial.println("download data");
    
    request->send(200, "text/html", "<!DOCTYPE html>\n<script>\n\nvar canIds = [];\nvar canData = [];\nvar gpsData = [];\nvar timeStamp = 0;\n\n\nfunction decompressGps(compressed){\n console.log(compressed);  \n\n  if (compressed.length>3){\n   timeStamp = compressed[0] + compressed[1]*256 + compressed[2]*65536 + compressed[3]*16777216;\n } \n  else{\n   timeStamp = 100000;\n }\n var i = 4;\n  console.log(\"too lazy to decompress gps\");\n\n}\n\nfunction decompressIds(compressed){\n  console.log(\"decompressing ids\");\n var i = 0;\n  canIds = [];\n  while (i<compressed.length-3){\n    canIds.push(compressed[i] + compressed[i+1]*256 + compressed[i+2]*65536 + compressed[i+3]*16777216);\n    i+=4;\n }\n}\n\n\n\nfunction decompressCan(compressed, ids){\n  console.log(\"decompressing can\");\n decompressed = [];\n  pastMsgs = {};\n  lastTime = timeStamp;\n i = 0;\n  console.log(\"ids\", ids)\n console.log(\"compressed\", compressed);\n  // return [];\n\n iterations = 0;\n while (i < compressed.length && iterations < 1000){\n   iterations += 1;\n    decompressedMsg = [];\n\n   if (compressed[i] == 255) { // more than 2.5 seconds\n      console.log(\"more than 2 spots\", compressed[i+1], compressed[i+2], compressed[i+3], compressed[i+4]);\n     lastTime = timeStamp + compressed[i+1] + compressed[i+2]*256 + compressed[i+3]*65536 + compressed[i+4]*16777216;\n      i+=5;\n   } else {\n\n      lastTime += compressed[i]/100;\n      lastTime = Math.round(lastTime*10)/10; // do this so keep 0.1-second precision\n      i+=1;\n   }\n   console.log(\"time stamp:\", lastTime);\n   decompressedMsg.push(lastTime);\n\n\n\n   idInd = compressed[i];\n    console.log(\"id index\", idInd);\n   decompressedMsg.push(ids[idInd]); // add id\n   console.log(\"id:\", ids[idInd]);\n   i+=1;\n\n   if (idInd in pastMsgs){\n   } else {\n      pastMsgs[idInd] = [0,0,0,0,0,0,0,0];\n    }\n\n   locations = compressed[i];\n    i+=1;\n   console.log(\"locations\", locations);\n    l = 2;\n    data = [];\n    var j=0;\n    while (j<8){\n      if (locations%l == 1) { // unique \n        pastMsgs[idInd][j] = compressed[i];\n       i+=1;\n       console.log(\"unique value\", compressed[i], \"index\", j);\n       locations-=l;\n     }\n\n     l = l * 2;\n      // console.log(\"locations\", locations, \"l\", l);\n     j+=1;\n   }\n   console.log(\"data\", pastMsgs[idInd]);\n   Array.prototype.push.apply(decompressedMsg, pastMsgs[idInd]);\n\n\n   console.log(\"msg:\", decompressedMsg);\n   Array.prototype.push.apply(decompressed, decompressedMsg);\n\n    \n  }\n console.log(\"decomp\", decompressed);\n  return decompressed;\n}\n\n// c = [255, 17, 11, 0, 0, 0, 255, 1, 2, 3, 4, 5, 6, 7, 8, 255, 217, 5, 0, 0, 1, 255, 1, 2, 3, 4, 5, 6, 7, 8, 255, 153, 1, 0, 0, 1, 0, 136, 1, 0, 94, 1, 0, 65, 1, 0, 33, 1, 0, 45, 1, 0, 13, 1, 0, 17, 1, 0, 20, 1, 0];\n// ids = [32, 536870912, 52428800, 204800, 800];\n\n// data = listToDictMsgs(decompressCan(c, ids));\n// sendCsv(\"Can_Data_run_\", data);\n\nfunction listToDictMsgs(msgs){\n res = {\"time\":[], \"id\":[], \"byte 0\": [], \"byte 1\": [], \"byte 2\": [], \"byte 3\": [], \"byte 4\": [], \"byte 5\": [], \"byte 6\": [], \"byte 7\": []};\n k = Object.keys(res);\n i = 0;\n  while (i<msgs.length){\n    res[k[i%10]].push(msgs[i]);\n\n   i+=1;\n }\n return res;\n}\n\n\n\nfunction getNames(){\n  console.log(\"getting names\");\n var xhttp = new XMLHttpRequest();\n xhttp.onreadystatechange = function(){  \n    if(xhttp.status==200 && xhttp.readyState==4) {\n      fileNames = JSON.parse(this.responseText);\n      console.log(fileNames);\n     fileDesc = {};\n      var i = 0;\n\n      while (i<fileNames.length) {\n        var runNum = fileNames[i].match(/\\d/g);\n        runNum = runNum.join(\"\");\n\n       i+=1;\n       if (i<fileNames.length){\n          if (runNum in fileDesc){\n            fileDesc[runNum] += fileNames[i];\n         } else {\n            fileDesc[runNum] = fileNames[i];\n          }\n         i+=1;\n       }\n     }\n     console.log(fileDesc);\n      keyys = Object.keys(fileDesc);\n      i=0;\n      while (i<keyys.length){\n       makeDownloads(keyys[i], fileDesc[keyys[i]]);\n        console.log(\"made download for\", keyys[i]);\n       i+=1;\n     }\n\n\n\n } };\n  xhttp.open('GET', '/_fileNames', true);   \n  xhttp.send();\n\n}\n\n\nfunction downloadGpsData(fileNum){\n  console.log(\"downloading\", fileNum);\n  var xhttp = new XMLHttpRequest();\n xhttp.onreadystatechange = function(){  \n    if(xhttp.status==200 && xhttp.readyState==4){ \n      data = new Uint8Array(this.response);\n     console.log(\"gps data\", data);\n      decompressGps(data);\n      downloadIdData(fileNum);\n    } };\n  var date = new Date();  \n  xhttp.open('GET', '/_streamFile?filename=' + fileNum + '_gps', true);   \n  xhttp.responseType = \"arraybuffer\";\n xhttp.send();\n}\n\nfunction downloadIdData(fileNum){\n console.log(\"downloading\", fileNum);\n  var xhttp = new XMLHttpRequest();\n xhttp.onreadystatechange = function(){  \n    if(xhttp.status==200 && xhttp.readyState==4){ \n      data = new Uint8Array(this.response);\n     console.log(\"id data\", data);\n     decompressIds(data);\n      downloadCanData(fileNum);\n    } };\n var date = new Date();  \n  xhttp.open('GET', '/_streamFile?filename=' + fileNum + '_canIDs', true);   \n xhttp.responseType = \"arraybuffer\";\n xhttp.send();\n}\n\nfunction downloadCanData(fileNum){\n  console.log(\"downloading\", fileNum);\n  var xhttp = new XMLHttpRequest();\n xhttp.onreadystatechange = function(){  \n    if(xhttp.status==200 && xhttp.readyState==4){ \n      candata = new Uint8Array(this.response);\n      data = decompressCan(candata, canIds);\n\n      console.log(\"can data\", data);\n      data = listToDictMsgs(data);\n      sendCsv(\"Can_Data_run_\" + fileNum, data);\n    } };\n var date = new Date();  \n  xhttp.open('GET', '/_streamFile?filename=' + fileNum + '_canMsgs', true);\n xhttp.responseType = \"arraybuffer\";  \n xhttp.send();\n}\n\n\n\n\nfunction deleteData(filename){\n  console.log(\"downloading\", filename);\n var xhttp = new XMLHttpRequest();\n xhttp.onreadystatechange = function(){  \n    if(xhttp.status==200 && xhttp.readyState==4){ \n      alert(\"file deleted\");\n    } };\n  var date = new Date();  \n  xhttp.open('GET', '/_delete?filename'+filename, true);   \n xhttp.send();\n}\n\n\nfunction sendCsv(csvName, fileData) {\n console.log(\"Sending csv\");\n keyys = Object.keys(fileData);\n  console.log(\"names: \" + keyys);\n var csvData = [];\n i=0;\n  while (i<keyys.length){\n   csvData.push(fileData[keyys[i]]);\n   i+=1;\n }\n csvData = csvData[0].map((_, colIndex) => csvData.map(row => row[colIndex]));\n csvData.splice(0, 0, keyys);\n\n  let csvContent = \"data:text/csv;charset=utf-8,\" + csvData.map(e => e.join(\",\")).join(\"\\n\");\n  var encodedUri = encodeURI(csvContent);\n var link = document.createElement('a');\n link.setAttribute('href', encodedUri);    \n  link.setAttribute('download', csvName + '.csv');    \n  link.click();\n}\n\n\nfunction makeDownloads(filename, filesize){\n let info = document.createElement(\"span\");\n  // name = convertToTime(filename)\n info.innerHTML = \"<br><br>Run Number \" + filename + \": ~\" + filesize/1000 + \" kb<br>\";\n  document.body.appendChild(info);\n  let btn = document.createElement(\"button\");\n btn.innerHTML = \"download\";\n btn.type = \"button\";\n  btn.onclick = function () {\n   downloadGpsData(filename);\n\n  };\n  document.body.appendChild(btn);\n\n let btn2 = document.createElement(\"button\");\n  btn2.innerHTML = \"delete\";\n\n  btn2.type = \"button\";\n btn2.onclick = function () {\n\n\n  };\n  document.body.appendChild(btn2);\n}\n\n\ndocumentsAvailable = {};\n\n\n\n\n// function convertToTime(timestamp){\n\n//  var date = new Date(timestamp * 1000);\n//  var formattedTime = date.getDate() + \"/\" + (date.getMonth()+1) + \"/\" + date.getFullYear() + \" \" + date.getHours() +  \":\" + date.getMinutes() + \":\" + date.getSeconds()\n//  return formattedTime;\n// }\n\n \n</script>\n\n<html>\n<title>Download CAN Data</title>\n\n<style>\n\n</style>\n</head>\n<body>\n<h1>Download Data</h1>\n<input type=\"button\" value=\"Back\" onclick=\"history.back()\">\n</body>\n</html>\n\n\n</body>\n</html> \n<script>\ngetNames();\n\n</script> \n\n\n\n"
                 );
  } );

 server.on("/livestream", [](AsyncWebServerRequest * request) {
    Serial.println("live stream");
    closeFiles();
    liveStream = true;
    request->send(200, "text/html", "<!DOCTYPE html>\n<script>\n\nmsgElements = [];\ndata = [];\nmaxMsgs = 20;\n\n\nfunction writeMsgs(){\n\n\n useHex = document.getElementById(\"useHex\").checked;\n var i = 0;\n  while (i<msgElements.length){\n   msgElements[i].remove();\n    i+=1;\n }\n msgElements = [];\n\n numMsgs = data.length;\n  if (numMsgs > maxMsgs*9){\n   console.log(\"too many messages\");\n   data = data.slice(0, maxMsgs*9);\n  }\n\n i=0;\n\n  while (i<data.length-8){\n\n    id = data[i];\n\n   i+=1;\n\n   if (useHex){\n      res = id.toString(16)\n\n   } else {\n      res = String(id);\n   }\n   msg = \"<pre>\" + res.padStart(9, '0');\n\n   msg += \"                 \";\n\n   j = 0;\n\n    while (j<8){\n\n      if (useHex){\n        res = data[i].toString(16);\n       res = res.padStart(2, '0');\n       msg += res + \"       \";\n     } else {\n        res = String(data[i]);\n        res = res.padStart(3, '0');\n       msg += res + \"      \";\n      }\n     j+=1\n      i+=1\n    }\n\n   msg += \"</pre>\";\n    console.log(\"msg\", msg);\n\n\n    let info = document.createElement(\"span\");\n    info.innerHTML = msg;\n   console.log(info);\n    document.body.appendChild(info);\n\n    msgElements.push(info);\n\n }\n}\n\n\nfunction getData(){\n console.log(\"getting data\");\n  var xhttp = new XMLHttpRequest();\n xhttp.onreadystatechange = function(){  \n    if(xhttp.status==200 && xhttp.readyState==4){ \n      var newdata = JSON.parse(this.responseText);\n      console.log(newdata);\n     Array.prototype.push.apply(newdata, data);\n      data = newdata;\n     writeMsgs();\n\n      \n    } }; \n xhttp.open('GET', '/_streamData', true);\n  xhttp.send();\n}\n\n  \n</script>\n\n<html lang=\"en\">\n<title>Live CAN Data</title>\n\n<style>\n\n</style>\n\n</head>\n<body>\n<h1>Live Data</h1>\nUse Hex<input type=\"checkbox\" id=\"useHex\" onclick=\"writeMsgs();\" checked><br>\nMax number of messages: <input id=\"maxMsgs\" value = 20 type = \"number\"><button onclick=\"maxMsgs=document.getElementById('maxMsgs').value; writeMsgs();\" id=\"set\">Set</button><br>\n<input type=\"button\" value=\"Back\" onclick=\"history.back()\"><br>\n<pre>   ID                    Data 1   Data 2   Data 3   Data 4   Data 5   Data 6   Data 7   Data 8</pre>\n</body>\n</html>\n\n\n</body>\n</html> \n\n<script>\n\nvar myInterval = setInterval(getData, 1000);\n\n</script>\n"
                 );
  } );

  server.on("/_streamData", [](AsyncWebServerRequest * request) {
    Serial.println("stream data");
    liveData[liveData.length()-1] = ']';
    Serial.println(liveData);
    request->send(200, "text/plain", liveData);
    liveData = "[ ";
  } );


  server.on("/_streamFile", [](AsyncWebServerRequest * request) {
    closeFiles();
    if (request->hasParam("filename")) {
      String streamFilename = "/" + request->getParam("filename")->value();
      Serial.println("streaming file: " + streamFilename);
      File file = MAIN_FS.open(streamFilename);
      if (!file || file.isDirectory()) {
        Serial.println("requested file not in directory");
        request->send(200, "text/plain", "");
      } else {
        
        request->send(MAIN_FS, streamFilename, "text/plain");
      }
      file.close();
    } else {
      Serial.println("no filename given");
      request->send(200, "text/plain", "");
    }
  } );

  server.on("/_fileNames", [](AsyncWebServerRequest * request) {
    closeFiles();
    String filenameString = listDir();
    Serial.println("file names: " + filenameString);
    request->send(200, "text/plain", filenameString);
  } );

  server.on("/_delete", [](AsyncWebServerRequest * request) {
    closeFiles();
    if (request->hasParam("filename")) {
      
      request->send(200, "text/plain", "yes");
    } else {
      Serial.println("no filename given");
      request->send(200, "text/plain", "no");
    }
  } );
  server.begin();
}

void setupAP() {
  Serial.println("\nConfiguring access point...");
  if (apPwd == "") {
    WiFi.softAP(apName.c_str());
  } else {
    WiFi.softAP(apName.c_str(), apPwd.c_str());
  }
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("IP address: " + WiFi.localIP());

  String mdnsName =  "ESP32";
  if (MDNS.begin(mdnsName.c_str())) {
    Serial.println("MDNS responder started as http://" + mdnsName + ".local/");
  }


#if USE_CAPTIVE_PORTAL
  dnsServer.start(53, "*", WiFi.softAPIP());
  server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER); //only when requested from AP
#else
  setupPages();
#endif

}


void setup() {
  Serial.begin(115200);


  // get settings set up
  readSettings();
//  listSpiffsDir();
//  listDir();

#if USE_WIFI

  setupAP();

  // set up access point

  // set up website

  // begin website

#endif


#if USE_SD
  // set up SD Card FS
  if (!setupSD()) {
    Serial.println("critical error setting up sd card");
  }
#endif

  // set up can
  if (!setupCan()) {
    Serial.println("critical error setting up CAN");
  }

}



void loop() {
#if USE_CAPTIVE_PORTAL
  dnsServer.processNextRequest();
#endif

  if (liveStream){
    readCanLive();
  } else {
    readCanMessages();
  }
 


  if (millis() - gpsReadRate * 1000 > lastGpsReadTime && false) {
    //    readGpsMessages();
  }


}



void criticalError(int errorType) {
  /*
    Flash the LED as long as there is an error
  */
  while (true) {
    Serial.println("Critical Error!");
    delay(10);
  }

}
