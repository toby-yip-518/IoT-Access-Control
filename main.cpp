// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//Libraries for the components used: Keypad, MFRC522, Servo, WiFi, LCD, ThingSpeak, json file.
#include "WiFiEsp.h"
#include "ThingSpeak.h"
#include "ArduinoJson.h" 
#include <Servo.h>   
#include <Wire.h>    
#include <SPI.h>     
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#incldue <secrets.h>
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//define for database
#define HOST "api.thingspeak.com"
#define PORT 80
#define ARRAYSIZE 10
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//KeyPad varibles
Servo ServoMotor;
LiquidCrystal_I2C lcd(0x27,16,2);
const byte ROWS = 4; //four rows
const byte COLS = 4; //three columns

char keys[ROWS][COLS] = {
{'1','2','3','A'},
{'4','5','6','B'},
{'7','8','9','C'},
{'*','0','#','D'}
};
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//For the input password array
char key_code[4];
char key_code_otp[4];
char sel[1];
//Set the default password
char password[4]={'4','3','2','1'};

//int k keeps track of the number of keys entered
unsigned int k=0;
byte rowPins[ROWS] = {A7,A6,A5,A4}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {A3,A2,A1,A0}; //connect to the column pinouts of the keypad
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//i is use for the post-increment operator ++, make sure the next key pressed will be stored in the next position of the key_code array
int i=0;
int s=0;

//Two cards' hexidecimal number for scanning the correct card or wrong card
const byte card1[4]{0x90, 0x86, 0x82, 0x1B};
const byte card2[4]{0x3C, 0xA9, 0x13, 0x30};

byte read_card[4];

//Pins for the MFRC522 RFID module
constexpr uint8_t RST_PIN = 5;     
constexpr uint8_t SS_PIN = 53;     
MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class
MFRC522::MIFARE_Key key; 
String array_entry_student[3];
String array_exit_student[3];
int entry_no = 0;
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//API key for the Thingspeak ThingHTTP
const String apiKey = "abc123";

// Emulate Serial1 on pins 0/1 if not present
#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
SoftwareSerial Serial1(0, 1); // RX, TX
#endif

char ssid[] = "abc123";   // your network SSID (name)
char pass[] = "abc123";        // your network password
int status = WL_IDLE_STATUS;     // Wifi status
char server[] = "api.thingspeak.com";

// Initialize the Ethernet client object
WiFiEspClient client;

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Variables for OTP
String        currentOTP = "ABCD";
int ui_setter = 0;
int ui_clear = 0;
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Variables for database
int datachecker = 0;
String array_sid[ARRAYSIZE];
String array_phone[ARRAYSIZE];
String array_password[ARRAYSIZE];
String array_rfid[ARRAYSIZE];
unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;
const char * myReadAPIKey = SECRET_READ_APIKEY;

unsigned long myChannelNumber2 = SECRET_CH_ID_2;
const char * myWriteAPIKey2 = SECRET_WRITE_APIKEY_2;
const char * myReadAPIKey2 = SECRET_READ_APIKEY_2;
bool read_data_check = false;
String current_user = "";
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//Variiables for RGB light & buzzer
int redPin = 10;
int greenPin = 9;
int bluePin = 8;
int buzzerPin = 3;
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void sendSmsMessage(void);

