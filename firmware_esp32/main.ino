//
// A simple server implementation showing how to:
//  * serve static messages
//  * read GET and POST parameters
//  * handle missing pages / 404s
//

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include "AsyncTCP.h"
#include "DHTesp.h"
#include "Ticker.h"
#include <WiFi.h>
#include <Preferences.h>
#include <QList.h>

#define AP_SSID "terraino" //can set ap hostname here

// WIFI settings
AsyncWebServer server(80);
Preferences preferences;
static volatile bool wifi_connected = false;
String wifiSSID, wifiPassword;
const char* PARAM_MESSAGE = "ssid";
QList<String> ssids;

// Temp settings
DHTesp dht;
const float tempTarget = 30;
float tempMin;
float tempMax;
const float tempHysteresis = 1;
const int heatingPin = 12;
int heatingState = false;
/** Task handle for the light value read task */
TaskHandle_t tempTaskHandle = NULL;
/** Ticker for temperature reading */
Ticker tempTicker;
/** Flag if task should run */
bool tasksEnabled = false;
/** Pin number for DHT11 data pin */
int dhtPin = 14;

int updateInterval = 60;

TempAndHumidity dhtData;

void WiFiEvent(WiFiEvent_t event)
{
  switch (event) {

    case SYSTEM_EVENT_AP_START:
      //can set ap hostname here
      WiFi.softAPsetHostname(AP_SSID);
      //enable ap ipv6 here
      WiFi.softAPenableIpV6();
      break;

    case SYSTEM_EVENT_STA_START:
      //set sta hostname here
      WiFi.setHostname(AP_SSID);
      break;
    case SYSTEM_EVENT_STA_CONNECTED:
      //enable sta ipv6 here
      WiFi.enableIpV6();
      break;
    case SYSTEM_EVENT_AP_STA_GOT_IP6:
      //both interfaces get the same event
      Serial.print("STA IPv6: ");
      Serial.println(WiFi.localIPv6());
      Serial.print("AP IPv6: ");
      Serial.println(WiFi.softAPIPv6());
      break;
    case SYSTEM_EVENT_STA_GOT_IP:
      wifiOnConnect();
      wifi_connected = true;
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      wifi_connected = false;
      wifiOnDisconnect();
      break;
    default:
      break;
  }
}


//when wifi connects
void wifiOnConnect()
{
  Serial.println("STA Connected");
  Serial.print("STA SSID: ");
  Serial.println(WiFi.SSID());
  Serial.print("STA IPv4: ");
  Serial.println(WiFi.localIP());
  Serial.print("STA IPv6: ");
  Serial.println(WiFi.localIPv6());
  WiFi.mode(WIFI_MODE_STA);     //close AP network
}

//when wifi disconnects
void wifiOnDisconnect()
{
  Serial.println("STA Disconnected");
  delay(1000);
  WiFi.begin(wifiSSID.c_str(), wifiPassword.c_str());
}

bool initTemp() {
  byte resultValue = 0;
  // Initialize temperature sensor
  dht.setup(dhtPin, DHTesp::DHT11);
  Serial.println("DHT initiated");

  // Start task to get temperature
  xTaskCreatePinnedToCore(
      tempTask,                       /* Function to implement the task */
      "tempTask ",                    /* Name of the task */
      4000,                           /* Stack size in words */
      NULL,                           /* Task input parameter */
      5,                              /* Priority of the task */
      &tempTaskHandle,                /* Task handle. */
      1);                             /* Core where the task should run */

  if (tempTaskHandle == NULL) {
    Serial.println("Failed to start task for temperature update");
    return false;
  } else {
    // Start update of environment data every 20 seconds
    tempTicker.attach(updateInterval, triggerGetTemp);
  }
  return true;
}

/**
 * triggerGetTemp
 * Sets flag dhtUpdated to true for handling in loop()
 * called by Ticker getTempTimer
 */
void triggerGetTemp() {
  if (tempTaskHandle != NULL) {
     xTaskResumeFromISR(tempTaskHandle);
  }
}

/**
 * Task to reads temperature from DHT11 sensor
 * @param pvParameters
 *    pointer to task parameters
 */
void tempTask(void *pvParameters) {
  Serial.println("tempTask loop started");
  while (1) // tempTask loop
  {
    if (tasksEnabled) {
      // Get temperature values
      getTemperature();
    }
    // Got sleep again
    vTaskSuspend(NULL);
  }
}

