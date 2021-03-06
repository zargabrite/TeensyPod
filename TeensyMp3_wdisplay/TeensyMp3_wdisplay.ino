#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <Bounce.h>

AudioPlaySdWav           playSdWav1;
AudioOutputI2S           i2s1;
AudioConnection          patchCord1(playSdWav1, 0, i2s1, 0);
AudioConnection          patchCord2(playSdWav1, 1, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;

Bounce button0 = Bounce(0, 15);
Bounce button1 = Bounce(1, 15);
Bounce button2 = Bounce(2, 15);  // 15 = 15 ms debounce time

// Use these with the Teensy Audio Shield
#define SDCARD_CS_PIN    10
#define SDCARD_MOSI_PIN  7
#define SDCARD_SCK_PIN   14

// Use these with the Teensy 3.5 & 3.6 SD card
#define SDCARD_CS_PIN    BUILTIN_SDCARD
#define SDCARD_MOSI_PIN  11  // not actually used
#define SDCARD_SCK_PIN   13  // not actually used

/*-----------------------------------------------------------------
 *                          DISPLAY
 *---------------------------------------------------------------*/
//setup hex codes for SHIFTOUT method
int dec_digits [10] {126, 48, 109, 121, 51, 91, 95, 112, 127, 123};

//declare the pins we're using on the Teensy
int latchPin = 8; //aka "RCLK" or "STCP"
int clockPin = 6; //aka "SRCLK" or "SHCP"
int dataPin = 12; //aka "SER" or "DS"
//------------------------------------------------------------------



//SETUP-------------------------------------------------------------
void setup() {
  
    //DISPLAY pin setup
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  
  Serial.begin(9600);
  AudioMemory(8);
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.5);
  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);
  if (!(SD.begin(SDCARD_CS_PIN))) {
    while (1) {
      Serial.println("Unable to access the SD card");
      delay(500);
    }
  }
  
  //pinMode(13, OUTPUT); // LED on pin 13
  pinMode(0, INPUT_PULLUP);
  pinMode(1, INPUT_PULLUP);
  pinMode(2, INPUT_PULLUP);
  delay(1000);
}

const char * filelist[4] = {
  "DOG.WAV", "DOG2.WAV", "DOG3.WAV", "DOG4.WAV"
};

int playState = playSdWav1.isPlaying();
int autostepper = false;
int filenumber = 0;  // which file to play
int playPos = playSdWav1.positionMillis();

//LOOP--------------------------------------------------------------
void loop() {
  const char *filename = filelist[filenumber];
  /*PRINT WHEN NOT PLAYING (GLOBALLY)
  if (playSdWav1.isPlaying() == false){
    Serial.println("NOTHING PLAYING");
  }*/

  //PLAY/STOP BUTTON
  button1.update();
  //PLAY FUNCTION
  if(button1.fallingEdge() && playSdWav1.isPlaying() == false){
    /*PRINT LOCATION*/
    Serial.println("@PLAY");
    /*PRINT FILE#
    Serial.print("FILENUMBER: ");
    Serial.println(filenumber);
    */
    autostepper = true;
    playSdWav1.play(filename);
    Serial.print("PLAYING: ");
    Serial.println(filename);
    delay(10);
    //Serial.println("delay over");
    /*PRINT BOOLEAN
    Serial.print("@else if (end), boolean: ");
    Serial.println(playState);
    */
  }
  //STOP FUNCTION
  else if(button1.fallingEdge() && playSdWav1.isPlaying() == true){
    autostepper = false;
    playSdWav1.stop();
    playState = false;
    Serial.println("STOPPED");
  }

  //AUTOSTEPPER (AFTER TRACK HAS FINISHED PLAYING)
  if(playSdWav1.isPlaying() == false && autostepper == true){
    Serial.println("AUTOSTEPPING");
    filenumber = filenumber + 1;
    if (filenumber >= 4){
      filenumber = 0; 
    }
    autostepper = false;
  }

  
  //SKIP FORWARD BUTTON
  button0.update(); 
  if (button0.fallingEdge()) {
    Serial.println("@FORWARD SKIP BUTTON");
    //FILE NUMBER STEPPER
    playSdWav1.stop();
    filenumber = filenumber + 1;
    if (filenumber >= 4){
      filenumber = 0;
    }
    autostepper = false;
    /*
    playSdWav1.play(filename);
    Serial.print("PLAYING: ");
    Serial.println(filename);
    delay(10);*/
  }

  //SKIP BACK BUTTON
  button2.update();
  if (button2.fallingEdge()) {
    Serial.println("@BACK SKIP BUTTON");
    playSdWav1.stop();
    filenumber = filenumber - 1;
    if (filenumber < 0) filenumber = filenumber + 4;
    autostepper = false;
  }

  //VOLUME KNOB
  // read the knob position (analog input A3)
  int knob = analogRead(A3);
  float vol = (float)knob / 1280.0;
  sgtl5000_1.volume(vol);
  //Serial.print("volume = ");
  //Serial.println(vol);

/*-----------------------------------------------------------------
 *                          DISPLAY
 *---------------------------------------------------------------*/
  digitalWrite(latchPin, LOW);
  /*change the number here to a variable, then set that variable
   * equal to the track numberq. This will need to be split into
   * separate digits...
   */
  shiftOut(dataPin, clockPin, LSBFIRST, dec_digits[filenumber+1]);
  shiftOut(dataPin, clockPin, LSBFIRST, dec_digits[0]);
  digitalWrite(latchPin, HIGH);
//------------------------------------------------------------------
}
