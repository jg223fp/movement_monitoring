/*---------------------- Imports ----------------------------------------------*/
//----------------------------------------------------------------------------//
#include <WiFi.h>
#include <Wire.h>
#include "esp_wifi.h"
#include <PubSubClient.h>
#include <Adafruit_AMG88xx.h>

/*---------------------- Definitions ----------------------------------------------*/
//----------------------------------------------------------------------------//
// FreeRTOS
#define ARDUINO_RUNNING_CORE0 0
#define ARDUINO_RUNNING_CORE1 1

// Pins
#define RED_LED 25
#define GRN_LED 26

// Wifi packet sniffer
#define initScanTimes 0 // The number of times the initscan is looping through the number of channels. 1 second per channel. e.g. 2* 13 = 26 seconds initiation 
#define maxCh 13 //max Channel EU = 13
#define macLimit 128 // maximum number of macs that the controller can store

// Mqtt & wifi
#define IO_USERNAME  "jg223fp"
#define IO_KEY       "aio_pHcJ80PKSkjPHgfLR486iaNrdY6m"


/*---------------------- Globals ----------------------------------------------*/
//----------------------------------------------------------------------------//

// wifi packet sniffer globals
String maclist[macLimit][2]; // list with collected macadresses [0]: macadress   [1]: TTL left
int listIndex = 0;  // variavble for knowing what index to put the new mac on in maclist
int macCount = 0; // variable to keep track of active mac adresses in the list
int curChannel = 1;  // current channel
String defaultTTL = "60"; // Elapsed time before device is consirded offline and then removed from the list
int initCount = 0;      // how many macs there are in init list
String initList[50];    // array used to store macs found under the initiation
bool initActive = true;  // tells program that initiation should be run.
bool verboseOutput = false; // change to true to get more info
const wifi_promiscuous_filter_t filt={ 
    .filter_mask=WIFI_PROMIS_FILTER_MASK_MGMT|WIFI_PROMIS_FILTER_MASK_DATA
};
typedef struct {
  uint8_t mac[6];
} __attribute__((packed)) MacAddr;
typedef struct {
  int16_t fctl;
  int16_t duration;
  MacAddr da;
  MacAddr sa;
  MacAddr bssid;
  int16_t seqctl;
  unsigned char payload[];
} __attribute__((packed)) WifiMgmtHdr;

// Mqtt & wifi globals
const char* ssid = "3807444";                         // try to move to def
const char* password = "berlin2022";                  // try to move to def
const char* mqtt_server = "io.adafruit.com";          // try to move to def
const int mqttPort = 1883;                            // try to move to def
String clientId = "movement_sensor_box";              // try to move to def
const char* mqttTopic = "jg223fp/feeds/Dayli_flow";   // try to move to def
WiFiClient espClient;
PubSubClient client(espClient);
float temperature = 0;    // REMOVE !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

// Movement monitoring globals
Adafruit_AMG88xx amg;
float pixels[64];
float a = 0;   // left block
float b = 0;   // right block
bool leftFlag = false;
bool rightFlag = false;
int loopsWithFlag = 0;   // Number of spins with a flag set. Counting variable. Can not be adjusted.
float hysteres = 0.5;  // Higher gives less sensitivity, lower more noise   0.4 is best so far
int flagLoopLimit = 15; // How many spins the loop can go with a flag set, waiting for a human to enter the other block. 10 with all 64 pixels and 16 pixels
int inRoom = 0;

// Tasks globals
void TaskBlinkRed( void *pvParameters );
void TaskBlinkGrn( void *pvParameters );
void TaskSniffPackets( void *pvParameters );
void TaskMqttWifi(void *pvParameters);
void TaskMovementMonitoring(void *pvParameters);


/*---------------------- Setup ----------------------------------------------*/
//----------------------------------------------------------------------------//
void setup() {
  Serial.begin(115200);
  Serial.println("Movement monitor boot");
  Serial.println("Starting tasks...");
  //------Pin setup-----------//
  pinMode(RED_LED, OUTPUT);
  pinMode(GRN_LED, OUTPUT);

  //------Tasks setup-----------//
  uint32_t blink_delay = 1000; // Delay between changing state on LED pin
  xTaskCreate(
    TaskBlinkRed
    ,  "Task Blink Red" // A name just for humans
    ,  1024        // The stack size can be checked by calling `uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);`
    ,  (void*) &blink_delay // Task parameter which can modify the task behavior. This must be passed as pointer to void.
    ,  1  // Priority
    ,  NULL // Task handle is not used here - simply pass NULL
    );

  xTaskCreate(
    TaskBlinkGrn
    ,  "Green blink"
    ,  1024  // Stack size
    ,  NULL  // When no parameter is used, simply pass NULL
    ,  2  // Priority
    ,  NULL // With task handle we will be able to manipulate with this task.
    );

  xTaskCreatePinnedToCore(
    TaskSniffPackets
    ,  "PACKET SNIFFER"
    ,  2048  // Stack size
    ,  NULL  // When no parameter is used, simply pass NULL
    ,  5  // Priority
    ,  NULL // With task handle we will be able to manipulate with this task.
    ,  ARDUINO_RUNNING_CORE0 // Core on which the task will run
    );

    xTaskCreatePinnedToCore(
    TaskMqttWifi
    ,  "Mqtt & wifi"
    ,  2048  // Stack size
    ,  NULL  // When no parameter is used, simply pass NULL
    ,  4  // Priority
    ,  NULL // With task handle we will be able to manipulate with this task.
    ,  ARDUINO_RUNNING_CORE0 // Core on which the task will run
    );

    xTaskCreatePinnedToCore(
    TaskMovementMonitoring
    ,  "Movement monitoring"
    ,  2048  // Stack size
    ,  NULL  // When no parameter is used, simply pass NULL
    ,  7  // Priority
    ,  NULL // With task handle we will be able to manipulate with this task.
    ,  ARDUINO_RUNNING_CORE1 // Core on which the task will run
    );

}

