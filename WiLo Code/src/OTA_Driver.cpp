
#include "OTA_Driver.h"


RTC_DATA_ATTR int OTA_Window_Missed = 0; // RTC memory variable to keep track of OTA window


// Static IP Configuration (for Android hotspot)
IPAddress local_IP(192, 168, 220, 50);  // ESP32 static IP
IPAddress gateway(192, 168, 220, 22);   // Android hotspot IP (gateway)
IPAddress subnet(255, 255, 255, 0);     // Subnet mask

const char* host = "esp32";
const char* ssid = "CSE";
const char* password = "12345678";

WebServer server(80);
const int ledPin = 15; // Built-in LED on most ESP32 boards (you can change this if needed)



/*
 * Login page
 */
const char* serverIndex = R"rawliteral(
  <!DOCTYPE html>
  <html>
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
      body {
        font-family: sans-serif;
        text-align: center;
        padding: 20px;
        background-color: #f4f4f4;
      }
      form {
        background: white;
        padding: 20px;
        border-radius: 10px;
        display: inline-block;
        box-shadow: 0 0 10px rgba(0,0,0,0.1);
      }
      input[type="file"] {
        margin: 10px 0;
      }
      input[type="submit"] {
        padding: 10px 20px;
        font-size: 16px;
        background-color: #007BFF;
        border: none;
        color: white;
        border-radius: 5px;
        cursor: pointer;
      }
      input[type="submit"]:hover {
        background-color: #0056b3;
      }
      #prg {
        margin-top: 10px;
        font-size: 14px;
        color: #333;
      }
    </style>
  </head>
  <body>
    <h2>ESP32 Firmware Uploader</h2>
    <form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>
      <input type='file' name='update'><br>
      <input type='submit' value='Update Firmware'>
    </form>
    <div id='prg'>Progress: 0%</div>
  
    <script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>
    <script>
      $('form').submit(function(e){
        e.preventDefault();
        var form = $('#upload_form')[0];
        var data = new FormData(form);
        $.ajax({
          url: '/update',
          type: 'POST',
          data: data,
          contentType: false,
          processData: false,
          xhr: function() {
            var xhr = new window.XMLHttpRequest();
            xhr.upload.addEventListener('progress', function(evt) {
              if (evt.lengthComputable) {
                var per = evt.loaded / evt.total;
                $('#prg').html('Progress: ' + Math.round(per*100) + '%');
              }
            }, false);
            return xhr;
          },
          success: function(d, s) {
            console.log('Success!');
          },
          error: function(a, b, c) {
            console.log('Error uploading firmware');
          }
        });
      });
    </script>
  </body>
  </html>
  )rawliteral";
  


uint8_t setupOTA() {


    pinMode(DEBUG_LED_PIN, OUTPUT);
    digitalWrite(ledPin, LOW); // LED OFF when not connected
  
    // Set static IP config and connect to WiFi
   WiFi.config(local_IP, gateway, subnet);
    WiFi.begin(ssid, password);
  
    Serial.print("Connecting to WiFi");
    int startTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < 15000) {
      delay(500);
      Serial.print(".");
    }
  
    digitalWrite(ledPin, HIGH); 
  
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Failed to connect to WiFi. Check your credentials.");
      return 0;
    }
  
    Serial.println("\nConnected to WiFi");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  
    digitalWrite(ledPin, HIGH); // LED ON when connected
  
    if (!MDNS.begin(host)) {
      Serial.println("Error setting up MDNS responder!");
      while (1) {
        delay(1000);
      }
    }
    Serial.println("mDNS responder started");
  
    // Serve the update form
    server.on("/", HTTP_GET, []() {
      server.sendHeader("Connection", "close");
      server.send(200, "text/html", serverIndex);
    });
  
    // Handle the OTA update
    server.on("/update", HTTP_POST, []() {
      server.sendHeader("Connection", "close");
      server.send(200, "text/plain", Update.hasError() ? "FAIL" : "OK");
      ESP.restart();
    }, []() {
      HTTPUpload& upload = server.upload();
      if (upload.status == UPLOAD_FILE_START) {
        Serial.printf("Update: %s\n", upload.filename.c_str());
        if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true)) {
          Serial.printf("Update Success: %u bytes\nRebooting...\n", upload.totalSize);
          OTA_Window_Missed = 1; 
        } else {
          Update.printError(Serial);
        }
      }
    });
  
    server.begin();
    return 1;
  }

  void LoopOTA(){
    while(WiFi.status() == WL_CONNECTED){
        server.handleClient();
        delay(1);
        }
        Serial.println("WiFi disconnected");
        digitalWrite(ledPin, LOW); // LED OFF when disconnected
        WiFi.disconnect();
        WiFi.mode(WIFI_OFF);
      }

      void disableWiFi() {
        WiFi.disconnect(true);   // Disconnect and erase credentials (true = erase)
        WiFi.mode(WIFI_OFF);     // Turn off WiFi hardware
        btStop();                // Optional: disable Bluetooth radio
      }