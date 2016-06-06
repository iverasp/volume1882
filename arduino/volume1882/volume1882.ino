#include <Wire.h>
#include <Encoder.h>
#include <EEPROM.h>

const short red = 9; // pwm
const short green = 10; // pwm
const short potVdd = A3;
const short encA = 2; // interrupt
const short encB = 3; // interrupt
const int encMax = 62;
const short encDiv = 4; // encoder counts 4 steps per click
Encoder encoder(encB, encA);

const int muteButtonPin = 4;
int muteButtonState;
int lastMuteButtonState = LOW;
long lastMuteDebounceTime = 0;
long debounceDelay = 50;

const int outputSelectPin = 7;
int outputSelectState;
int lastOutputSelectState;
long lastOutputSelectDebounceTime = 0;
const int outputRelayPin = 8;
const int headphonesOutput = 0;
const int powerampOutput = 1;

bool mute = false;
byte muteAddress = 0;
byte attenuationAddress = 1;
byte headphonesMuteAddress = 0;
byte headphonesAttenuationAddress = 1;
byte powerampMuteAddress = 2;
byte powerampAttenuationAddress = 3;
byte lastAttenuation;
byte muteAttenuation = 63;
byte fadeStart = 50;
byte fadeStep = fadeStart;
byte fadeAmount = 5;

// A0, A1, A2 of DS1882 are shorted to ground
const byte potAddress = B0101000;
const byte potConfig = B10000110; // volatile memory, zero-crossing detection, 63 positions 

unsigned long attenuationChangeTimer = 0;
unsigned long startTime = 0;
unsigned long attenuationChangeTimeout = 2000;
byte lastStoredAttenuation = 0;

void setup() {
  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);
  delay(300); // wait for Vcc on DS1882 to stabilize
  pinMode(potVdd, OUTPUT);
  digitalWrite(potVdd, HIGH);
  
  Serial.begin(9600);
  Serial.println("Starting execution");
  
  Wire.begin();
  //TWBR = 12; // Sets i2c speed to 400kHz (max)
  Wire.beginTransmission(potAddress);
  byte error = Wire.endTransmission();
  if (error == 0) {
    Serial.print("DS1882 found at address ");
    Serial.println(potAddress);
  }
  Wire.requestFrom(potAddress, 3); // read pot0, pot1 and config
  byte pot1, pot2, conf;
  while (Wire.available()) {
    pot1 = Wire.read();
    pot2 = Wire.read();
    conf = Wire.read();
  }
  if (conf != potConfig) {
    Serial.println("DS1882 config is incorrect. Uploading correct config...");
    Wire.beginTransmission(potAddress);
    Wire.write(potConfig);
    byte error = Wire.endTransmission();
    Serial.println(error);
  }

  pinMode(outputSelectPin, INPUT);
  int lastOutputSelectState = digitalRead(outputSelectPin);
  pinMode(outputRelayPin, OUTPUT);
  selectOutput(digitalRead(outputSelectPin));
}

void loop() {
  if (!mute) {
    long value = readEncoder();
    analogWrite(red, ((((value+2) * 4) - 255)*-1)+1);
    analogWrite(green, value * 4);
    if (value != lastAttenuation) {
      adjustAttenuation(value);
      attenuationChangeTimer = millis();
    }
    if (millis() - attenuationChangeTimer > attenuationChangeTimeout) {
      storeAttenuation(value);
    }
  } else {
    analogWrite(red, fadeStep);
    analogWrite(green, (fadeStep - 255)*-1);
    fadeStep += fadeAmount;
    if (fadeStep < fadeStart || fadeStep > 200) {
      fadeAmount = -fadeAmount;
    }
    delay(30);
  }
  readMuteButton();
  readToggleSwitch();
}

long readEncoder() {
  long value = encoder.read();
  if (value > encMax * encDiv) {
    encoder.write(encMax * encDiv);
    return encMax;
  }
  if (value < 0) {
    encoder.write(0);
    return 0;
  }
  return value / encDiv;
}

void readMuteButton() {
  int reading = digitalRead(muteButtonPin);
  if (reading != lastMuteButtonState) {
    lastMuteDebounceTime = millis();
  }
  if ((millis() - lastMuteDebounceTime) > debounceDelay) {
    if (reading != muteButtonState) {
      muteButtonState = reading;
      if (muteButtonState == HIGH) {
        toggleMute();
      }
    }
  }
  lastMuteButtonState = reading;
}

void readToggleSwitch() {
  int reading = digitalRead(outputSelectPin);
  if (reading != lastOutputSelectState) {
    lastOutputSelectDebounceTime = millis();
  }
  if ((millis() - lastOutputSelectDebounceTime) > debounceDelay) {
    if (reading != outputSelectState) {
      outputSelectState = reading;
      selectOutput(outputSelectState);
    }
  }
  lastOutputSelectState = reading;
}

void toggleMute() {
  mute = !mute;
  if (EEPROM.read(muteAddress) != mute) {
    EEPROM.write(muteAddress, mute);
  }
  fadeStep = fadeStart;
  if (mute) {
    storeAttenuation(readEncoder());
    adjustAttenuation(muteAttenuation);
  } else {
    encoder.write(lastAttenuation * encDiv);
    adjustAttenuation(readEncoder());
  }
}

void adjustAttenuation(long value) {
  if (value != muteAttenuation) {
    lastAttenuation = value;
    Serial.print("Adjusting attenuation to ");
    Serial.print(value);
    Serial.println(" dB");
  } else {
    Serial.println("Muting");
  }

  Wire.beginTransmission(potAddress);
  Wire.write(value);
  Wire.endTransmission();
  Wire.beginTransmission(potAddress);
  Wire.write(value | B01000000);
  Wire.endTransmission();

}

void storeAttenuation(long value) {
  if (lastStoredAttenuation != value) {
    Serial.println("Saving attenuation value to EEPROM...");
    EEPROM.write(attenuationAddress, value);
    lastStoredAttenuation = value;
  }
}

void selectOutput(byte output) {
  if (output == 0) {
    Serial.println("Switching output to headphones");
    muteAddress = headphonesMuteAddress;
    attenuationAddress = headphonesAttenuationAddress;
  } else if (output == 1) {
    Serial.println("Switching output to poweramp");
    muteAddress = powerampMuteAddress;
    attenuationAddress = powerampAttenuationAddress;
  }
  digitalWrite(outputRelayPin, output);
  setInitialValues();
}

void setInitialValues() {
  mute = EEPROM.read(muteAddress);
  byte currentAttenuation = EEPROM.read(attenuationAddress);
  encoder.write(currentAttenuation * encDiv);
  lastAttenuation = currentAttenuation;
  lastStoredAttenuation = currentAttenuation;
  if (mute) {
    adjustAttenuation(muteAttenuation);
  } else {
    adjustAttenuation(currentAttenuation);
  }
}

