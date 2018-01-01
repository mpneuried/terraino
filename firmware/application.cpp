#include "application.h"
#include "DHT.h"
#include "Adafruit_SSD1306.h"
#include "Adafruit_GFX.h"

#define USEDISPLAY
//#define LOGGING
#define NOLED

#define PUBLISH_KEY_T "stathat-webhook-test-t"
#define PUBLISH_KEY_H "stathat-webhook-test-h"
// #define PUBLISH_KEY_T "stathat-webhook-terra-t"
// #define PUBLISH_KEY_H "stathat-webhook-terra-h"

#define STATHAT_UKEY "ODkzOCCWoLVcCgVRiuRYZyx5ZYF7"
// Terra TEST
#define STATHAT_KEY_T "JCITlBZ-T8t-fqpqfVCeSyBDTGRqbg~~"
#define STATHAT_KEY_H "80bUeMTWJ2tfSUMkTtaRgiBZSXp0Uw~~"
// Terra Left
//#define STATHAT_KEY_T "WlvGjNiq3Wn1_jpRajNBfiBDVEFLTQ~~"
//#define STATHAT_KEY_H "4IOEoN0qaoL2Lvvp4jWzWiBFaWlpcA~~"

#define DHTPIN D7
// Terra TEST
//#define DHTTYPE DHT11
// Terra Left
#define DHTTYPE DHT22


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
char auth[] = "f00748976def43f3b0ff738810903d1b";

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
    int _ts = Time.second();
    if( _ts != _lastS ){
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
    _lastS = _ts;
    _lastM = Time.minute();
    _lastH = Time.hour();
    _lastD = Time.day();
}

void fnDay(){
    #ifdef LOGGING
    Serial.println( "TIMER >\tDay" );
    #endif
    Particle.syncTime();
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

    #ifdef LOGGING
    Serial.print( "PUBLISH >\tT " );
    Serial.println( String( temperature ) );
    Serial.print( "PUBLISH >\tH " );
    Serial.println( String( humidity ) );
    #endif

    if( temperature > -10 && temperature < 60 ){
        Particle.publish( PUBLISH_KEY_T, String(temperature), 60, PRIVATE);
    }
    if( humidity > 10 && humidity < 60 ){
        Particle.publish( PUBLISH_KEY_H, String(humidity), 60, PRIVATE);
    }

    processRelais();
}

void fnSecond(){
    #ifdef LOGGING
    Serial.print( "TIMER >\tSecond " );
    Serial.println( _lastS );
    #endif

    readSensors();
    #ifdef USEDISPLAY
    drawDisplay();
    #endif
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

    char _status[30];
    sprintf(_status, "LIGHT:%i HEAT:%i TIME:%i", lightState, heatState, _time);
    Particle.publish( "STATUS", _status, 60, PRIVATE );
}




/**

GENERAL

*/
// allow us to use itoa() in this scope
extern char* itoa(int a, char* buffer, unsigned char radix);

void publishMetrics(){
    char _stmp[4];
    itoa( _t, _stmp, 10 );
    Particle.publish( "temp", _stmp );

    char _shum[4];
    itoa( _h, _shum, 10 );
    Particle.publish( "hum", _shum );
}

void setup() {
    Serial.begin(9600);

    //Blynk.begin(auth);

    #ifdef NOLED
    RGB.control(true);
    RGB.brightness(0);
    #endif

    Time.zone(1); // Winter-time
    //Time.zone(2); // Summer-time

    tsetup();

    #ifdef USEDISPLAY
    setupDisplay();
    #endif

    dht.begin();

    setupRelais();
    processRelais();

    #ifdef USEDISPLAY
    drawDisplay();
    #endif
}

void loop() {
    ttick();
    //Blynk.run();
    delay(5000);
}
