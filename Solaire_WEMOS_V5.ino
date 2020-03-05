// Device: WEMOS D1 
// Type de carte: LOLIN(WEMOS) D1 mini Pro
// 192.168.8.151:81
// 
// Install SSD1306Ascii, Arduinojson 5.13.5 

#include <Wire.h>

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

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

//#define UART_BAUD0 115200            // Baudrate
#define UART_BAUD0 9600              // Baudrate
#define SERIAL_PARAM0 SERIAL_8N1     // Data/Parity/Stop
#define bufferSize 2048
#define I2C_ADDRESS 0x3C             // adress I2C OLED display 

unsigned long startTime_p_routed; 
unsigned long startTime_OLED; 

int p_routed=0;       // p_routed
int p_inject=0;       // p_inject
int p_linky=0;        // p_linky
int p_solar=0;        // p_solar
String TIME;
String PTEC;
bool latch=true;
bool TARIFF=true;

struct data {
  int PULSE1, HCHP_tic1, HCHP_tic2, HCHP_tic3, CUMULUS_temp; String xml_TIME, xml_PTEC;
};

data ref_index = {0, 0, 0, 0, 0};     
data current_index = {0, 0, 0, 0, 0}; 

int CUMULUS_temperature=0;

ESP8266WebServer server (81);   // create Objects
SSD1306AsciiWire oled; 

//*************************************************************************************************//
//                                    HTML * AJAX [START]                                          //
//************************************************************************************************//

