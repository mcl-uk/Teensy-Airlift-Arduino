
// AirLift_APwWebServer http image transfer test sketch
// For Teensy4.1 with AirLift breakout board 
// Derived from WiFiNINA_Generic's example sketch AP_SimpleWebServer.ino
// Instead of serving just two lines of static text we attempt here to serve
// a ~2MB jpg file from the Teensy's SD-Card at /test.jpg
//
// Uses modified WiFiNINA_generic_1.8.15-1 library originally from:
//   https://www.arduinolibraries.info/libraries/wi-fi-nina_generic
// May also rq: WiFiMulti_generic, WiFi101, WiFi101_Generic, WiFiEspAT, ESP_AT_Lib
//
// If you need to deviate from the default wiring be sure to edit
//   <lib_path>/WiFiNINA_Generic/src/WiFiNINA_Pinout_Generic.h accordingly 
//
// First insert an SD card with a ~2MB JPG file at /test.jpg & run the sketch,
// connect a WiFi device to the "Teensy41" SSID, pwd "TeensyWeensy",
// point device's browser to:  http://192.168.4.1 and watch the image load.
//
//-----------------------------------------------------------------------------------------------------------

#include <SPI.h>
#include <WiFi_Generic.h>
#include <WiFiNINA_Generic.h>
#include <SD.h>

// Wiring rig:
//#define SPIWIFI      SPI   // SPI=SPI0, SPI1=SPI1
//  SPI:  MISO=12, MOSI=11,  SCK=13  Or...
//  SPI1: MISO1=1, MOSI1=26, SCK1=27
//#define SPIWIFI_SS    10   // CS pin (aka SS)
//#define SPIWIFI_ACK    9   // BUSY pin (aka ACK or READY)
//#define ESP32_RESETN  14   // RST pin
//#define ESP32_GPIO0   15   // GP0 pin (Required, do not leave unconnected)
//
// For _Generic versions of the library these wiring settings must be
// coded into WiFiNINA_Pinout_Generic.h

#define GO_BLK setLEDs( 0, 0, 0)
#define GO_RED setLEDs(99, 0, 0)
#define GO_GRN setLEDs( 0,10, 0)
#define GO_BLU setLEDs( 0, 0,99)
#define GO_YEL setLEDs(99,10, 0)
#define GO_MAG setLEDs(99, 0,99)
#define GO_CYN setLEDs( 0,10,99)
#define GO_WHT setLEDs(99,10,99)

#define WIFI_BLK_SZ 403200  // Block size profoundly affects WiFi data transfer speed.

// --- Globals -------------------------------------------------------------------

char        FileName[] = "/test.jpg";       // test file-name (2MB jpg)
char        ssid[]     = "Teensy41";        // your network SSID (name)
char        pass[]     = "TeensyWeensy";    // your network password (use for WPA, or use as key for WEP)
int         keyIndex   = 0;                 // your network key Index number (needed only for WEP)
uint8_t     CHAN       = 3;                 // irrelevant, NINA always uses Ch#1
int         status     = WL_IDLE_STATUS;
uint8_t     buf[WIFI_BLK_SZ];               // data txfr buffer
WiFiServer  server(80);                     // port 80 for http

// -------------------------------------------------------------------------------

// Airlift LED control
void setLEDs(uint8_t red, uint8_t green, uint8_t blue)
 {
  WiFiDrv::pinMode(25, OUTPUT); WiFiDrv::analogWrite(25, green);
  WiFiDrv::pinMode(26, OUTPUT); WiFiDrv::analogWrite(26, red);
  WiFiDrv::pinMode(27, OUTPUT); WiFiDrv::analogWrite(27, blue);
 }

// Restart Teensy, without invoking bootloader
void reStart(uint32_t dlyms=100)
 {
   Serial.flush(); // if using Serial
   if (dlyms > 0) delay(dlyms);
   *(volatile uint32_t *)0xE000ED0C = 0x5FA0004;
 }

// -------------------------------------------------------------------------------

