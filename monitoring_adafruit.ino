#include <Wire.h>
#include <WiFi.h>
#include <LiquidCrystal_I2C.h>
#include <MQ2.h>
#include <HTTPClient.h>
#include "DHT.h"
#include "esp_wpa2.h"
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#define TOUTCH_PIN T8
#define DHTPIN 4
#define DHTTYPE DHT22

//-------------- if eduroam use this -----------------
//#define EAP_IDENTITY "ibrahim@students.undip.ac.id"
//#define EAP_PASSWORD "12QWqw=+"
//const char* ssid = "eduroam";
//------------ if normal wifi use this ---------------
#define WLAN_SSID       "PRO_Atas"
#define WLAN_PASS       "hiyahiyahiya"

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "maghassiz"
#define AIO_KEY         "1d52a87e10724b1893ce3a4c3c23bfd9"


const int mqPin = 35;
const int butPin = 32;
int co, button, out;
int touch_value = 100;
float h, t;
String k;
DHT dht(DHTPIN, DHTTYPE);

WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Publish Suhu = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temperatur");
Adafruit_MQTT_Publish Kelembapan = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/humidity");
Adafruit_MQTT_Publish Api = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/fire");
void MQTT_connect();
MQ2 mq2(mqPin);

// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);



void setup()
{
  byte error = 0;
  // initialize the LCD
  lcd.init();
  dht.begin();
  mq2.begin();
  lcd.backlight();
  delay(10);

  //------------ if normal wifi use this ---------------
 Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");}
  //----------------------------------------------------

  //-------------- if eduroam use this -----------------
//  WiFi.disconnect(true);  //disconnect from wifi to set new wifi connection
//  error += esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)EAP_IDENTITY, strlen(EAP_IDENTITY));
//  error += esp_wifi_sta_wpa2_ent_set_username((uint8_t *)EAP_IDENTITY, strlen(EAP_IDENTITY));
//  error += esp_wifi_sta_wpa2_ent_set_password((uint8_t *)EAP_PASSWORD, strlen(EAP_PASSWORD));
//  if (error != 0) {
//    lcd.setCursor(0, 0);
//    lcd.print("SORRY");
//  }
//  WiFi.enableSTA(true);
//  esp_wpa2_config_t config = WPA2_CONFIG_INIT_DEFAULT();
//  if (esp_wifi_sta_wpa2_ent_enable(&config) != ESP_OK) {
//    lcd.clear();
//    lcd.setCursor(0, 0);
//    lcd.print("SORRY BRO");
//  }
//  WiFi.begin(WLAN_SSID);
//  WiFi.setHostname("RandomHostname");
//  while (WiFi.status() != WL_CONNECTED) {
//    delay(500);
//    lcd.setCursor(0, 0);
//    lcd.print("CONNECTING..");
//  }
  //----------------------------------------------------

  lcd.setCursor(0, 0);
  lcd.print("   MONITORING");
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP());
  delay(3000);
  lcd.clear();
}

void loop()
{
  MQTT_connect();
  lcd.clear();
  float* values = mq2.read(false);
  touch_value = touchRead(TOUTCH_PIN);
  h = dht.readHumidity();
  t = dht.readTemperature();
  co = mq2.readCO();
  if (isnan(h) || isnan(t)) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Failed sensor!"));
    return;
  }
  if (! Suhu.publish(t)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }

  if (! Kelembapan.publish(h)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }

  Api.publish(h);
  
  if (t > 40 || co > 1000 || touch_value < 25) {
    k = "DANGER";
    out = 1;
  }
  else {
    k = "SAFE";
    out = 0;
  }
  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.setCursor(2, 0);
  lcd.print(t);
  lcd.setCursor(6, 0);
  lcd.print("C");
  lcd.setCursor(9, 0);
  lcd.print("H:");
  lcd.setCursor(11, 0);
  lcd.print(h);
  lcd.setCursor(15, 0);
  lcd.print("%");
  lcd.setCursor(0, 1);
  lcd.print("K:");
  lcd.setCursor(2, 1);
  lcd.print(k);
  lcd.setCursor(9, 1);
  lcd.print(co);
  lcd.setCursor(13, 1);
  lcd.print("ppm");
  delay(60000);
}
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 1 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}
