
// Device: ESP01 or WEMOS D1 
// Output L1, L2 et L3 active powers from Triphase Router 
// Solar Rx / Tx in JSON format to OLED 128*64 and server web
// Install SSD1306Ascii, Arduinojson 5.13.5 

#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h> 
#include <ArduinoJson.h>             // Arduinojson 5.13.5
#include <ESP8266HTTPClient.h>

#include <WiFiClient.h>
#include <WiFiClientSecure.h>

#include "SSD1306Ascii.h"            // SSD1306Ascii 1.2.5
#include "SSD1306AsciiWire.h"        // for WEMOS D1 or ESP
      
#define ssid      "sceslaw_1"        // WiFi SSID
#define password  "155835ef8b"       // WiFi password

#define UART_BAUD0 9600              // Baudrate
#define SERIAL_PARAM0 SERIAL_8N1     // Data/Parity/Stop
#define bufferSize 2048
#define I2C_ADDRESS 0x3C             // adress I2C OLED display 

unsigned long refTime;
unsigned long startTime_OLED; 
unsigned long startTime_p_routed; 

String phases_info;
String routing="NO ACTIVE";

int16_t i=0; 
int16_t L1=0; 
int16_t L2=0;
int16_t L3=0;
int16_t ACTIVE_POWER=0;

int16_t p_routed=0;       // p_routed

int16_t LOAD_0=0;
int16_t LOAD_1=0;
int16_t LOAD_2=0;

bool latch=true;
bool TARIFF=true;


char json[bufferSize];


ESP8266WebServer server (80);   // create Objects
SSD1306AsciiWire oled; 

//*************************************************************************************************//
//                                    HTML * AJAX [START]                                          //
//************************************************************************************************//

