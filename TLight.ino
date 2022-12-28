//Proyek Sistem Benam
//Iman Herlambang Suherman
//1806147924

//Memasukkan Library
#include <WiFi.h>
#include <WebServer.h>
#include <FastLED.h>
#include <stdlib.h>

// Define Variable statis
#define ONBOARD_LED 2
#define LED_PIN     5
#define NUM_LEDS    42
#define BRIGHTNESS  255
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define UPDATES_PER_SECOND 100

//Touch Variable
boolean touchStarted = false;
unsigned long touchTime = 0;
int threshold = 50;
int touchMinDuration = 1000; 
int count;
int count2;
int pallete_selector=1;

//Variabel FastLED
CRGB leds[NUM_LEDS];
CRGBPalette16 currentPalette;
TBlendType    currentBlending;

/* Put your SSID & Password */
const char* ssid = "TLight";  // Enter SSID here
const char* password = "12345678";  //Enter Password here

/* Put IP Address details */
IPAddress local_ip(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

WebServer server(80); //Port Wifi

//Define Variable Public
char hex[10];
String temp_hex = "";
char hex1[10];
String temp_hex1 = "";
char hex2[10];
char hex3[10]; 
String temp_hex2 = "";
char *pointah;
float nilai_timer = 0;
float timer_starts = 0;

bool LEDtoggle = LOW;
bool LEDtoggledualcolor = LOW;
bool LEDtimer = LOW;
bool LEDtimer_toggle = LOW;

void touched() {              //Variabel Ketika On/Off ditekan
  LEDtoggle=!LEDtoggle;
  LEDtoggledualcolor = LOW;
  LEDtimer = LOW;
  LEDtimer_toggle = LOW;
}
void touched_cycle() {        //Variabel Ketika Cycle ditekan
  pallete_selector=pallete_selector%16; 
  if (pallete_selector==1)strcpy(hex,"ffffff");
  else if(pallete_selector==2)strcpy(hex,"ff0000");
  else if(pallete_selector==3)strcpy(hex,"d52a00");
  else if(pallete_selector==4)strcpy(hex,"ab5500");
  else if(pallete_selector==5)strcpy(hex,"ab7f00");
  else if(pallete_selector==6)strcpy(hex,"abab00");
  else if(pallete_selector==7)strcpy(hex,"56d500");
  else if(pallete_selector==8)strcpy(hex,"00ff00");
  else if(pallete_selector==9)strcpy(hex,"00d52a");
  else if(pallete_selector==10)strcpy(hex,"00ab55");
  else if(pallete_selector==11)strcpy(hex,"0056aa");
  else if(pallete_selector==12)strcpy(hex,"0000ff");
  else if(pallete_selector==13)strcpy(hex,"2a00d5");
  else if(pallete_selector==14)strcpy(hex,"5500ab");
  else if(pallete_selector==15)strcpy(hex,"7f0081");
  else if(pallete_selector==16)strcpy(hex,"ab0055");
  else if(pallete_selector==0)strcpy(hex,"d5002b"); 
}

void setup() {
  strcpy(hex,"ffffff");       //Set Variabel awal agar nilai inisial tersimpan dan dapat ditampilkan di webserver
  strcpy(hex1,"ffffff");
  strcpy(hex2,"ffffff");
  strcpy(hex3,"ffffff");
  Serial.begin(115200);       //Set Serial untuk debugging

  WiFi.softAP(ssid, password);                  //Buat Access Point
  delay(2000);                                  //Delay 2 detik hingga access point terbuat
  WiFi.softAPConfig(local_ip, gateway, subnet); //Set COnfig
  delay(100);
  pinMode(ONBOARD_LED, OUTPUT);
  digitalWrite(ONBOARD_LED, LOW);

  //Set Interrupt untuk HTTP REST
  server.on("/", handle_OnConnect);
  server.on("/warna", handle_warna);
  server.on("/duawarna", handle_dua_warna);
  server.on("/delay", delay_satu_warna);
  server.on("/offled", handle_off);
  server.onNotFound(handle_NotFound);
  //Mulai Serer
  server.begin();
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(BRIGHTNESS);
  currentBlending = LINEARBLEND;
  Serial.println("HTTP server started");
}
void loop() {
  //FastLED.show();
  server.handleClient();
  if (LEDtoggle)                            //1 Warna
  {
    //digitalWrite(LED1pin, HIGH);
    for (int dot = 0; dot < NUM_LEDS; dot++) {
      leds[dot] = strtoul(hex, &(pointah), 16);
    }
    FastLED.show();
  }
  else if (LEDtoggledualcolor) {            //Transisi 2 Warna
    //    Serial.print("Dual Color Start");
    static uint8_t startIndex = 0;
    startIndex = startIndex + 1; /* motion speed */
    SetupDuaWarna();

    FillLEDsFromPaletteColors(startIndex);

    FastLED.show();
    FastLED.delay(1000 / UPDATES_PER_SECOND);
  }
  else if (LEDtimer) {                      //Mode Timer / Mode Que

    if ((millis() - timer_starts) >= nilai_timer) {
      //      Serial.print("Waktu = ");
      //      Serial.println(millis());
      digitalWrite(ONBOARD_LED, HIGH);
      delay(300);
      digitalWrite(ONBOARD_LED, LOW);
      delay(100);
      digitalWrite(ONBOARD_LED, HIGH);
      delay(200);
      digitalWrite(ONBOARD_LED, LOW);
      delay(100);
      digitalWrite(ONBOARD_LED, HIGH);
      delay(100);
      digitalWrite(ONBOARD_LED, LOW);
      delay(100);
      LEDtimer = LOW;
      LEDtimer_toggle = HIGH;
    }
    else if (millis() % 1000 >= 0 && millis() % 1000 <= 100) {
      digitalWrite(ONBOARD_LED, HIGH);
    }
    else {
      digitalWrite(ONBOARD_LED, LOW);
    }
  }

  else if (LEDtimer_toggle) {                   //Nyalakan lampu saat timer selesai
    for (int dot = 0; dot < NUM_LEDS; dot++) {
      leds[dot] = strtoul(hex3, &(pointah), 16);
    }
    FastLED.show();
  }
  else
  {
    //digitalWrite(LED2pin, LOW);
    for (int dot = 0; dot < NUM_LEDS; dot++) {
      leds[dot] = CRGB::Black;
    }
    FastLED.show();
  }
  //Perintah untuk memonitor pin 4 dan membaca capacitive touch
  int t = touchRead(4);
//  Serial.print("First=");
//  Serial.print(t);
  if (t < threshold && !touchStarted) { // ketika baru disentuh
    touchStarted = true;
    touchTime = millis();
    //    Serial.println("Awal Sentuhan");
    //    Serial.print("millis = ");
    //      Serial.println(touchTime);
  }
  else if (t >= threshold && touchStarted && count > 120) { // sentuhan selesai
    if (millis() - touchTime > touchMinDuration) {
      //      Serial.println("Akhir Sentuhan");
      //      Serial.print("millis = ");
      //      Serial.println(millis());
      touched();
      touchStarted = false;
      count = 0;
    }
  }
  else if (t < threshold) {     //Penghitungan filter untuk mencegah adanya sentuhan yang tidak dikehendaki
    count++;
  }
  else {                        //Reset perhitungan ketika kondisi bukan sentuhan sesungguhnya
    count = 0;
  }

  //Perintah untuk memonitor pin 27 dan membaca capacitive touch
  int t2 = touchRead(27);
//  Serial ntln(t2);
  if (t2 < threshold && !touchStarted) { // ketika baru disentuh
    touchStarted = true;
    touchTime = millis();
    //    Serial.println("Awal Sentuhan");
    //    Serial.print("millis = ");
    //      Serial.println(touchTime);
  }
  else if (t2 >= threshold && touchStarted && count2 > 120) { // sentuhan selesai
    if (millis() - touchTime > touchMinDuration) {
      //      Serial.println("Akhir Sentuhan");
      //      Serial.print("millis = ");
      //      Serial.println(millis());
      pallete_selector++;
      touched_cycle();
      touchStarted = false;
      count2 = 0;
    }
  }
  else if (t2 < threshold) {      //Penghitungan filter untuk mencegah adanya sentuhan yang tidak dikehendaki
    count2++;
  }
  else {                          //Reset perhitungan ketika kondisi bukan sentuhan sesungguhnya
    count2 = 0;
  }
}

void handle_OnConnect() {       //Interrupt untuk kondisi HTTP Connect
  Serial.println("All COnnected | Starting");
  server.send(200, "text/html", SendHTML());
}

void handle_warna() {         //Interrupt untuk kondisi HTTP dengan request warna?hex=
  LEDtoggle = HIGH;
  LEDtoggledualcolor = LOW;
  LEDtimer = LOW;
  for (int i = 0; i < server.args(); i++) {
    //hex = server.arg("hex").toCharArray(char_array, str_len);
    //Serial.print(server.args());
    temp_hex = server.arg("hex");
    temp_hex.toCharArray(hex, server.arg("hex").length() + 1);
  }
  Serial.print("Nilai Hex: #");
  Serial.println(hex);
  server.send(200, "text/html", SendHTML());
}

void delay_satu_warna() {     //Interrupt untuk kondisi HTTP dengan request delay?hex=&time=
  LEDtoggle = LOW;
  LEDtoggledualcolor = LOW;
  LEDtimer = HIGH;
  LEDtimer_toggle = LOW;
  String timer_temp = "";
  char timer_char[10] = "";
  for (int i = 0; i < server.args(); i++) {
    //hex = server.arg("hex").toCharArray(char_array, str_len);
    //Serial.print(server.args());
    temp_hex = server.arg("hex");
    temp_hex.toCharArray(hex3, server.arg("hex").length() + 1);
    timer_temp = server.arg("time");
    timer_temp.toCharArray(timer_char, server.arg("time").length() + 1);
  }
  nilai_timer = atof(timer_char) * 1000;
  Serial.print("Nilai Hex: #");
  Serial.println(hex3);
  Serial.print("Nilai timer: ");
  Serial.print(nilai_timer);
  Serial.println(" ms");
  timer_starts = millis();
  server.send(200, "text/html", SendHTML());
}

void handle_dua_warna() {         //Interrupt untuk kondisi HTTP dengan request duawarna?hex1=&hex2
  LEDtoggledualcolor = HIGH;
  LEDtoggle = LOW;
  LEDtimer = LOW;
  for (int i = 0; i < server.args(); i++) {
    temp_hex1 = server.arg("hex1");
    temp_hex1.toCharArray(hex1, server.arg("hex1").length() + 1);
    temp_hex2 = server.arg("hex2");
    temp_hex2.toCharArray(hex2, server.arg("hex2").length() + 1);
  }
  Serial.print("Nilai Hex1: #");
  Serial.println(hex1);
  Serial.print("Nilai Hex2: #");
  Serial.println(hex2);
  server.send(200, "text/html", SendHTML());
}

void handle_off() {             //Interrupt untuk kondisi HTTP dengan url /offled
  LEDtoggle = LOW;
  LEDtoggledualcolor = LOW;
  LEDtimer = LOW;
  LEDtimer_toggle = LOW;
  Serial.println("Lampu Dimatikan");
  server.send(200, "text/html", SendHTML());
}

void handle_NotFound() {        //Interrupt untuk kondisi HTTP not found
  server.send(404, "text/plain", "Not found");
}

String SendHTML() {             //Kirim halaman HTML (kode ada di drive)
  String html = "<!DOCTYPE html> <html> <head> </head> <body> <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\"> <title>TLight &#169</title> <style> html{ font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;} body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;font-size:7vw;} h2 {font-size:5vw} h3 {color: #444444;margin-bottom: 0px;font-size:3vw} h4 {color: #444444;font-size:3vw} hr.solid { border-top: 3px dashed #bbb; } .button {display: block;width: 40px;background-color: #3498db;border: none;color: white;padding: 13px 20px;text-decoration: none;font-size: 3vw;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;width: 15%;} .button2 {display: block;width: 40px;background-color: #ff0000;border: none;color: white;padding: 13px 20px;text-decoration: none;font-size: 3vw;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;width: 50%;} input[type=\"color\"].custom {padding: 0;border: none;height: 50px;width: 25%;vertical-align: middle;} </style> <h1>TLight &#169 WebApp</h1> <h4>by Iman Herlambang S</h4> <h4>1806147924</h4> <hr class=\"solid\"> <!--label for=\"colorpicker\">Pilih :</label--> <h2>Pilih Warna</h1> <p> <input type=\"color\" id=\"colorpicker\" value=\"#";
  html += hex;
  html += "\" class=\"custom\"> <p> <a class=\"button\" href=\"#\" id=\"change_color\" role=\"button\">Ganti Warna</a> <hr class=\"solid\"> <h2>Transisi 2 Warna</h1> <h3>Pilih Warna1</h3> <p> <input type=\"color\" id=\"colorpicker1\" value=\"#";
  html += hex1;
  html += "\" class=\"custom\"> <p> <h3>Pilih Warna2</h3> <p> <input type=\"color\" id=\"colorpicker2\" value=\"#";
  html += hex2;
  html += "\" class=\"custom\"> <p> <a class=\"button\" href=\"#\" id=\"change_color2\" role=\"button\">Ganti Warna</a> <hr class=\"solid\"> <!--label for=\"colorpicker\">Pilih :</label--> <h2>Pilih Warna dan Waktu</h1> <p> <input type=\"color\" id=\"colorpicker3\" value=\"#";
  html += hex3;
  html += "\" class=\"custom\"> <p> <h3>Delay dalam detik(max 10 menit):</h3> <input type=\"number\" id=\"timer\" min=\"0\" max=\"600\"> <p> <a class=\"button\" href=\"#\" id=\"change_color3\" role=\"button\">Start Timer</a> <hr class=\"solid\"> <a class=\"button2\" href=\"/offled\" id=\"turn_off\" role=\"button\">&#9888 Matikan &#9888</a> </body> <script> document.getElementById(\"colorpicker\").onchange = function() { backRGB = this.value; console.log(backRGB); document.getElementById(\"change_color\").href=\"warna?hex=\" + document.getElementById(\"colorpicker\").value.substring(1); } </script> <script> document.getElementById(\"change_color\").href=\"warna?hex=\" + document.getElementById(\"colorpicker\").value.substring(1); </script> <script> document.getElementById(\"colorpicker1\").onchange = function() { backRGB1 = this.value; console.log(backRGB1); document.getElementById(\"change_color2\").href=\"duawarna?hex1=\" + document.getElementById(\"colorpicker1\").value.substring(1)+\"&hex2=\"+ document.getElementById(\"colorpicker2\").value.substring(1); } </script> <script> document.getElementById(\"colorpicker2\").onchange = function() { backRGB2 = this.value; console.log(backRGB2); document.getElementById(\"change_color2\").href=\"duawarna?hex1=\" + document.getElementById(\"colorpicker1\").value.substring(1)+\"&hex2=\"+ document.getElementById(\"colorpicker2\").value.substring(1); } </script> <script> document.getElementById(\"change_color2\").href=\"duawarna?hex1=\" + document.getElementById(\"colorpicker1\").value.substring(1)+\"&hex2=\"+ document.getElementById(\"colorpicker2\").value.substring(1); </script>";
  html += "\n";
  html += " <script> document.getElementById(\"colorpicker3\").onchange = function() { backRGB3 = this.value; console.log(backRGB3); document.getElementById(\"change_color3\").href=\"delay?hex=\" + document.getElementById(\"colorpicker3\").value.substring(1)+\"&time=\"+ document.getElementById(\"timer\").value; } ";
  html += "\n";
  html += "document.getElementById(\"timer\").onchange = function() { ";
  html += "\n";
  html += "timer = this.value; console.log(timer); document.getElementById(\"change_color3\").href=\"delay?hex=\" + document.getElementById(\"colorpicker3\").value.substring(1)+\"&time=\"+ document.getElementById(\"timer\").value; } </script> </html>";
  return html;
}

void FillLEDsFromPaletteColors( uint8_t colorIndex)     //Kode untuk mengisi warna sesuai dengan color pallete (transisi 2 warna)
{
  uint8_t brightness = 255;

  for ( int i = 0; i < NUM_LEDS; ++i) {
    leds[i] = ColorFromPalette( currentPalette, colorIndex, brightness, currentBlending);
    colorIndex += 3;
  }
}

void SetupDuaWarna()                                  //Kode transisi 2 warna
{
  //    CRGB purple = CHSV( HUE_PURPLE, 255, 255);
  //    CRGB green  = CHSV( HUE_GREEN, 255, 255);
  //    CRGB black  = CRGB::Black;
  CRGB warnapertama = strtoul(hex1, &(pointah), 16);
  CRGB warnakedua  = strtoul(hex2, &(pointah), 16);
  CRGB black  = CRGB::Black;

  currentPalette = CRGBPalette16(
                     warnakedua,  warnakedua,  black,  black,
                     warnapertama, warnapertama, black,  black,
                     warnakedua,  warnakedua,  black,  black,
                     warnapertama, warnapertama, black,  black );
}
