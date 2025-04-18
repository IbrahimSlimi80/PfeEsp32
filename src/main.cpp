#include <Arduino.h>

#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif

#include <Firebase_ESP_Client.h>
#include <FirebaseJson.h>
#include <time.h>
#include "DHT.h"  // Include the DHT sensor library

// WiFi credentials
#define WIFI_SSID "YourWiFiSSID"
#define WIFI_PASSWORD "YourWiFiPassword"

// Firebase credentials
#define API_KEY "Your Firebase Web API Key"
#define DATABASE_URL "https://your-project-id.firebaseio.com/"

// Firebase user credentials (used to authenticate to Firebase)
#define USER_EMAIL "yourfirebaseuser@example.com"
#define USER_PASSWORD "yourUserPassword"

// DHT sensor settings
#define DHTPIN 14        // GPIO14 (D14)
#define DHTTYPE DHT11    // DHT11 sensor type

DHT dht(DHTPIN, DHTTYPE);  // Initialize DHT sensor

// Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;
bool isSendingData = true;  // Variable to control whether data is being sent or not
bool lastSendingState = true;  // Variable to track the last state of isSendingData

// Initialize NTP time
void initTime() {
  configTime(0, 0, "pool.ntp.org");
  Serial.print("Waiting for NTP time sync");
  while (!time(nullptr)) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("\nTime synchronized.");
}

// Get current date and time string
String getCurrentDateTime() {
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    char strftime_buf[64];
    strftime(strftime_buf, sizeof(strftime_buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
    return String(strftime_buf);
  }
  return "Time not available";
}

// Function to check the control variable in Firebase
void checkControlVariable() {
  String controlPath = "/control";  // Path to control variable in Firebase

  // Use Firebase.RTDB.getString() to fetch the data
  if (Firebase.RTDB.getString(&fbdo, controlPath)) {
    String controlValue = fbdo.stringData();
    if (controlValue == "stop" && isSendingData) {
      isSendingData = false;  // Stop sending data
      Serial.println("Data sending stopped.");
    } else if (controlValue == "send" && !isSendingData) {
      isSendingData = true;   // Start sending data
      Serial.println("Data sending started.");
    }
  } else {
    Serial.println("Failed to read control value.");
    Serial.println("REASON: " + fbdo.errorReason());
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }

  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  // Initialize DHT sensor
  dht.begin();

  // Initialize time
  initTime();

  // Firebase config
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  // Set user credentials
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // Wait for sign-in
  while (auth.token.uid == "") {
    Serial.print(".");
    delay(300);
  }

  Serial.println();
  Serial.println("Firebase authentication successful.");
  signupOK = true;
}

void loop() {
  checkControlVariable();  // Check the control variable in Firebase

  // Only log if the sending state has changed
  if (isSendingData != lastSendingState) {
    lastSendingState = isSendingData;
    if (isSendingData) {
      Serial.println("Data sending started.");
    } else {
      Serial.println("Data sending stopped.");
    }
  }

  // Check if it's time to send data (every 15 seconds)
  if (Firebase.ready() && signupOK && isSendingData &&
      (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)) {
    
    sendDataPrevMillis = millis();

    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    if (isnan(temperature) || isnan(humidity)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }

    String currentDateTime = getCurrentDateTime();
    String tempPath = "sensors/temperature/";
    String humidPath = "sensors/humidity/";

    // Push temperature data to Firebase
    FirebaseJson tempData;
    tempData.set("value", temperature);
    tempData.set("datetime", currentDateTime);

    if (Firebase.RTDB.pushJSON(&fbdo, tempPath, &tempData)) {
      String tempKey = fbdo.pushName();
      Serial.println("Temperature write: PASSED");
      Serial.println("KEY: " + tempKey);
      Serial.println("VALUE: " + String(temperature) + ", DateTime: " + currentDateTime);
    } else {
      Serial.println("Temperature write: FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    // Push humidity data to Firebase
    FirebaseJson humidData;
    humidData.set("value", humidity);
    humidData.set("datetime", currentDateTime);

    if (Firebase.RTDB.pushJSON(&fbdo, humidPath, &humidData)) {
      String humidKey = fbdo.pushName();
      Serial.println("Humidity write: PASSED");
      Serial.println("KEY: " + humidKey);
      Serial.println("VALUE: " + String(humidity) + ", DateTime: " + currentDateTime);
    } else {
      Serial.println("Humidity write: FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
  }

  // Small delay to prevent flooding Firebase
  delay(2000);  // Delay added to prevent too frequent requests to Firebase
}