String SendHTML(int16_t ACTIVE_POWER,int16_t L1,int16_t L2,int16_t L3,int16_t p_routed,String routing){
  String ptr = "<!DOCTYPE html>";
  ptr +="<html>";
  ptr +="<head>";
  ptr +="<title>ROUTEUR SOLAIRE</title>";
  ptr +="<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  
  // We have used Google commissioned Open Sans web font for our web page. 
  // Note that you cannot see Google font, without active internet connection on the device. Google fonts are loaded on the fly.
  
  ptr +="<link href='https://fonts.googleapis.com/css?family=Open+Sans:300,400,600' rel='stylesheet'>";
  
  ptr +="<style>";
  ptr +="html { font-family: 'Open Sans', sans-serif; display: block; margin: 0px auto; text-align: center;color: #444444;}";
  ptr +="body{margin: 0px;} ";
  ptr +="h1 {margin: 50px auto 30px;} ";
  ptr +=".side-by-side{display: table-cell;vertical-align: middle;position: relative;}";
  ptr +=".text{font-weight: 600;font-size: 19px;width: 200px;}";
  ptr +=".reading{font-weight: 300;font-size: 50px;padding-right: 25px;}";
  ptr +=".ACTIVE_POWER .reading{color: #F29C1F;}";
  ptr +=".L1 .reading{color: #3B97D3;}";
  ptr +=".L2 .reading{color: #3B97D3;}";
  ptr +=".L3 .reading{color: #3B97D3;}";
  ptr +=".COSPHI .reading{color: #3B97D3;}";
  ptr +=".superscript{font-size: 17px;font-weight: 600;position: absolute;top: 10px;}";
  ptr +=".data{padding: 10px;}";
  ptr +=".container{display: table;margin: 0 auto;}";
  ptr +=".icon{width:65px}";
  ptr +="</style>";

  // Dynamically load data with AJAX
  ptr +="<script>\n";
  ptr +="setInterval(loadDoc,1000);\n";
  ptr +="function loadDoc() {\n";
  ptr +="var xhttp = new XMLHttpRequest();\n";
  ptr +="xhttp.onreadystatechange = function() {\n";
  ptr +="if (this.readyState == 4 && this.status == 200) {\n";
  ptr +="document.body.innerHTML =this.responseText}\n";
  ptr +="};\n";
  ptr +="xhttp.open(\"GET\", \"/\", true);\n";
  ptr +="xhttp.send();\n";
  ptr +="}\n";
  ptr +="</script>\n";
  // end script Dynamically load data with AJAX
  
  ptr +="</head>";
  ptr +="<body>";
  ptr +="<h1>ROUTEUR SOLAIRE</h1>";
  ptr +="<div class='container'>";
  ptr +="<div class='data ACTIVE_POWER'>";
  ptr +="<div class='side-by-side icon'>";
  
  //icone ACTIVE POWER
  ptr +="<svg enable-background='new 0 0 19.438 54.003'height=54.003px id=Layer_1 version=1.1 viewBox='0 0 19.438 54.003'width=19.438px x=0px xml:space=preserve xmlns=http://www.w3.org/2000/svg xmlns:xlink=http://www.w3.org/1999/xlink y=0px><g><path d='M11.976,8.82v-2h4.084V6.063C16.06,2.715,13.345,0,9.996,0H9.313C5.965,0,3.252,2.715,3.252,6.063v30.982";
  ptr +="C1.261,38.825,0,41.403,0,44.286c0,5.367,4.351,9.718,9.719,9.718c5.368,0,9.719-4.351,9.719-9.718";
  ptr +="c0-2.943-1.312-5.574-3.378-7.355V18.436h-3.914v-2h3.914v-2.808h-4.084v-2h4.084V8.82H11.976z M15.302,44.833";
  ptr +="c0,3.083-2.5,5.583-5.583,5.583s-5.583-2.5-5.583-5.583c0-2.279,1.368-4.236,3.326-5.104V24.257C7.462,23.01,8.472,22,9.719,22";
  ptr +="s2.257,1.01,2.257,2.257V39.73C13.934,40.597,15.302,42.554,15.302,44.833z'fill=#F29C21 /></g></svg>";
  // end icone ACTIVE POWER
  
  ptr +="</div>";
  ptr +="<div class='side-by-side text'>ACTIVE POWER</div>";
  ptr +="<div class='side-by-side reading'>";
  ptr +=(int16_t)ACTIVE_POWER;
  ptr +="<span class='superscript'>W</span></div>";
  ptr +="</div>";
  ptr +="<div class='data L1'>";
  ptr +="<div class='side-by-side icon'>";

  //icone L1
  ptr +="<svg enable-background='new 0 0 29.235 40.64'height=40.64px id=Layer_1 version=1.1 viewBox='0 0 29.235 40.64'width=29.235px x=0px xml:space=preserve xmlns=http://www.w3.org/2000/svg xmlns:xlink=http://www.w3.org/1999/xlink y=0px><path d='M14.618,0C14.618,0,0,17.95,0,26.022C0,34.096,6.544,40.64,14.618,40.64s14.617-6.544,14.617-14.617";
  ptr +="C29.235,17.95,14.618,0,14.618,0z M13.667,37.135c-5.604,0-10.162-4.56-10.162-10.162c0-0.787,0.638-1.426,1.426-1.426";
  ptr +="c0.787,0,1.425,0.639,1.425,1.426c0,4.031,3.28,7.312,7.311,7.312c0.787,0,1.425,0.638,1.425,1.425";
  ptr +="C15.093,36.497,14.455,37.135,13.667,37.135z'fill=#3C97D3 /></svg>";
  //end icon L1
  
  ptr +="</div>";
  ptr +="<div class='side-by-side text'>L1</div>";
  ptr +="<div class='side-by-side reading'>";
  ptr +=(int16_t)L1;
  ptr +="<span class='superscript'>W</span></div>";
  ptr +="</div>";
  ptr +="<div class='data L2'>";
  ptr +="<div class='side-by-side icon'>";

  //icone L2
  ptr +="<svg enable-background='new 0 0 29.235 40.64'height=40.64px id=Layer_1 version=1.1 viewBox='0 0 29.235 40.64'width=29.235px x=0px xml:space=preserve xmlns=http://www.w3.org/2000/svg xmlns:xlink=http://www.w3.org/1999/xlink y=0px><path d='M14.618,0C14.618,0,0,17.95,0,26.022C0,34.096,6.544,40.64,14.618,40.64s14.617-6.544,14.617-14.617";
  ptr +="C29.235,17.95,14.618,0,14.618,0z M13.667,37.135c-5.604,0-10.162-4.56-10.162-10.162c0-0.787,0.638-1.426,1.426-1.426";
  ptr +="c0.787,0,1.425,0.639,1.425,1.426c0,4.031,3.28,7.312,7.311,7.312c0.787,0,1.425,0.638,1.425,1.425";
  ptr +="C15.093,36.497,14.455,37.135,13.667,37.135z'fill=#3C97D3 /></svg>";
  //end icon L2
  
  ptr +="</div>";
  ptr +="<div class='side-by-side text'>L2</div>";
  ptr +="<div class='side-by-side reading'>";
  ptr +=(int16_t)L2;
  ptr +="<span class='superscript'>W</span></div>";
  ptr +="</div>";
  ptr +="<div class='data L3'>";
  ptr +="<div class='side-by-side icon'>";
  
  //icone L3
  ptr +="<svg enable-background='new 0 0 29.235 40.64'height=40.64px id=Layer_1 version=1.1 viewBox='0 0 29.235 40.64'width=29.235px x=0px xml:space=preserve xmlns=http://www.w3.org/2000/svg xmlns:xlink=http://www.w3.org/1999/xlink y=0px><path d='M14.618,0C14.618,0,0,17.95,0,26.022C0,34.096,6.544,40.64,14.618,40.64s14.617-6.544,14.617-14.617";
  ptr +="C29.235,17.95,14.618,0,14.618,0z M13.667,37.135c-5.604,0-10.162-4.56-10.162-10.162c0-0.787,0.638-1.426,1.426-1.426";
  ptr +="c0.787,0,1.425,0.639,1.425,1.426c0,4.031,3.28,7.312,7.311,7.312c0.787,0,1.425,0.638,1.425,1.425";
  ptr +="C15.093,36.497,14.455,37.135,13.667,37.135z'fill=#3C97D3 /></svg>";
  //end icon L3
  
  ptr +="</div>";
  ptr +="<div class='side-by-side text'>L3</div>";
  ptr +="<div class='side-by-side reading'>";
  ptr +=(int16_t)L3;
  ptr +="<span class='superscript'>W</span></div>";
  ptr +="</div>";

  ptr +="<div class='data p_routed'>";
  ptr +="<div class='side-by-side icon'>";

  //icone p_routed
  ptr +="<svg enable-background='new 0 0 29.235 40.64'height=40.64px id=Layer_1 version=1.1 viewBox='0 0 29.235 40.64'width=29.235px x=0px xml:space=preserve xmlns=http://www.w3.org/2000/svg xmlns:xlink=http://www.w3.org/1999/xlink y=0px><path d='M14.618,0C14.618,0,0,17.95,0,26.022C0,34.096,6.544,40.64,14.618,40.64s14.617-6.544,14.617-14.617";
  ptr +="C29.235,17.95,14.618,0,14.618,0z M13.667,37.135c-5.604,0-10.162-4.56-10.162-10.162c0-0.787,0.638-1.426,1.426-1.426";
  ptr +="c0.787,0,1.425,0.639,1.425,1.426c0,4.031,3.28,7.312,7.311,7.312c0.787,0,1.425,0.638,1.425,1.425";
  ptr +="C15.093,36.497,14.455,37.135,13.667,37.135z'fill=#3C97D3 /></svg>";
  //end icon p_routed
  
  ptr +="</div>";
  ptr +="<div class='side-by-side text'>P_routed</div>";
  ptr +="<div class='side-by-side reading'>";
  ptr +=(int)p_routed;
  ptr +="<span class='superscript'>Wh</span></div>";
  ptr +="</div>";

  ptr +="<div class='data routing'>";
  ptr +="<div class='side-by-side icon'>";

  //icone routing
  ptr +="<svg enable-background='new 0 0 29.235 40.64'height=40.64px id=Layer_1 version=1.1 viewBox='0 0 29.235 40.64'width=29.235px x=0px xml:space=preserve xmlns=http://www.w3.org/2000/svg xmlns:xlink=http://www.w3.org/1999/xlink y=0px><path d='M14.618,0C14.618,0,0,17.95,0,26.022C0,34.096,6.544,40.64,14.618,40.64s14.617-6.544,14.617-14.617";
  ptr +="C29.235,17.95,14.618,0,14.618,0z M13.667,37.135c-5.604,0-10.162-4.56-10.162-10.162c0-0.787,0.638-1.426,1.426-1.426";
  ptr +="c0.787,0,1.425,0.639,1.425,1.426c0,4.031,3.28,7.312,7.311,7.312c0.787,0,1.425,0.638,1.425,1.425";
  ptr +="C15.093,36.497,14.455,37.135,13.667,37.135z'fill=#3C97D3 /></svg>";
  //end icon routing
  
  ptr +="</div>";
  ptr +="<div class='side-by-side text'>Routing</div>";
  ptr +="<div class='side-by-side reading'>";
  ptr +=(String)routing;
  ptr +="<span class='superscript'></span></div>";
  ptr +="</div>";
  
  ptr +="</div>";
  ptr +="</body>";
  ptr +="</html>";
  return ptr;
}

