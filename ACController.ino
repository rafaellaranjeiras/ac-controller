#include <ESP8266WiFi.h>
#include <IRremote.h>
 
const char* ssid = "<SSID>";
const char* password = "<password>";
 
const String ROOMS[2] = {"Quarto", "Escritorio"};
const String ROOM_COLOR[2] = {"#cc8899", "#222222"};
const int SENT_PINS[2] = {14, 13};
const int FLOWS[6]  = {0, 1, 2, 3, 4, 5};
const int SWING_DEGREES[6] = {5, 21, 37, 53, 69, 85};
const unsigned long SWING_CODES[6] = {0x8813048, 0x8813059, 0x881306A, 0x881307B, 0x881308C, 0x881309D};

int room;
int temperature;
int acON;
int flow;
int airSwing;
int jetModeON;

WiFiServer server(80);
 
void setup() {
  Serial.begin(9600);  
  connectWifi();
  room = 0;
  IrSender.begin(SENT_PINS[room]);
  temperature = 18;
  acON = 0;
  flow = 2; 
  airSwing = 4; 
  jetModeON = 0;
}
 
void loop() {
  if(WiFi.status() != WL_CONNECTED) {
    connectWifi();    
  }

  WiFiClient client = server.available();
  if (!client) {
    return;
  }  
  
  String request = client.readStringUntil('\r');
  Serial.print("Request: ");
  Serial.println(request);
  client.flush(); 
  int value = LOW;
  proccessRequest(request);
  returnHtml(client);
  delay(1);
}

void proccessRequest(String request) {
  if (request.indexOf("/TOGGLEROOM") != -1)  {
    toggleRoom();
    return;
  } 
  if (request.indexOf("/POWER") != -1)  {
    togglePower();
  } 
  if(acON == 0) {
    return;
  }
  if (request.indexOf("/TEMPDOWN") != -1)  {
    decreaseTemp();
  } else if (request.indexOf("/TEMPUP") != -1)  {
    increaseTemp();
  } else if (request.indexOf("/FLOWDOWN") != -1)  {
    decreaseFlow();
  } else if (request.indexOf("/FLOWUP") != -1)  {
    increaseFlow();
  } else if (request.indexOf("/SWING") != -1)  {
    toggleSwing();
  } else if (request.indexOf("/JETMODE") != -1)  {
    jetMode();
  } else if (request.indexOf("/LIGHT") != -1)  {
    toggleLight();
  }   
}

void toggleRoom() {
  if(room == 0) {
    room = 1;
  } else {
    room = 0;
  }
  IrSender.setSendPin(SENT_PINS[room]);
}

void togglePower() {
  if(acON == 0) {
    powerOn();
  } else {
    powerOff();
  }
}

void powerOff() {
  send(0x88C0051);
  acON = 0;
  jetModeON = 0;
}

void powerOn() {
  jetModeON = 0;
  unsigned long code;
  int AC_MSBITS1 = 8;
  int AC_MSBITS2 = 8;
  int AC_MSBITS3 = 0;
  int AC_MSBITS4 = 0;
  int AC_MSBITS5 = temperature - 15;
  int AC_MSBITS6 = FLOWS[flow];
  int AC_MSBITS7 = (AC_MSBITS3 + AC_MSBITS4 + AC_MSBITS5 + AC_MSBITS6) & B00001111;
  code = AC_MSBITS1 << 4 ;
  code = (code + AC_MSBITS2) << 4;
  code = (code + AC_MSBITS3) << 4;
  code = (code + AC_MSBITS4) << 4;
  code = (code + AC_MSBITS5) << 4;
  code = (code + AC_MSBITS6) << 4;
  code = (code + AC_MSBITS7);
  send(code);
  acON = 1;
}

void toggleSwing() {
  if(airSwing == 5){
    airSwing = -1;
  } else {
    airSwing++;
  }
  if(airSwing == -1) {
    send(0x8813149);
  } else {
    send(SWING_CODES[airSwing]);
  }  
}

void increaseFlow() {
  jetModeON = 0;
  if(flow < 3) {
    flow++;        
  }
  powerOn();
}

void decreaseFlow() {
  jetModeON = 0;
  if(flow > 0) {
    flow--;    
  }
  powerOn();
}

void increaseTemp() {
  jetModeON = 0;
  if(temperature < 30) {
    temperature++;        
  }
  powerOn();
}

void decreaseTemp() {
  jetModeON = 0;
  if(temperature > 18) {
    temperature--;    
  }
  powerOn();
}

