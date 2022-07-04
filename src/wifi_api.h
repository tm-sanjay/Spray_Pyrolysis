#ifndef wifi_api_h
#define wifi_api_h

#include <ESPAsyncWebServer.h>

AsyncWebServer server(80);

const char* PARAM_KEY = "fun"; //query parameter KEY

void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Page Not found");
}

void setup_wifi() {
  WiFi.mode(WIFI_AP_STA);
    WiFi.softAP("esp-AP","12345678");
    
    Serial.print("AP IP address: ");
    IPAddress myIP = WiFi.softAPIP();
    Serial.println(myIP);

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(200, "text/plain", "Spray Pyrolysis");
    });

    //start process
    server.on("/start", HTTP_GET, [](AsyncWebServerRequest *request){
      Serial.println("Strated Form http");
      request->send(200, "text/plain", "ESP:Started");
    });
    //stop Process
    server.on("/stop", HTTP_GET, [](AsyncWebServerRequest *request){
      Serial.println("Stoped Form http");
      request->send(200, "text/plain", "ESP:Stopped");
    });
    //update temperature
    server.on("/temp", HTTP_POST, [](AsyncWebServerRequest *request){
      String message;
      if (request->hasParam(PARAM_KEY)) {
        message = request->getParam(PARAM_KEY)->value();
        Serial.print("http /temp:");
        Serial.println(message.toInt());
      } else {
        message = "No message sent";
      }
      request->send(200, "text/plain", "Recived: " + message);
    });
    //update Pump speed
    server.on("/pump", HTTP_POST, [](AsyncWebServerRequest *request){
      String message;
      if (request->hasParam(PARAM_KEY)) {
        message = request->getParam(PARAM_KEY)->value();
        Serial.print("http /pump:");
        Serial.println(message.toInt());
      } else {
        message = "No message sent";
      }
      request->send(200, "text/plain", "Recived: " + message);
    });
    //update plotter speed
    server.on("/plotter", HTTP_POST, [](AsyncWebServerRequest *request){
      String message;
      if (request->hasParam(PARAM_KEY)) {
        message = request->getParam(PARAM_KEY)->value();
        Serial.print("http /plotter:");
        Serial.println(message.toInt());
      } else {
        message = "No message sent";
      }
      request->send(200, "text/plain", "Recived: " + message);
    });
    server.onNotFound(notFound);

    server.begin();
}

#endif