String SendHTML(String TIME,int p_routed,int p_inject,int p_linky,int p_solar,int CUMULUS_temperature){
  String ptr = "<!DOCTYPE html>";
  ptr +="<html>";
  ptr +="<head>";
  ptr +="<title>INFO ROUTAGE</title>";
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
  ptr +=".Time .reading{color: #F29C1F;}";
  ptr +=".P_routed .reading{color: #3B97D3;}";
  ptr +=".P_inject .reading{color: #3B97D3;}";
  ptr +=".P_linky .reading{color: #3B97D3;}";
  ptr +=".P_solar .reading{color: #3B97D3;}";
  ptr +=".Cumulus .reading{color: #3B97D3;}";
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
  ptr +="<h1>INFO ROUTAGE</h1>";
  ptr +="<div class='container'>";


  ptr +="<div class='data TIME'>";
  ptr +="<div class='side-by-side icon'>";

  //icone TIME
  ptr +="<svg enable-background='new 0 0 19.438 54.003'height=54.003px id=Layer_1 version=1.1 viewBox='0 0 19.438 54.003'width=19.438px x=0px xml:space=preserve xmlns=http://www.w3.org/2000/svg xmlns:xlink=http://www.w3.org/1999/xlink y=0px><g><path d='M11.976,8.82v-2h4.084V6.063C16.06,2.715,13.345,0,9.996,0H9.313C5.965,0,3.252,2.715,3.252,6.063v30.982";
  ptr +="C1.261,38.825,0,41.403,0,44.286c0,5.367,4.351,9.718,9.719,9.718c5.368,0,9.719-4.351,9.719-9.718";
  ptr +="c0-2.943-1.312-5.574-3.378-7.355V18.436h-3.914v-2h3.914v-2.808h-4.084v-2h4.084V8.82H11.976z M15.302,44.833";
  ptr +="c0,3.083-2.5,5.583-5.583,5.583s-5.583-2.5-5.583-5.583c0-2.279,1.368-4.236,3.326-5.104V24.257C7.462,23.01,8.472,22,9.719,22";
  ptr +="s2.257,1.01,2.257,2.257V39.73C13.934,40.597,15.302,42.554,15.302,44.833z'fill=#F29C21 /></g></svg>";
  //end icon TIME
  
  ptr +="</div>";
  ptr +="<div class='side-by-side text'>Time</div>";
  ptr +="<div class='side-by-side reading'>";
  ptr +=(String)TIME;
  ptr +="<span class='superscript'></span></div>";
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


  ptr +="<div class='data p_inject'>";
  ptr +="<div class='side-by-side icon'>";

  //icone p_inject
  ptr +="<svg enable-background='new 0 0 29.235 40.64'height=40.64px id=Layer_1 version=1.1 viewBox='0 0 29.235 40.64'width=29.235px x=0px xml:space=preserve xmlns=http://www.w3.org/2000/svg xmlns:xlink=http://www.w3.org/1999/xlink y=0px><path d='M14.618,0C14.618,0,0,17.95,0,26.022C0,34.096,6.544,40.64,14.618,40.64s14.617-6.544,14.617-14.617";
  ptr +="C29.235,17.95,14.618,0,14.618,0z M13.667,37.135c-5.604,0-10.162-4.56-10.162-10.162c0-0.787,0.638-1.426,1.426-1.426";
  ptr +="c0.787,0,1.425,0.639,1.425,1.426c0,4.031,3.28,7.312,7.311,7.312c0.787,0,1.425,0.638,1.425,1.425";
  ptr +="C15.093,36.497,14.455,37.135,13.667,37.135z'fill=#3C97D3 /></svg>";
  //end icon p_inject
  
  ptr +="</div>";
  ptr +="<div class='side-by-side text'>P_inject</div>";
  ptr +="<div class='side-by-side reading'>";
  ptr +=(int)p_inject;
  ptr +="<span class='superscript'>Wh</span></div>";
  ptr +="</div>";


  ptr +="<div class='data p_linky'>";
  ptr +="<div class='side-by-side icon'>";

  //icone p_linky
  ptr +="<svg enable-background='new 0 0 29.235 40.64'height=40.64px id=Layer_1 version=1.1 viewBox='0 0 29.235 40.64'width=29.235px x=0px xml:space=preserve xmlns=http://www.w3.org/2000/svg xmlns:xlink=http://www.w3.org/1999/xlink y=0px><path d='M14.618,0C14.618,0,0,17.95,0,26.022C0,34.096,6.544,40.64,14.618,40.64s14.617-6.544,14.617-14.617";
  ptr +="C29.235,17.95,14.618,0,14.618,0z M13.667,37.135c-5.604,0-10.162-4.56-10.162-10.162c0-0.787,0.638-1.426,1.426-1.426";
  ptr +="c0.787,0,1.425,0.639,1.425,1.426c0,4.031,3.28,7.312,7.311,7.312c0.787,0,1.425,0.638,1.425,1.425";
  ptr +="C15.093,36.497,14.455,37.135,13.667,37.135z'fill=#3C97D3 /></svg>";
  //end icon p_linky
  
  ptr +="</div>";
  ptr +="<div class='side-by-side text'>P_linky</div>";
  ptr +="<div class='side-by-side reading'>";
  ptr +=(int)p_linky;
  ptr +="<span class='superscript'>Wh</span></div>";
  ptr +="</div>";


  ptr +="<div class='data p_solar'>";
  ptr +="<div class='side-by-side icon'>";

  //icone p_solar
  ptr +="<svg enable-background='new 0 0 29.235 40.64'height=40.64px id=Layer_1 version=1.1 viewBox='0 0 29.235 40.64'width=29.235px x=0px xml:space=preserve xmlns=http://www.w3.org/2000/svg xmlns:xlink=http://www.w3.org/1999/xlink y=0px><path d='M14.618,0C14.618,0,0,17.95,0,26.022C0,34.096,6.544,40.64,14.618,40.64s14.617-6.544,14.617-14.617";
  ptr +="C29.235,17.95,14.618,0,14.618,0z M13.667,37.135c-5.604,0-10.162-4.56-10.162-10.162c0-0.787,0.638-1.426,1.426-1.426";
  ptr +="c0.787,0,1.425,0.639,1.425,1.426c0,4.031,3.28,7.312,7.311,7.312c0.787,0,1.425,0.638,1.425,1.425";
  ptr +="C15.093,36.497,14.455,37.135,13.667,37.135z'fill=#3C97D3 /></svg>";
  //end icon p_solar
  
  ptr +="</div>";
  ptr +="<div class='side-by-side text'>P_solar</div>";
  ptr +="<div class='side-by-side reading'>";
  ptr +=(int)p_solar;
  ptr +="<span class='superscript'>Wh</span></div>";
  ptr +="</div>";


  
  ptr +="<div class='data CUMULUS_temperature'>";
  ptr +="<div class='side-by-side icon'>";

  //icone CUMULUS_temperature
  ptr +="<svg enable-background='new 0 0 29.235 40.64'height=40.64px id=Layer_1 version=1.1 viewBox='0 0 29.235 40.64'width=29.235px x=0px xml:space=preserve xmlns=http://www.w3.org/2000/svg xmlns:xlink=http://www.w3.org/1999/xlink y=0px><path d='M14.618,0C14.618,0,0,17.95,0,26.022C0,34.096,6.544,40.64,14.618,40.64s14.617-6.544,14.617-14.617";
  ptr +="C29.235,17.95,14.618,0,14.618,0z M13.667,37.135c-5.604,0-10.162-4.56-10.162-10.162c0-0.787,0.638-1.426,1.426-1.426";
  ptr +="c0.787,0,1.425,0.639,1.425,1.426c0,4.031,3.28,7.312,7.311,7.312c0.787,0,1.425,0.638,1.425,1.425";
  ptr +="C15.093,36.497,14.455,37.135,13.667,37.135z'fill=#3C97D3 /></svg>";
  //end icon CUMULUS_temperature
  
  ptr +="</div>";
  ptr +="<div class='side-by-side text'>Cumulus</div>";
  ptr +="<div class='side-by-side reading'>";
  ptr +=(int)CUMULUS_temperature;
  ptr +="<span class='superscript'>deg</span></div>";
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
  server.send(200, "text/html", SendHTML(TIME,p_routed,p_inject,p_linky,p_solar,CUMULUS_temperature)); 
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}

