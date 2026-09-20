# Pertemuan 3 Protokol Komunikasi IoT (HTTP & MQTT dengan Format JSON)

## Alat dan Bahan

- Board ESP8266 DevKit
- Kabel USB Micro-USB
- Laptop/PC dengan Arduino IDE (sudah terpasang board manager ESP32, pustaka ArduinoJson, dan PubSubClient)
- Jaringan WiFi yang terhubung ke internet
- Aplikasi client MQTT (MQTT Explorer / HiveMQ WebSocket Client) untuk verifikasi data
- Broker MQTT publik broker.hivemq.com (port 1883) dan endpoint uji HTTP httpbin.org/post

## Library / Dependencies

- WiFi.h – Pustaka bawaan board manager ESP32 pada Arduino IDE, digunakan untuk mengatur dan memonitor koneksi jaringan WiFi.

- HTTPClient.h – Pustaka bawaan ESP32 untuk menangani protokol HTTP client (seperti metode GET dan POST) dalam pertukaran data ke server web.

- PubSubClient.h – Pustaka pihak ketiga (by Nick O'Leary) untuk memfasilitasi komunikasi protokol MQTT (Publish/Subscribe) pada platform Arduino/ESP32.

- ArduinoJson.h – Pustaka pihak ketiga (by Benoit Blanchon) untuk membuat, mengedit, mem-parsing, serta mengagregasi data berformat JSON pada mikrokontroler.

---

## Percobaan 3A: Komunikasi Data Menggunakan HTTP

### Konfigurasi Rangkaian

ESP32 terhubung ke PC melalui kabel USB untuk daya dan komunikasi Serial Monitor, serta terhubung secara nirkabel ke jaringan WiFi eksternal yang memiliki akses internet.

### Penjelasan Code

- WiFi.begin(ssid, password) memulai proses koneksi ESP32 ke jaringan WiFi eksternal, diikuti perulangan while (WiFi.status() != WL_CONNECTED) untuk menunggu hingga status jaringan berhasil terhubung.

- HTTPClient http; dan http.begin(serverUrl) menginisialisasi objek HTTP client serta menentukan alamat endpoint target ([https://httpbin.org/post](https://httpbin.org/post)).

- http.addHeader("Content-Type", "application/json") menyisipkan header HTTP untuk memberi tahu server bahwa data yang dikirimkan dalam body request berformat JSON.

- JsonDocument doc; digunakan untuk membuat struktur data JSON, diikuti pengisian pasangan key-value seperti doc["suhu"] = 28.5 dan doc["kelembaban"] = 65.0.

- serializeJson(doc, requestBody) mengonversi objek JSON menjadi string teks agar siap dikirimkan melalui jaringan.

- http.POST(requestBody) mengeksekusi pengiriman data JSON ke server menggunakan metode HTTP POST dan mengembalikan kode status response HTTP dari server.

- http.getString() mengambil balasan isi (response body) dari server untuk ditampilkan ke Serial Monitor, lalu http.end() dipanggil untuk menutup sesi koneksi HTTP.

- Pengiriman data dilakukan secara periodik setiap 10 detik sekali menggunakan fungsi delay(10000).

### Penjelasan Code Modifikasi (tambahan parameter waktu millis())

- Di dalam pembentukan struktur JSON, ditambahkan kunci/field baru yaitu doc["uptime_ms"] = millis();.

- Fungsi millis() mengambil jumlah milidetik yang telah berlalu sejak board ESP32 pertama kali dinyalakan.

- Penambahan field ini memungkinkan server tidak hanya menerima data sensor (suhu dan kelembaban), tetapi juga mengetahui timestamp/waktu relatif pengiriman data secara real-time langsung dari mikrokontroler tanpa memerlukan RTC eksternal.

---

## Percobaan 3B: Komunikasi MQTT


### Penjelasan Code Original

- WiFiClient espClient dan PubSubClient client(espClient) menginisialisasi instance client TCP dasar dan mengaitkannya ke modul klien MQTT.

- client.setServer(mqttServer, mqttPort) mendaftarkan alamat broker target (broker.hivemq.com) beserta port komunikasinya (1883).

- hubungkanMQTT() beroperasi di dalam perulangan while (!client.connected()) untuk secara kontinu mencoba menghubungkan ESP32 ke broker MQTT dengan Client ID acak yang dibuat menggunakan random(0xffff).

- client.loop() dipanggil pada setiap iterasi loop() untuk menjaga kestabilan sesi koneksi TCP dengan broker, memproses antrean pesan, dan mengolah ping/keep-alive.

- JsonDocument doc; digunakan untuk mengonstruksi data JSON yang diisi nilai suhu dan kelembaban, lalu dikonversi ke array karakter/buffer menggunakan serializeJson(doc, buffer).

- client.publish(mqttTopic, buffer) mempublikasikan payload JSON yang tersimpan di dalam buffer ke topic MQTT yang telah ditentukan agar dapat diterima oleh subscriber (seperti MQTT Explorer).

- Publikasi data diatur secara berkala setiap 5 detik menggunakan delay(5000).