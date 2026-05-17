# IoT Access Control System

**University group project (3 members) · My role: integration lead.**

An Arduino-based campus access control prototype integrating three independent authentication paths: 
RFID card scan, keypad password, and SMS-delivered OTP — backed by 
a cloud-hosted user record database.

## Authentication Paths

The system accepts any of three authentication methods:

1. **RFID Card** — Card UID is scanned via MFRC522, hex-decoded, 
   and matched against cloud-stored records
2. **Keypad Password** — 4-digit code entered on a 4×4 matrix keypad
3. **OTP** — User enters a Student ID, system generates a 4-digit 
   OTP, sends via SMS through Twilio (relayed via ThingSpeak's 
   ThingHTTP), and verifies the entered code

Successful authentication actuates a servo motor (door open) with 
green-LED + buzzer feedback. Failed authentication shows red-LED 
feedback on the LCD.

## Tech Stack

**Hardware** &nbsp; Arduino Mega · ESP8266 · MFRC522 RFID · 4×4 Keypad · 
Servo · 16×2 I2C LCD · RGB LED · Buzzer  
**Cloud / API** &nbsp; ThingSpeak (user records + entry/exit logs) · 
Twilio SMS via ThingHTTP  
**Libraries** &nbsp; ArduinoJson · WiFiEsp · MFRC522 · LiquidCrystal_I2C · 
Keypad

## Architecture
```
┌─────────────────────────────────────┐
│  Arduino Mega                       │
│   ├── MFRC522 RFID  ─┐              │
│   ├── 4×4 Keypad    ─┼─▶ Auth Logic─▶ Servo / LCD / RGB / Buzzer
│   └── ESP8266 WiFi  ─┘              │
└─────────────────┬───────────────────┘
│  HTTP / JSON
▼
┌──────────────────────┐         
│  ThingSpeak Cloud    │ ───────▶  Twilio SMS   
│  (channel: users +   │    via     (OTP send)  
│   entry/exit logs)   │  ThingHTTP
└──────────────────────┘
```
## My Role

Led the integration workstream — consolidating individual hardware 
modules from team members into a working end-to-end system, including 
the state machine that handles transitions between RFID / password / 
OTP authentication paths on the LCD UI.

## Documented Security Limitations

The prototype was deliberately scoped as a functional demo, with 
known limitations documented in the project report:

- **Plaintext credential storage** — passwords and OTPs are stored 
  and compared in plaintext; production systems would hash with 
  bcrypt or similar
- **Hardcoded card UID matching** — production would use a 
  cryptographic challenge-response (e.g., DESFire) instead of UID 
  comparison, which is spoofable
- **No replay protection on OTP** — the OTP has no time-bound 
  expiry; a captured SMS could be reused within the session window
- **No audit logging beyond entry/exit events** — failed attempts 
  are not persisted

## Build / Run

Requires Arduino IDE with the listed libraries installed. Configure 
WiFi credentials, ThingSpeak channel IDs, and API keys via a local 
`secrets.h` (not committed). Upload to Arduino Mega.
