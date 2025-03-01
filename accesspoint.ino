#include <WiFiManager.h>
#include <WiFi.h>
#include <WebSocketsServer.h>

WiFiManager wm;
WiFiServer server(80);
WebSocketsServer webSocket(81);

const int ledpin = 4;

void setup()

{
  Serial.begin(115200);
  pinMode(ledpin, OUTPUT);
  digitalWrite(ledpin, LOW);
  if (!wm.autoConnect("samsung", "12131415"))
  {
    Serial.println("Failed to connect, restarting...!");
    delay(3000);
    ESP.restart();
  }
  Serial.println("connected to the WiFi...!");
  Serial.println("IP Address: ");
  Serial.println(WiFi.localIP());
  server.begin();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

}
void loop()
{
  WiFiClient client = server.available();
  if(client)
  {
    Serial.println("New Client Connected");
    String request = " ";
    while(client.connected())
    {
      if(client.available())
      {
        char c = client.read();
        request += c;
        if(c== '\n')
        {
          if(request.indexOf("/LED=ON")!= -1)
          {
            digitalWrite(ledpin, HIGH);
            Serial.println("LED Turned ON");
          }
          else if(request.indexOf("/LED=OFF")!= -1)
          {
            digitalWrite(ledpin, LOW);
            Serial.println("LED Turned OFF");
          }
          client.println("HTTP/1.1 200 OK");
          client.println("content-type:text/html");
          client.println();
          client.println("<html><head>");
          client.println("<script>");
          client.println("var socket = new WebSocket('ws://'+window.location.hostname + ':81/');");
          client.println("function toggleLED(state) {socket.send(state);}");
          client.println("</script>");
          client.println("</head><body>");
          client.println("<h2>ESP32 LED Control</h2>");
          client.println("<p><a href='/LED=ON'><button>Turn On</button>");
          client.println("<p><a href='/LED=OFF'><button>Turn Off</button>");
          client.println("</body></html>");
          client.println();
          client.stop();
          Serial.println("client Dissconnected");
          break;

        }
      }
    }
  }
  webSocket.loop();
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length)
{
  String message = (char *)payload;

  if(type == WStype_TEXT)
  {
    Serial.print("Received: ");
    Serial.println(message);

    if(message == "ON"){
      digitalWrite(ledpin, HIGH);
    } else if(message == "OFF") {
      digitalWrite(ledpin, LOW);
    }

    String status = digitalRead(ledpin) ? "ON" : "OFF";
    webSocket.sendTXT(num, status);
  }
}

