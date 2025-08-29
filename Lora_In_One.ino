/*
  E22-900T30D  <->  Arduino UNO (tek kod, iki yönlü, broadcast düzeltmeli)
  - Fixed mod ve Transparent mod arasında sahadan komutla geçiş (WF / WT)
  - Broadcast için hem 0xFFFF hem 0x0000 denemesi (PB komutu)
  - Adresli PING (P) ile “çalıştığından emin ol” yaklaşımı
  - AUX/M0/M1 yazılımdan yönetim
  - D8 ile ID seçimi (GND→ID=1, aksi→ID=2)

  Bağlantı:
    E22 TXD -> D2   (UNO RX)
    E22 RXD -> D3   (UNO TX)
    E22 AUX -> D4
    E22 M0  -> D6
    E22 M1  -> D7
    E22 VCC -> 5V (harici güçlü besleme önerilir)
    E22 GND -> GND
*/

#include <SoftwareSerial.h>
#include "LoRa_E22.h"

// -------- Pinler --------
#define PIN_E22_RX   2   // Arduino RX (E22 TXD)
#define PIN_E22_TX   3   // Arduino TX (E22 RXD)
#define PIN_E22_AUX  4
#define PIN_E22_M0   6
#define PIN_E22_M1   7
#define PIN_ID_SEL   8   // GND -> ID=1, aksi -> ID=2

// -------- RF parametreleri --------
const uint8_t RF_CHAN = 10;     // 0..80 (868MHz bandında kanal offset)
const uint8_t NET_ID  = 0x00;

// Hedef adres (setup'ta karşı düğüme göre set edilir)
uint8_t targetAddH = 0x00;
uint8_t targetAddL = 0x02;

// Broadcast varyasyonları
const uint8_t BCAST_FF_H = 0xFF, BCAST_FF_L = 0xFF;
const uint8_t BCAST_00_H = 0x00, BCAST_00_L = 0x00;

// Sürücü
SoftwareSerial ss(PIN_E22_RX, PIN_E22_TX); // RX, TX
LoRa_E22 e22(&ss, PIN_E22_AUX, PIN_E22_M0, PIN_E22_M1);

// Bu düğümün adresi (D8'e göre ayarlanacak)
uint8_t myAddH = 0x00;
uint8_t myAddL = 0x01;

// ---------- Yardımcılar ----------
void divider() {
  Serial.println(F("------------------------------------------------------------"));
}

void showMenu() {
  divider();
  Serial.println(F("[Komutlar]"));
  Serial.println(F("  ?            : Menüyü göster"));
  Serial.println(F("  I            : Module Info (model, versiyon, özellikler)"));
  Serial.println(F("  R            : RF Config oku"));
  Serial.println(F("  WF           : Fixed mod konfigürasyonu yaz (kalıcı)"));
  Serial.println(F("  WT           : Transparent mod konfigürasyonu yaz (kalıcı)"));
  Serial.println(F("  P            : Adresli PING gönder (A<->B)"));
  Serial.println(F("  PB           : Broadcast PING gönder (0xFFFF ve 0x0000 dene)"));
  Serial.println(F("  S metin...   : Hedefe (adresli) metin gönder"));
  Serial.println(F("  T HH LL CC   : Hedefi değiştir (hex)  (örn: T 00 02 0A)"));
  divider();
}

void dumpConfig(Configuration &c) {
  Serial.print(F("ADDR = ")); Serial.print(c.ADDH, HEX); Serial.print(F(":")); Serial.println(c.ADDL, HEX);
  Serial.print(F("NET  = 0x")); Serial.println(c.NETID, HEX);
  Serial.print(F("CHAN = ")); Serial.println(c.CHAN);
  Serial.print(F("UART = ")); Serial.print(c.SPED.uartBaudRate);
  Serial.print(F("  Air=")); Serial.print(c.SPED.airDataRate);
  Serial.print(F("  Parity=")); Serial.println(c.SPED.uartParity);
  Serial.print(F("TX PWR = ")); Serial.println(c.OPTION.transmissionPower);
  Serial.print(F("RSSI EN= ")); Serial.print(c.TRANSMISSION_MODE.enableRSSI);
  Serial.print(F("  Fixed=")); Serial.println(c.TRANSMISSION_MODE.fixedTransmission);
}

