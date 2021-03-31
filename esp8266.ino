#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <PubSubClient.h>
const char* ssid = "";
const char* password = "";
#define IP  "183.230.40.39"
#define MQTT_PORT 6002
#define PRODUCT_ID    "404894"
#define DEVICE_ID    "684949185"
#define API_KEY    "dvGX=5TeA=dvqpt41yMSOrN0we0="
#define HOST  "api.heclouds.com"

String readString;
WiFiClient espClient;
PubSubClient client(espClient);
HTTPClient http;
void callback(char *topic, byte * payload, unsigned int length) {
  String luminance = "";
  for (int i = 0; i < length; i++) {
    luminance += (char) payload[i];
  }

  if (luminance == "r") {
    digitalWrite(12, HIGH);
    digitalWrite(14, LOW);
    digitalWrite(13, LOW);
  }

  if (luminance == "g") {
    digitalWrite(13, HIGH);
    digitalWrite(14, LOW);
    digitalWrite(12, LOW);
  }

  if (luminance == "b") {
    digitalWrite(14, HIGH);
    digitalWrite(12, LOW);
    digitalWrite(13, LOW);
  }

  if (luminance == "m") {

    digitalWrite(5, LOW);
    delay(100);
    digitalWrite(5, HIGH);

  }

  if (luminance != "r" && luminance != "g" && luminance != "b" && luminance != "m" ) {
    analogWrite(12, luminance.toInt());
  }


  //  analogWrite(12, luminance.toInt());
  //  Serial.print(luminance);
}

#define echoPin 4
#define trigPin 0
void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);


  pinMode(14, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);
  pinMode(5, OUTPUT);
  digitalWrite(5, HIGH);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  client.setServer(IP, MQTT_PORT);
  client.setCallback(callback);
  while (!client.connected()) {

    client.connect(DEVICE_ID, PRODUCT_ID, API_KEY);

  }
}



int distance() {
  long duration; // variable for the duration of sound wave travel
  int distance; // variable for the distance measurement



  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  // Clears the trigPin condition
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin HIGH (ACTIVE) for 10 microseconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  // Calculating the distance
  distance = duration * 0.034 / 2; // Speed of sound wave divided by 2 (go and back)
  // Displays the distance on the Serial Monitor
  //  Serial.print("Distance: ");
  //  Serial.print(distance);
  //  Serial.println(" cm");

  return distance;
}


void loop() {


  client.loop();

  Serial.println(distance());
  uploadData("distance", distance());
  delay(1000);


}


void uploadData(String dataName, int dataValue) {
  Serial.print(dataName);
  Serial.print("  ");
  Serial.println(dataValue);
  DynamicJsonDocument doc(100); //定义动态json文档


  JsonArray arr1 = doc.createNestedArray("datastreams"); //创建一个嵌套Json数组

  JsonObject node_1 = arr1.createNestedObject(); //创建一个嵌套Json对象

  node_1["id"] = dataName.c_str(); //String转char

  JsonArray arr2 = node_1.createNestedArray("datapoints"); //创建一个嵌套Json数组

  JsonObject node_1_1 = arr2.createNestedObject(); //创建一个嵌套Json对象
  node_1_1["value"] = dataValue;

  String output = "";
  serializeJson(doc, output); //序列化Json
  Serial.println(output);

  http.begin("http://api.heclouds.com/devices/" + String(DEVICE_ID) + "/datapoints");  //onenet上报数据点api
  http.addHeader("Content-Type", "application/json"); //设置请求头为application/json
  http.addHeader("api-key", API_KEY); //设置api-key
  int httpCode = http.POST(output);  //提交请求后的状态码
  String payload = http.getString(); //获取onenet服务器响应

  Serial.println(httpCode);
  Serial.println(payload);
  http.end(); //关闭连接

}