/*---------------------- Main loop ----------------------------------------------*/
//----------------------------------------------------------------------------//
void loop(){

}

/*---------------------- Tasks ----------------------------------------------*/
//----------------------------------------------------------------------------//

void TaskBlinkRed(void *pvParameters){  // This is a task.
  uint32_t blink_delay = *((uint32_t*)pvParameters);

  while (true){ // A Task shall never return or exit.
    digitalWrite(RED_LED, HIGH);   // turn the LED on (HIGH is the voltage level)
    // arduino-esp32 has FreeRTOS configured to have a tick-rate of 1000Hz and portTICK_PERIOD_MS
    // refers to how many milliseconds the period between each ticks is, ie. 1ms.
    delay(blink_delay);
    digitalWrite(RED_LED, LOW);    // turn the LED off by making the voltage LOW
    delay(blink_delay);
  }
}



void TaskBlinkGrn(void *pvParameters){ 
  (void) pvParameters;

  while (true){ // A Task shall never return or exit.
    digitalWrite(GRN_LED, HIGH);   
    delay(300);
    digitalWrite(GRN_LED, LOW);    
    delay(300);
  }
}



void TaskSniffPackets(void *pvParameters){ 
  (void) pvParameters;

//---------SETUP-----------//
// setup wifi for sniffing
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  esp_wifi_init(&cfg);
  esp_wifi_set_storage(WIFI_STORAGE_RAM);
  esp_wifi_set_mode(WIFI_MODE_NULL);
  esp_wifi_start();
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_filter(&filt);
  esp_wifi_set_promiscuous_rx_cb(&sniffer);
  esp_wifi_set_channel(curChannel, WIFI_SECOND_CHAN_NONE);

// Scan for background noice
  Serial.println("PACKET SNIFFER: Initiating, scanning for background mac addresses...");
  Serial.println("PACKET SNIFFER: Addressess found: ");

  for (int i=0; i<(maxCh*initScanTimes); i++){
    if (curChannel > maxCh){ 
      curChannel = 1;
    }
    esp_wifi_set_channel(curChannel, WIFI_SECOND_CHAN_NONE);
    delay(1000); 
    Serial.print(initCount);
    Serial.print("...");
    curChannel++;
  }
  Serial.println();
  Serial.println("PACKET SNIFFER: Scan completed...");
  Serial.println("PACKET SNIFFER: Found " + String(initCount));
  initActive = false;  // turn of init scan
  curChannel = 1;
  Serial.println("PACKET SNIFFER: Statring sniffer...");


//---------MAIN LOOP-----------//
  while (true){
    // loop through wifi channels
    if(curChannel > maxCh){ 
      curChannel = 1;
    }
    Serial.println("in room: " + String(inRoom));
    esp_wifi_set_channel(curChannel, WIFI_SECOND_CHAN_NONE);
    delay(1000);
    if (verboseOutput) {
      printTime();
      Serial.println("PACKET SNIFFER: list index: " + String(listIndex));
    }
    checkTtl();   
    Serial.println("PACKET SNIFFER: Number of active macs: " + String(macCount));
    curChannel++;
  }
}


void TaskMqttWifi(void *pvParameters){ 
  (void) pvParameters;

  //---------SETUP-----------//
  connectWifi();

  //---------MAIN LOOP-----------//
  while(true){
    if (WiFi.status() != WL_CONNECTED) {
    connectWifi();

    } else if (!client.connected()) {
      connectMqtt();

    } else {
      client.loop();
      delay(10000);
      // publish
      // Convert the value to a char array
      char macString[8];
      dtostrf(macCount, 1, 2, macString);
      client.publish(mqttTopic, macString);
      Serial.println("MQTT&WIFI: Published macCount");
    }   
  }
}


