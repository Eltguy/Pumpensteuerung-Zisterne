/*
Titel     : Pumpensteuerung mit Konduktivsensor
--------------------------------------------------------------------------------------
Funktion  : Pumpensteuerung für Zisterne mit Konduktiv- und digitalem Temperatursensor.
            Anzeige über zweizeiliges Standard-LCD. Kommunikation von Display mittels 
            I²C-Bus. Temperatursensor kommuniziert über OneWire Bus. 
Projekt   : D:\_Elektronik\Eigenprojekte\_HAUSAUTOMATISATION\Zisterne mit Kunduktivsensor
Hardwareversion: V1.0
--------------------------------------------------------------------------------------
Prozessor : ARDUINO Nano Board
Takt      : 16MHz extern 
Datum     : 11.05.2025
Version   : 1.0
Autor     : c 2025 by Peter Lampe

*/
//------------------------------------- Libraries -------------------------------------
#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
//#include <Wire.h>
#include <LiquidCrystal_I2C.h>
//--------------------------------------- Defines -------------------------------------
#if defined(ARDUINO) && ARDUINO >= 100
#define printByte(args)  write(args);
#else
#define printByte(args)  print(args,BYTE);
#endif

#define ONSWITCH 10                                     //Input/Pullup: Taster EIN, links
#define OFFSWITCH 11                                    //Input/Pullup: Taster AUS, rechts
#define SKIM 2                                          //Input: Skimmerschalter, H-aktiv
#define LV0 7                                           //Input: Konduktivsonde für Level 0; H-aktiv
#define LV1 6                                           //Input: Konduktivsonde für Level 1; H-aktiv
#define LV2 4                                           //Input: Konduktivsonde für Level 2; H-aktiv
#define LV3 5                                           //Input: Konduktivsonde für Level 3; H-aktiv
#define LV4 3                                           //Input: Konduktivsonde für Level 4; H-aktiv
#define REL 12                                          //Output: zum Schalten des Pumpenrelais; H-aktiv
#define OFF 0                                           //Schaltzustand "aus"
#define ON 1                                            //Schaltzustand "ein"
#define SOMMER 1                                        //Deffinition Jahreszeit 
#define WINTER 0
#define FROSTTEMP 2                                     //Umschalttemperatur für Frosterkennung
#define ONTIME 6                                       //Nachlaufzeit der Pumpe in Sekunden

//---------------------------------- globale Variablen --------------------------------
bool Frost=false;                                       //Flag zur Frost-Erfassung (0=kein Frost, 1=Frost)
bool Season=SOMMER;                                     //Jahreszeit, mit Sommer initialisieren
bool Level[5]={0, 0, 0, 0, 0};                          //Feld mit Schaltzuständen KONduktivsensor
                                                        //zum Start "Zisterne leer" initialisieren
uint8_t OnLevel=LV4;                                    //Einschaltlevel (für Sommer initialisiert)
uint8_t OFFLevel=LV1;                                   //Abschaltlevel, unabhängig von der Jahreszeit
volatile uint8_t TimeDelay=0;                           //Timer1 Verzögerungszähler in Sekunden
                           

uint8_t my1[8] = {0x0,0x4,0x4,0x4,0x4,0x4,0x0};         //Sonderzeichendefinition für Display
uint8_t my2[8] = {0x0,0x1,0x2,0x4,0x8,0x10,0x0};
uint8_t my3[8] = {0x0,0x0,0x0,0x1f,0x0,0x0,0x0};
uint8_t my4[8] = {0x0,0x10,0x8,0x4,02,0x1,0x0};
uint8_t my5[8] = {0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f};
uint8_t my6[8] = {0x3,0x2,0x2,0x2,0x2,0x2,0x3};
uint8_t my7[8] = {0x1f,0x0,0x0,0x0,0x0,0x0,0x1f};
uint8_t my8[8] = {0x18,0x1,0x3,0x7,0x3,0x1,0x18};