void setNormalMode()   { e22.setMode(MODE_0_NORMAL);   }
void setProgramMode()  { e22.setMode(MODE_2_PROGRAM);  }

// ---- FIXED konfigürasyonunu yaz (kalıcı) ----
bool writeConfigFixed() {
  setProgramMode();

  ResponseStructContainer rsc = e22.getConfiguration();
  if (rsc.status.code != E22_SUCCESS) {
    Serial.println(F("[ERR] getConfiguration basarisiz!"));
    return false;
  }
  Configuration cfg = *(Configuration*)rsc.data; rsc.close();

  cfg.ADDH = myAddH;
  cfg.ADDL = myAddL;
  cfg.NETID = NET_ID;
  cfg.CHAN  = RF_CHAN;

  cfg.SPED.uartBaudRate = UART_BPS_9600;
  cfg.SPED.airDataRate  = AIR_DATA_RATE_010_24; // 2.4 kbps
  cfg.SPED.uartParity   = MODE_00_8N1;

  cfg.OPTION.subPacketSetting  = SPS_240_00;
  cfg.OPTION.RSSIAmbientNoise  = RSSI_AMBIENT_NOISE_DISABLED;
  // Güç enum'u bazı sürümlerde sınırlı olabilir. İstersen şu satırı açıp POWER_10 yapabilirsin:
  // cfg.OPTION.transmissionPower = POWER_10;

  cfg.TRANSMISSION_MODE.enableRSSI        = RSSI_DISABLED;
  cfg.TRANSMISSION_MODE.fixedTransmission = FT_FIXED_TRANSMISSION; // *** FIXED ***
  cfg.TRANSMISSION_MODE.enableRepeater    = REPEATER_DISABLED;
  cfg.TRANSMISSION_MODE.enableLBT         = LBT_DISABLED;
  cfg.TRANSMISSION_MODE.WORTransceiverControl = WOR_RECEIVER;
  cfg.TRANSMISSION_MODE.WORPeriod         = WOR_2000_011;

  ResponseStatus rs = e22.setConfiguration(cfg, WRITE_CFG_PWR_DWN_SAVE);
  Serial.print(F("[WF] ")); Serial.println(rs.getResponseDescription());
  setNormalMode();

  if (rs.code == E22_SUCCESS) { dumpConfig(cfg); return true; }
  return false;
}

// ---- TRANSPARENT konfigürasyonunu yaz (kalıcı) ----
bool writeConfigTransparent() {
  setProgramMode();

  ResponseStructContainer rsc = e22.getConfiguration();
  if (rsc.status.code != E22_SUCCESS) {
    Serial.println(F("[ERR] getConfiguration basarisiz!"));
    return false;
  }
  Configuration cfg = *(Configuration*)rsc.data; rsc.close();

  cfg.ADDH = myAddH;
  cfg.ADDL = myAddL;
  cfg.NETID = NET_ID;
  cfg.CHAN  = RF_CHAN;

  cfg.SPED.uartBaudRate = UART_BPS_9600;
  cfg.SPED.airDataRate  = AIR_DATA_RATE_010_24; // 2.4 kbps
  cfg.SPED.uartParity   = MODE_00_8N1;

  cfg.OPTION.subPacketSetting  = SPS_240_00;
  cfg.OPTION.RSSIAmbientNoise  = RSSI_AMBIENT_NOISE_DISABLED;
  // cfg.OPTION.transmissionPower = POWER_10; // isteğe bağlı

  cfg.TRANSMISSION_MODE.enableRSSI        = RSSI_DISABLED;
  cfg.TRANSMISSION_MODE.fixedTransmission = FT_TRANSPARENT_TRANSMISSION; // *** TRANSPARENT ***
  cfg.TRANSMISSION_MODE.enableRepeater    = REPEATER_DISABLED;
  cfg.TRANSMISSION_MODE.enableLBT         = LBT_DISABLED;
  cfg.TRANSMISSION_MODE.WORTransceiverControl = WOR_RECEIVER;
  cfg.TRANSMISSION_MODE.WORPeriod         = WOR_2000_011;

  ResponseStatus rs = e22.setConfiguration(cfg, WRITE_CFG_PWR_DWN_SAVE);
  Serial.print(F("[WT] ")); Serial.println(rs.getResponseDescription());
  setNormalMode();

  if (rs.code == E22_SUCCESS) { dumpConfig(cfg); return true; }
  return false;
}

