
#include "OTA_Driver.h"


RTC_DATA_ATTR int OTA_Window_Missed = 0; // RTC memory variable to keep track of OTA window


// Static IP Configuration (for Android hotspot)
IPAddress static_IP_Android(192, 168, 146, 150);
IPAddress static_IP_Iphone(172, 20, 10, 4); // Static IP address
IPAddress gateway;
IPAddress subnet;



const char* host = "esp32";
const char* ssid = "CSE";
const char* password = "12345678";

WebServer server(80);

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
    form, .sync-box {
      background: white;
      padding: 20px;
      border-radius: 10px;
      display: inline-block;
      box-shadow: 0 0 10px rgba(0,0,0,0.1);
      margin-top: 20px;
    }
    input[type="file"] {
      margin: 10px 0;
    }
    input[type="submit"], button {
      padding: 10px 20px;
      font-size: 16px;
      background-color: #007BFF;
      border: none;
      color: white;
      border-radius: 5px;
      cursor: pointer;
    }
    input[type="submit"]:hover, button:hover {
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
  <form method='POST' action='/update' enctype='multipart/form-data' id='upload_form'>
    <input type='file' name='update'><br>
    <input type='submit' value='Update Firmware'>
  </form>
  <div id='prg'>Progress: 0%</div>

  <div class="sync-box">
    <h3>Sync RTC with Mobile Time</h3>
    <button onclick="sendTime()">Sync Time</button>
    <p id="syncStatus"></p>
  </div>

  <script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>
  <script>
    function sendTime() {
      const now = new Date();
      const iso = now.toISOString(); // e.g., "2025-06-13T17:34:56.123Z"
      const trimmed = iso.slice(0, 19); // Remove milliseconds and Z
      const localTime = new Date(now.getTime() - now.getTimezoneOffset() * 60000).toISOString().slice(0,19);
      fetch("/set-time?dt=" + encodeURIComponent(localTime))
        .then(response => response.text())
        .then(data => document.getElementById("syncStatus").innerText = data)
        .catch(err => document.getElementById("syncStatus").innerText = "Error syncing time");
    }

    $('form#upload_form').submit(function(e){
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




/*
 * Login page
 */
/*const char* serverIndex = R"rawliteral(
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
<form method='POST' action='/update' enctype='multipart/form-data' id='upload_form'>
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
  
*/

  uint8_t setupOTA() {
    pinMode(RTC_EN_PIN, OUTPUT);
    digitalWrite(RTC_EN_PIN, LOW); // LED and RTC OFF initially

    Serial.println("Starting AP for OTA...");

    // Start ESP32 in Access Point mode
    WiFi.mode(WIFI_AP);
    bool apStarted = WiFi.softAP(ssid, password);

    if (!apStarted) {
        Serial.println("Failed to start AP");
        return 0;
    }

    IPAddress ip = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(ip);

    Serial.print("Waiting for client to connect");
    unsigned long startAttemptTime = millis();
    setupRTC();
    while ((WiFi.softAPgetStationNum() == 0) && (millis() - startAttemptTime < WIFI_TIMEOUT)) {
        delay(500);
        Serial.print(".");
    }

    if (WiFi.softAPgetStationNum() == 0) {
        Serial.println("\nNo client connected within timeout.");
        WiFi.softAPdisconnect(true); // Stop AP
        return 0;
    }

    Serial.println("\nClient connected!");

    digitalWrite(RTC_EN_PIN, HIGH); // LED and RTC ON to show AP and client active
    delay(3000);


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

server.on("/set-time", HTTP_GET, []() {
  if (server.hasArg("dt")) {
    String dt = server.arg("dt");  // Expected format: "2025-06-13T19:22:45"
    int y, mo, d, h, mi, s;
    if (sscanf(dt.c_str(), "%d-%d-%dT%d:%d:%d", &y, &mo, &d, &h, &mi, &s) == 6) {
      rtc.adjust(DateTime(y, mo, d, h, mi, s));
      server.send(200, "text/plain", "RTC set to: " + dt);
      Serial.printf("RTC set to: %04d-%02d-%02d %02d:%02d:%02d\n", y, mo, d, h, mi, s);
    } else {
      server.send(400, "text/plain", "Invalid format. Use YYYY-MM-DDTHH:MM:SS");
    }
  } else {
    server.send(400, "text/plain", "Missing 'dt' parameter");
  }
});

    server.begin();
    return 1;
}


// uint8_t setupOTA() {


//     pinMode(DEBUG_LED_PIN, OUTPUT);
//     digitalWrite(ledPin, LOW); // LED OFF when not connected
  
//     // Set static IP config and connect to WiFi

//    //WiFi.begin(ssid, password);


  
//     Serial.print("Connecting to WiFi");
//     int startTime = millis();
//     while (WiFi.status() != WL_CONNECTED && millis() - startTime < 15000) {
//       delay(500);
//       Serial.print(".");
//     }
  
//     digitalWrite(ledPin, HIGH); 
  
//     if (WiFi.status() != WL_CONNECTED) {
//       Serial.println("Failed to connect to WiFi. Check your credentials.");
//       return 0;
//     }
  
//     Serial.println("\nConnected to WiFi");
//     Serial.print("IP address: ");
//     Serial.println(WiFi.localIP());

//     // gateway = WiFi.gatewayIP();
//     // subnet = WiFi.subnetMask();
//     // Serial.println("\nDHCP Connected!");
//     // Serial.print("Gateway: "); Serial.println(gateway);
//     // Serial.print("Subnet : "); Serial.println(subnet);
  
//     // // Step 2: Disconnect and reconnect with static IP
//     // WiFi.disconnect(true);
//     // delay(1000);
  
//     // WiFi.config(static_IP_Android, gateway, subnet);
//     // WiFi.begin(ssid, password);
  
//     // Serial.print("Reconnecting with static IP");
//     // while (WiFi.status() != WL_CONNECTED) {
//     //   delay(500);
//     //   Serial.print(".");
//     // }
  
//     // Serial.println("\nStatic IP set!");
//     // Serial.print("ESP32 IP: ");
//     // Serial.println(WiFi.localIP());
//     IPAddress ip = WiFi.localIP();
//     String hostname = ip.toString();
//     hostname.replace(".", "-"); // Convert IP to valid hostname
  
//     WiFi.disconnect(true); // Disconnect and erase previous settings
//     delay(1000);
  
//     // Set new hostname BEFORE connecting
//     WiFi.setHostname(hostname.c_str());
  
//     Serial.print("\nReconnecting with hostname: ");
//     Serial.println(WiFi.getHostname());
  
//     WiFi.begin(ssid, password);
  
//     while (WiFi.status() != WL_CONNECTED) {
//       delay(500);
//       Serial.print(".");
//     }
  
//     Serial.println("\nConnected!");
//     Serial.print("IP: ");
//     Serial.println(WiFi.localIP());
//     Serial.print("Hostname: ");
//     Serial.println(WiFi.getHostname());
  
  
//     digitalWrite(ledPin, HIGH); // LED ON when connected
  
//     if (!MDNS.begin(host)) {
//       Serial.println("Error setting up MDNS responder!");
//       while (1) {
//         delay(1000);
//       }
//     }
//     Serial.println("mDNS responder started");
  
//     // Serve the update form
//     server.on("/", HTTP_GET, []() {
//       server.sendHeader("Connection", "close");
//       server.send(200, "text/html", serverIndex);
//     });
  
//     // Handle the OTA update
//     server.on("/update", HTTP_POST, []() {
//       server.sendHeader("Connection", "close");
//       server.send(200, "text/plain", Update.hasError() ? "FAIL" : "OK");
//       ESP.restart();
//     }, []() {
//       HTTPUpload& upload = server.upload();
//       if (upload.status == UPLOAD_FILE_START) {
//         Serial.printf("Update: %s\n", upload.filename.c_str());
//         if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
//           Update.printError(Serial);
//         }
//       } else if (upload.status == UPLOAD_FILE_WRITE) {
//         if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
//           Update.printError(Serial);
//         }
//       } else if (upload.status == UPLOAD_FILE_END) {
//         if (Update.end(true)) {
//           Serial.printf("Update Success: %u bytes\nRebooting...\n", upload.totalSize);
//           OTA_Window_Missed = 1; 
//         } else {
//           Update.printError(Serial);
//         }
//       }
//     });
  
//     server.begin();
//     return 1;
//   }

  // void LoopOTA(){
  //   while(WiFi.status() == WL_CONNECTED){
  //       server.handleClient();
  //       delay(1);
  //       }
  //       Serial.println("WiFi disconnected");
  //       digitalWrite(ledPin, LOW); // LED OFF when disconnected
  //       WiFi.disconnect();
  //       WiFi.mode(WIFI_OFF);
  //     }

      void disableWiFi() {
        WiFi.disconnect(true);   // Disconnect and erase credentials (true = erase)
        WiFi.mode(WIFI_OFF);     // Turn off WiFi hardware
        btStop();                // Optional: disable Bluetooth radio
      }

  void LoopOTA() {
    

    while (WiFi.softAPgetStationNum() > 0) {
        server.handleClient();
        delay(1);
    }

    Serial.println("No more clients connected to AP.");
    digitalWrite(RTC_EN_PIN, LOW); // LED and RTC OFF when no clients

    WiFi.softAPdisconnect(true); // Stop AP
    WiFi.mode(WIFI_OFF);
}