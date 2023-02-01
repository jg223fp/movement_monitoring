
#include <FreeRTOS.h>
#include <task.h>

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

// Running on core0
void setup() {
  Serial.begin(115200);
  xTaskCreate(blink, "BLINK", 128, nullptr, 1, nullptr);
  delay(5000);
}

void loop() {
  Serial.printf("C0\n");
  delay(1000);
}


// Running on core1
void setup1() {
  delay(5000);
  Serial.printf("C1: Red leader standing by...\n");
}

void loop1() {
  Serial.printf("C1\n");
  delay(2000);
}