//*************************************************************************************************//
//                                  HTML * AJAX [END]                                              //
//*************************************************************************************************//



//************************************************************************************************//
//                                  FUNCTIONS [START]                                             //
//************************************************************************************************//

void handle_OnConnect() {
  server.send(200, "text/html", SendHTML(ACTIVE_POWER,L1,L2,L3,p_routed,routing)); 
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}

void oled_index_display() {
    oled.setFont ( System5x7 );
    oled.clear ( );
    oled.set2X ( );
    oled.println ("***********"); 
    oled.println ("* GET WES *");
    oled.println ("* INDEX.. *");
    oled.println ("***********");
}

void oled_display () {
    if ((LOAD_0 == 100) || (LOAD_0 > 0)) {
      routing = "Cumulus";
      oled.setFont ( System5x7 );
      oled.clear ( );
      oled.set2X ( );
      oled.print ("PW: ");                // Total ACTIVE POWER
      oled.print (ACTIVE_POWER);
      oled.println ("W");
      oled.print ("L2= ");               // L2 ACTIVE POWER
      oled.print (L2);
      oled.println ("W");
      oled.print ("PR= ");               // P_routed
      oled.print (p_routed);
      oled.println ("Wh"); 
      oled.print ("RT= ");               // Routing RT= CU 100%
      oled.print ("CU ");
      oled.print (LOAD_0);  
      oled.println ("%");  
    }

   if ((LOAD_2 == 100) || (LOAD_2 > 0)) {
      routing = "EDF INJECT";
      oled.setFont ( System5x7 );
      oled.clear ( );
      oled.set2X ( );
      oled.print ("PW: ");                // Total ACTIVE POWER
      oled.print (ACTIVE_POWER);
      oled.println ("W");
      oled.print ("L2= ");               // L2 ACTIVE POWER
      oled.print (L2);
      oled.println ("W");
      oled.print ("PR= ");               // P_routed
      oled.print (p_routed);
      oled.println ("W"); 
      oled.print ("RT= ");               // Routing LOAD_2
      oled.println ("EDF INJ");   
    }
    
   if ((LOAD_0 + LOAD_1 + LOAD_2) == 0) {
     routing = "OFF";
     oled.setFont ( System5x7 );
     oled.clear ( );
     oled.set2X ( );
  
     oled.print ("PW: ");
     oled.print (ACTIVE_POWER);
     oled.println ("W");

     oled.print ("L1= ");
     oled.print (L1);
     oled.println ("W");
  
     oled.print ("L2= ");
     oled.print (L2);
     oled.println ("W");

     oled.print ("L3= ");
     oled.print (L3);
     oled.println ("W");
    } 
 }
 

