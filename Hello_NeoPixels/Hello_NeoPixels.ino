#include <FastLED.h>

#define NUM_LEDS 16
#define DATA_PIN 6

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <string>

#ifndef STASSID
#define STASSID "NETGEAR50"
#define STAPSK  "freshbanana499"
#endif


const char* ssid = STASSID;
const char* password = STAPSK;

const char *mainPagep1 = "<!DOCTYPE html>"
                         "<html>"
                         "<style>"
                         "input[type=color] {"
                         "background:none;"
                         "width: 200px;"
                         "height: 50px;"
                         "padding: 0px 0px;"
                         "margin: 8px 0;"
                         "}"
                         ".center {"
                         "  text-align: center;"
                         "}"
                         ".header {"
                         "text-align: center;"
                         "font-size: 25px;"
                         "padding: 60px;"
                         "}"
                         "body {"
                         "background-color: beige;"
                         "}"
                         ".dropbtn {"
                         "    width: 200px;"
                         "    background-color: #4CAF50;"
                         "    color: white;"
                         "    padding: 16px;"
                         "    font-size: 16px;"
                         "    width"
                         "    border: none;"
                         "  }"
                         "  .dropbtn:hover {background-color: #337a36;}"
                         "  .dropdown {"
                         "    position: relative;"
                         "    display: inline-block;"
                         "    margin: 5px;"
                         "  }"
                         "  .dropdown-content {"
                         "    display: none;"
                         "    position: absolute;"
                         "    background-color: #f1f1f1;"
                         "    min-width: 200px;"
                         "    box-shadow: 0px 8px 16px 0px rgba(0,0,0,0.2);"
                         "    z-index: 1;"
                         "  }"
                         "  .dropdown-content a {"
                         "    color: black;"
                         "    padding: 12px 16px;"
                         "    text-decoration: none;"
                         "    display: block;"
                         "  }"
                         "  .dropdown-content a:hover {background-color: #ddd;}"
                         "  .dropdown:hover .dropdown-content {display: block;}"
                         "  .dropdown:hover .dropbtn {background-color: #3e8e41;}"
                         "</style>"
                         "<div class='header'>"
                         "<head>ESP8266/ESP12 ARGB fan controller</head>"
                         "</div>"
                         "<body>"
                         "<div class='center'><div><input type='color' id='color' name='body' value=";
const char *mainPagep2 =  "></div>"
                          "<div><button class='dropbtn' onclick='turnOn()'> Change color.</button></div>"
                          "<div><button class='dropbtn' onclick='setRainbow()'>Toggle rainbow</button></div>"
                          "<div><button class='dropbtn' onclick='turnOff()'> Turn off leds.</button></div>"
                          "<div class='dropdown'>"
                          "<button class='dropbtn'>Animations</button>"
                          "<div class='dropdown-content'>"
                          "<a onclick='changeAnimation(0)' href='#'>Spin left</a>"
                          "<a onclick='changeAnimation(1)' href='#'>Spin right</a>"
                          "<a onclick='changeAnimation(2)' href='#'>Static</a>"
                          "</div>"
                          "</div>"
                          "</div>"
                          "<script>"
                          "function turnOn() {\n"
                          "var color = document.getElementById('color').value.substring(1);\n"
                          "console.log(color);\n"
                          "location.replace('/color?v='+ color);\n"
                          "}\n"
                          "function turnOff() {\n"
                          "location.replace('/color?v=000000')\n"
                          "}\n"
                          "function changeAnimation(x) {\n"
                          "console.log(document.getElementById('color').value);\n"
                          "location.replace('/animation?v=' + x.toString())\n"
                          "}\n"
                          "document.getElementById('color').addEventListener('change', updateColor);\n"
                          "function updateColor()\n"
                          "{\n"
                          "var color = document.getElementById('color').value.substring(1);\n"
                          "console.log(color);\n"
                          "location.replace('/color?v='+ color);\n"
                          "}\n"
                          "function setRainbow() {\n"
                          "location.replace('/rainbow');\n"
                          "}\n"
                          "</script>"
                          "</body>"
                          "</html>";

const char *getColor1 = "<!DOCTYPE html>"
                         "<html>";
const char *getColor2 = "</html>";  
                       
HTTPClient http;
ESP8266WebServer server(80);

const int led = 13;

class rainbow {
  private:
    bool On = false;
    int Rainbow = 0;
  public:
    bool isOn() {
      return this->On;
    };
    void setOn() {
      Serial.println(this -> On);
      this->On = !this->On;
      Serial.println(this -> On);
    }
    int Get() {
      return this-> Rainbow;
    }
    void Next() {
      if (this->Rainbow < 255)
        this->Rainbow = this->Rainbow + 1;
      else
        this->Rainbow = 0;
    }
} RAINBOW;