bool getTemperature() {
  // Reading temperature for humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (it's a very slow sensor)
  TempAndHumidity newValues = dht.getTempAndHumidity();
  dhtData = newValues;
  // Check if any reads failed and exit early (to try again).
  if (dht.getStatus() != 0) {
    Serial.println("DHT11 error status: " + String(dht.getStatusString()));
    return false;
  }
  float temp = newValues.temperature;
  if (temp < tempMin)
  {
    tempMin = temp;
  }
  if (temp > tempMax)
  {
    tempMax = temp;
  }

  if( temp > ( tempTarget + tempHysteresis ) ){
    // cooling
    Serial.println("Cooling till: " + String( tempTarget + tempHysteresis ));
    heatingState = false;
    digitalWrite(heatingPin, LOW);
  } else if( temp < ( tempTarget - tempHysteresis ) ){
    // heating
    heatingState = true;
    digitalWrite(heatingPin, HIGH);
    Serial.println("Heating till: " + String( tempTarget - tempHysteresis ));
  } else {
    // hold state
    Serial.println("In Hysterese");
  }
  Serial.println(" T:" + String(temp) + " H:" + String(newValues.humidity));
  return true;
}

void setup()
{
  Serial.begin(115200);
  uint8_t chipid[6];
  esp_efuse_read_mac(chipid);
  Serial.printf("CHIP-ID: %X\n",chipid);
  
  // Heating
  pinMode(heatingPin, OUTPUT);
  digitalWrite(heatingPin, LOW);
  
  initTemp();
  getTemperature();
  // Signal end of setup() to tasks
  tasksEnabled = true;
  
  // Wifi
  Serial.println("scan start");

    // WiFi.scanNetworks will return the number of networks found
    int n = WiFi.scanNetworks();
    Serial.println("scan done");
    if (n == 0) {
        Serial.println("no networks found");
    } else {
        Serial.print(n);
        Serial.println(" networks found");
        for (int i = 0; i < n; ++i) {
            // Print SSID and RSSI for each network found
            //String( WiFi.SSID(i) ) + " (" + String( WiFi.RSSI(i) ) + ")" +(WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*"
            ssids.push_back(String( WiFi.SSID(i) ));  
        }
    }
    Serial.println("");

  
  WiFi.onEvent(WiFiEvent);
  WiFi.mode(WIFI_MODE_APSTA);
  WiFi.softAP(AP_SSID);
  Serial.println("AP Started");
  Serial.print("AP SSID: ");
  Serial.println(AP_SSID);
  Serial.print("AP IPv4: ");
  Serial.println(WiFi.softAPIP());
  
  preferences.begin("wifi", false);
  wifiSSID = preferences.getString("ssid", "none");         //NVS key ssid
  wifiPassword = preferences.getString("password", "none"); //NVS key password
  preferences.end();
  Serial.print("Stored SSID: ");
  Serial.println(wifiSSID);

  WiFi.begin(wifiSSID.c_str(), wifiPassword.c_str());
  if (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.printf("WiFi Failed!\n");
    setupWifiCredentials();
  } else {
    setupServerApp();  
  }
  

  Serial.print("start Webserver");
  server.begin();
}