void parsing_json () {
  StaticJsonBuffer<200> jsonBuffer;
  //char json[] = "{\"L1\":30,\"L2\":40,\"L3\":100,\"LOAD_0\":0,\"LOAD_1\":0,\"LOAD_2\":1}";  //pour les tests
  phases_info=String(json);
  JsonObject& object = jsonBuffer.parseObject(phases_info);
  
  if (!object.success()) {
    L1=999;L2=999;L3=999;ACTIVE_POWER=999; routing="ERROR JSON"; // if parsing failed L1=999;L2=999;L3=999;
    oled.setFont ( System5x7 );
    oled.clear ( );
    oled.set2X ( );
    oled.println ("***********"); 
    oled.println ("*  JSON   *");
    oled.println ("*  ERROR  *");
    oled.println ("***********");
    delay(3000);   
   }

  else {                       // if JSON parsing OK then get L1, L2 and L3 values
  L1= object["L1"];            // L1 active power
  L2= object["L2"];            // L2 active power
  L3= object["L3"];            // L3 active power
  ACTIVE_POWER= L1 + L2 + L3;  // Total active power
  LOAD_0= object["LOAD_0"];    // LOAD_0 information
  LOAD_1= object["LOAD_1"];    // LOAD_1 information
  LOAD_2= object["LOAD_2"];    // LOAD_2 information
  TARIFF= object["TARIFF"];    // OFF_PEAK_TARIFF

  //myBroker.publish("broker/ACTIVE_POWER", (String)ACTIVE_POWER);   // need to convert to String to publish on MQTT 
  
  }
}

// Exemple MQTT client
// mosquitto_pub -h 192.168.7.237 -p 1883 -t "MQTT_ref" -m "400"

// http://192.168.120.3/pulse.cgx
// <data>
// <impulsion>
// <PULSE1>0</PULSE1>
// <INDEX1>376</INDEX1>

  