CHSV rainbow() {
  RAINBOW.Next();
  return CHSV(RAINBOW.Get() * 2, 255, 255);
}


class ANIMATION
{
  private:
    String animations[3] = {"Spin left", "Spin right", "Static"};
    int animation = 0;
  public:
    void Set(int s) {
      this->animation = s;
    }
    int Get() {
      return animation;
    }
} ANIMATION;

class RGBCOLOR
{
  private:
    long color = 0xFF0000;
  public:
    void Set(long s) {
      this->color = s;
    }
    long Get() {
      return color;
    }
} COLOR;

void handleRoot() {
  char NUM[50];
  sprintf(NUM, "%X", COLOR.Get());
  String num = (String) NUM;
  while (num.length() < 6) num = "0" + num;
  server.send(sizeof(mainPagep1) + sizeof((String)COLOR.Get()) + sizeof(mainPagep1) + 1, "text/html", (String) mainPagep1 + "#" + num + (String) mainPagep2);
  delay(50);
  digitalWrite(led, 0);
}

void setColor()
{
  Serial.println(server.arg(0));
  COLOR.Set(strtol(server.arg(0).c_str(), NULL, 16));
  http.POST(server.arg(0).c_str());
  Serial.println(strtol(server.arg(0).c_str(), NULL, 16));
  handleRoot();
}

void setAnimation()
{
  Serial.println(server.args());
  String arg = server.arg(0);
  Serial.println(arg);
  ANIMATION.Set(arg.toInt());
  handleRoot();
}

void handleNotFound() {
  String url = server.uri();
  Serial.println(url);
  Serial.println(url.substring(0, 2));
  Serial.println("-----");
  handleRoot();
}
CRGB leds[NUM_LEDS];

void getColor() {
  Serial.println("i");
  if (RAINBOW.isOn())
  {
    char chr[24];
    itoa(RAINBOW.Get(), chr, 10);
    server.send(8, "text/plain", (String) chr);
    Serial.println(itoa(RAINBOW.Get(), chr, 10));
  }
  else
  {
    char chr[24];
    itoa(COLOR.Get(), chr, 10);
    server.send(sizeof(getColor1)+sizeof(getColor2)+sizeof(chr), "html/plain", (String) getColor1 + (String) chr + (String) getColor2);
    Serial.println(itoa(RAINBOW.Get(), chr, 10));
  }
}


void setup() {
  http.begin("http://192.168.1.2:16000");
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);
  server.on("/color", setColor);
  server.on("/animation", setAnimation);
  server.on("/rainbow", []() {
    RAINBOW.setOn();
    http.POST("rainbow");
    handleRoot();
  });
  server.on("/getColor", getColor);
  server.on("/favicon.ico", []() {
    server.send(10, "text/plain", "");
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");

  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  Serial.println("Hello There");
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
    FastLED.show();
    delay(50);
  }
}
void Spin() {
  leds[NUM_LEDS] = CRGB::Black;
  leds[NUM_LEDS - 1] = CRGB::Black;
  leds[NUM_LEDS - 2] = CRGB::Black;
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i - 1] = CRGB::Black;
    if (RAINBOW.isOn()) {
      leds[i] = rainbow();
    }
    else {
      leds[i] = COLOR.Get();
    }
    //Serial.println(colors[color]);
    //Serial.println(color);
    //Serial.println("--");
    FastLED.show();
    delay(50);
  };
}
void SpinReverse() {
  leds[0] = CRGB::Black;
  leds[1] = CRGB::Black;
  leds[2] = CRGB::Black;
  for (int i = NUM_LEDS - 1; i >= 0; i--) {
    Serial.println(i);
    leds[i + 1] = CRGB::Black;
    if (RAINBOW.isOn()) {
      leds[i] = rainbow();
    }
    else {
      leds[i] = COLOR.Get();
    }
    FastLED.show();
    delay(50);
  };
  leds[NUM_LEDS] = COLOR.Get();
}
void Static() {
  CHSV color = rainbow();
  for (int i = 0; i < NUM_LEDS; i++)
  {
    if (RAINBOW.isOn()) {
      leds[i] = color;
    }
    else {
      leds[i] = COLOR.Get();
    }
    FastLED.show();
  }
}

void loop() {
  server.handleClient();
  

  MDNS.update();
  millis();
  switch (ANIMATION.Get()) {
    case 0:
      Spin();
      break;
    case 1:
      SpinReverse();
      break;
    case 2:
      Static();
      break;
  }
  
  millis();
}
