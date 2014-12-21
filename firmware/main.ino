#include "Adafruit_SSD1306.h"
#include "Adafruit_GFX.h"
#include "DHT.h"
#include "rest_client.h"

#define OLED_MOSI   D2
#define OLED_CLK    D3
#define OLED_DC     D4
#define OLED_CS     D5
#define OLED_RESET  D6


#define SLAVE_ADDRESS 0x09

#define DHTPIN D7
// Terra Test
#define DHTTYPE DHT11
// Terra links
//#define DHTTYPE DHT22

#define PREFIX ""

#define STATHAT_DOMAIN "api.stathat.com"
#define STATHAT_PORT 80
// Terra Test
#define STATHAT_KEY_T "JFNVs0_D2DeQ3FQxEv1pBCBDTlB5MA~~&"
#define STATHAT_KEY_H "fQyhLTsFJ8yvR9MB_xRu0iBJMDNv"
#define STATHAT_UKEY "ODkzOCCWoLVcCgVRiuRYZyx5ZYF7"

Adafruit_SSD1306 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);
RestClient stathatRest = RestClient( STATHAT_DOMAIN );


float _t = 0;
float _h = 0;
double temperature = 999999;
double humidity = 999999;

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

void sendStatHat( char type[1], float val ) {

    Serial.println("connecting .stathat ...");
    Serial.println( val );

    char* _body;
    strcpy (_body,"key=");
     if( type == "t" ){
        strcat (_body, STATHAT_KEY_T);
    }else{
        strcat (_body, STATHAT_KEY_H);
    }
    strcat (_body,"&ukey=" );
    strcat (_body, STATHAT_UKEY );
    strcat (_body,"&uvalue=" );
    char* _val;
    sprintf(_val, "%f", val);
    strcat (_body, _val );

    String response = "";
    int statusCode = stathatRest.post("/v", _body, &response);
    Serial.print("Status code from server: ");
    Serial.println(statusCode);
    Serial.print("Response body from server: ");
    Serial.println(response);
}


void ttick(){
    if( Time.day() != _lastD ){
        fnDay();
    }
    if( Time.hour() != _lastH ){
        fnHour();
    }
    if( Time.minute() != _lastM ){
        fnMinute();
    }
    if( Time.second() != _lastS ){
        fnSecond();
    }
    _lastD = Time.day();
    _lastH = Time.hour();
    _lastM = Time.minute();
    _lastS = Time.second();
}

DHT dht(DHTPIN, DHTTYPE);

// allow us to use itoa() in this scope
extern char* itoa(int a, char* buffer, unsigned char radix);

int lightPin = A7;
int heatPin = A6;

int lightState = 0;

int onTime = 800;
int offTime = 1930;

int setTimeOn( String _time );
int setTimeOff( String _time );

float heat_to = 29;

int heatState = 0;
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

int setTimeOn( String _time ){
    onTime = _time.toInt();
    return lightState;
}

int setTimeOff( String _time ){
    offTime = _time.toInt();
    return lightState;
}

void fnDay(){
    //Serial.println( "Day" );
    Spark.syncTime();
}

void fnHour(){
    //Serial.println( "Hour" );
}

void fnMinute(){
    //Serial.println( "Minute" );
    processHeating();
    Serial.println(WiFi.localIP());
    sendStatHat( "t", _t );
    //sendStatHat( "h", _h );
}

void fnSecond(){
    //Serial.println( "Second" );
    readSensors();
    drawDisplay();
}

void readSensors(){
    _t = dht.readTemperature();
    temperature = (double) _t;
    _h = dht.readHumidity();
    humidity = (double) _h;
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

void processHeating(){
    int _time = ( Time.hour() * 100 ) + Time.minute();

    Serial.print( "time: " );
    Serial.println( _time );

    if( _time >= onTime && _time < offTime ){
        Serial.println("switch light on");
        //Spark.publish( "lightstate", "ON" );
        digitalWrite( lightPin, HIGH );
        lightState = 1;
        Serial.println("light on");
    }else{
        Serial.println("switch light off");
        //Spark.publish( "lightstate", "OFF" );
        digitalWrite( lightPin, LOW );
        lightState = 0;
        Serial.println("light off");
    }

	Serial.print( "temp DHT: " );
    Serial.println( _t );
    Serial.print( "humi DHT: " );
    Serial.println( _h );

    if( lightState == 1 && _t < 1000 && _t < heat_to ){
        digitalWrite( heatPin, HIGH );
        Serial.println("switch heat on");
        heatState = 1;
    }else{
        digitalWrite( heatPin, LOW );
        Serial.println("switch heat off");
        heatState = 0;
    }

	char _stmp[4];
    itoa( _t, _stmp, 10 );
	Spark.publish( "temp", _stmp );

	char _shum[4];
    itoa( _h, _shum, 10 );
	Spark.publish( "hum", _shum );
}


void setup() {
    Serial.begin(9600);

	Serial.println("Start Terra");
    tsetup();

    display.begin(SSD1306_SWITCHCAPVCC);
    display.display();

    // text display tests
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.clearDisplay();

    Time.zone(1);

    Spark.function("seton", setTimeOn);
    Spark.function("setoff", setTimeOff);

    //Spark.variable("t1_light", &, INT);
    Spark.variable("t1_timeon", &onTime, INT);
    Spark.variable("t1_timeoff", &offTime, INT);

    pinMode(lightPin, OUTPUT);

    dht.begin();
    Wire.begin();

    Spark.variable("t1_temp", &temperature, DOUBLE);
    Spark.variable("t1_humi", &humidity, DOUBLE);
    //Spark.variable("t1_dhtstatus", &dhtstatus, INT);
    //Spark.variable("t1_heat", &heatState, INT);

    pinMode(heatPin, OUTPUT);

    processHeating();
    drawDisplay();
}

void loop() {
    ttick();
}
