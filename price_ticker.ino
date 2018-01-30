#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include "SSD1306.h"
#include <WiFiClientSecure.h>


// Initialize the OLED display using brzo_i2c
// D3 -> SDA
// D5 -> SCL
// SSD1306Brzo display(0x3c, D3, D5);
// or
// SH1106Brzo  display(0x3c, D3, D5);

// Initialize the OLED display using Wire library
SSD1306  display(0x3c, D3, D5);

const char* ssid     = "-";
const char* password = "-";

// API server
const char* host = "api.coinmarketcap.com";


void setup() {

  // Serial
  Serial.begin(115200);
  delay(10);

   // Initialize display
  display.init();
  display.flipScreenVertically();
  display.clear();
  display.display();

  // We start by connecting to a WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {

  // Connect to API
  Serial.print("connecting to ");
  Serial.println(host);
  
  // Use WiFiClient class to create TCP connections
  WiFiClientSecure client;
  const int httpPort = 443;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    delay(5000);
    return;
  }
  
  // We now create a URI for the request
  String url = "/v1/ticker/ethereum/?convert=EUR";
  
  Serial.print("Requesting URL: ");
  Serial.println(url);
  
  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");
  delay(500);

  //skip headers
  while(client.available()){
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      break;
    }
  }
  
  // Read json body
  String answer;
  while(client.available()){
    String line = client.readStringUntil('\r');
    answer += line;
  }

  client.stop();
  Serial.println();
  Serial.println("closing connection");


  displayPrice(answer);

  // Wait 5 seconds
  delay(5000);
}

void displayPrice(String answer){
  // Process answer
  Serial.println();

  const size_t bufferSize = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(18) + 430;
  DynamicJsonBuffer jsonBuffer(bufferSize);

  //const char* json = "[{\"id\":\"ethereum\",\"name\":\"Ethereum\",\"symbol\":\"ETH\",\"rank\":\"2\",\"price_usd\":\"1082.84\",\"price_btc\":\"0.10662\",\"24h_volume_usd\":\"4078200000.0\",\"market_cap_usd\":\"105372149167\",\"available_supply\":\"97310913.0\",\"total_supply\":\"97310913.0\",\"max_supply\":null,\"percent_change_1h\":\"0.21\",\"percent_change_24h\":\"-8.22\",\"percent_change_7d\":\"8.83\",\"last_updated\":\"1517349853\",\"price_eur\":\"873.92009892\",\"24h_volume_eur\":\"3291364326.6\",\"market_cap_eur\":\"85041962823.0\"}]";

  JsonArray& root = jsonBuffer.parseArray(answer);

  JsonObject& root_0 = root[0];
  String root_0_percent_change_1h = root_0["percent_change_1h"]; // "0.21"
  String root_0_price_eur = root_0["price_eur"]; // "873.92009892"
  

  // Display on OLED
  display.clear();
 

  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0, "ETH");

  display.setFont(ArialMT_Plain_24);
  display.drawString(0, 14, root_0_price_eur.substring(0,7));

  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 46, root_0_percent_change_1h+"%");
    
  display.display();
}