void oled_index_display() {
    oled.setFont ( System5x7 );
    oled.clear ( );
    oled.set2X ( );
    
    oled.print ("PR= ");                // Line 1 "PR= 1000Wh"
    oled.print (p_routed);
    oled.println ("Wh"); 
    
    oled.print ("PI= ");                // Line 2 "PI= 1000Wh" 
    oled.print (p_inject); 
    oled.println ("Wh"); 

    oled.print ("PS= ");                // Line 3 "PS= 3000Wh"
    oled.print (p_solar);
    oled.println ("Wh"); 
    
    oled.print ("CUM= ");               // Line 4 "CUM= 56deg "
    oled.print (CUMULUS_temperature);
    oled.println ("deg");           
}


void oled_HC_display() {
    oled.setFont ( System5x7 );
    oled.clear ( );
    oled.set2X ( );
    oled.println ("*H CREUSE*");        // Line 1 "H CREUSE"
      
    oled.print ("PR= ");                // Line 1 "PR= 1000Wh"
    oled.print (p_routed);
    oled.println ("Wh"); 
      
    oled.print ("PI= ");                // Line 3 "PI= 1000Wh" 
    oled.print (p_inject); 
    oled.println ("Wh"); 

    oled.print ("CUM= ");               // Line 4 "CUM= 56deg "
    oled.print (CUMULUS_temperature);
    oled.println ("deg");      
}


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



