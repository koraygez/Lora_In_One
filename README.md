# E22-900T30D & Arduino UNO Haberleşme / Communication Project

## Açıklama / Description

**Türkçe:**  
Arduino UNO ile E22-900T30D LoRa modülü arasında iki yönlü haberleşme örneği.  
Fixed ve Transparent modlar arasında seri komutlarla geçiş yapılabilir.  
Özellikler:  
- Broadcast (0xFFFF ve 0x0000)  
- Adresli PING/PONG  
- AUX, M0, M1 pin kontrolü  
- D8 pini ile donanım tabanlı ID seçimi  

**English:**  
Two-way communication example between Arduino UNO and the E22-900T30D LoRa module.  
It supports switching between Fixed and Transparent modes via serial commands.  
Features:  
- Broadcast (0xFFFF and 0x0000)  
- Addressed PING/PONG  
- AUX, M0, M1 pin control  
- Hardware-based ID selection via D8 pin  

---

## Donanım Bağlantıları / Hardware Connections

| E22 Pin | Arduino UNO Pin |
|---------|-----------------|
| TXD     | D2 (RX)         |
| RXD     | D3 (TX)         |
| AUX     | D4              |
| M0      | D6              |
| M1      | D7              |
| VCC     | 5V (external supply recommended) |
| GND     | GND             |

---

## Seri Komutlar / Serial Commands

### Türkçe
- `?` : Menüyü göster  
- `I` : Modül bilgisi  
- `R` : RF konfigürasyonu oku  
- `WF` : Fixed mod ayarla (kalıcı)  
- `WT` : Transparent mod ayarla (kalıcı)  
- `P` : Adresli PING gönder  
- `PB` : Broadcast PING gönder  
- `S metin...` : Adrese metin gönder  
- `T HH LL CC` : Hedef adres/kanal değiştir  

### English
- `?` : Show menu  
- `I` : Module info  
- `R` : Read RF config  
- `WF` : Set Fixed mode (permanent)  
- `WT` : Set Transparent mode (permanent)  
- `P` : Send addressed PING  
- `PB` : Send broadcast PING  
- `S text...` : Send text to target  
- `T HH LL CC` : Change target address/channel  

---

## Kullanım / Usage

### Türkçe
1. Donanımı tabloya göre bağlayın.  
2. Arduino UNO’ya kodu yükleyin.  
3. Seri monitörü 9600 baud hızında açın.  
4. `?` komutu ile menüyü görüntüleyin.  

### English
1. Connect the hardware as shown in the table.  
2. Upload the code to Arduino UNO.  
3. Open the Serial Monitor at 9600 baud.  
4. Use `?` to display the menu.  

---

## Lisans / License

Bu proje **MIT Lisansı** ile lisanslanmıştır.  
This project is licensed under the **MIT License**.
