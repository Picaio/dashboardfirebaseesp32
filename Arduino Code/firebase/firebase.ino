

#include <Arduino.h>
#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>


// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "NOMBRE WIFI"
#define WIFI_PASSWORD "CONTRASEÑA WIFI"

// Insert Firebase project API Key
#define API_KEY "API FIREBASE"

// Insert Authorized Email and Corresponding Password
#define USER_EMAIL "CORREO"
#define USER_PASSWORD "CONTRASEÑA"

// Insert RTDB URLefine the RTDB URL
#define DATABASE_URL "BASE DE DATOS"

// Define Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Variable to save USER UID
String uid;

// Variables to save database paths
String databasePath;
String tempPath;
String humPath;
String presPath;
String ledPath;

// BME280 sensor
//Adafruit_BME280 bme; // I2C
float temperature;
float humidity;
float pressure;
const int led = 4;//led
const int potPin = 34;//analogo read
int potValue = 0;

// Timer variables (send new readings every three minutes)
unsigned long sendDataPrevMillis = 0;
unsigned long timerDelay = 2000;

// Initialize WiFi
void initWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
  Serial.println();
}

// Write float values to the database
void sendFloat(String path, float value){
  if (Firebase.RTDB.setFloat(&fbdo, path.c_str(), value)){
    Serial.print("Writing value: ");
    Serial.print (value);
    Serial.print(" on the following path: ");
    Serial.println(path);
    Serial.println("PASSED");
    Serial.println("PATH: " + fbdo.dataPath());
    Serial.println("TYPE: " + fbdo.dataType());
  }
  else {
    Serial.println("FAILED");
    Serial.println("REASON: " + fbdo.errorReason());
  }
}
void readData(String path){
  String readIncoming ="";
  if (Firebase.RTDB.getString(&fbdo, path.c_str())){
    Serial.println("PATH: " + fbdo.dataPath());
    Serial.println("TYPE: " + fbdo.dataType());
    if(fbdo.dataType()=="string"){
      readIncoming =fbdo.stringData();
       Serial.println("DATA: " + readIncoming);
      if(readIncoming=="ON"){
        digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
        }
        else{
         digitalWrite(led, LOW);   // turn the LED on (HIGH is the voltage level)
        }
     }
    }
  else {
    Serial.println("FAILED");
    Serial.println("REASON: " + fbdo.errorReason());
  }
}

void setup(){
  Serial.begin(115200);
  pinMode(led, OUTPUT);
  // Initialize BME280 sensor
  //initBME();
  initWiFi();

  // Assign the api key (required)
  config.api_key = API_KEY;

  // Assign the user sign in credentials
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  // Assign the RTDB URL (required)
  config.database_url = DATABASE_URL;

  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(4096);

  // Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  // Assign the maximum retry of token generation
  config.max_token_generation_retry = 5;

  // Initialize the library with the Firebase authen and config
  Firebase.begin(&config, &auth);

  // Getting the user UID might take a few seconds
  Serial.println("Getting User UID");
  while ((auth.token.uid) == "") {
    Serial.print('.');
    delay(1000);
  }
  // Print user UID
  uid = auth.token.uid.c_str();
  Serial.print("User UID: ");
  Serial.println(uid);

  // Update database path
  databasePath = "/UsersData/" + uid;

  // Update database path for sensor readings
  tempPath = databasePath + "/temperature"; // --> UsersData/<user_uid>/temperature
  humPath = databasePath + "/humidity"; // --> UsersData/<user_uid>/humidity
  presPath = databasePath + "/pressure"; // --> UsersData/<user_uid>/pressure
  ledPath = databasePath + "/led"; // --> UsersData/<user_uid>/led
}

void loop(){
  // Send new readings to database
  if (Firebase.ready() && (millis() - sendDataPrevMillis > timerDelay || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();
    

    // Get latest sensor readings
    potValue = analogRead(potPin);

    //prepare data
    temperature = potValue*0.1;
    humidity = potValue*0.2;
    pressure = potValue*0.3;


    // Send readings to database:
    sendFloat(tempPath, temperature);
    sendFloat(humPath, humidity);
    sendFloat(presPath, pressure);
    readData(ledPath);
  }
}
