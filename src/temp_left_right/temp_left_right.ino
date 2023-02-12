

#include <Wire.h>
#include <Adafruit_AMG88xx.h>

Adafruit_AMG88xx amg;

//float pixels[AMG88xx_PIXEL_ARRAY_SIZE];
float pixels[64];


float a = 0;   // left block
float b = 0;   // right block
bool leftFlag = false;
bool rightFlag = false;
int loopsWithFlag = 0;   // Number of spins with a flag set. Counting variable. Can not be adjusted.
float hysteres = 0.4;  // Higher gives less sensitivity, lower more noise   0.4 is best so far
int flagLoopLimit = 10; // How many spins the loop can go with a flag set, waiting for a human to enter the other block. 10 with all 64 pixels and 16 pixels
int inRoom = 0;

void setup() {
    Serial.begin(115200);
    Serial.println(F("AMG88xx pixels"));

    bool status;
    
    status = amg.begin();
    if (!status) {
        Serial.println("Could not find a valid AMG88xx sensor, check wiring!");
        while (1);
    }
    
    Serial.println("-- Pixels Test --");

    Serial.println();

    delay(100); // let sensor boot up

    // Set led outputs
    pinMode (6,OUTPUT); // RED led
    pinMode (8,OUTPUT); // Green led

    //turn leds on
    digitalWrite(6,HIGH);
    digitalWrite(8,HIGH);

    delay(3000);

    //Turn leds off
    digitalWrite(6,LOW);
    digitalWrite(8,LOW);
}


void loop() { 
    // Read all the pixels
    amg.readPixels(pixels);

    // Write all LEDS low
    digitalWrite (6,LOW);
    digitalWrite (8,LOW);

    
/*   // Code for printing all pixels
    Serial.print("[");
    //for(int i=1; i<=AMG88xx_PIXEL_ARRAY_SIZE; i++){
    for(int i=1; i<=64; i++) {
      Serial.print(pixels[i-1]);
      Serial.print(", ");
      if( i%8 == 0 ) Serial.println();
    }
    Serial.println("]");
    Serial.println();
    

    //delay a second
    //delay(15000);

*/
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

/* // 64 pixels. 32 in each block
// divide pixels into two blocks. 
    for (int i=0; i<=63; i=i+8) {
      for (int j = 0; j<4; j++) {
        a = a + pixels[i+j];
        b = b + pixels[i+j+4]; 
      }
    }
 
//obtain average value for each block
      a = (a/32);
      b = (b/32);

  /*    Serial.print("a: ");
      Serial.print(a);
      Serial.print("    b: ");
      Serial.print(b);
      Serial.println();

*/

// check how many spins a flag has been raised. If nothing happens, reset the flags
    if (leftFlag || rightFlag) {
      loopsWithFlag += 1;

      if (loopsWithFlag > flagLoopLimit) {
      leftFlag = false;
      rightFlag = false;
      loopsWithFlag = 0;
      }
    }


    // check left block
    if (a>b) {
      if(rightFlag) {
          inRoom = inRoom - 1;
          rightFlag = false;
        } else {
          if (a-b > hysteres) {
          digitalWrite (6,HIGH);
          leftFlag = true;
          }
        }
    } else if ((b>a)) {    // check right block
      if(leftFlag) {
        inRoom = inRoom + 1;
        leftFlag = false;
      } else {
        if (b-a > hysteres) {
        digitalWrite (8,HIGH);
        rightFlag = true;
        }
      }
    }


    Serial.println(inRoom);
    a=0;
    b=0;
}
         


      
    
