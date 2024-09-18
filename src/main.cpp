#define BLYNK_TEMPLATE_ID "TMPL6CqaW-fmk"
#define BLYNK_TEMPLATE_NAME "ESP32"
#define BLYNK_AUTH_TOKEN "XjuPF2my3B-eLx51Js3H0cDuNq1B8201"

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// Define the pulse sensor settings
const int pulsePin = 32; // the pulse sensor pin
const int ledPin = 2;    // the LED pin
int pulseValue;          // the pulse sensor value
int bpm;                 // the heart rate in beats per minute

// Your WiFi credentials.
// Set password to "" for open networks.
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "What";
char pass[] = "anything321";
BlynkTimer timer;

// Initialize the LCD
LiquidCrystal_I2C lcd(0x27, 16, 2); // Set the LCD address to 0x27 for a 16 chars and 2 line display

// Function declaration
void scanI2CDevices();

void scanI2CDevices()
{
  byte error, address;
  int nDevices;

  Serial.println("Scanning for I2C devices...");

  nDevices = 0;
  for (address = 1; address < 127; address++)
  {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0)
    {
      Serial.print("I2C device found at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.print(address, HEX);
      Serial.println("  !");

      nDevices++;
    }
    else if (error == 4)
    {
      Serial.print("Unknown error at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.println(address, HEX);
    }
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("done\n");
}

void setup()
{
  // Start the serial communication
  Serial.begin(115200);

  // Initialize I2C
  Wire.begin();

  // Scan for I2C devices
  scanI2CDevices();

  // Initialize the LCD
  lcd.init();
  lcd.backlight();
  lcd.print("Connecting...");

  // Connect to WiFi
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Connecting to WiFi...");
    lcd.setCursor(0, 1);
    lcd.print(".");
  }
  Serial.println("Connected to WiFi");
  lcd.clear();
  lcd.print("WiFi Connected");

  // Connect to Blynk
  Blynk.begin(auth, ssid, pass);
  while (!Blynk.connected())
  {
    Serial.println("Connecting to Blynk...");
    lcd.setCursor(0, 1);
    lcd.print(".");
    delay(1000);
  }
  Serial.println("Connected to Blynk");
  lcd.clear();
  lcd.print("Blynk Connected");

  // Set up the pulse sensor
  pinMode(pulsePin, INPUT);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
}

void loop()
{
  // Read the pulse sensor value
  pulseValue = analogRead(pulsePin);

  // Detect the pulse
  if (pulseValue > 600)
  {
    digitalWrite(ledPin, HIGH); // turn on the LED
    delay(250);                 // wait for a short time
    digitalWrite(ledPin, LOW);  // turn off the LED
    bpm = 60000 / pulseValue;   // calculate the heart rate in beats per minute
    Serial.print("Heart rate: ");
    Serial.print(bpm);
    Serial.println(" BPM");

    // Send the heart rate to Blynk
    Blynk.virtualWrite(V0, bpm);
    delay(500);

    // Print the heart rate on the serial monitor
    String message = "Heart rate: " + String(bpm) + " BPM";
    Serial.println(message);

    // Display the heart rate on the LCD
    lcd.clear();
    lcd.print("Heart rate:");
    lcd.setCursor(0, 1);
    lcd.print(String(bpm) + " BPM");
  }

  // Run the Blynk loop
  Blynk.run();
}