#include <WiFi.h>
#include <Wire.h>

#include "esp_wifi.h"


#define maxCh 13 //max Channel EU = 13
#define macLimit 128 // number of macs that the controller can store
String maclist[macLimit][3]; 
int listcount = 0;  // variavble for knowing how many macs there are in the list
int curChannel = 1;  
String defaultTTL = "60"; // Elapsed time before device is consirded offline


// Ignore list: Devices in this list will not be counted. E.g. nearby network equipment
#define numOfKnown 2
String KnownMac[numOfKnown][2] = { 
  {"Johan-PC","E894Bffffff3"},
  {"NAME","MACADDRESS"}
};



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


  


//This is where packets end up after they get sniffed
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

// check if mac is in ignore list
  bool known = false;
  for(int i=0; i < numOfKnown; i++) {
    if (mac == KnownMac[i][1]) {
      known = true;
    }
  }

  if (!known) {
    // check if mac is already in array. if so reset TTL
    bool inList = false;
    for(int i=0;i<=macLimit;i++){ 
      if(mac == maclist[i][0]){
        maclist[i][1] = defaultTTL;
        inList = true;
      }
    }
    // If its new. add it to the array.
    if(!inList){ 
      maclist[listcount][0] = mac;
      maclist[listcount][1] = defaultTTL;
      listcount ++;
      if(listcount >= 64){                          // NEED TO CHANGE THIS
        Serial.println("Too many addresses");
        listcount = 0;
      }
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
}

// checks if TTL is over 0. if not, the mac is removed
void checkTtl(){ 
  for(int i=0;i<=macLimit;i++) {
    if(!(maclist[i][0] == "")){
      int ttl = (maclist[i][1].toInt());
      ttl --;
      if(ttl <= 0) {
        maclist[i][0] = "";
        listcount --;
      }else {
        maclist[i][1] = String(ttl);
      }
    }
  }
}

// This prints the time left of the devices TTL
void printTime(){ 
  for(int i=0;i<=macLimit;i++){
    if(!(maclist[i][0] == "")){ 
      Serial.println(maclist[i][0] + "  timeleft: " + maclist[i][1]);      
    }
  }
}


void loop() {
    // loop through wifi channels
    if(curChannel > maxCh){ 
      curChannel = 1;
    }
    esp_wifi_set_channel(curChannel, WIFI_SECOND_CHAN_NONE);
    delay(1000);
   // printTime();  
    checkTtl();   
    Serial.println(listcount);
    curChannel++;
}

// todo:
// reset variable evrytime it is sent
// Change to int instead of string in array

// chek if in known macs
// if -> drop
// check if in array
// if not -> add, if -> reset TTL  
// update time
// after ttl -> remove