void setup()
 {
  Serial.println("\n ------- WiFi-AP Web Server ----------");
  Serial1.begin(115200);
  GO_RED;

  // Requires modified WiFi_NINA_generic v1.8.15-1 library & WiFiNina_Pinout_Generic.h

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) { Serial.println("WiFi module comms failed."); reStart(); } // &&& ERROR1

  // by default the local IP address of will be 192.168.4.1 override it with:
  // WiFi.config(IPAddress(10, 0, 0, 1));

  status = WiFi.beginAP(ssid, pass, CHAN); // sometimes fails if a client device is still associated. 
  if (status != WL_AP_LISTENING) { Serial.println("Failed to create AP."); reStart(); }  // &&& ERROR2
  Serial.print("Access point ready on SSID: '"); Serial.print(ssid);
  //Serial.print(" on channel: "); Serial.println(WiFi.channel(0)); // does not work
  server.begin();  // Start server socket on port 80
  IPAddress ip = WiFi.localIP();  // Teensy's IP address:
  Serial.print("', Server IP: "); Serial.println(ip);
  Serial.print("Pls browse to: http://"); Serial.println(ip);
  // start SD card reader
  SD.begin(BUILTIN_SDCARD);
  GO_MAG;
 }

// -----------------------------------------------------------------------------

void loop()
 {
  bool        validRq     = false;
  uint32_t    rdlen       = 1;
  uint32_t    wrlen       = 1;
  uint32_t    n           = 0;
  String      currentLine = "";
  uint32_t    t0, flen;
  File        srcFile;
  WiFiClient  client;
  char        c;

  WiFi.noLowPowerMode();    // no effect
  WiFi.setTimeout(1);       // no effect

  // look for any change in WiFi status
  if (status != WiFi.status())
   {
    status = WiFi.status();
    if (status == WL_AP_CONNECTED) Serial.println("Device associated to AP");
    else                           Serial.println("Device disassociated from AP");
   }
  (status == WL_AP_CONNECTED) ? GO_YEL : GO_MAG;

  if (client = server.available())          // Got a TCP client yet?
   {
    Serial.print("Hello "); Serial.println(client.remoteIP());
    currentLine = "";                       // String holds incoming header lines from the client
    while (client.connected())              // loop while the client is connected
     {
      GO_GRN;
      if (client.available())               // is there data to read from the client?
       {
        c = client.read();             // read client's request header char-by-char
        //Serial.write(c);
        if (c == '\n')                      // \n\n = end of request header
         {
          if (currentLine.length() == 0)
           {
            // If the current line is blank then we have our \n\n terminator
            // which completes the client HTTP request header. Note that the validRq
            // flag filters out all other spurious requests (eg for favicon.ico etc)
            if (validRq && (srcFile = SD.open(FileName)))
             {
              validRq = false;
              // Send response headers, \n\n, jpg data 
              flen = srcFile.size();
              client.println("HTTP/1.1 200 OK");
              client.println("Content-type:image/jpeg");
              client.println("Refresh:10");  // &&& DEBUG ONLY
              client.print("Content-Disposition:filename=\""); client.print(FileName); client.println("\"");
              client.print("Content-Length:"); client.println(flen);
              client.println(); // \n\n terminates response header, now send data...
              Serial.print("Sending "); Serial.print(flen); Serial.print(" Bytes "); Serial.println(FileName);
              GO_WHT;
              t0 = millis();
              n = 0;
              while(((rdlen = srcFile.read(buf, WIFI_BLK_SZ)) > 0) && (wrlen > 0) && (WiFi.status() == WL_AP_CONNECTED) && client.connected())
               {
                wrlen = client.write(buf, rdlen);
                n++;
               }
              t0 = millis() - t0;
              n = ((n-1) * WIFI_BLK_SZ) + wrlen;
              srcFile.close();
              Serial.print(n); Serial.print(" Bytes in ");
              Serial.print(t0/1000); Serial.print(" secs = ");
              if (t0 > 0) { Serial.print(n/t0); Serial.println(" KB/s"); }
              if (n != flen)
               {
                Serial.print("FAILED: ");
                if (!client.connected()) Serial.println(" TCP socket closed.");
                else if (WiFi.status() != WL_AP_CONNECTED) Serial.println(" WiFi link dropped.");
                else Serial.println(" ???");
               }
             }
             else
             {
              client.println("HTTP/1.1 404 nak");
              client.println();
              client.flush();       
             }
            client.stop();
           }  
          currentLine = "";
         }
         else if (c != '\r')
         {
          currentLine += c;
          // validRq |= (currentLine == "GET / ");
          if (currentLine == "GET / ") validRq = true;
         }
       }
     }
    client.stop();    // close the socket
    Serial.println("Goodbye");
    Serial.flush();
   }
 }