void setup() {
  Serial.begin(9600);
  delay(1000);
//set pin mode for RGB
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
// initialize serial for ESP module
  Serial1.begin(115200);
  ServoMotor.attach(4);
  ServoClose();
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522 
  lcd.begin();                      
  lcd.backlight();
  //intiliaze the RFID variables
  for (byte i = 0; i < 6; i++) {key.keyByte[i] = 0xFF;}
  //show main screen of the system
  lcd.setCursor(0, 0);
  lcd.print("Scan ID Card Or");
  lcd.setCursor(0, 1);
  lcd.print("1.Password 2.OTP");
  
//Initialize for ESP8266 WiFi module and connecting to WiFi
  delay(2000);
  WiFi.init(&Serial1);
  // check for WiFi status if not, keep doing
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue
    while (true);
  }
  //Connect to WiFi when not connected
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, pass);
  }

  //keep get data from database until successful get all data
  while(read_data_check == false){
    read_data_check = ReadData();
    datachecker = 0;
  }
}
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void loop() {
 //set door close when system start
 ServoMotor.write(90);
 //clear the input RFID card variables
 byte nuidPICC[4];
    for(byte i = 0; i < 4; i++)
      nuidPICC[i] = 0;

 //keep checking for user input in keypad
 char selection = keypad.getKey();
 if(selection != NO_KEY){
    sel[i++]=selection;
    s=i;
    delay(200);
    i=0;
 }
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 //When Stat the Password function
 if(sel[0]== '1' && k == 0){
    if(ui_clear == 0){
     lcd.clear();
     ui_clear = 1;
    }
    lcd.setCursor(0, 0);
    lcd.print("Please Enter the");
    lcd.setCursor(0, 1);
    lcd.print("Password:");
    
//Reads keypad inputs and stores them in the 'key_code' array until four keys are entered
    char key = keypad.waitForKey();
    if(key != NO_KEY){ 
      key_code[i++]=key;
      lcd.setCursor(i+8, 1);
      lcd.print("*");     
      k=i;
      delay(200);
    }
     
    if(k==4){
//Check the input passwords
      if(!strncmp(password,key_code,4)){
//Print "Access Granted" on the LCD monitor
        Access_Granted();
        lcd.clear(); 
        lcd.setCursor(0, 0);
        lcd.print("Access Granted");
//The motor will turn 90 degrees
        ServoOpen();
//Reset the key code  
        i=k=0;   
        setup2();
      }else{
//Print "Access Denied" on the LCD monitor
        Access_Denied();
        lcd.clear(); 
        lcd.setCursor(0, 0);
        lcd.print("Access Denied");
        delay(1000); 
//Reset the key code 
        i=k=0;
        setup2();
      }
    }
    k = 0;
 }else
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//When user prees "2" on keypad for using OTP to unlock door
 if(sel[0]== '2' && k == 0){
   if(ui_clear == 0){
    lcd.clear();
    ui_clear = 1;
   }
   //UI when start the OTP function
   if(ui_setter == 0){
      lcd.setCursor(0, 0);
      lcd.print("Enter SID to get");
      lcd.setCursor(0, 1);
      lcd.print("OTP:");
      char key = keypad.waitForKey();
      if(key != NO_KEY){ 
         key_code_otp[i++]=key;
         lcd.setCursor(i+3, 1);
         lcd.print(key_code_otp[i-1]);     
         k=i;
         delay(200);
      }
//Requset user to enter SID on keypad to get OTP send to their phone
      if(k==4){
         String check = String(key_code_otp[0])+String(key_code_otp[1])+String(key_code_otp[2])+String(key_code_otp[3])+String(key_code_otp[4]);
         if(CheckStudentExist(check)){
           current_user = check;
           if(status == WL_CONNECTED) {
             delay(1000);
             lcd.clear() ; lcd.setCursor(0, 0);lcd.print("Sending SMS......");
             Serial.println("Sending SMS......");
             sendSmsMessage(ReturnPhone(check),check);
             i=k=0;
             ui_setter = 1;
           }else{
            lcd.clear(); 
            lcd.setCursor(0, 0);lcd.print("Fail to send");
            lcd.setCursor(0, 1);lcd.print("OTP......");
            setup2();
           }
         }else{
           lcd.clear() ; lcd.setCursor(0, 0);lcd.print("Invalid SID!!!");
           delay(1000);
           i=k=0;
           setup2();
         }
      }
   }else
   //UI After OTP is sent
   if(ui_setter == 1){
      lcd.setCursor(0, 0);
      lcd.print("Please Enter the");
      lcd.setCursor(0, 1);
      lcd.print("OTP:");
      char key = keypad.waitForKey();
      if(key != NO_KEY){ 
         key_code_otp[i++]=key;
         lcd.setCursor(i+3, 1);
         lcd.print("*");     
         k=i;
         delay(200);
      }
// Enter OTP sent from their phone
      if(k==4){
         //show the correct OTP
         Serial.println("Correct OTP is: " + currentOTP);
         //input OTP of user
         String check = String(key_code_otp[0])+String(key_code_otp[1])+String(key_code_otp[2])+String(key_code_otp[3]);
         //show the input OTP of user
         Serial.println("Entered OTP is: " + check);
         //entered OTP is correct
         if(check.compareTo(String(currentOTP)) == 0){
            delay(1000);
            Access_Granted();
            lcd.clear(); lcd.setCursor(0, 0); lcd.print("Access Granted");
            lcd.setCursor(0, 1); lcd.print("Welcome SID:" + current_user);
            ServoOpen();  
            i=k=0;
            setup2();
         }else{
            Access_Denied();
            lcd.clear() ; lcd.setCursor(0, 0);lcd.print("Access Denied");
            delay(2000); 
            i=k=0;
            setup2();
         }
         k = 0;
      }
   }
 } 
 k = 0;
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//Used to check if a new RFID card is present and being detected by the MFRC522 RFID module
    if(!rfid.PICC_IsNewCardPresent())
      return;
//Verify if the NUID has been readed
    if(!rfid.PICC_ReadCardSerial())
      return;

//Used to determine the type of the RFID card that has been detected by the MFRC522 RFID module
    MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  
//Store NUID into nuidPICC array
    for (byte i = 0; i < 4; i++){
      nuidPICC[i] = rfid.uid.uidByte[i];
    }
 //Check the RFID card is it have the same array of hexidecimal number of the correct card
    String rfidexist = CheckRFIDExistReturnSID(nuidPICC);
    if(rfidexist != "-1"){
//Print "Access Granted" on the LCD monitor
      Access_Granted();
      Buzzer_Beep();
      lcd.clear(); 
      lcd.setCursor(0, 0);
      lcd.print("Access Granted");
      lcd.setCursor(0, 1); lcd.print("Welcome SID:" + rfidexist);
//The motor will turn 90 degrees
      ServoOpen();
      setup2();  
//Check the RFID card is it have the same array of hexidecimal number of the wrong card
    }else {
//Print "Access Denied" on the LCD monitor
      Access_Denied();
      lcd.clear();
      lcd.setCursor(0, 0);lcd.print("Access Denied");
//The motor will not move
      ServoClose();
//Print "Wrong card" on the LCD monitor
      lcd.setCursor(0, 1);lcd.print("IDCard not exist");
      delay(2000);
      setup2();
    }
}
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//make HTTP request to Twilio
void sendSMS(String number, String message)
{
  // check WiFi connection well
  while ( status != WL_CONNECTED) {
    //Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, pass);
  }
  
  // Make TCP to the ThingSpeak Host
  if (client.connect(server, 80)){
    Serial.println("Connected to ThingSpeak!");  
    
    client.print("GET /apps/thinghttp/send_request?api_key=");
    client.print(apiKey);
    //Serial.println(apiKey);
    client.print("&number=");
    client.print(number);
    client.print("&message=");
    client.print(message);
    client.println(" HTTP/1.1");
    client.print("Host: ");
    client.println(server);
    client.println("Connection: close");
    Serial.println("Connection: closed");
    client.println();
    
  }
  else
  {
    Serial.println(F("Connection failed"));
    lcd.clear(); 
    lcd.setCursor(0, 0);lcd.print("Faile to send");
    lcd.setCursor(0, 1);lcd.print("OTP......");
    setup2();
  }

  //check the connection of the ThingSpeak
  while (client.connected())
  {
    if ( client.available() )
    {
      char c = client.read();
      Serial.print(c);
    }
  }
  client.stop();
}
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//SMS sending function
void sendSmsMessage(String phone, String sid) {
  byte p;
  currentOTP = String(random(0,9)) + String(random(0,9)) + String(random(0,9)) + String(random(0,9));
  Serial.print("SMS TO: "); Serial.println("+852"+phone);
  sendSMS("+852"+phone, currentOTP);
  Serial.println("Current OTP sent is:" + currentOTP);       
}

