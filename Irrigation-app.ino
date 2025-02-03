#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>

// Wi-Fi credentials
#define ssid "Maales" //my wifi information, paste yours
#define password "burkay04"

// Firebase configuration
#define FIREBASE_HOST "https://irrigation-app-502f4-default-rtdb.firebaseio.com/"// paste your full URL with "https://"
#define FIREBASE_AUTH "" // paste your own code here

// Firebase objects
FirebaseData firebaseData;
FirebaseAuth auth;
FirebaseConfig config;


void connectToWiFi();

const int prob = A0; //The only analog pin a0, there is a soil sensor connected to
const int motor = 2; //D4 pin, there is a relay connected to 

// Objects and Variables
float percentage;
String water;

void setup() {
  Serial.begin(9600);
  pinMode(motor, OUTPUT_OPEN_DRAIN);
  digitalWrite(motor, HIGH);
  water = "false";

  //For the cloud;
  connectToWiFi();
  config.host = FIREBASE_HOST;
  config.signer.tokens.legacy_token = FIREBASE_AUTH;
  // Initialize Firebase
  Firebase.begin(&config, &auth);

  if(FIREBASE_AUTH==""){
    Serial.println("FILL FIREBASE CODE NAMED \"FIREBASE_AUTH\" ");
  }
}


unsigned long previousMillis = 0;
unsigned long wateringStartMillis = 0;
const long readInterval = 2000;  // Interval for reading sensor data (16s)
const long wateringDuration = 10000;  // Watering duration (10s)

void loop() {
    unsigned long currentMillis = millis();

    // Read sensor and update Firebase at intervals
    if (currentMillis - previousMillis >= readInterval) {
      previousMillis = currentMillis;
      
      // Calculate soil humidity percentage
      float percentage = ((1024.0 - float(analogRead(prob))) / 1024.0) * 100;
      
      // Communication with the cloud
      if (Firebase.RTDB.getString(&firebaseData, "/water")) {
        String data = firebaseData.stringData();
        
        if(data == "false"){
          water = "false";
        }else{
          water = "true";
        }
        
        if (Firebase.RTDB.setString(&firebaseData, "/percentage", String(percentage))) {
            Serial.println("Percentage parameter updated.");
        } else {
            Serial.println("Failed to update percentage parameter.");
        }
        Serial.println("\nWater data obtained: " + data + ".");
        Serial.println("Percentage: " + String(percentage));
      } 
      else {
          Serial.println("Failed to read data from Firebase.");
      }
      
      // Watering condition
      if ((percentage <= 40 && percentage > 1) || water == "true") {
        water = "true";
        wateringStartMillis = currentMillis;
        
        if (Firebase.RTDB.setString(&firebaseData, "/water", "true")) {
            Serial.println("Watering started.");
        } else {
            Serial.println("Failed to update water parameter.");
        }
        digitalWrite(motor, LOW);
      }
    }
    
    // Stop watering after duration
    if (water == "false" || (currentMillis - wateringStartMillis >= wateringDuration)) {
      water = "false";
      
      if (Firebase.RTDB.setString(&firebaseData, "/water", "false")) {
          Serial.println("Watering stopped.");
      } else {
          Serial.println("Failed to update water parameter.");
      }
      digitalWrite(motor, HIGH);
    }
}



//with using delay() func
/*
void loop() {
  // Calculate soil humidity percentage
  percentage = ((1024.0 - float(analogRead(prob))) / 1024.0) * 100;
  
  //Communication with the cloud
  if(Firebase.RTDB.getString(&firebaseData, "/water")){
    String data = firebaseData.stringData();
        
    if(data == "false"){
      water = "false";
    }else{
      water = "true";
    }
    if (Firebase.RTDB.setString(&firebaseData, "/percentage", String(percentage))) {
      Serial.println("Parameter updated.");
    } else {
      Serial.println("Failed to update parameter.");
    }
    Serial.println("\nData obtained:"+data+".");
    Serial.println("Percentage: "+ String(percentage));
    
  }
  else {
    Serial.println("Failed to read data from Firebase.");
  }

  // Watering condition
  if ((percentage <= 40 && percentage > 1 ) || (water == "true")) {
    
    if (Firebase.RTDB.setString(&firebaseData, "/water", "true")) {
      Serial.println("Parameter updated.");
    } else {
      Serial.println("Failed to update parameter.");
    }
    Serial.println("Watering...");
    Serial.println("Water parameter=" + water);
    digitalWrite(motor, LOW);
    delay(10000);
    
    if (Firebase.RTDB.setString(&firebaseData, "/water", "false")) {
      Serial.println("Water parameter updated.");
    } else {
      Serial.println("Failed to update parameter.");
    }

  }else{
    digitalWrite(motor, HIGH);
  }

  delay(16000);
}
*/

void connectToWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("\nConnecting");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(200);
  }

  Serial.println("\nWiFi connected");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());
}