// ---- Gönderimler ----
void sendFixedTo(uint8_t addh, uint8_t addl, uint8_t chan, const String& payload, const char* tag) {
  setNormalMode();
  Serial.print(tag); Serial.print(F(" Hedef ")); Serial.print(addh, HEX); Serial.print(F(":")); Serial.print(addl, HEX);
  Serial.print(F(" CH=")); Serial.print(chan); Serial.print(F("  LEN=")); Serial.println(payload.length());

  ResponseStatus rs = e22.sendFixedMessage(addh, addl, chan, (uint8_t*)payload.c_str(), payload.length());
  Serial.print(F("[TX] ")); Serial.println(rs.getResponseDescription());
  Serial.print(F("[AUX] ")); Serial.println(digitalRead(PIN_E22_AUX) ? F("HIGH (idle)") : F("LOW (busy)"));
}

void sendPingAddressed() {
  String msg = String("PING from 0x") + String(myAddL, HEX);
  sendFixedTo(targetAddH, targetAddL, RF_CHAN, msg, "[P]");
}

// Broadcast iki varyasyon: 0xFFFF ve 0x0000
void sendPingBroadcast() {
  String msgFF = String("BCAST(FFFF) PING from 0x") + String(myAddL, HEX);
  sendFixedTo(BCAST_FF_H, BCAST_FF_L, RF_CHAN, msgFF, "[PB-FFFF]");

  delay(150);
  String msg00 = String("BCAST(0000) PING from 0x") + String(myAddL, HEX);
  sendFixedTo(BCAST_00_H, BCAST_00_L, RF_CHAN, msg00, "[PB-0000]");
}

// ---- Alıcı ----
void handleReceived() {
  if (e22.available() > 0) {
    ResponseContainer rc = e22.receiveMessage();
    if (rc.status.code == E22_SUCCESS) {
      String data = rc.data;
      Serial.print(F("[RX] ")); Serial.println(data);

      // Otomatik PONG (adresli)
      if (data.indexOf("PING") >= 0) {
        String pong = String("PONG to 0x") + String(targetAddL, HEX) + " from 0x" + String(myAddL, HEX);
        sendFixedTo(targetAddH, targetAddL, RF_CHAN, pong, "[AUTO-PONG]");
      }
    } else {
      Serial.print(F("[RX ERR] ")); Serial.println(rc.status.getResponseDescription());
    }
  }
}

