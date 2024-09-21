#define BLYNK_TEMPLATE_ID "TMPL6CqaW-fmk"
#define BLYNK_TEMPLATE_NAME "ESP32"
#define BLYNK_AUTH_TOKEN "XjuPF2my3B-eLx51Js3H0cDuNq1B8201"

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial
#define DHT_PIN 4

#include <Arduino.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHTesp.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <time.h>

// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Define the pulse sensor settings
const int pulsePin = 32; // the pulse sensor pin
const int ledPin = 2;    // the LED pin
int pulseValue;          // the pulse sensor value
int bpm;                 // the heart rate in beats per minute

// Your WiFi credentials.
// Set password to "" for open networks.
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "iPhone 14 Pro Max";
char pass[] = "87654321";
BlynkTimer timer;

// Firebase configuration
#define DATABASE_URL "https://smart-first-aid-4e3b8-default-rtdb.firebaseio.com/"
#define API_KEY "AIzaSyAB6LZag9FmU2AEpEU3UuYXu5_feD6Z7ys"
#define USER_EMAIL "esp32@gmail.com"
#define USER_password "esp32user"

// Define Firebase objects
FirebaseData fbdo;
FirebaseAuth firebaseAuth;
FirebaseConfig firebaseConfig;
DHTesp dht;
// Function to configure NTP
void configTimeForNTP()
{
  const char *ntpServer = "pool.ntp.org"; // NTP server
  const long gmtOffset_sec = 6 * 3600;    // Adjust for GMT+6 (or your local timezone)
  const int daylightOffset_sec = 0;       // Adjust for daylight saving time if applicable

  // Set up the NTP server for time synchronization
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  // Wait for time synchronization
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "Time synchronized: %Y-%m-%d %H:%M:%S");
}

// Function to get current epoch time
unsigned long getTime()
{
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to get local time");
    return 0;
  }
  time(&now);
  return now;
}

void setup()
{
  // Start the serial communication
  Serial.begin(115200);

  // Connect to WiFi
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Connect to Blynk
  Blynk.begin(auth, ssid, pass);
  while (!Blynk.connected())
  {
    Serial.println("Connecting to Blynk...");
    delay(1000);
  }
  Serial.println("Connected to Blynk");

  // Set up the pulse sensor
  pinMode(pulsePin, INPUT);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  dht.setup(DHT_PIN, DHTesp::DHT11); // GPIO 4

  // Configure NTP time synchronization
  configTimeForNTP();

  // Firebase configuration
  firebaseConfig.api_key = API_KEY;
  firebaseAuth.user.email = USER_EMAIL;
  firebaseAuth.user.password = USER_password;
  firebaseConfig.database_url = DATABASE_URL;
  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(4096);

  // Set the token callback function
  firebaseConfig.token_status_callback = tokenStatusCallback;
  firebaseConfig.max_token_generation_retry = 5;

  Firebase.begin(&firebaseConfig, &firebaseAuth);
}

void loop()
{

  // Get the current time
  unsigned long timestamp = getTime();

  // Read the temperature and humidity

  delay(dht.getMinimumSamplingPeriod());

  float humidity = dht.getHumidity();
  float temperature = dht.getTemperature();

  Serial.print("Temperature (C): ");
  Serial.print(temperature, 1);
  Serial.print("°C");
  Serial.print("\tHumidity: ");
  Serial.print(humidity, 1);
  Serial.println("%");

  // Send the temperature and humidity to Blynk
  Blynk.virtualWrite(V1, temperature);
  Blynk.virtualWrite(V2, humidity);

  // Read the pulse sensor value
  pulseValue = analogRead(pulsePin);

  // Detect the pulse
  if (pulseValue > 600)
  {
    digitalWrite(ledPin, HIGH); // turn on the LED
    delay(200);                 // wait for a short time
    digitalWrite(ledPin, LOW);  // turn off the LED
    bpm = 60000 / pulseValue;   // calculate the heart rate in beats per minute
    Serial.print("Heart rate: ");
    Serial.print(bpm);
    Serial.println(" BPM");

    // Send the heart rate to Blynk
    Blynk.virtualWrite(V0, bpm);

    // Create Firebase JSON object
    FirebaseJson json;
    json.set("/bpm", String(bpm));
    json.set("/temperature", String(temperature));
    json.set("/humidity", String(humidity));
    json.set("/timestamp", String(timestamp));

    // Send data to Firebase
    String parentPath = "/ESP32Data/" + String(timestamp);
    if (Firebase.ready())
    {
      if (Firebase.RTDB.setJSON(&fbdo, parentPath.c_str(), &json))
      {
        Serial.println("Data sent to Firebase");
      }
      else
      {
        Serial.println(fbdo.errorReason());
      }
    }

    delay(300);

    // Print the heart rate on the serial monitor
    String message = "Heart rate: " + String(bpm) + " BPM";
    Serial.println(message);
  }

  // Run the Blynk loop
  Blynk.run();
}