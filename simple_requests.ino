// go to http://192.168.4.1 if ap


#define USE_AP true
#include "max6675.h"
 
int thermoDO = 19;
int thermoCS = 23;
int thermoCLK = 5;
 
MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);



#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>
#include <WebServer.h>

#include <ESPmDNS.h> // not needed for ap


// Set these to your desired credentials.
String ssid = "ESP32_AP";
const char *password = "WIFI12345";

WebServer server(80);


void setup() {
  Serial.begin(115200);


  if (USE_AP){
    Serial.println("\nConfiguring access point...");
    // You can remove the password parameter if you want the AP to be open.
    String ssidName = ssid;
    String chipId = String((uint32_t)(ESP.getEfuseMac() >> 24), HEX);
    ssidName += '_';
    ssidName += chipId;
    WiFi.softAP(ssidName.c_str());
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    
    esp_err_t err = mdns_init();
    if (err) {
      printf("MDNS Init failed: %d\n", err);
      return;
    }
    
    //set hostname
    mdns_hostname_set("my-esp32");

    
  } else {
      WiFi.mode(WIFI_STA);
      WiFi.begin("ssid", "pwd");
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
      }
    
      Serial.println("");
      Serial.print("Connected to ");
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());
      if (MDNS.begin("esp32")) {
        Serial.println("MDNS responder started");
      }
      esp_err_t err = mdns_init();
      if (err) {
          printf("MDNS Init failed: %d\n", err);
          return;
      }
  
      //set hostname
      mdns_hostname_set("my-esp32");
      Serial.println("hostname set");
  }


  server.on("/", [](){
      Serial.println("main requested");
      float blah = getReading();
      server.send(200, "text/plain", String(blah)); 
     });

  server.begin();
  Serial.println("Server started"); 
}

float getReading(){
  return thermocouple.readFahrenheit();
}

void loop() {
  server.handleClient();
  
}
