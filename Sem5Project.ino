#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <HTTPClient.h>

#define BUZZER_PIN 14
#define trig 2
#define echo 5

long duration;
float distance;

const char* ssid = "Indihome";
const char* password = "02081958";
const char* URL="https://maker.ifttt.com/trigger/value_sent/with/key/f03dHfzQlSuUJcNzSBt6wQnjoNoBGUafolaHpKYd3I4";
int value = 0; // variable to store the sensor value
String text = "";
WebServer server(80);

const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<style>
.card{
     max-width: 400px;
     min-height: 250px;
     background: #e1eeff;
     padding: 30px;
     box-sizing: border-box;
     color: #000000;
     margin:20px;
     box-shadow: 0px 2px 18px -4px rgba(0,0,0,0.75);
}
</style>
<body>

<div class="card">
  <h1>ESP32 Web Server</h4><br>
  <h2>Sensor Value:<span id="WaterLevel">0</span> cm</h1><br>
</div>
  
<script>

setInterval(function() {
  // Call a function repetatively with 2 Second interval
  getData();
}, 2000); //2000mSeconds update rate

function getData() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange=function(){
    if (this.readyState == 4 && this.status == 200){
      document.getElementById("WaterLevel").innerHTML=
      this.responseText;
    }
  };
  xhttp.open("GET", "readLevel", true);
  xhttp.send();
}
</script>

</body>
</html>
)=====";

void handleRoot(){
 String s = MAIN_page; //Read HTML contents
 server.send(200, "text/html", s); //Send web page
}

void handleLevel(){
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig,HIGH);
  delayMicroseconds(10);
  digitalWrite(trig,LOW);

  duration=pulseIn(echo, HIGH);
  distance=44-(float)duration*0.034/2;
  Serial.print("Distance: ");
  Serial.println(distance);
  String WaterLevel= String(distance);

  if(WiFi.status()==WL_CONNECTED){
    HTTPClient http;
    http.begin(URL);
    http.addHeader("Content-Type", "application/json");
    String httpRequestData=String("{\"value1\":\"")+ distance + "\"}";
    int httpResponseCode = http.POST(httpRequestData);
    while(httpResponseCode!=200){
      httpResponseCode = http.POST(httpRequestData);
      Serial.print("HTTP Response Code: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  }
  
  if (distance > 40){
    digitalWrite(BUZZER_PIN, HIGH);
    delay(5000);
    digitalWrite(BUZZER_PIN, LOW);
  }
  
  server.send(200, "text/plain", WaterLevel);
}

void setup() {
  Serial.begin(115200);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

   server.on("/", handleRoot);      //This is display page
   server.on("/readLevel", handleLevel);

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  
  server.handleClient();
  delay(1);
} 