void setupServerApp(){
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Hostname: ");
  Serial.println(WiFi.getHostname());

  Serial.print("Setip App Endpoints ");
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    String resp = "";
    resp += "<html><head>";
      resp += "<meta charset=\"utf-8\">";
      resp += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1, shrink-to-fit=no\">";
      resp += "<style>";
        resp += "*,*:after,*:before{box-sizing:inherit}html{box-sizing:border-box;font-size:62.5%}body{color:#606c76;font-family: 'Helvetica Neue', 'Helvetica', 'Arial', sans-serif;font-size:1.6em;font-weight:300;letter-spacing:.01em;line-height:1.6}blockquote{border-left:0.3rem solid #d1d1d1;margin-left:0;margin-right:0;padding:1rem 1.5rem}blockquote *:last-child{margin-bottom:0}.button,button,input[type='button'],input[type='reset'],input[type='submit']{background-color:#9b4dca;border:0.1rem solid #9b4dca;border-radius:.4rem;color:#fff;cursor:pointer;display:inline-block;font-size:1.1rem;font-weight:700;height:3.8rem;letter-spacing:.1rem;line-height:3.8rem;padding:0 3.0rem;text-align:center;text-decoration:none;text-transform:uppercase;white-space:nowrap}.button:focus,.button:hover,button:focus,button:hover,input[type='button']:focus,input[type='button']:hover,input[type='reset']:focus,input[type='reset']:hover,input[type='submit']:focus,input[type='submit']:hover{background-color:#606c76;border-color:#606c76;color:#fff;outline:0}.button[disabled],button[disabled],input[type='button'][disabled],input[type='reset'][disabled],input[type='submit'][disabled]{cursor:default;opacity:.5}.button[disabled]:focus,.button[disabled]:hover,button[disabled]:focus,button[disabled]:hover,input[type='button'][disabled]:focus,input[type='button'][disabled]:hover,input[type='reset'][disabled]:focus,input[type='reset'][disabled]:hover,input[type='submit'][disabled]:focus,input[type='submit'][disabled]:hover{background-color:#9b4dca;border-color:#9b4dca}.button.button-outline,button.button-outline,input[type='button'].button-outline,input[type='reset'].button-outline,input[type='submit'].button-outline{background-color:transparent;color:#9b4dca}.button.button-outline:focus,.button.button-outline:hover,button.button-outline:focus,button.button-outline:hover,input[type='button'].button-outline:focus,input[type='button'].button-outline:hover,input[type='reset'].button-outline:focus,input[type='reset'].button-outline:hover,input[type='submit'].button-outline:focus,input[type='submit'].button-outline:hover{background-color:transparent;border-color:#606c76;color:#606c76}.button.button-outline[disabled]:focus,.button.button-outline[disabled]:hover,button.button-outline[disabled]:focus,button.button-outline[disabled]:hover,input[type='button'].button-outline[disabled]:focus,input[type='button'].button-outline[disabled]:hover,input[type='reset'].button-outline[disabled]:focus,input[type='reset'].button-outline[disabled]:hover,input[type='submit'].button-outline[disabled]:focus,input[type='submit'].button-outline[disabled]:hover{border-color:inherit;color:#9b4dca}.button.button-clear,button.button-clear,input[type='button'].button-clear,input[type='reset'].button-clear,input[type='submit'].button-clear{background-color:transparent;border-color:transparent;color:#9b4dca}.button.button-clear:focus,.button.button-clear:hover,button.button-clear:focus,button.button-clear:hover,input[type='button'].button-clear:focus,input[type='button'].button-clear:hover,input[type='reset'].button-clear:focus,input[type='reset'].button-clear:hover,input[type='submit'].button-clear:focus,input[type='submit'].button-clear:hover{background-color:transparent;border-color:transparent;color:#606c76}.button.button-clear[disabled]:focus,.button.button-clear[disabled]:hover,button.button-clear[disabled]:focus,button.button-clear[disabled]:hover,input[type='button'].button-clear[disabled]:focus,input[type='button'].button-clear[disabled]:hover,input[type='reset'].button-clear[disabled]:focus,input[type='reset'].button-clear[disabled]:hover,input[type='submit'].button-clear[disabled]:focus,input[type='submit'].button-clear[disabled]:hover{color:#9b4dca}code{background:#f4f5f6;border-radius:.4rem;font-size:86%;margin:0 .2rem;padding:.2rem .5rem;white-space:nowrap}pre{background:#f4f5f6;border-left:0.3rem solid #9b4dca;overflow-y:hidden}pre>code{border-radius:0;display:block;padding:1rem 1.5rem;white-space:pre}hr{border:0;border-top:0.1rem solid #f4f5f6;margin:3.0rem 0}input[type='email'],input[type='number'],input[type='password'],input[type='search'],input[type='tel'],input[type='text'],input[type='url'],input[type='color'],input[type='date'],input[type='month'],input[type='week'],input[type='datetime'],input[type='datetime-local'],input:not([type]),textarea,select{-webkit-appearance:none;-moz-appearance:none;appearance:none;background-color:transparent;border:0.1rem solid #d1d1d1;border-radius:.4rem;box-shadow:none;box-sizing:inherit;height:3.8rem;padding:.6rem 1.0rem;width:100%}input[type='email']:focus,input[type='number']:focus,input[type='password']:focus,input[type='search']:focus,input[type='tel']:focus,input[type='text']:focus,input[type='url']:focus,input[type='color']:focus,input[type='date']:focus,input[type='month']:focus,input[type='week']:focus,input[type='datetime']:focus,input[type='datetime-local']:focus,input:not([type]):focus,textarea:focus,select:focus{border-color:#9b4dca;outline:0}select{background:url('data:image/svg+xml;utf8,<svg xmlns=\"http://www.w3.org/2000/svg\" height=\"14\" viewBox=\"0 0 29 14\" width=\"29\"><path fill=\"%23d1d1d1\" d=\"M9.37727 3.625l5.08154 6.93523L19.54036 3.625\"/></svg>') center right no-repeat;padding-right:3.0rem}select:focus{background-image:url('data:image/svg+xml;utf8,<svg xmlns=\"http://www.w3.org/2000/svg\" height=\"14\" viewBox=\"0 0 29 14\" width=\"29\"><path fill=\"%239b4dca\" d=\"M9.37727 3.625l5.08154 6.93523L19.54036 3.625\"/></svg>')}textarea{min-height:6.5rem}label,legend{display:block;font-size:1.6rem;font-weight:700;margin-bottom:.5rem}fieldset{border-width:0;padding:0}input[type='checkbox'],input[type='radio']{display:inline}.label-inline{display:inline-block;font-weight:normal;margin-left:.5rem}.container{margin:0 auto;max-width:112.0rem;padding:0 2.0rem;position:relative;width:100%}.row{display:flex;flex-direction:column;padding:0;width:100%}.row.row-no-padding{padding:0}.row.row-no-padding>.column{padding:0}.row.row-wrap{flex-wrap:wrap}.row.row-top{align-items:flex-start}.row.row-bottom{align-items:flex-end}.row.row-center{align-items:center}.row.row-stretch{align-items:stretch}.row.row-baseline{align-items:baseline}.row .column{display:block;flex:1 1 auto;margin-left:0;max-width:100%;width:100%}.row .column.column-offset-10{margin-left:10%}.row .column.column-offset-20{margin-left:20%}.row .column.column-offset-25{margin-left:25%}.row .column.column-offset-33,.row .column.column-offset-34{margin-left:33.3333%}.row .column.column-offset-50{margin-left:50%}.row .column.column-offset-66,.row .column.column-offset-67{margin-left:66.6666%}.row .column.column-offset-75{margin-left:75%}.row .column.column-offset-80{margin-left:80%}.row .column.column-offset-90{margin-left:90%}.row .column.column-10{flex:0 0 10%;max-width:10%}.row .column.column-20{flex:0 0 20%;max-width:20%}.row .column.column-25{flex:0 0 25%;max-width:25%}.row .column.column-33,.row .column.column-34{flex:0 0 33.3333%;max-width:33.3333%}.row .column.column-40{flex:0 0 40%;max-width:40%}.row .column.column-50{flex:0 0 50%;max-width:50%}.row .column.column-60{flex:0 0 60%;max-width:60%}.row .column.column-66,.row .column.column-67{flex:0 0 66.6666%;max-width:66.6666%}.row .column.column-75{flex:0 0 75%;max-width:75%}.row .column.column-80{flex:0 0 80%;max-width:80%}.row .column.column-90{flex:0 0 90%;max-width:90%}.row .column .column-top{align-self:flex-start}.row .column .column-bottom{align-self:flex-end}.row .column .column-center{-ms-grid-row-align:center;align-self:center}@media (min-width: 40rem){.row{flex-direction:row;margin-left:-1.0rem;width:calc(100% + 2.0rem)}.row .column{margin-bottom:inherit;padding:0 1.0rem}}a{color:#9b4dca;text-decoration:none}a:focus,a:hover{color:#606c76}dl,ol,ul{list-style:none;margin-top:0;padding-left:0}dl dl,dl ol,dl ul,ol dl,ol ol,ol ul,ul dl,ul ol,ul ul{font-size:90%;margin:1.5rem 0 1.5rem 3.0rem}ol{list-style:decimal inside}ul{list-style:circle inside}.button,button,dd,dt,li{margin-bottom:1.0rem}fieldset,input,select,textarea{margin-bottom:1.5rem}blockquote,dl,figure,form,ol,p,pre,table,ul{margin-bottom:2.5rem}table{border-spacing:0;width:100%}td,th{border-bottom:0.1rem solid #e1e1e1;padding:1.2rem 1.5rem;text-align:left}td:first-child,th:first-child{padding-left:0}td:last-child,th:last-child{padding-right:0}b,strong{font-weight:bold}p{margin-top:0}h1,h2,h3,h4,h5,h6{font-weight:300;letter-spacing:-.1rem;margin-bottom:2.0rem;margin-top:0}h1{font-size:4.6rem;line-height:1.2}h2{font-size:3.6rem;line-height:1.25}h3{font-size:2.8rem;line-height:1.3}h4{font-size:2.2rem;letter-spacing:-.08rem;line-height:1.35}h5{font-size:1.8rem;letter-spacing:-.05rem;line-height:1.5}h6{font-size:1.6rem;letter-spacing:0;line-height:1.4}img{max-width:100%}.clearfix:after{clear:both;content:' ';display:table}.float-left{float:left}.float-right{float:right}";
      resp += "</style>";
    resp += "</head>";
      resp += "<body>";
        resp += "<h1>Terraino</h1>";
        resp += "<dl class=\"dl-horizontal\">";
          resp += "<dt>Temperatur:</dt>";
          resp += "<dd>" + String(dhtData.temperature) + " &deg;C <small>( " + String(tempMin) + " &deg;C -> " + String(tempMax) + " &deg;C )</small></dd>";
          resp += "<dt>Humidity:</dt>";
          resp += "<dd>" + String( dhtData.humidity ) + " %</dd>";
          resp += "<dt>Target Temp:</dt>";
          resp += "<dd>" + String( tempTarget ) + " &deg;C</dd>";
          resp += "<dt>Heating:</dt>";
          if( heatingState){
          resp += "<dd>ON</dd>";
          }else{
          resp += "<dd>OFF</dd>";
          }
        resp += "</dl>";
      resp += "</body>";
    resp += "<html>";
    request->send(200, "text/html", resp);
  });

  // Send a GET request to <IP>/get?message=<message>
  server.on("/get", HTTP_GET, [](AsyncWebServerRequest *request) {
    String message;
    if (request->hasParam(PARAM_MESSAGE))
    {
      message = request->getParam(PARAM_MESSAGE)->value();
    }
    else
    {
      message = "No message sent";
    }
    request->send(200, "text/plain", "Hello, GET: " + message);
  });

  // Send a POST request to <IP>/post with a form field message set to <message>
  server.on("/post", HTTP_POST, [](AsyncWebServerRequest *request) {
    String message;
    if (request->hasParam(PARAM_MESSAGE, true))
    {
      message = request->getParam(PARAM_MESSAGE, true)->value();
    }
    else
    {
      message = "No message sent";
    }
    request->send(200, "text/plain", "Hello, POST: " + message);
  });

  server.onNotFound(notFound);

  server.begin();
}