//------------------------------------- Prototypes ------------------------------------
void get_Temp (void);                       //Temperatur auslesen, darstellen und Flag setzen
void show_Intro (void);                     //Anzeige Startbildschirm
void show_Level(void);                      //Anzeige der Pegelstände im Display
void move_Wheel(bool action);               //zeigt Aktivitätssymbole für Pumpe an (0=aus; 1=an)

//--------------------------- fundamentale Systemeinstellungen ------------------------
#define ONE_WIRE_BUS 9                      //OneWire-Bus an D2 (2) bis D12 (12)möglich, D13 nicht!
OneWire oneWire(ONE_WIRE_BUS);              //OneWire-Instanz des OneWire-Busses erzeugen
DallasTemperature sensors(&oneWire);        //Übergeben Sie unsere oneWire-Referenz an DS18B20
LiquidCrystal_I2C lcd(0x27,20,4);           //LCD an I²C-Adresse 0x27; 16 Zeichen; 2 Zeilen 
                                            //SDA=A4; SCL=A5 at ARDUINO NANO by default

//--------------------------------------- Setup ---------------------------------------
void setup(void)
{
  //Serial.begin(115200);                     //serial port initialisieren (nur für Debugzwecke)
  sensors.begin();                          //Startup Sensor-Library
  lcd.init();                               //LCD-Display initialisieren
  lcd.backlight();                          //Hintergrundlicht an
  
  pinMode(ONSWITCH, INPUT_PULLUP);          //Input/Pullup: linker Taster EIN (grün)
  pinMode(OFFSWITCH, INPUT_PULLUP);         //Input/Pullup: rechter Taster AUS (rot)
  pinMode(SKIM, INPUT);                     //Input SKIMmer-Schalter
  pinMode(LV0, INPUT);                      //Input Levelsonde 0
  pinMode(LV1, INPUT);                      //Input Levelsonde 1
  pinMode(LV2, INPUT);                      //Input Levelsonde 2
  pinMode(LV3, INPUT);                      //Input Levelsonde 3
  pinMode(LV4, INPUT);                      //Input Levelsonde 4
  pinMode(REL, OUTPUT);                     //Schaltet Relais H-aktiv
                                            
                                            //Initialisierung der Sonderzeichen
  lcd.createChar(0, my1);                   //Action-Symbol 1       
  lcd.createChar(1, my2);                   //Action-Symbol 2
  lcd.createChar(2, my3);                   //Action-Symbol 3
  lcd.createChar(3, my4);                   //Action-Symbol 4
  lcd.createChar(4, my5);                   //Brunnensegment "voll"
  lcd.createChar(5, my6);                   //Brunnenteufe
  lcd.createChar(6, my7);                   //Brunnensegment "leer"
  lcd.createChar(7, my8);                   //oberer Brunnenrand
  lcd.home();

  show_Intro();                             //Eingangsbildschirm anzeigen
  lcd.printByte(5);                         //"Brunnenboden" statisch anzeigen
  show_Level();                             //initiale Pegelanzeige  
  lcd.setCursor(0, 1);                      //Tastenmenü positionieren
  lcd.print("On <-- S --> Off");            //und anzeigen
  move_Wheel(OFF);                          //Animation aus
                                            //Timer1 initialisieren 
  TCCR1A=0x00;                              //Normal Mode
  TCCR1B=(1<<CS12);                         //Prescaler = 256; timer läuft
  TCNT1=0xBDC;                              //Timer1 Preloading für 1s im Hex-Format
  TIMSK1=(1<<TOIE1);                        //Interruptfreigabe

//while(1);//Debugstop                        //Debugstop mit Terminalausgabe
}

//------------------------------------- Main loop -------------------------------------

