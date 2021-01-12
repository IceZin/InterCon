#include "WebSocketClient.h"

WebSocketClient::WebSocketClient() {
  client = new WiFiClient;
  WiFi.mode(WIFI_STA);
}

void WebSocketClient::scan() {
  int aps = WiFi.scanNetworks();

  for (int i = 0; i < aps; i++) {
    Serial.print("[*] ");
    Serial.println(WiFi.SSID(i));
  }
}

void WebSocketClient::connectToWifi() {
  //scan();

  Serial.println("[*] Connecting to WIFI");
  
  WiFi.begin("2.4GHz Nucleon", "VHgp!!07MHgp@)05");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print('.');
  }
  
  Serial.println("\n[*] Connected to WIFI");
}

String WebSocketClient::generateKey() {
  String key = "";
  
  for (int i = 0; i < 22; ++i) {
    int r = random(0, 3);
    if (r == 0)
      key += (char) random(48, 58);
    else if (r == 1)
      key += (char) random(65, 91);
    else if (r == 2)
      key += (char) random(97, 128);
  }
  
  return key + "==";
}

void WebSocketClient::connectToWs(String path) {
  upgrading = true;
  
  String handshake = "GET " + path + " HTTP/1.1\r\n"
      "Host: 192.168.0.10:8080\r\n"
      "Connection: Upgrade\r\n"
      "Upgrade: websocket\r\n"
      "Sec-WebSocket-Version: 13\r\n"
      "Sec-WebSocket-Key: " + generateKey() + "\r\n"
      "dvc_name: ESP8266 LED\r\n"
      "dvc_addr: esp_led\r\n"
      "owner: fb501a2157d3eefc\r\n"
      "\r\n";

  client->write(handshake.c_str());
  awaitTime = millis();
}

void WebSocketClient::readWs() {
  Serial.println("[*] Receiving data");
  byte i = 0;

  if (awaitingUpgrade) {
    while(client->available() > 0) {
      char c = client->read();

      Serial.print(c);
      
      if (c == ' ' or c == '\0') {
        data[i] = '\0';
        
        if (!strcmp(data, "101")) {
          flushBuffer();
          Serial.println("[*] Connected to WS Server");
          awaitingUpgrade = false;
          upgrading = false;
        }
        
        memset(data, 0, sizeof(data));
        i = 0;
        return;
      }
      data[i++] = c;
    }

    Serial.println();
  } else {
    decodeData();
  }
}

void WebSocketClient::decodeData() {
  unsigned int FIN = client->read();
  int LEN = client->read();
  bool mask = false;
  
  if (LEN & 0x80) {
    mask = true;
    LEN = LEN & ~0x80;
  }
  
  for (int i = 0; i < LEN; i++) {
    data[i] = client->read();
  }
  
  data[LEN+1] = '\0';

  if (!strcmp(data, "ping")) {
    sendPong();
  } else {
    Serial.println(data);
  }
}

void WebSocketClient::sendPong() {
  const char* pong_frame = "pong";
  client->write((uint8_t)(1 << 7 | 0 << 6 | 0 << 5 | 0 << 4 | 10));
  client->write((uint8_t)(1 << 7 | 0x04));

  uint8_t mask[4];
  mask[0] = random(0, 256);
  mask[1] = random(0, 256);
  mask[2] = random(0, 256);
  mask[3] = random(0, 256);

  client->write(mask[0]);
  client->write(mask[1]);
  client->write(mask[2]);
  client->write(mask[3]);

  for (int i = 0; i < 4; ++i) {
    client->write(pong_frame[i] ^ mask[i % 4]);
  }
}

void WebSocketClient::flushBuffer() {
  while (client->available() > 0) {
    client->read();
  }
}

void WebSocketClient::update() {
  if (WiFi.status() != WL_CONNECTED) connectToWifi();
  
  if (!client->connected()) {
     client->connect("192.168.0.10", 8080);
     awaitingUpgrade = true;
  }
  
  if (awaitingUpgrade and !upgrading) {
    Serial.println("[*] Connecting to WS Server");
    connectToWs("/dvcCon");
  }
  
  if (client->available() > 0) readWs();
  
  if (upgrading and millis() - awaitTime >= awaitTimeout) {
    Serial.println("[!] Timed out");
    upgrading = false;
  }
}

void WebSocketClient::sendCmd(String command) {
  Serial.print("{" + command + "}");
}
