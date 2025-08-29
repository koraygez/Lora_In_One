Proje Açıklaması / Project Description

Türkçe:
Bu proje, Arduino UNO ile E22-900T30D LoRa modülü arasında iki yönlü haberleşme örneği sunar. Fixed ve Transparent modlar arasında seri komutlarla geçiş yapılabilir. Broadcast (0xFFFF ve 0x0000), adresli PING/PONG iletişimi, AUX-M0-M1 pin kontrolü ve donanım üzerinden ID seçimi özelliklerini içerir. Saha testleri için menü tabanlı esnek bir yapı sağlar.

English:
This project demonstrates two-way communication between Arduino UNO and the E22-900T30D LoRa module. It allows switching between Fixed and Transparent modes via serial commands. Features include broadcast (0xFFFF and 0x0000), addressed PING/PONG, AUX-M0-M1 pin control, and hardware-based ID selection. Provides a flexible, menu-driven framework for field testing.

Donanım Bağlantıları / Hardware Connections
E22 Pin	Arduino UNO Pin
TXD	D2 (RX)
RXD	D3 (TX)
AUX	D4
M0	D6
M1	D7
VCC	5V (harici güç önerilir)
GND	GND