void loop(void)
{                                               //immer wird
get_Temp();                                     //Temperaturerfassung
show_Level();                                   //und Pegelanzeige vorgenommen

if (Frost==false)                               //ist Brunnen frostfrei?
  {                                             //ja, dann Tastenabfrage und Automatikbetrieb möglich
    if (!digitalRead(ONSWITCH))                 //EIN-Schalter gedrückt?
      {                                         //ja, dann
        digitalWrite(REL, ON);                  //Relais an
      }

    if (!digitalRead(OFFSWITCH))                //Aus-Schalter gedrückt?
      {                                         //ja, dann
        digitalWrite(REL, OFF);                 //Relais aus
      }

    if (!digitalRead(ONSWITCH) && !digitalRead(OFFSWITCH))
                                                //beide Schalter gleichzeitig gedrückt?
      {                                         //ja, dann erst mal
        digitalWrite(REL, OFF);                 //Relais aus
        lcd.setCursor(0, 1);                    //und Curser für Tastenmenü positionieren
      
        if(Season==SOMMER)                      //ist aktuell SOMMER eingestellt?
          {                                     //ja, dann
            Season=WINTER;                      //auf WINTER schalten,
            lcd.print("On <-- W --> Off");      //Tastermenü aktualisieren
            OnLevel=LV2;                        //und oberen Level für diese Betriebsart festlegen
          }
        else                                    //nein, aktuell ist WINTER eingestellt
          {                                     //deshalb
            Season=SOMMER;                      //auf SOMMER schalten
            lcd.print("On <-- S --> Off");      //und Tastermenü aktualisieren
            OnLevel=LV4;                        //oberen Level für diese Betriebsart festlegen
          }
        _delay_ms(500);                         //zusätzliche Verzögerung, um Umspringen
      }                                         //bei längerem Drücken zu vermeiden

    if(digitalRead(REL))                        //ist Relais an?
      {                                         //ja, dann 
        move_Wheel(ON);                         //animiertes Symbol ausgeben
      }
      else                                      //nein,
      {                                         //dann
        move_Wheel(OFF);                        //statisches Symbol anzeigen
      }
 


//-----------------------------neue Passage ab hier---------------------------------
    if(digitalRead (OnLevel) || digitalRead (SKIM))
                                                //Abpumplevel erreicht oder Schwimmerschalter an?
      {                                         //ja, dann
        TCCR1B=(1<<CS12);                       //Prescaler = 256; Timer1 läuft, Nachlaufzeit
      }                                         
     
    if(!digitalRead(LV1))                       //unterer Abschaltpegel unterschritten?
      {
        if(!digitalRead(ONSWITCH))              //auch Ein-Schalter gedrückt?
          {                                     //ja, dann  
            digitalWrite(REL, ON);              //Relais an,solange gedrückt ist
         }
         else if(!digitalRead(REL))             //nein, Ein-Schalter nicht gedrückt aber Relais aus?
         {                                      //dann
           TCCR1B=(1<<CS12);                    //Prescaler = 256; Timer für Nachlauf starten
         }
      }                                      
//------------------------------------neuePassage Ende  ----------------------------------------    
 
 

  }
else                                            //Frost wurde erkannt,
  {                                             //alle Funktionen aus
    digitalWrite(REL, OFF);                     //und Relais aus
  }
}
//while(1);//Debugstop                        //Debugstop mit Terminalausgabe

