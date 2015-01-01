#include "application.h"
#include "HttpClient.h"
#include "DHT.h"
#include "Adafruit_SSD1306.h"
#include "Adafruit_GFX.h"

//#define LOGGING

#define STATHAT_DOMAIN "api.stathat.com"
#define STATHAT_PATH "/v"
#define STATHAT_PORT 80
#define STATHAT_UKEY "ODkzOCCWoLVcCgVRiuRYZyx5ZYF7"
// Terra TEST
#define STATHAT_KEY_T "JCITlBZ-T8t-fqpqfVCeSyBDTGRqbg~~"
#define STATHAT_KEY_H "80bUeMTWJ2tfSUMkTtaRgiBZSXp0Uw~~"
// Terra Left
//#define STATHAT_KEY_T "WlvGjNiq3Wn1_jpRajNBfiBDVEFLTQ~~"
//#define STATHAT_KEY_H "4IOEoN0qaoL2Lvvp4jWzWiBFaWlpcA~~"

#define DHTPIN D7
// Terra TEST
#define DHTTYPE DHT11
// Terra Left
//#define DHTTYPE DHT22


#define OLED_MOSI   D2
#define OLED_CLK    D3
#define OLED_DC     D4
#define OLED_CS     D5
#define OLED_RESET  D6

#define LIGHTPIN    A7
#define HEATPIN     A6


/**

ALL

*/

float heat_to = 29;

float _t = 0;
float _h = 0;
double temperature = 999999;
double humidity = 999999;

int heatState = 0;
int lightState = 0;

int onTime = 730;
int offTime = 1830;




/**

STATHAT

*/
HttpClient http;

// Headers currently need to be set at init, useful for API keys etc.
http_header_t headers[] = {
    { "Content-Type", "application/x-www-form-urlencoded" },
    { "Accept" , "application/json" },
    { "Accept" , "*/*"},
    { NULL, NULL } // NOTE: Always terminate headers will NULL
};

http_request_t request;
http_response_t response;

void sendStatHat( char type[1], float val ) {
    
    #ifdef LOGGING
    Serial.println("STATHAT >\tSTART");
    Serial.println( val );
    #endif
    
    request.hostname = STATHAT_DOMAIN;
    request.port = STATHAT_PORT;
    request.path = STATHAT_PATH;

    char _data[128];
    if ( type == "t" ){
        sprintf(_data, "key=%s&ukey=%s&value=%.2f", STATHAT_KEY_T, STATHAT_UKEY, val );
    }else{
        sprintf(_data, "key=%s&ukey=%s&value=%.2f", STATHAT_KEY_H, STATHAT_UKEY, val );    
    }

    // The library also supports sending a body with your request:
    request.body = _data;

    // Get request
    http.post(request, response, headers);
    Serial.print("STATHAT >\tResponse status: ");
    Serial.print( type );
    Serial.print( " : " );
    Serial.println(response.status);

    #ifdef LOGGING
    Serial.print("STATHAT >\ttHTTP Response Body: ");
    Serial.println(response.body);
    #endif
}





/**

DHT

*/
DHT dht(DHTPIN, DHTTYPE);


void readSensors(){
    _t = dht.readTemperature();
    temperature = (double) _t;
    _h = dht.readHumidity();
    humidity = (double) _h;
    #ifdef LOGGING
    Serial.print( "DHT >\tTEMP: " );
    Serial.println( _t );
    Serial.print( "DHT >\tHUM: " );
    Serial.println( _h );
    #endif
}




/**

TIMER

*/

int _lastD;
int _lastH;
int _lastM;
int _lastS;

void tsetup() {
    _lastD = Time.day();
    _lastH = Time.hour();
    _lastM = Time.minute();
    _lastS = Time.second();
}

void ttick(){
    if( Time.second() != _lastS ){
        fnSecond();
    }
    if( Time.minute() != _lastM ){
        fnMinute();
    }
    if( Time.hour() != _lastH ){
        fnHour();
    }
    if( Time.day() != _lastD ){
        fnDay();
    }
    _lastS = Time.second();
    _lastM = Time.minute();
    _lastH = Time.hour();
    _lastD = Time.day();
}

void fnDay(){
    #ifdef LOGGING
    Serial.println( "TIMER >\tDay" );
    #endif
    Spark.syncTime();
}

void fnHour(){
    #ifdef LOGGING
    Serial.println( "TIMER >\tHour" );
    #endif
}

void fnMinute(){
    #ifdef LOGGING
    Serial.println( "TIMER >\tMinute" );
    #endif

    processRelais();
    sendStatHat( "t", _t );
    sendStatHat( "h", _h );
}

