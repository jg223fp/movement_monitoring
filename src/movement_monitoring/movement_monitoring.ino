
#include <FreeRTOS.h>
#include <task.h>
#include <Wire.h>
#include <Adafruit_AMG88xx.h>

// Global variables
Adafruit_AMG88xx amg;
float pixels[AMG88xx_PIXEL_ARRAY_SIZE];

// Tasks
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

void amg_read(void *param) {
  (void) param;
  amg_init();
  
  while(true) {
    //read all the pixels
    amg.readPixels(pixels);

    Serial.print("[");
    for(int i=1; i<=AMG88xx_PIXEL_ARRAY_SIZE; i++){
      Serial.print(pixels[i-1]);
      Serial.print(", ");
      if( i%8 == 0 ) Serial.println();
    }
    Serial.println("]");
    Serial.println();

    delay(1);
  }
}



// Running on core0
void setup() {
  Serial.begin(115200);
  delay(100);
  xTaskCreate(blink, "BLINK", 128, nullptr, 1, nullptr);
  xTaskCreate(amg_read, "TempRead", 1024, nullptr, 7, nullptr);
}

void loop() {
}


// Running on core1
//void setup1() {
  //xTaskCreate(amg_read, "TempRead", 1024, nullptr, 7, nullptr);
//  delay(1000);
//}

//void loop1() {
//}