String xmlTakeParam(String inStr,String needParam)   // parser XML 
{
  if(inStr.indexOf("<"+needParam+">")>0){
     int CountChar=needParam.length();
     int indexStart=inStr.indexOf("<"+needParam+">");
     int indexStop= inStr.indexOf("</"+needParam+">");  
     return inStr.substring(indexStart+CountChar+2, indexStop);
  }
  return "not found";
}


void reconnect() {   
    WiFi.begin ( ssid, password );
    while (WiFi.status() != WL_CONNECTED) {  
        delay(500);  
    }  
}  



int papp()                                         // function GET apparent power
{
  if (WiFi.status() == WL_CONNECTED) {
  HTTPClient http;
  http.begin("http://192.168.120.3/tic2.cgx");    // get XML file from WES
  http.setAuthorization("admin", "wes");
  int httpCode = http.GET();
   
  if (httpCode > 0) // httpCode will be negative on error. HTTP header has been send and Server response header has been handled
  { 
    String payload = http.getString();
    String xml = xmlTakeParam(payload,"PAP"); 
    return xml.toFloat();                         // Converts string to float
     
  } else {
      return 0;
  }

  http.end();
  
 }
}

//************************************************************************************************//
//                                       FUNCTIONS [END]                                          //
//************************************************************************************************//

 
//************************************************************************************************//
//                                       SETUP [START]                                            //
//************************************************************************************************//

void setup()
 {
  ESP.wdtDisable();                                         // Watchdog disable                                    
  
  Serial.begin ( UART_BAUD0, SERIAL_PARAM0, SERIAL_FULL );  // Serial begin
  
  WiFi.begin ( ssid, password );                            // WiFi begin
  
  while ( WiFi.status() != WL_CONNECTED ) {                 // Wait for connection
    delay (500);                                            // WiFi connexion is OK
  }
  
  
  server.begin();                                           // HTTP server starts
  server.on("/", handle_OnConnect);
  server.onNotFound(handle_NotFound);

  
  //Wire.pins(SDA, SCL);                                    // on Wemos D1
  Wire.pins(0, 2);                                          // on ESP-01.
  Wire.begin();
  
  oled.begin( &Adafruit128x64, I2C_ADDRESS );               // OLED init
  
  oled.setFont ( System5x7 );
  oled.clear ( );
  oled.set2X ( );
  oled.println ("***********"); 
  oled.println ("* ROUTEUR *");
  oled.println ("* SOLAIRE *");
  oled.println ("***********");
  delay (3000); 

  ESP.wdtEnable(3000);                                     // Watchdog enable   

  startTime_OLED = millis();                               // startTime_OLED
}

//**********************************************************************************************//
//                                       SETUP [END]                                            //
//**********************************************************************************************//


//**********************************************************************************************//
//                                       LOOP [START]                                           //
//**********************************************************************************************//

void loop() {
      
  if (WiFi.status() != WL_CONNECTED) {    // if not wifi
    reconnect();                          // try to reconnect
    }  
  
  server.handleClient();                  // check if client and provide HTML page
                              

  if ( Serial.available ( ) ) refTime = millis ( ); 
  while ( ( millis ( ) - refTime ) < 100 ) {
    if ( Serial.available ( ) ) {
      json[i] = Serial.read ( );         // read serial interface
      if ( i < bufferSize - 1 ) i++;
    }
  }
  i = 0;
  parsing_json();                        // JSON parsing UART Tx Rx to get L1, L2, L3 and total ACTIVE_POWER informations
  ESP.wdtFeed();                         // Reset watchdog

  if((millis() - startTime_OLED) > 3000) // Update OLED display every 3s
  {
    oled_display();                      // Display total active power, L1, L2 and L3 on OLED 128*64 
    startTime_OLED = millis();           // New startime value
  }

  if((millis() - startTime_p_routed) > 300000) // Update p_routed value every 5mm
  {                             
    // p-routed update every 5mm  //
    // true if off-peak tariff (HC) is active
    // false if on-peak tariff (HP) is active

      if ((TARIFF == false) && (latch == true)) // détection on-peak tariff (HP)
        {
        oled_index_display();
        ref_index=pulse_1();   // store ref_index
        latch=false; 
        }                      // on-peak tariff (HP) is active

      if ((TARIFF == false) && (latch == false))  // if on-peak tariff (HP)
        { 
        oled_index_display();
        current_index=pulse_1();
        p_routed= (current_index - ref_index);} // calculation p_routed during on-peak tariff (HP)
        
      else {
        latch=true; 
      } // off-peak tariff (HC) is active
                               
           
    startTime_p_routed = millis(); // new startTime value
  }

  
}

 //**********************************************************************************************//
//                                       [LOOP END]                                              //
//***********************************************************************************************//