// ---- Komutlar ----
void parseCommandLine(String line) {
  line.trim();
  if (line == "?") { showMenu(); return; }

  if (line == "I") {
    ResponseStructContainer mi = e22.getModuleInformation();
    if (mi.status.code == E22_SUCCESS) {
      ModuleInformation info = *(ModuleInformation*) mi.data;
      Serial.print(F("Model: "));    Serial.println(info.model);
      Serial.print(F("Version: "));  Serial.println(info.version);
      Serial.print(F("Features: ")); Serial.println(info.features);
    } else {
      Serial.println(mi.status.getResponseDescription());
    }
    mi.close();
    return;
  }

  if (line == "R") {
    ResponseStructContainer rsc = e22.getConfiguration();
    if (rsc.status.code == E22_SUCCESS) {
      Configuration cfg = *(Configuration*)rsc.data; dumpConfig(cfg);
    } else {
      Serial.println(rsc.status.getResponseDescription());
    }
    rsc.close();
    return;
  }

  if (line == "WF") { writeConfigFixed(); return; }
  if (line == "WT") { writeConfigTransparent(); return; }

  if (line == "P")  { sendPingAddressed(); return; }
  if (line == "PB") { sendPingBroadcast(); return; }

  if (line.length() > 2 && line[0] == 'S' && line[1] == ' ') {
    String payload = line.substring(2);
    sendFixedTo(targetAddH, targetAddL, RF_CHAN, payload, "[S]");
    return;
  }

  if (line.length() >= 11 && line[0] == 'T') {
    // T HH LL CC  (örn: T 00 02 0A)
    int h1 = line.indexOf(' ');
    int h2 = line.indexOf(' ', h1+1);
    int h3 = line.indexOf(' ', h2+1);
    if (h1>0 && h2>h1 && h3>h2) {
      targetAddH = (uint8_t) strtoul(line.substring(h1+1, h2).c_str(), nullptr, 16);
      targetAddL = (uint8_t) strtoul(line.substring(h2+1, h3).c_str(), nullptr, 16);
      uint8_t chan = (uint8_t) strtoul(line.substring(h3+1).c_str(), nullptr, 16);
      Serial.print(F("[TARGET] ")); Serial.print(targetAddH, HEX); Serial.print(F(":")); Serial.print(targetAddL, HEX);
      Serial.print(F(" CH=")); Serial.println(chan);
      // Not: CH runtime kullanılacaksa RF_CHAN globali yerine 'chan'ı sendFixedTo'da parametreleştir.
    } else {
      Serial.println(F("Kullanım: T HH LL CC   (örn: T 00 02 0A)"));
    }
    return;
  }

  Serial.println(F("Bilinmeyen komut. ? ile menüyü göster."));
}

// ---- Setup/Loop ----
void setup() {
  pinMode(PIN_ID_SEL, INPUT_PULLUP); // GND -> 0

  Serial.begin(9600);
  while (!Serial) { ; }

  e22.begin(); // UART & pin init

  // ID seçimi
  if (digitalRead(PIN_ID_SEL) == LOW) {
    myAddL = 0x01;        // bu kart 0x0001
    targetAddL = 0x02;    // hedef 0x0002
  } else {
    myAddL = 0x02;        // bu kart 0x0002
    targetAddL = 0x01;    // hedef 0x0001
  }
  myAddH = 0x00;
  targetAddH = 0x00;

  divider();
  Serial.println(F("E22-900T30D  (Fixed/Transparent toggled, broadcast düzeltmeli)"));
  Serial.print  (F("Node ADDR  : 0x")); Serial.print(myAddH, HEX); Serial.print(F(":0x")); Serial.println(myAddL, HEX);
  Serial.print  (F("Target ADDR: 0x")); Serial.print(targetAddH, HEX); Serial.print(F(":0x")); Serial.println(targetAddL, HEX);
  Serial.print  (F("Channel    : "));  Serial.println(RF_CHAN);

  // Varsayılan: FIXED yazarak başla (kalıcı)
  writeConfigFixed();
  setNormalMode();

  showMenu();
  delay(300);
  sendPingAddressed(); // ilk test: adresli ping
}

void loop() {
  if (Serial.available()) {
    String line = Serial.readStringUntil('\n');
    parseCommandLine(line);
  }
  handleReceived();

  // AUX durumunu periyodik raporla
  static unsigned long t = 0;
  if (millis() - t > 2000) {
    Serial.print(F("[AUX] "));
    Serial.println(digitalRead(PIN_E22_AUX) ? F("HIGH (idle)") : F("LOW (busy)"));
    t = millis();
  }
}