//------------------------------------- Functions -------------------------------------
void get_Temp (void)                        //Temperatur auslesen, darstellen und Flag setzen
{
  sensors.requestTemperatures();            //globale Temperaturanforderungen an alle Geräte auf dem Bus
  int Temp = sensors.getTempCByIndex(0);    //Temperatur vom ersten Sensor als Integer holen

  if (Temp != DEVICE_DISCONNECTED_C)        //erfolgreiche Datenerfassung?
  {                                         //ja, dann
    lcd.setCursor(11, 0);                   //Curser positionieren
    lcd.print(Temp);                        //Wert ausgeben
    lcd.printByte(223);                     //Maßeinheit
    lcd.print("C  ");                       //anhängen
  }                                         
  else                                      //nein, Sensor ab oder defekt
  {
    lcd.setCursor(11, 0);                   //Curser positionieren
    lcd.print("---   ");
    return;                                 //und ohne Temperaturänderung zurück
  }

  if(Temp<FROSTTEMP)                        //Frostgefahr?
  {
    Frost=true;                             //ja, dann Frost-Flag setzen
    lcd.setCursor(8, 0);                    //Curser setzen,
    lcd.print("!!");                        //Frostwarnung ausgeben
    digitalWrite(REL, OFF);                 //und Relais ausschalten

  }
  else if (Temp>FROSTTEMP)                  //nein, kein Frost
  {                                         //dann
    Frost=false;                            //Frost-Flag löschen
    lcd.setCursor(9, 0);                    //"*" für "OK"
    lcd.print("*");                         //ausgeben
  }
  return;                                   //und zurück
}                                          
 
//-------------------------------------------------------------------------------------------
void show_Intro (void)                      //Intro-Bildschirm anzeigen
{
  lcd.clear();                              //Bildschirm putzen
  lcd.print(" ZISTERNE  V1.0");             //Text erste Zeile ausgeben
  lcd.setCursor(0, 1);                      //Text zweite Zeile ausgeben
  lcd.print("c2025 by P.Lampe ");
  _delay_ms(3000);                          //Anzeigezeit abwarten
  lcd.clear();                              //und dann Bildschirm wieder putzen
  return;
}

//-------------------------------------------------------------------------------------------
void show_Level (void)                      //Anzeige der Pegelstände im Display
{
  Level[0]=digitalRead(LV0);                //aktuelle Sonden-Level in Array einlesen
  Level[1]=digitalRead(LV1);
  Level[2]=digitalRead(LV2);
  Level[3]=digitalRead(LV3);
  Level[4]=digitalRead(LV4);
  
  for (int i=0; i<5; i++)                   //Darstellung des Status im Display
  {
    lcd.setCursor(i+1, 0);                  //Curser an entsprechende Stelle platzieren
    if(Level[i])                            //Level erreicht?
    {                                       //ja, dann
      lcd.printByte(4);                     //Segment "voll" anzeigen
    }
    else                                    //nein,
    {                                       //dann
      lcd.printByte(6);                     //Segment "leer" anzeigen
    }

  }
  lcd.setCursor(6, 0);                      //Curser an letzte Position
  lcd.printByte(7);                         //"Brunnenrand" anzeigen
  return;
}

//-------------------------------------------------------------------------------------------
void move_Wheel(bool action)                //zeigt Aktivitätssymbole für Pumpenrelais an 
{                                           //0=aus; 1=an
  static int counter;                       //als "static" deklarieren, damit Wert erhalten bleibt
  if(action == true)                        //Animation erzeugen?
  {                                         //ja, dann
    if(counter>3)                           //Wert für counter begrenzen
    {
      counter=0;              
    }
    lcd.setCursor(8, 0);                    //Curser platzieren
    lcd.printByte(0+counter);               //Animationsframe anzeigen
    counter++;                              //für nächsten Aufruf inkrementiern
  }
  else
  {
    lcd.setCursor(8, 0);                    //Curser platzieren
    lcd.printByte(0);                       //starres Symbol "|" anzeigen
  }
  return;                                   //Rücksprung
}
//-------------------------------------------------------------------------------------------
ISR(TIMER1_OVF_vect)
{
  TimeDelay++;                              //Verzögerungszeit hochzählen  
  if (TimeDelay<=ONTIME)
  {
     digitalWrite(REL,ON);                  //Relais an
  }
  else
  {
    digitalWrite(REL, OFF);                 //Relais aus
    TCCR1B=0;                               //Timer anhalten und
    TimeDelay==0;                           //Verzögerungszzähler wieder zurücksetzen
  }
  TCNT1 = 0xBDC;                            //erneutes Timer-Preloading für 1s im Hex-Format  
}
//-------------------------------------------------------------------------------------------
// Ende der Datei main.cpp