void fnSecond(){
    #ifdef LOGGING
    Serial.print( "TIMER >\tSecond " );
    Serial.println( _lastS );
    #endif

    readSensors();
    drawDisplay();
}


/**

DISPLAY

*/
Adafruit_SSD1306 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

bool showTDots = false;

void drawVBar( Adafruit_SSD1306 dsp, float value, int x, int y, int width, float height, int factor, int line ) {
    int _v = value * ( height / factor );
    if( line ){
        int _line = height - line * ( height / factor );
        display.drawLine( x - 1, _line, ( x + width ), _line, WHITE);
    }
    display.drawRect( x, y, width, height, WHITE);
    display.fillRect( x+1, ( height - ( _v + y + 1 ) ), width-2, _v, WHITE);
}

void drawDisplay(){
    display.clearDisplay();
    display.setTextSize(1);
    if( heatState == 1 ){
        display.setTextColor(BLACK, WHITE);
        display.drawRect( 0, 54, 20, 9, WHITE);
    }else{
        display.setTextColor(WHITE);    
    }
    display.setCursor(1,55);
    display.print( (int)temperature );
    display.print("C");
    if( lightState == 1 ){
        drawVBar( display, temperature, 1, 0, 5, 53, 50, heat_to );
    }else{
        drawVBar( display, temperature, 1, 0, 5, 53, 50, 0 );
    }
    display.setTextColor(WHITE);
    display.setCursor(110,55);
    display.print( (int)humidity );
    display.print("%");
    drawVBar( display, humidity, display.width()-7, 0, 5, 53, 100, 0 );
    
    display.setTextColor(WHITE);
    display.setTextSize(4);
    
    display.setCursor(15,15);
    int _hour = Time.hour();
    if( _hour < 10 ){
        char _shr[8];
        sprintf(_shr, "%02d", _hour);
        display.print( _shr );
    }else{
        display.print( _hour );
    }
    
    if( !showTDots ){
        display.setCursor(54,15);
        display.print( ":" );
        showTDots = true;
    }else{
        showTDots = false;
    }
    
    display.setCursor(70,15);
    int _minute = Time.minute();
    if( _minute < 10 ){
        char _smnt[8];
        sprintf(_smnt, "%02d", _minute);
        display.print( _smnt );
    }else{
        display.print( _minute );
    }
    display.display();
}


void setupDisplay(){
    display.begin(SSD1306_SWITCHCAPVCC);
    display.display();
    
    // text display tests
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.clearDisplay();
}

/**

RELAIS

*/

void setupRelais(){
    pinMode(LIGHTPIN, OUTPUT);
    pinMode(HEATPIN, OUTPUT);
}

void processRelais(){
    int _time = ( Time.hour() * 100 ) + Time.minute();
    
    if( _time >= onTime && _time < offTime ){
        #ifdef LOGGING
        Serial.println("RELAIS >\tswitch light on");
        #endif

        digitalWrite( LIGHTPIN, HIGH );
        lightState = 1;
        
        #ifdef LOGGING
        Serial.println("RELAIS >\tlight on");
        #endif
    }else{
        #ifdef LOGGING
        Serial.println("RELAIS >\tswitch light off");
        #endif

        digitalWrite( LIGHTPIN, LOW );
        lightState = 0;

        #ifdef LOGGING
        Serial.println("RELAIS >\tlight off");
        #endif
    }

    if( lightState == 1 && _t < 1000 && _t < heat_to ){
        digitalWrite( HEATPIN, HIGH );
        heatState = 1;

        #ifdef LOGGING
        Serial.println("RELAIS >\tswitched heat on");
        #endif
    }else{
        digitalWrite( HEATPIN, LOW );
        heatState = 0;

        #ifdef LOGGING
        Serial.println("RELAIS >\tswitch heat off");
        #endif
    }
}




/**

GENERAL

*/
// allow us to use itoa() in this scope
extern char* itoa(int a, char* buffer, unsigned char radix);

void publishMetrics(){
    char _stmp[4];
    itoa( _t, _stmp, 10 );
    Spark.publish( "temp", _stmp );
    
    char _shum[4];
    itoa( _h, _shum, 10 );
    Spark.publish( "hum", _shum );
}

void setup() {
    Serial.begin(9600);

    Time.zone(1); // Winter-time
    //Time.zone(2); // Summer-time

    tsetup();
    setupDisplay();
    setupRelais();

    dht.begin();

    processRelais();
    drawDisplay();
}

void loop() {
    ttick();
}

