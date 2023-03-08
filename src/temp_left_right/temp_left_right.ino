

#include <Wire.h>
#include <Adafruit_AMG88xx.h>

Adafruit_AMG88xx amg;


#define TEMP_DIFF 0.5  // Higher gives less sensitivity, lower more noise   0.4 is best so far
#define FLAG_LOOP_LIMIT 200 // How many spins the loop can go with a flag set, waiting for a human to enter the other block. 10 with all 64 pixels and 16 pixels
#define RED_LED 25
#define GRN_LED 26

//float pixels[AMG88xx_PIXEL_ARRAY_SIZE];
float pixels[64];


float a = 0;   // left block
float b = 0;   // right block
bool leftFlag = false;
bool rightFlag = false;
int loopsWithFlag = 0;   // Number of spins with a flag set. Counting variable. Can not be adjusted.
int inRoom = 0;
bool countedUp = false;
bool countedDwn = false;

// TEST
int start = millis();
int loopCount = 0;

void setup() {
    Serial.begin(115200);
    Serial.println(F("AMG88xx pixels"));

    bool status;
    
    status = amg.begin();
    if (!status) {
        Serial.println("Could not find a valid AMG88xx sensor, check wiring!");
        while (1);
    }
    

    delay(100); // let sensor boot up

    // Set led outputs
    pinMode (26,OUTPUT); // RED led
    pinMode (25,OUTPUT); // Green led

    //turn leds on
    digitalWrite(26,HIGH);
    digitalWrite(25,HIGH);

    delay(3000);

    //Turn leds off
    digitalWrite(26,LOW);
    digitalWrite(25,LOW);
}


void loop() { 
    // Read all the pixels
    amg.readPixels(pixels);

    // 16 pixels: the two rows in the middle on each side
    for (int i=2; i<=63; i=i+8) {
      a = a + pixels[i];
      a = a + pixels[i+1];
      b = b + pixels[i+2];
      b = b + pixels[i+3];
    }

    //obtain average value for each block
    a = (a/16);
    b = (b/16);

// check how many spins a flag has been raised. If nothing happens, reset the flags
    if (leftFlag || rightFlag) {
      loopsWithFlag += 1;

      if (loopsWithFlag > FLAG_LOOP_LIMIT) {
      leftFlag = false;
      rightFlag = false;
      loopsWithFlag = 0;
      }
    }

  // check left block
    if (b>a) {
      if(b-a > TEMP_DIFF && !countedDwn) {
        if (rightFlag && inRoom != 0) { // protection against negative counting
          inRoom = inRoom - 1;
          countedDwn = true;
          rightFlag = false;
        } else {
          digitalWrite (GRN_LED, HIGH);   // LEDS are commented out in the movement sencing for permormance gain.
          digitalWrite (RED_LED, LOW);    // Uncomment them to look for background noice and sensor borders
          leftFlag = true;
          } 
      } else {
          digitalWrite (GRN_LED, LOW);
      }
        
    } else if ((a>b)) {    // check right block
      if(a-b > TEMP_DIFF && !countedUp) {
        if (leftFlag) {
          inRoom = inRoom + 1;
          countedUp = true;
          leftFlag = false;
        } else {
          digitalWrite (RED_LED, HIGH);
          digitalWrite (GRN_LED, LOW);
          rightFlag = true;
          } 
      } else {
          digitalWrite (RED_LED, LOW);
      }
    }


    // reset counting controll when zone is empty
    if (a-b < TEMP_DIFF && b-a < TEMP_DIFF) {
      countedDwn = false;
      countedUp = false;
    }

    // reset block sums
    a = 0;
    b = 0;

    Serial.println(inRoom);

   // uncomment this function to get the  time of how long the flag_loop_limit takes to pass
   // Right now: 1 loop takes 12.24 ms
   /*
    loopCount += 1;
    if (loopCount > FLAG_LOOP_LIMIT) {
      loopCount = 0;
      int time = millis() - start;
      Serial.println(time);
      start = millis();
    }
    */
}
         


      
    
