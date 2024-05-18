#include <LCD_I2C.h>
#include <EEPROM.h>
#include <Wire.h>
#include <Adafruit_MCP3008.h>
#include <ESP8266WiFi.h>
const int relayPin = D4;
#include "ThingSpeak.h" // always include thingspeak header file after other header files and custom macros
char ssid[] = "iot";   // your network SSID (name) 
char pass[] = "012345678";   // your network password
int keyIndex = 0;            // your network key Index number (needed only for WEP)
WiFiClient  client;
unsigned long relayStartTime = 0;
unsigned long relayDuration = 5000; 
Adafruit_MCP3008 adc;
LCD_I2C lcd(0x27);
int pwm = 0;
String stat="";
float ch,dh,te;
const int VOLTAGE_SENSOR_1_CHANNEL = 0;
const int CURRENT_SENSOR_1_CHANNEL = 1;
#define AC_VOLTAGE_CALIB_INTERCEPT -0.04 // Adjust based on calibration testing
#define AC_VOLTAGE_CALIB_SLOPE 0.0405   // Adjust based on calibration testing
#define AC_CURRENT_CALIB_CONST 355.55   // Adjust based on calibration testing
#define DC_VOLTAGE_R1 30000.0 // Resistor values in divider (in ohms)
#define DC_VOLTAGE_R2 7500.0
#define REF_VOLTAGE 5.0
#define SAMPLES 1000 // Number of samples for RMS calculation
void setup() {
  Serial.begin(9600);
  adc.begin();
  lcd.begin();
  lcd.backlight();
  pinMode(D4, OUTPUT);
digitalWrite(D4,HIGH);
  // You can set up the PWM pin as needed
  //analogWrite(PWM_PIN, 0); // Example: 50% duty cycle
WiFi.mode(WIFI_STA); 
  ThingSpeak.begin(client);
  // Connect or reconnect to WiFi
  if(WiFi.status() != WL_CONNECTED){
    while(WiFi.status() != WL_CONNECTED){
      WiFi.begin(ssid, pass);  // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      Serial.print(".");
      delay(5000);     
    } 
    Serial.println("\nConnected.");
  }
  relayStartTime = millis();
}

void loop() {
   int stat = ThingSpeak.readIntField(1702429, 1);
   int tim = ThingSpeak.readIntField(1702429, 2);
if(stat==1)
{
  relayDuration=tim*1000;
  Serial.println(relayStartTime);
if (millis()-relayStartTime <= relayDuration) {
    // Turn on the relay
    digitalWrite(relayPin, LOW);

    // Record the start time
   
  }
  else {
     digitalWrite(relayPin, HIGH);
    }
}
else if(stat==2){
 relayStartTime = millis();
   digitalWrite(relayPin, HIGH);
  } 
  
  // Read AC voltage from channel 0
  float acVoltageRMS = calculateACVoltageRMS(0);

  // Read AC current from channel 1
  float acCurrentRMS = calculateACCurrentRMS(1);


  // Print the readings
  Serial.println("\nAC Voltage RMS: " + String(acVoltageRMS) + " V");
  Serial.println("AC Current RMS: " + String(acCurrentRMS) + " mAh");
//  Serial.println("DC Voltage: " + String(dcVoltage) + " V");

 lcd.setCursor(0, 0);
  lcd.print("AC V:");
  lcd.print(acVoltageRMS);
  lcd.setCursor(0, 1);
  lcd.print("AC mAh:");
   lcd.print(acCurrentRMS);
  delay(1000); // Display for 2 seconds

ThingSpeak.setField(1,acVoltageRMS);
  ThingSpeak.setField(2, acCurrentRMS);
  ThingSpeak.setField(3, acVoltageRMS*(acCurrentRMS/1000));
//  ThingSpeak.setField(4, dcVoltage);
    int xuk = ThingSpeak.writeFields(1013258, "QNRWO798ZZV2OEIL");
 
}

float 
calculateACVoltageRMS(int channel) {
  float sumSquared = 0;
int flag=0;
  for (int i = 0; i < SAMPLES; i++) {
    int adcValue = adc.readADC(channel);
//Serial.println(adcValue);
if((adcValue>(flag+5))||adcValue<(flag-5))
{
  sumSquared++;
  }
  flag=adcValue;
  }
//Serial.println(sumSquared);
  //float rms = sqrt(sumSquared / SAMPLES);
  sumSquared=sumSquared*2;
  if(sumSquared>300)
  {
    sumSquared=sumSquared-70;
    }
  return sumSquared; // Further calibrations for amplitude
}

float calculateACCurrentRMS(int channel) {
  float sumSquared = 0;
int flag=0;
  for (int i = 0; i < SAMPLES; i++) {
    int adcValue = adc.readADC(channel);
    Serial.println(adcValue);
if((adcValue>(flag+5))||adcValue<(flag-5))
     { sumSquared++;
      }
    flag=adcValue;
  }

  //float rms = sqrt(sumSquared / SAMPLES);
  return sumSquared/10;
}

float calculateDCVoltage(int channel) {
  int adcValue = adc.readADC(channel);
  float adcVoltage = (adcValue * REF_VOLTAGE) / 1024.0;
  return adcVoltage / (DC_VOLTAGE_R2 / (DC_VOLTAGE_R1 + DC_VOLTAGE_R2));
}