//Function of turning 90 degrees of the motor
void ServoOpen()
{
    delay(1000);
    ServoMotor.write(80);
    delay(450);
    ServoMotor.write(90);
    delay(1000);
    ServoMotor.write(100);
    delay(520);
}
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//Function of not moving the motor
void ServoClose()
{
    ServoMotor.write(90);
}
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//setup for initialize
void setup2(){
    ServoMotor.attach(4);
    ServoClose();
    SPI.begin(); // Init SPI bus
    rfid.PCD_Init(); // Init MFRC522 
    lcd.begin();                      
    lcd.backlight();
    for (byte i = 0; i < 6; i++) {key.keyByte[i] = 0xFF;}
    lcd.setCursor(0, 0);
    lcd.print("Scan ID Card Or");
    lcd.setCursor(0, 1);
    lcd.print("1.Password 2.OTP");
    sel[0] = 0;
    currentOTP = "ZZZZ";
    ui_setter = 0;
    ui_clear = 0;
    current_user = "";
    analogWrite(redPin, 0);
    analogWrite(greenPin, 0);
}
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//read data form ThingSpeak database
bool ReadData (){

  if( !client.connect( HOST, PORT ) )
  {
    Serial.println( "connection failed" );
    return false;
  }else{
//String GET = "GET /channels/" + String(myChannelNumber) + "/fields/" + 2 + ".json?key=" + myReadAPIKey + "&results=1";
    String GET = "abc123";

    Serial.println( "**-- Get a Channel Fiels Feed --**" );
    String getStr = GET + " HTTP/1.1\r\n";;
    client.print( getStr );
    client.print( "Host: api.thingspeak.com\n" );
    client.print( "Connection: close\r\n\r\n" );    
    delay(50);

    String section="HEAD";
    while(client.available())
    {
      String line = client.readStringUntil('\r');
//** parsing the JSON response here ! **//
// parse the HTML body
      if(section == "HEAD" )  // HEAD
      {
        Serial.print( "." );
        if( line == "\n" )  
        {
          section = "LENGTH";
        }
      }
      else if( section == "LENGTH" )
      {       
// String content_length = line.substring(1);
        section = "JSON";
      }
      else if( section == "JSON" )    // print the good stuff
      {
        Serial.println( "" );
        section = "END";
        String jsonStr = line.substring(1); 
        int size = jsonStr.length() + 1;
        char json[size];
        jsonStr.toCharArray(json, size);
        Serial.println( json );
        char* feeds = (strstr( json, "feeds") + 8);
        Serial.println(feeds);      
        for(int i = 0; i < size; i++)
        {
          if(json[i] == '}')
          {
            if(json[i+1] == ',')
            {
              if(json[i+2]=='{')
              {
                json[i+1] = ' ';
              }
            }
          }
        }
        char* field;
        field = strtok( feeds, " " );
        while(field) 
        {
          Serial.println("-- Field --");
          Serial.println(field);
          StaticJsonBuffer<2000> jsonBuffer;
          JsonObject& jsonParsed = jsonBuffer.parseObject(field);
          if (!jsonParsed.success())
          {
            Serial.println("parseObject() failed");
            return false;
          }
          const char *createdat = jsonParsed["created_at"];
          int entryid = jsonParsed["entry_id"];
          String id = jsonParsed["field1"];
          String phoneNo = jsonParsed["field2"];
          String password = jsonParsed["field3"];
          String Rfid = jsonParsed["field4"]; 

          array_sid[datachecker] = String(id);
          array_phone[datachecker] = String(phoneNo);
          array_password[datachecker] = String(password);
          array_rfid[datachecker] = String(Rfid);
          Serial.println("-- Database Array --");
          Serial.print( "SID      : " ); Serial.println( array_sid[datachecker] );
          Serial.print( "PhoneNo  : " ); Serial.println( array_phone[datachecker] );
          Serial.print( "Password : " ); Serial.println( array_password[datachecker] );
          Serial.print( "RFID     : " ); Serial.println( array_rfid[datachecker] );
          datachecker++;
          field = strtok( NULL, " " );
        } 
        delete []field;              
      }
    }  
    return true;
  }
  client.stop();
  datachecker = 0;
  return false;
}
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//check the give sid exist or not
bool CheckStudentExist(String sid){
  for(int i = 0;i < 10;i++){
    if(sid == array_sid[i])
    return true;
  }
  return false;
}
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//return the corresponding phone number with the give sid
String ReturnPhone(String sid){
  for(int i = 0;i < 10;i++){
    if(sid == array_sid[i])
    return array_phone[i];
  }
  return "-1";
}
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//chech the RFID exist then return the sid of that user
String CheckRFIDExistReturnSID(byte rfid_from_user[4]){
  byte rfid_from_database[4];

  for(int j=0; j<4; j++){
    int n = 0;
    for(int i=0; i<8; i+=2){
      rfid_from_database[n] = hex_to_dec(array_rfid[j].charAt(i))*16 + hex_to_dec(array_rfid[j].charAt(i+1));
      n++;
    }    
    if(rfid_from_database[0] == rfid_from_user[0] && 
       rfid_from_database[1] == rfid_from_user[1] && 
       rfid_from_database[2] == rfid_from_user[2] && 
       rfid_from_database[3] == rfid_from_user[3]){
//       array_entry_student[entry_no++] = array_rfid[j];
       String entry_sid = array_sid[j];
//       EntryTime (entry_sid);
       return entry_sid;
    }
  }
  return "-1";
}
// -----------------------------------------------------------------------------------------------------------------------------------------------------------------------
//return a dec number with input hex number
int hex_to_dec(char hex){
      switch (hex) {
        case 65:
        return 10;
        break;
        case 66:
        return 11;
        break;
        case 67:
        return 12;
        break;
        case 68:
        return 13;
        break;
        case 69:
        return 14;
        break;
        case 70:
        return 15;
        break;
        default:
        return hex - 48;
        break;
      }
}
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//make a green RGB LED light
void Access_Granted(){
  analogWrite(greenPin, 127);
}
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//make a red RGB LED light
void Access_Denied(){
  analogWrite(redPin, 127);
}
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//Make a beep sound
void Buzzer_Beep(){
    digitalWrite(buzzerPin,HIGH);
    delay(100);
    digitalWrite(buzzerPin,LOW);
}