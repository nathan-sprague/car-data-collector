/*
  Manages how data is upload to the user's computer.

  Depending on the settings, it can:
  Creates an access point
  sets up website

*/


#if USEWIFI



// variables found in settings


int numIdsToSave = 0;
int *idsToSave;





#if USE_CAPTIVE_PORTAL
class CaptiveRequestHandler : public AsyncWebHandler {
  public:
    CaptiveRequestHandler() {}
    virtual ~CaptiveRequestHandler() {}

    bool canHandle(AsyncWebServerRequest *request) {
      //request->addInterestingHeader("ANY");
      return true;
    }

    void handleRequest(AsyncWebServerRequest *request) {

      String requestName = request->url().c_str();

      if (requestName == "/hotspot-detect.html" || requestName == "/redirect") { //
        request->send(200, "text/html", "<a href=\"../html-link.htm\" target=\"popup\" onclick=\"window.open('../html-link.htm','name','width=600,height=400')\">Open page in new window</a><br><button onclick=\"window.location.href='/hello?'\">go somewhere</button><br><br>Hello, world (blank)");
      } else if (requestName == "/hello") {
        request->send(200, "text/plain", "Hello, world");


      } else if (requestName == "/_streamFile") {
        Serial.println("streaming file: " + request->getParam("filename")->value());
        File file = SPIFFS.open("/" + request->getParam("filename")->value() + ".txt");
        if (!file || file.isDirectory()) {
          request->send(200, "text/plain", "");
        } else {

          //      request->send(200, "text/plain", "");
          request->send(SPIFFS, "/" + request->getParam("filename")->value() + ".txt", "text/plain");
        }
        file.close();
      } else {
        request->send(200, "text/html", "<p>Page not found. Try going to <url>esp32.local</url> in a browser to see data</p>");
      }
    }
};

#else