void setupWifiCredentials(){
   Serial.println("Credentials request: ");
  server.onNotFound([](AsyncWebServerRequest *request) {
    request->send(200, "text/html", "<html><bod><form action=\"\\credentials\" method=\"POST\"><div><input type=\"text\" name=\"ssid\" placeholder=\"Wifi Name (SSID) ...\"/></div><div><input type=\"text\" name=\"sec\" placeholder=\"Password\"/></div><div><button>Save</button></div></form></body></html>");
  });

  server.on("/credentials", HTTP_POST, [](AsyncWebServerRequest *request) {
    String ssid;
    String sec;
    if (request->hasParam("ssid", true) && request->hasParam("sec", true) )
    {
      ssid = request->getParam("ssid", true)->value();
      sec = request->getParam("sec", true)->value();
      
      request->send(200, "text/plain", "Credentials for SSID: " + ssid + "save ... restart");

      preferences.begin("wifi", false); // Note: Namespace name is limited to 15 chars
      Serial.println("Writing new ssid");
      preferences.putString("ssid", ssid);

      Serial.println("Writing new pass");
      preferences.putString("password", sec);
      delay(300);
      preferences.end();
      
      delay(2000);
      ESP.restart();
      return;
    }
    
    request->send(400, "text/plain", "Missing data...");
    
  });
}

void loop()
{
}


void notFound(AsyncWebServerRequest *request)
{
  request->send(404, "text/plain", "Not found");
}