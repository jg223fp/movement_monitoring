#include <WiFi.h>
#include <Wire.h>

#include "esp_wifi.h"

#define initScanTimes 2 // The number of times the initscan is looping through the number of channels. 1 second per channel. e.g. 2* 13 = 26 seconds initiation 
#define maxCh 13 //max Channel EU = 13
#define macLimit 128 // maximum number of macs that the controller can store
String maclist[macLimit][2]; // list with collected macadresses [0]: macadress   [1]: TTL left
int listIndex = 0;  // variavble for knowing what index to put the new mac on in maclist
int macCount = 0; // variable to keep track of active mac adresses in the list
int curChannel = 1;  // current channel
String defaultTTL = "60"; // Elapsed time before device is consirded offline and then removed from the list
int initCount = 0;      // how many macs there are in init list
String initList[50];    // array used to store macs found under the initiation
bool initActive = true;  // tells program that initiation should be run.
bool verboseOutput = true; // change to true to get more info


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


//Packets end up here after they get sniffed
void sniffer(void* buf, wifi_promiscuous_pkt_type_t type) {
  wifi_promiscuous_pkt_t *p = (wifi_promiscuous_pkt_t*)buf;
  int len = p->rx_ctrl.sig_len;
  WifiMgmtHdr *wh = (WifiMgmtHdr*)p->payload;
  len -= sizeof(WifiMgmtHdr);
  if (len < 0){
    Serial.println("Receuved 0");
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
        Serial.println("Too many addresses, reseting list index");
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
          Serial.println("1 adress removed");
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
  Serial.println("Active macadresses:");
  for(int i=0;i<=macLimit;i++){
    if(!(maclist[i][0] == "")){ 
      Serial.println(maclist[i][0] + "  timeleft: " + maclist[i][1]);      
    }
  }
}



void setup() {

  Serial.begin(115200);

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
  Serial.println("Scanning for background mac addresses....");

  for (int i=0; i<(maxCh*initScanTimes); i++){
    if (curChannel > maxCh){ 
      curChannel = 1;
    }
    esp_wifi_set_channel(curChannel, WIFI_SECOND_CHAN_NONE);
    delay(1000); 
    Serial.println(initCount);
    curChannel++;
  }
  Serial.println("Scan completed...");
  Serial.println("Found " + String(initCount));
  initActive = false;  // turn of init scan
  curChannel = 1;
  Serial.println("Statring sniffer...");

}


void loop() {

    // loop through wifi channels
    if(curChannel > maxCh){ 
      curChannel = 1;
    }

    esp_wifi_set_channel(curChannel, WIFI_SECOND_CHAN_NONE);
    delay(1000);

    if (verboseOutput) {
      printTime();
      Serial.println("list index: " + String(listIndex));
    }

    checkTtl();   
    Serial.println("Number of active macs: " + String(macCount));
    Serial.println();
    
    curChannel++;
}


