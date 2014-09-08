#include <DallasTemperature.h>
#include <OneWire.h>
#include <SPI.h>
#include <Ethernet.h>
#include "callback.h"


// Define time intervals based on milliseconds
#define SECONDS 1000L
#define MINUTES 60*SECONDS
#define HOURS 60*MINUTES
#define DAYS 24*HOURS

#define ONE_WIRE_PORT 2
#define MAX_SENSORS 7
#define MAX_HISTORY 5

OneWire oneWire(ONE_WIRE_PORT);
DallasTemperature sensors(&oneWire);

DeviceAddress addresses[MAX_SENSORS];
float lastTemperature[MAX_SENSORS];
float lastMinutes[MAX_SENSORS][MAX_HISTORY];

byte mac[] = {
  0x90,0xA2,0xDA,0x00,0x26,0xEB};
byte ip[] = {
  192,168,192,206};
EthernetServer server(80);

typedef void(*callback)(void);

// Used to regularly invoke a given callback
void doInterval(unsigned long* timeBuffer, unsigned long interval, callback cb) {
  if( (long)( millis() - *timeBuffer) >= 0) {
    (*cb)();
    *timeBuffer = millis() + interval;
  }
}

void pushInArray(float* array, float value) {
  for(int i = MAX_HISTORY - 2; i >= 0; i--) {
    array[i + 1] = array[i];
  }
  array[0] = value;
}  

float avg(float* array) {
  float result = 0.0;
  float divisor = 0.0;
  for(int i = 0; i < MAX_HISTORY && array[i] > 0.0; i++) {
    result += array[i];
    divisor += 1.0;
  }
  return divisor == 0.0 ? 0.0 : result / divisor;
}

// function to print a device address
void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    // zero pad the address if necessary
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}

unsigned long fiveSecondBuffer;
boolean queryOrFetch = true;
void everyFiveSeconds() {
  if (queryOrFetch) {
    Serial.println("Requesting Temperature....");
    sensors.setWaitForConversion(false);
    sensors.requestTemperatures();
  } 
  else {
    Serial.println("Querying Temperature....");
    for(int i = 0; i < min(MAX_SENSORS, sensors.getDeviceCount()); i++) {
      lastTemperature[i] = sensors.getTempCByIndex(i);
      printAddress(addresses[i]);
      Serial.print(" = ");
      Serial.print(lastTemperature[i]);
      Serial.print(" C ");
      Serial.println();
    }
  }
  queryOrFetch = !queryOrFetch;
}

unsigned long oneMinuteBuffer;
void everyMinute() {
  Serial.println("Updating Minutes...");
  for(int i = 0; i < min(MAX_SENSORS, sensors.getDeviceCount()); i++) {
    pushInArray(lastMinutes[i], lastTemperature[i]);
    Serial.print("Temp: ");
    printAddress(addresses[i]);
    Serial.println();
  }
}


void tempLoop(void) {
  doInterval(&fiveSecondBuffer, 5*SECONDS, &everyFiveSeconds);  
  doInterval(&oneMinuteBuffer, 1*MINUTES, &everyMinute);  
}



void setup() {  //setup stuff
  Serial.begin(9600);
  sensors.begin();
  Serial.print("Found: ");
  Serial.print(sensors.getDeviceCount());
  sensors.setResolution(12);
  Serial.println(" sensors");
  for(int i = 0; i < min(MAX_SENSORS, sensors.getDeviceCount()); i++) {
    sensors.getAddress(addresses[i], i);
    Serial.print("ROM: ");
    printAddress(addresses[i]);
    Serial.println();
  }
  Ethernet.begin(mac, ip);
  server.begin();
}

void loop()
{
  tempLoop();
  EthernetClient client = server.available();
  if (client) {
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: application/json");
          client.println("Connnection: close");
          client.println();
          client.println();
          client.println("sensors({");
          for(int si = 0; si < min(MAX_SENSORS, sensors.getDeviceCount()); si++) {
            if (si > 0) {
              client.print(",");
            }
            client.print("\"");
            for(int j = 0; j < 8; j++) {
              if (addresses[si][j] < 16) client.print("0");
              client.print(addresses[si][j], HEX);
            }
            client.println("\" : {");
            client.print(" temp : ");
            client.print(lastTemperature[si]);
            client.println(",");
            client.print("lastMinutes : [");
            for(int j = 0; j < MAX_HISTORY; j++) {
              if (j > 0) {
                client.print(",");
              }             
              client.print(lastMinutes[si][j]);
            }
            client.println("]");
            client.println("}");
          }
          client.println("})");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } 
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
  }
}