//
//void setupPages() {
//
//  server.on("/", [](AsyncWebServerRequest * request) {
//    request->send(200, "<!DOCTYPE html>\n<script>\n\n\n  function setup(){\n\n }\n var fileNames = [];\n var settings = [1000,1,\"ssid\",\"pwd\",0]//[0,1,2,3,4,5,6,7,8,9,10,11,12,13]; \n var settingChangeWarning = 0;\n\n\n\n function getSettings(){\n   console.log(\"getting settings\");\n    var xhttp = new XMLHttpRequest();\n   xhttp.onreadystatechange = function(){  \n      if(xhttp.status==200 && xhttp.readyState==4){ \n          settings = stringListConvert(xhttp.responseText);\n         console.log(\"settings: \", settings);\n          console.log(xhttp.responseText);\n          updateSettings();\n         functionToRun+=1;\n         setupFunctions[functionToRun]();\n        \n      } };\n    var date = new Date();  \n    xhttp.open('GET', '/_info', true);   \n   xhttp.send();\n\n }\n\n function saveSettings(){\n    console.log(\"saving settings\");\n   var xhttp = new XMLHttpRequest();\n   xhttp.onreadystatechange = function(){  \n      if(xhttp.status==200 && xhttp.readyState==4){ \n          settings = stringListConvert(xhttp.responseText);\n         console.log(\"settings: \", settings);\n          console.log(xhttp.responseText);\n          updateSettings();\n         functionToRun+=1;\n         setupFunctions[functionToRun]();\n        \n      } };\n\n    xhttp.open('GET', '/_saveSettings', true);   \n   xhttp.send();\n\n }\n\n\n \n</script>\n\n<html>\n<title>CAN BUS LOGGER</title>\n\n<style>\n\n* {\n  box-sizing: border-box;\n}\n\n/* Create two equal columns that floats next to each other */\n.column {\n  float: left;\n  width: 200px;\n /* padding: 10px;\n  /*height: 300px; /* Should be removed. Only for demonstration */\n}\n\n/* Clear floats after the columns */\n.row:after {\n  content: \"\";\n  display: table;\n  clear: both;\n}\n</style>\n</head>\n<body>\n<p>\nCAN DATA <span id=\"recording\"></span> RECORDING<br><br>\n<button onclick=\"window.location.href='/livestream'\" id=\"liveStream\">Live Data Stream</button><br><br>\n<button onclick=\"window.location.href='/download?file=search'\" id=\"download\">Download Data</button><br><br>\n<a href=\"../html-link.htm\" target=\"popup\" onclick=\"window.open('../html-link.htm','name','width=600,height=400')\">Open page in new window</a>\n<button onclick=\"\" id=\"saveSettings\">Save Settings</button><br>\n<br>\n<br>\n</p>\n\nLog Unknown Message IDs: <input type=\"checkbox\" id=\"logUnknown\" checked>  <br>\n\n<div class=\"row\">\n  <div class=\"column\">\n    <h2>CAN IDs to Log</h2>\n    <select id=\"canToLog\" size=2 style=\"width:80%;\"></select>\n   \n\n  </div>\n  <div class=\"column\">\n    <p style=\"padding-top:100px; padding-left:20%;\">\n    <button>>>>>>></button><br>\n    <button><<<<<<</button><br>\n</p>\n\n  </div>\n  <div class=\"column\">\n    <h2>CAN IDs NOT Logged</h2>\n    <select id=\"canNotLog\" size=2 style=\"width:80%;\"></select>\n  </div>\n</div>\n\n</body>\n</html>\n\n\n</body>\n</html> \n"
//    closeFiles();
//  } );
//
//  server.on("/downloadData", [](AsyncWebServerRequest * request) {
//    request->send(200, "text/html", "<!DOCTYPE html>\n<script>\n\nvar canIds = [];\nvar canData = [];\nvar gpsData = [];\nvar timeStamp = 0;\n\n\nfunction decompressGps(compressed){\n if (compressed.length>3){\n   timeStamp = compressed[0] + compressed[1]*256 + compressed[2]*65536 + compressed[3]*16777216;\n } \n  else{\n   timeStamp = 100000;\n }\n var i = 4;\n  console.log(\"too lazy to decompress gps\");\n\n}\n\nfunction decompressIds(compressed){\n  console.log(\"decompressing ids\");\n var i = 0;\n  canIds = [];\n  while (i<compressed.length-3){\n    canIds.push(compressed[i] + compressed[i+1]*256 + compressed[i+2]*65536 + compressed[i+3]*16777216);\n    i+=1;\n }\n}\n\n\n\nfunction decompressCan(compressed, ids){\n  decompressed = [];\n  pastMsgs = {};\n  lastTime = timeStamp;\n i = 0;\n  console.log(\"ids\", ids)\n console.log(\"compressed\", compressed);\n  // return [];\n\n iterations = 0;\n while (i < compressed.length){\n    decompressedMsg = [];\n\n   if (compressed[i] == 255) { // more than 2.5 seconds\n      lastTime = timeStamp + compressed[i+1] + compressed[i+2]*256 + compressed[i+3]*65536 + compressed[i+4]*16777216;\n      i+=5;\n   } else {\n\n      lastTime += compressed[i]/100;\n      lastTime = Math.round(lastTime*10)/10; // do this so keep 0.1-second precision\n      i+=1;\n   }\n   decompressedMsg.push(lastTime);\n\n\n   idInd = compressed[i];\n    decompressedMsg.push(ids[idInd]); // add id\n   i+=1;\n\n   if (idInd in pastMsgs){\n   } else {\n      pastMsgs[idInd] = [0,0,0,0,0,0,0,0];\n    }\n\n   locations = compressed[i];\n    i+=1;\n\n   data = [];\n    var j=0;\n    while (j<8){\n      if (locations%2==1){ // unique \n       pastMsgs[idInd][j] = compressed[i];\n       i+=1;\n     }\n\n     data.push(pastMsgs[idInd][j])\n     locations = locations / 2;\n    }\n\n\n   console.log(decompressedMsg);\n   Array.prototype.push.apply(decompressed, decompressedMsg);\n\n    \n  }\n return decompressed;\n}\n\n\nfunction listToDictMsgs(msgs){\n res = {\"time\":[], \"id\":[], \"byte 0\": [], \"byte 1\": [], \"byte 2\": [], \"byte 3\": [], \"byte 4\": [], \"byte 5\": [], \"byte 6\": [], \"byte 7\": []};\n k = Object.keys(res);\n i = 0;\n  while (i<msgs.length){\n    res[k[i%10]].push(msgs[i]);\n\n   i+=1;\n }\n return res;\n}\n\n\n\nfunction getNames(){\n  console.log(\"getting names\");\n var xhttp = new XMLHttpRequest();\n xhttp.onreadystatechange = function(){  \n    if(xhttp.status==200 && xhttp.readyState==4) {\n      fileNames = JSON.parse(this.responseText);\n\n      fileDesc = {};\n      var i = 0;\n\n      while (i<fileNames.length) {\n        var runNum = fileNames[i].match(/\\d/g);\n        runNum = runNum.join(\"\");\n       i+=1;\n       if (i<fileNames.length){\n          if (runNum in fileDesc){\n            fileDesc[runNum] += fileNames[i];\n         } else {\n            fileDesc[runNum] = fileNames[i];\n          }\n         i+=1;\n       }\n     }\n     keyys = Object.keys(fileDesc);\n      i=0;\n      while (i<keyys.length){\n       makeDownloads(keyys[i], fileDesc[keyys[i]]);\n      }\n\n\n\n } };\n  xhttp.open('GET', '/_fileNames', true);   \n  xhttp.send();\n\n}\n\n\nfunction downloadGpsData(fileNum){\n  console.log(\"downloading\", filename);\n var xhttp = new XMLHttpRequest();\n xhttp.onreadystatechange = function(){  \n    if(xhttp.status==200 && xhttp.readyState==4){ \n      data = this.response;\n     decompressGps(data);\n      downloadIdData(fileNum);\n    } };\n  var date = new Date();  \n  xhttp.open('GET', '/_download?filename=' + fileNum + '_gps', true);   \n  xhttp.send();\n}\n\nfunction downloadIdData(filename){\n  console.log(\"downloading\", filename);\n var xhttp = new XMLHttpRequest();\n xhttp.onreadystatechange = function(){  \n    if(xhttp.status==200 && xhttp.readyState==4){ \n      data = this.response;\n     decompressIds(data);\n      downloadCanData();\n    } };\n  var date = new Date();  \n  xhttp.open('GET', '/_download?filename=' + fileNum + '_canIDs', true);   \n xhttp.send();\n}\n\nfunction downloadCanData(filename){\n console.log(\"downloading\", filename);\n var xhttp = new XMLHttpRequest();\n xhttp.onreadystatechange = function(){  \n    if(xhttp.status==200 && xhttp.readyState==4){ \n      data = decompressCan(this.response, canIds);\n      sendCsv(filename, data);\n    } };\n  var date = new Date();  \n  xhttp.open('GET', '/_download?filename=' + fileNum + '_canMsgs', true);  \n xhttp.send();\n}\n\n\n\n\n\nfunction deleteData(filename){\n  console.log(\"downloading\", filename);\n var xhttp = new XMLHttpRequest();\n xhttp.onreadystatechange = function(){  \n    if(xhttp.status==200 && xhttp.readyState==4){ \n      alert(\"file deleted\");\n    } };\n  var date = new Date();  \n  xhttp.open('GET', '/_delete?filename'+filename, true);   \n xhttp.send();\n}\n\n\nfunction sendCsv(csvName, fileData) {\n console.log(\"Sending csv\");\n keyys = Object.keys(fileData);\n  console.log(\"names: \" + keyys);\n var csvData = [];\n i=0;\n  while (i<keyys.length){\n   csvData.push(fileData[keyys[i]]);\n   i+=1;\n }\n csvData = csvData[0].map((_, colIndex) => csvData.map(row => row[colIndex]));\n csvData.splice(0, 0, keyys);\n\n  let csvContent = \"data:text/csv;charset=utf-8,\" + csvData.map(e => e.join(\",\")).join(\"\\n\");\n  var encodedUri = encodeURI(csvContent);\n var link = document.createElement('a');\n link.setAttribute('href', encodedUri);    \n  link.setAttribute('download', csvName + '.csv');    \n  link.click();\n}\n\n\nfunction makeDownloads(filename, filesize){\n let info = document.createElement(\"span\");\n  // name = convertToTime(filename)\n info.innerHTML = \"<br><br>Run Number \" + filename + \": ~\" + filesize/1000 + \" kb<br>\";\n  document.body.appendChild(info);\n  let btn = document.createElement(\"button\");\n btn.innerHTML = \"download\";\n btn.type = \"button\";\n  btn.onclick = function () {\n   downloadGpsData(filename);\n\n  };\n  document.body.appendChild(btn);\n\n let btn2 = document.createElement(\"button\");\n  btn2.innerHTML = \"delete\";\n\n  btn2.type = \"button\";\n btn2.onclick = function () {\n\n\n  };\n  document.body.appendChild(btn2);\n}\n\n\ndocumentsAvailable = {};\n\n\n\n\n// function convertToTime(timestamp){\n\n//  var date = new Date(timestamp * 1000);\n//  var formattedTime = date.getDate() + \"/\" + (date.getMonth()+1) + \"/\" + date.getFullYear() + \" \" + date.getHours() +  \":\" + date.getMinutes() + \":\" + date.getSeconds()\n//  return formattedTime;\n// }\n\n \n</script>\n\n<html>\n<title>Download CAN Data</title>\n\n<style>\n\n</style>\n</head>\n<body>\n<h1>Download Data</h1>\n<input type=\"button\" value=\"Back\" onclick=\"history.back()\">\n</body>\n</html>\n\n\n</body>\n</html> \n<script>\ngetNames();\n\n</script> \n\n\n\n"
//  } );
//
//
//  server.on("/_streamFile", [](AsyncWebServerRequest * request) {
//    if request->hasParam("filename") {
//      Serial.println("streaming file: " + request->getParam("filename")->value());
//      File file = MAIN_FS.open("/" + request->getParam("filename")->value() + ".txt");
//      if (!file || file.isDirectory()) {
//        request->send(200, "text/plain", "");
//      } else {
//        request->send(MAIN_FS, "/" + request->getParam("filename")->value(), "text/plain");
//      }
//      file.close();
//    } else {
//      Serial.println("no filename given");
//      request->send(200, "text/plain", "");
//    }
//  } );
//
//  server.on("/_sendFilenames", [](AsyncWebServerRequest * request) {
//    request->send(200, "text/plain", listDir);
//  } );
//
//  server.on("/_delete", [](AsyncWebServerRequest * request) {
//    if request->hasParam("filename") {
//
//    } else {
//      Serial.println("no filename given");
//      request->send(200, "text/plain", "");
//    }
//  } );
//
//  server.begin();
//
//}


#endif




void uploadData() {

}

#endif
