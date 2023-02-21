/*---------------------- Imports ----------------------------------------------*/
//----------------------------------------------------------------------------//

/*---------------------- Definitions ----------------------------------------------*/
//----------------------------------------------------------------------------//
#define ARDUINO_RUNNING_CORE0 0
#define ARDUINO_RUNNING_CORE1 1
#define RED_LED 25
#define GRN_LED 26

/*---------------------- Globals ----------------------------------------------*/
//----------------------------------------------------------------------------//



void TaskBlink( void *pvParameters );
void TaskAnalogRead( void *pvParameters );
TaskHandle_t analog_read_task_handle; // You can (don't have to) use this to be able to manipulate a task from somewhere else.


/*---------------------- Setup ----------------------------------------------*/
//----------------------------------------------------------------------------//
void setup() {
   Serial.begin(115200);

  //------Pin setup-----------//
  pinMode(RED_LED, OUTPUT);
  pinMode(GRN_LED, OUTPUT);

  //------Tasks setup-----------//
  uint32_t blink_delay = 1000; // Delay between changing state on LED pin
  xTaskCreate(
    TaskBlink
    ,  "Task Blink" // A name just for humans
    ,  2048        // The stack size can be checked by calling `uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);`
    ,  (void*) &blink_delay // Task parameter which can modify the task behavior. This must be passed as pointer to void.
    ,  2  // Priority
    ,  NULL // Task handle is not used here - simply pass NULL
    );

  xTaskCreatePinnedToCore(
    TaskAnalogRead
    ,  "Analog Read"
    ,  2048  // Stack size
    ,  NULL  // When no parameter is used, simply pass NULL
    ,  1  // Priority
    ,  &analog_read_task_handle // With task handle we will be able to manipulate with this task.
    ,  ARDUINO_RUNNING_CORE0 // Core on which the task will run
    );
}

/*---------------------- Main loop ----------------------------------------------*/
//----------------------------------------------------------------------------//
void loop(){

}

/*---------------------- Tasks ----------------------------------------------*/
//----------------------------------------------------------------------------//

void TaskBlink(void *pvParameters){  // This is a task.
  uint32_t blink_delay = *((uint32_t*)pvParameters);


  while (true){ // A Task shall never return or exit.
    digitalWrite(RED_LED, HIGH);   // turn the LED on (HIGH is the voltage level)
    // arduino-esp32 has FreeRTOS configured to have a tick-rate of 1000Hz and portTICK_PERIOD_MS
    // refers to how many milliseconds the period between each ticks is, ie. 1ms.
    Serial.println("blink");
    delay(blink_delay);
    digitalWrite(RED_LED, LOW);    // turn the LED off by making the voltage LOW
    delay(blink_delay);
  }
}

void TaskAnalogRead(void *pvParameters){ 
  (void) pvParameters;

  while (true){
    // read the input on analog pin:
    int sensorValue = 11;
    // print out the value you read:
    Serial.println(sensorValue);
    delay(5000); // 100ms delay
  }
}

/*---------------------- Functions ----------------------------------------------*/
//----------------------------------------------------------------------------//