data esp_cgx() // fonction GET index compteur Cumulus
{
  if (WiFi.status() == WL_CONNECTED) {
  HTTPClient http;
  http.begin("http://192.168.120.3/esp.cgx"); // get XML file from WES device
  http.setAuthorization("admin", "wes");
  int httpCode = http.GET();
   
  if (httpCode > 0) // httpCode will be negative on error. HTTP header has been send and Server response header has been handled
  { 
    String payload = http.getString();
    String xml_PULSE1 = xmlTakeParam(payload,"PULSE1"); 
    String xml_HCHP_tic1 = xmlTakeParam(payload,"HCHP_1");
    String xml_HCHP_tic2 = xmlTakeParam(payload,"HCHP_2"); 
    String xml_HCHP_tic3 = xmlTakeParam(payload,"HCHP_3"); 
    String xml_CUMULUS_temp = xmlTakeParam(payload,"SONDE2");
    String xml_TIME = xmlTakeParam(payload,"TIME");
    String xml_PTEC = xmlTakeParam(payload,"PTEC");
    
    int PULSE1 = xml_PULSE1.toInt();             //Converts string to int
    int HCHP_tic1 = xml_HCHP_tic1.toInt();
    int HCHP_tic2 = xml_HCHP_tic2.toInt();
    int HCHP_tic3 = xml_HCHP_tic3.toInt();
    int CUMULUS_temp = round(xml_CUMULUS_temp.toFloat());
    
    return {PULSE1, HCHP_tic1, HCHP_tic2, HCHP_tic3, CUMULUS_temp, xml_TIME, xml_PTEC}; 
   
  } else {
      oled.setFont ( System5x7 );
      oled.clear ( );
      oled.set2X ( );
      oled.println ("***********"); 
      oled.println ("*  ERROR  *");
      oled.println ("* IP WES  *");
      oled.println ("***********");
      return {0, 0, 0, 0, 0}; 
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

  //******* SERIAL Tx Rx *******************
  Serial.begin ( UART_BAUD0, SERIAL_PARAM0, SERIAL_FULL );  // Serial begin
  //******* END SERIAL Tx Rx ***************

  //******* WIFI *******************
  WiFi.begin ( ssid, password );                            // WiFi begin
  while ( WiFi.status() != WL_CONNECTED ) {                 // Wait for connection
    delay (500);                                            // WiFi connexion is OK
  }
  //******* END WIFI *****************
  
  //******* SERVER *******************
  server.begin();                                           // HTTP server starts
  server.on("/", handle_OnConnect);
  server.onNotFound(handle_NotFound);
  //******* SERVER END *************** 

  //******* I2C ***********************
  Wire.pins(SDA, SCL);                                     // on Wemos D1
  //Wire.pins(0, 2);                                       // on ESP-01.
  Wire.begin();
  //******* I2C END *******************

  //******* OLED ***********************
  oled.begin( &Adafruit128x64, I2C_ADDRESS );               // OLED init
  
  oled.setFont ( System5x7 );
  oled.clear ( );
  oled.set2X ( );
  oled.println ("***********"); 
  oled.println ("*  WEMOS  *");
  oled.println ("*  INDEX  *");
  oled.println ("***********");
  delay (3000); 
  //******* OLED END ********************

  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname("Solaire_WEMOS");
  ArduinoOTA.begin();
  
  current_index=esp_cgx();
  CUMULUS_temperature=current_index.CUMULUS_temp;
  TIME=current_index.xml_TIME;
  PTEC=current_index.xml_PTEC;
  oled_index_display();

  startTime_p_routed = millis();                            // starttime_p_routed
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

    
  ArduinoOTA.handle();                    // Surveillance des demandes de mise à jour en OTA
  
  server.handleClient();                  // check if client and provide HTML page
                                          // à chaque itération, la fonction handleClient traite les requêtes 

  
  if((millis() - startTime_p_routed) > 300000)                     // Update p_routed value every 5mm
  {                             
      current_index=esp_cgx();
      PTEC=current_index.xml_PTEC;

      if ((PTEC == "H. Pleines") && (latch == true))               // détection on-peak tariff (HP)
        {
        ref_index=current_index;                                   // store ref_index
        latch=false;                                               // on-peak tariff (HP) is active
        }                      

      if ((PTEC == "H. Pleines") && (latch == false))               // on-peak tariff (HP) is active
        { 
        p_routed = (current_index.PULSE1 - ref_index.PULSE1);       // calculation p_routed during on-peak tariff (HP)
        p_solar = (current_index.HCHP_tic1 - ref_index.HCHP_tic1);
        p_inject = (current_index.HCHP_tic2 - ref_index.HCHP_tic2);
        p_linky  = (current_index.HCHP_tic3 - ref_index.HCHP_tic3);
        CUMULUS_temperature=current_index.CUMULUS_temp;
        TIME=current_index.xml_TIME;
        oled_index_display();
        }

      if ((PTEC == "H. Creuses") && (latch == false))
        {
        latch=true;    
        }
        
      if (PTEC == "H. Creuses")
       {  
       p_routed = (current_index.PULSE1 - ref_index.PULSE1);
       p_inject = (current_index.HCHP_tic2 - ref_index.HCHP_tic2); 
       CUMULUS_temperature=current_index.CUMULUS_temp;
       TIME=current_index.xml_TIME;
       oled_HC_display();  
       }  
                                                
    startTime_p_routed = millis(); // new startTime value
  }

  
}

 //**********************************************************************************************//
//                                       [LOOP END]                                              //
//***********************************************************************************************//