void jetMode() {
  airSwing = 1;
  jetModeON = 1;
  send(0x8810089);
}

void toggleLight() {
  send(0x88C00A6);
}

void send(unsigned long code) {
  Serial.print("0x");
  Serial.println(code, HEX);
  IrSender.sendLG(code, 28);
}

void connectWifi() {
  Serial.print("Connecting to ");
  Serial.println(ssid); 
  WiFi.begin(ssid, password); 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  server.begin();
  Serial.print("Server started: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");
}

void returnHtml(WiFiClient client) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("");
  client.println("<!DOCTYPE html>");
  client.println("<html>");
  client.println("<link href=\"https://fonts.cdnfonts.com/css/ds-digital\" rel=\"stylesheet\">");
  client.println("<link rel=\"stylesheet\" href=\"https://cdnjs.cloudflare.com/ajax/libs/font-awesome/4.7.0/css/font-awesome.min.css\" />");
  client.println("<style>");
  client.println("  .parent {display: grid; text-align: center; grid-template-columns: repeat(3, 1fr); grid-template-rows: repeat(4, 1fr); grid-column-gap: 0px; grid-row-gap: 80px;}");
  client.println("  button {font-size:50px;width:80%; height:110%;border:none;padding:30px 64px;text-align:center; border-radius: 50%; box-shadow: 0 16px 32px 0 rgba(0,0,0,0.2), 0 6px 20px 0 rgba(0,0,0,0.19);}");
  client.println("  .button:hover {background-color: #4CAF50;color: white;}");
  client.println("</style>");
  client.println("<head>");
  client.println("<link rel=\"icon\" type=\"image/x-icon\" href=\"https://www.snow.com/Assets/images/sites/snow/favicon.ico\">");
  client.println("<meta name=\"viewport\" content=\"user-scalable=no\" />");
  client.println("</head>");
  client.println("<body>");
  client.println("  <div class=\"parent\" style=\"padding-top: 10%;\">");
  client.println("    <a href=\"/POWER\"><button style=\"background-color: #ff9d5c;\"><i class=\"fa fa-power-off\"></i></button></a>");    
  if(acON == 0) {
    client.print("    <div style=\"font-family: 'DS-Digital'; font-size: 100px; text-align: center; max-height: 100%; line-height:40px;\">");
    client.println("OFF");
    client.println("</div>");
  } else {
    client.print("    <div style=\"font-family: 'DS-Digital'; font-size: 180px; text-align: center; max-height: 100%; line-height:40px;\">");
    if (jetModeON == 1) {
      client.println("Po");
      client.print("    <br/><span style=\"font-size: 40px;\">");
    } else {
      client.println(temperature);
      client.print("    <br/><span style=\"font-size: 40px;\">F");
      client.print(FLOWS[flow]);  
    }
    client.println("</span>");
    client.println("<div style=\"font-size: 40px;\">");
    if(airSwing > -1) {      
      client.print("  <span style=\"position: absolute; font-size: 70px; transform-origin: top center; transform: rotate(");
      client.print(SWING_DEGREES[airSwing]);
      client.println("deg);\">I</span>");
    } else {
      client.println("<span style=\"position: absolute; font-size: 70px;\">></span>");
    }
    client.println("</div> </div>");
  }
  client.println("    <a href=\"/LIGHT\"><button>Light off</button></a>");
  client.println("    <a href=\"/TEMPUP\"><button style=\"background-color: #8cd3ff;\">Temp +</button></a>");
  client.println("    <a href=\"/JETMODE\"><button>Jet Mode</button></a>");
  client.println("    <a href=\"/FLOWUP\"><button style=\"background-color: #acd8a7;\">Vel +</button></a>");
  client.println("    <a href=\"/TEMPDOWN\"><button style=\"background-color: #8cd3ff;\">Temp -</button></a>");
  client.println("    <a href=\"/SWING\"><button>Swing</button></a>");
  client.println("    <a href=\"/FLOWDOWN\"><button style=\"background-color: #acd8a7;\">Vel -</button></a>");
  client.println("    <p/> <p/> <p/> <p/> <p/>");
  client.print("    <a href=\"/TOGGLEROOM\"><button style=\"background-color: ");
  client.print(ROOM_COLOR[room]);
  client.print("; color: white;\">");
  client.print(ROOMS[room]);
  client.println("</button></a>");  
  client.println("  </div>");
  client.println("</body>");
  client.println("</html>");
}
 