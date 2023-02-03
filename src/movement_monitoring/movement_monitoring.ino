
#include <FreeRTOS.h>
#include <task.h>
#include <Wire.h>
#include <Adafruit_AMG88xx.h>

// ##############  Global variables  ##############
Adafruit_AMG88xx amg;
float pixels[AMG88xx_PIXEL_ARRAY_SIZE];
int row1[8];  // Bottom of sensor (ey towards face and text is readable)
int row2[8];   
int row3[8];
int row4[8];
int row5[8];
int row6[8];
int row7[8];
int row8[8]; // Top of sensor
int average_temp;


// ################  Tasks  ########################

// Blinks the builtin led
void blink(void *param) {
  (void) param;
  pinMode(LED_BUILTIN, OUTPUT);
  while (true) {
    digitalWrite(LED_BUILTIN, LOW);
    delay(750);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
  }
}

// Reads all the pixels on the AMG8833 and prints the values in the serial communication.
void amg_read(void *param) {
  (void) param;
  amg_init();
  
  while(true) {
    //read all the pixels
    amg.readPixels(pixels);
  /*
    Serial.print("[");
    for(int i=1; i<=AMG88xx_PIXEL_ARRAY_SIZE; i++){
      Serial.print(pixels[i-1]);
      Serial.print(", ");
      if( i%8 == 0 ) Serial.println();
    }
    Serial.println("]");
    Serial.println();
  */

  for (int i = 0; i < 8; i++) {
    int index = i * 8;
    row1[i] = pixels[index];
    index++;
    row2[i] = pixels[index];
    index++;
    row3[i] = pixels[index];
    index++;
    row4[i] = pixels[index];
    index++;
    row5[i] = pixels[index];
    index++;
    row6[i] = pixels[index];
    index++;
    row7[i] = pixels[index];
    index++;
    row8[i] = pixels[index];

   // Serial.print(row8[i]);   // print top row
    //Serial.print(" ");
  }
  //Serial.println();

  //Gör så att kalkyleringen av average är en task som händer  typ en gång i minuten
  // kolla sedan rad för rad på sensorn vad som händer. Jämför mit average
  delay(1);
  }
}

// Calculate the average temp of all pixels of AMG8833 sensor
void amg_avg_temp(void *param) {
  (void) param;
  while(true) {
    int sum = 0;
    for (int i = 0; i < AMG88xx_PIXEL_ARRAY_SIZE; i++) {     
       sum = sum + pixels[i];    
  }
  average_temp = sum/AMG88xx_PIXEL_ARRAY_SIZE;
  delay(10000);
  Serial.println(average_temp);
  }
}


// ###################  Functions ############

// Initiates communication with the AMG8833 via i2c.
void amg_init() {
    Serial.println("Initiating AMG8833");
    bool status;
    status = amg.begin();
    if (!status) {
        Serial.println("Could not find a valid AMG88xx sensor, check wiring!");
        while (1);
    }
    delay(1000); // let sensor boot up
}


// ########## Main programs ######################
// Running on core0
void setup() {
  Serial.begin(115200);
  delay(100);
  xTaskCreate(blink, "BLINK", 128, nullptr, 6, nullptr);
  xTaskCreate(amg_read, "TempRead", 1024, nullptr, 7, nullptr);
  delay(100);
  xTaskCreate(amg_avg_temp, "avgTempCalc", 256, nullptr, 5, nullptr);
}

void loop() {
}


// Running on core1
void setup1() {
  delay(5000);
}

void loop1() {
}
