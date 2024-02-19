// Servos
#include <Servo.h>

// DHT
#include <DHT.h>

// WiFi
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>


// Servos
#define LEFT_SERVO 14
#define MIDDLE_SERVO 12
#define RIGHT_SERVO 13
Servo servo;
int DOWN = 170;
int NEUTRAL = 130;
int UP = 90;

// FSRs
#define LEFT_FSR 36
#define MIDDLE_FSR 39
#define RIGHT_FSR 34

// DHT
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
float previousHumidity;
float previousTemperature;

// Photoresistor
#define LIGHTPIN 35
int lightInit;  // initial value
int ADJUSTLIGHT = 42;

// Microphone
#define MICROPHONE 32

// WiFi
const char* ssid = "Owo";
const char* password = "yahoo123";
String serverName = "http://172.20.10.2";
WebServer server(80);


void setup() {
  // Serial
  Serial.begin(115200);  // originally had 9600

  // Servos
  servo.attach(LEFT_SERVO);
  servo.attach(MIDDLE_SERVO);
  servo.attach(RIGHT_SERVO);

  // DHT
  dht.begin();
  previousHumidity = dht.readHumidity();
  previousTemperature = dht.readTemperature();

  // Photoresistor
  pinMode(LIGHTPIN, INPUT);
  lightInit = analogRead(LIGHTPIN);

  // Microphone
  pinMode(MICROPHONE, INPUT);

  // WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi... ");
  }
  Serial.println("Connected to WiFi");
  Serial.println(WiFi.localIP());

  // Set up routes
  server.on("/", HTTP_GET, handleRoot);

  // Start server
  server.enableCORS();
  server.begin();
  Serial.println("HTTP server started");

  // Initialization
  mechanicalInit();
}

void loop() {
  // Servos

  // FSRs
  int leftForce = analogRead(LEFT_FSR);
  Serial.print("The left force sensor value = ");
  Serial.println(leftForce);

  int middleForce = analogRead(MIDDLE_FSR);
  Serial.print("The middle force sensor value = ");
  Serial.println(middleForce);

  int rightForce = analogRead(RIGHT_FSR);
  Serial.print("The right force sensor value = ");
  Serial.println(rightForce);

  // DHT (requires 1000ms delay minimum)
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();  // in Celsius

  if ((!(isnan(humidity))) && (!(isnan(temperature)))) { // if neither are nan
    previousHumidity = humidity;
    previousTemperature = temperature;
  } else {
    humidity = previousHumidity;
    temperature = previousTemperature;
  }
  float heatIndex = dht.computeHeatIndex(temperature, humidity, false);
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.print("%\nHumidity: ");
  Serial.print(temperature);
  Serial.print("°C\n");
  Serial.print("Heat index: ");
  Serial.print(heatIndex);
  Serial.println("°C");

  // Photoresistor
  int lightVal = analogRead(LIGHTPIN);
  //lightVal = lightVal/300*100;
  Serial.print("Light Level: ");
  int lightLevel = lightVal / ADJUSTLIGHT;
  Serial.println(lightLevel);
  
  // Microphone
  int minSound = 1024;
  int maxSound = 0;
  for (int i = 0; i < 1000; ++i) {
    int soundVal = analogRead(MICROPHONE);
    minSound = min(minSound, soundVal);
    maxSound = max(maxSound, soundVal);
  }
  int delta = maxSound - minSound;
  Serial.print("Sound: ");
  Serial.println(delta);

  // WiFi
  server.handleClient();
  handleRoot();
  delay(250);
}








// leftForce, middleForce, rightForce
// humidity, temperature, lightval, delta
// delta is the sound value!


void mechanicalInit() {
  // ALL UP
  servo.write(LEFT_SERVO, UP);
  servo.write(MIDDLE_SERVO, UP);
  servo.write(RIGHT_SERVO, UP);
  delay(2000);
  // ALL DOWN
  servo.write(LEFT_SERVO, DOWN);
  servo.write(MIDDLE_SERVO, DOWN);
  servo.write(RIGHT_SERVO, DOWN);
  delay(2000);
  // LU MD RU
  servo.write(LEFT_SERVO, UP);
  servo.write(MIDDLE_SERVO, DOWN);
  servo.write(RIGHT_SERVO, UP);
  delay(2000);
  // LD MU RD
  servo.write(LEFT_SERVO, DOWN);
  servo.write(MIDDLE_SERVO, UP);
  servo.write(RIGHT_SERVO, DOWN);
  delay(2000);
  // ALL NEUTRAL
  servo.write(LEFT_SERVO, NEUTRAL);
  servo.write(MIDDLE_SERVO, NEUTRAL);
  servo.write(RIGHT_SERVO, NEUTRAL);
  delay(2000);
}

void handleRoot() {
    // Create a JSON document
    StaticJsonDocument<200> doc;

    // Populate the JSON document
    doc["leftForce"] = analogRead(LEFT_FSR);
    doc["middleForce"] = analogRead(MIDDLE_FSR);
    doc["rightForce"] = analogRead(RIGHT_FSR);
    doc["humidity"] = dht.readHumidity();
    doc["temperature"] = dht.readTemperature();
    doc["heatIndex"] = dht.computeHeatIndex(dht.readTemperature(), dht.readHumidity(), false);
    doc["lightVal"] = analogRead(LIGHTPIN)/ADJUSTLIGHT; // test
    doc["sound"] = calculateDelta();

    // Serialize JSON document to a String
    String jsonString;
    serializeJson(doc, jsonString);

    // Set content type
    server.sendHeader("Content-Type", "application/json");
    server.send(200, "application/json", jsonString);
}

int calculateDelta() {
    int minSound = 1024;
    int maxSound = 0;
    for (int i = 0; i < 1000; ++i) {
        int soundVal = analogRead(MICROPHONE);
        minSound = min(minSound, soundVal);
        maxSound = max(maxSound, soundVal);
    }
    return maxSound - minSound;
}