void TaskMovementMonitoring(void *pvParameters){ 
  (void) pvParameters;

//---------SETUP---------------//
  bool status;  
  status = amg.begin();
  if (!status) {
      Serial.println("MOVEMENT MONITORING: ERROR: Could not find a valid AMG88xx sensor, check wiring!");
      while (1);
  }
  delay(100); // let sensor boot up

//---------MAIN LOOP-----------//
  while (true) {
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
          digitalWrite (26,HIGH);
          leftFlag = true;
          }
        }
    } else if ((b>a)) {    // check right block
      if(leftFlag) {
        inRoom = inRoom + 1;
        leftFlag = false;
      } else {
        if (b-a > hysteres) {
        digitalWrite (25,HIGH);
        rightFlag = true;
        }
      }
    }

  // reset block sums
    a = 0;
    b = 0;
  
  }
}


/*---------------------- Functions ----------------------------------------------*/
//----------------------------------------------------------------------------//

//----Wifi packet sniffer functions------//
//Packets end up here after they get sniffed
void sniffer(void* buf, wifi_promiscuous_pkt_type_t type) {
  wifi_promiscuous_pkt_t *p = (wifi_promiscuous_pkt_t*)buf;
  int len = p->rx_ctrl.sig_len;
  WifiMgmtHdr *wh = (WifiMgmtHdr*)p->payload;
  len -= sizeof(WifiMgmtHdr);
  if (len < 0){
    Serial.println("PACKET SNIFFER: Receuved 0");
    return;
  }
  String packet;
  String mac;
  int fctl = ntohs(wh->fctl);
  for(int i=8;i<=8+6+1;i++){ // This reads the first couple of bytes of the packet. This is where you can read the whole packet replaceing the "8+6+1" with "p->rx_ctrl.sig_len"
    packet += String(p->payload[i],HEX);
  }
  for(int i=4;i<=15;i++){ // This removes the 'nibble' bits from the stat and end of the data we want. So we only get the mac address.
    mac += packet[i];
  }
  mac.toUpperCase();

// check if mac is in ignore list that was built during init, if it is it will be dropped.
  bool drop = false;
  for(int i=0; i <= initCount; i++) {
    if (mac == initList[i]) {
      drop = true;
    }
  }

  //if(drop && verboseOutput) {    // verbose outoput
  //  Serial.println("Mac already in maclist. Dropping...");
  //}

  bool inList = false;
  if (initActive) { // this part runs during initiation to scan for background network activity
    // check if mac is already in init array.
    for(int i=0;i<=50;i++){ 
      if(mac == initList[i]){
        inList = true;
      }
    }
    // If its new. add it to the array.
    if(!inList){ 
      initList[initCount] = mac;
      initCount ++;
    }

  } else if (!drop) { // this part runs during the standard sniffing
    // check if mac is already in array. if so reset TTL
    for(int i=0;i<=macLimit;i++){ 
      if(mac == maclist[i][0]){
        maclist[i][1] = defaultTTL;
        inList = true;
      }
    }
    // If its new. add it to the array.
    if(!inList){ 
      maclist[listIndex][0] = mac;
      maclist[listIndex][1] = defaultTTL;
      listIndex ++;
      if(listIndex >= macLimit) { 
        Serial.println("PACKET SNIFFER: Too many addresses, reseting list index");
        listIndex = 0;
      }
    }
  }
}

// check if TTL is over 0. if not, the mac is removed from the array
void checkTtl(){ 
  int tempMacCount = 0;
  for(int i=0;i<=macLimit;i++) {
    if(!(maclist[i][0] == "")){
      tempMacCount ++;
      int ttl = (maclist[i][1].toInt());
      ttl --;
      if(ttl <= 0) {
        maclist[i][0] = "";
        if (verboseOutput) {
          Serial.println("PACKET SNIFFER: 1 adress removed");
        }
      } else {
        maclist[i][1] = String(ttl);
      }
    }
  }
  macCount = tempMacCount;
}

// Prints the time left of the devices TTL
void printTime(){ 
  Serial.println("PACKET SNIFFER: Active macadresses:");
  for(int i=0;i<=macLimit;i++){
    if(!(maclist[i][0] == "")){ 
      Serial.println("PACKET SNIFFER: " + maclist[i][0] + "  timeleft: " + maclist[i][1]);      
    }
  }
}
//------------------------------------------------------------------//

//-----Mqtt & wifi functions------//
void connectWifi() {
  delay(10);
  Serial.println();
  Serial.print("MQTT&WIFI: Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("MQTT&WIFI: WiFi connected");
  Serial.println("MQTT&WIFI: IP address: ");
  Serial.println(WiFi.localIP());
}

void connectMqtt() {
  client.setServer(mqtt_server, mqttPort);
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("MQTT&WIFI: Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(clientId.c_str(), IO_USERNAME, IO_KEY)) {
            Serial.println("MQTT&WIFI: Mqtt connected");
    } else {
      Serial.print(" MQTT&WIFI: failed, rc=");
      Serial.print(client.state());
      Serial.println("MQTT&WIFI: try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

//------------------------------------------------------------------//
