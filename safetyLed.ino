/*****************************
This program is run on an ESP32 and accepts API requests to change an LED Ring's colors and send button presses back to a main console. 

*****************************/

//Include LIbraries
#include <WiFi.h>
#include <AsyncTCP.h>
#include <AsyncEventSource.h>
#include <AsyncJson.h>
#include <AsyncWebSocket.h>
#include <AsyncWebSynchronization.h>
#include <ESPAsyncWebServer.h>
//#include <SPIFFSEditor.h>
//#include <StringArray.h>
//#include <WebAuthentication.h>
//#include <WebHandlerImpl.h>
//#include <WebResponseImpl.h>
#include <FastLED.h>
#include <Preferences.h>

//Setup preferences
Preferences profiles;


//Declare Variables
//const char* ssid = ";
//const char* password = "";

const char* ssid = ""; //Set WiFi SSID
const char* password = ""; //Set WiFi password

const char* apiuser = "api";
const char* apikey = ""; //set apikey

const int ledPin = 2;
const int buttonPin = 4;
const int buzzerPin = 0;
int buttonState = 0;
#define NUM_LEDS 12
CRGB leds[NUM_LEDS];

String header;


//Set web server port
AsyncWebServer server(80);

//Create index file
const char index_html[] PROGMEM = R"rawliteral(
  <!DOCTYPE HTML>
  <html>
    <head>
      <title>Safety LED Configuratiom</title>
    </head>
    <body>
      <h1>Profiles</h1>
      <label for='r'>Red:</label><input type=number id='r' name='r'><br>
      <label for='b'>Blue:</label><input type=number id='b' name='b'><br>
      <label for='g'>Green:</label><input type=number id='g' name='g'><br>
    </body>
  </html>
)rawliteral";

bool updateDome(String profileID){
  //Start preferences
  profiles.begin("profiles", true);

  //Set color variable
  String colorProfile = profileID + "-color";
  char colorProfileChar[9];
  colorProfile.toCharArray(colorProfileChar, 9);
  unsigned long color = profiles.getULong(colorProfileChar);

  //Set hertz variable
  String hertzProfile = profileID + "-hertz";
  char hertzProfileChar[9];
  hertzProfile.toCharArray(hertzProfileChar, 9);
  int hertz = profiles.getInt(hertzProfileChar);

  //Set timeLength variable
  String timeLengthProfile = profileID + "-timeLength";
  char timeLengthProfileChar[14];
  timeLengthProfile.toCharArray(timeLengthProfileChar, 14);
  int timeLength = profiles.getInt(timeLengthProfileChar);

  for (int i = 0; i < NUM_LEDS; i++){
      leds[i] = color;
    }
    FastLED.show();

    ledcWriteTone(0, hertz);
    delay(timeLength);
    ledcWriteTone(0, 0);
    
  profiles.end();

  return profiles.isKey(colorProfileChar);

}

void setup() {
  Serial.begin(115200);  //Turn on serial connection
  delay(1000);
  //WiFi.softAP(ssid, password);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  //IPAddress IP = WiFi.softAPIP();

  Serial.println("\nConnecting");
  while(WiFi.status() != WL_CONNECTED){
        Serial.print(".");
        delay(100);
    }

  IPAddress IP = WiFi.localIP();
  Serial.println("AP IP address: ");
  Serial.println(IP);

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  server.on("/api", HTTP_GET, [](AsyncWebServerRequest *request){
    if(!request->authenticate(apiuser, apikey))
     return request->requestAuthentication();
    String profileNumber;
    if (request->hasParam("profile")){
      profileNumber = request->getParam("profile")->value();
      if(updateDome(profileNumber)){
        request->send(200, "text/html", "Success");
      }
      else if(!updateDome(profileNumber)){
        request->send(406, "text/html", "Profile does not exist");
      }
    }
  });

  FastLED.addLeds<NEOPIXEL, ledPin>(leds, NUM_LEDS);
  pinMode(buttonPin, INPUT);

  //Start server
  server.begin();

  profiles.begin("profiles", false);

  profiles.putULong("00-color", 0x000000);
  profiles.putInt("00-hertz", 0);
  profiles.putInt("00-timeLength", 0);

  profiles.putULong("01-color", 0xFFFF00);
  profiles.putInt("01-hertz", 5000);
  profiles.putInt("01-timeLength", 1000);

  profiles.begin("profiles", false);
  profiles.putULong("02-color", 0x00FFFF);
  profiles.putInt("02-hertz", 1000);
  profiles.putInt("02-timeLength", 3000);

  profiles.end();

  ledcAttachPin(buzzerPin, 0);
  
}

void loop() {
  /*buttonState = digitalRead(buttonPin);
  if (buttonState == HIGH){
    updateDome(2);
  }
*/
}
