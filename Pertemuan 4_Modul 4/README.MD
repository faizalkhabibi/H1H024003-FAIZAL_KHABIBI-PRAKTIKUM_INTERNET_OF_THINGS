# Pertemuan 4 Komunikasi dan Pertukaran Data Menggunakan MQTT (Komunikasi Dua Arah)

## Alat dan Bahan

- Board ESP8266 DevKit
- LED Indikator (sebagai simulasi aktuator)
- Breadboard & Kabel Jumper
- Kabel USB Micro-USB
- Laptop dengan Arduino IDE (sudah terpasang board manager ESP8266, pustaka ArduinoJson, dan PubSubClient)
- Jaringan WiFi yang terhubung ke internet
- Aplikasi client MQTT (MQTT Explorer / HiveMQ WebSocket Client) untuk mengirim perintah kendali dan memantau data
- Broker MQTT publik `broker.hivemq.com` (port 1883)

## Library / Dependencies

- **ESP8266WiFi.h** – Pustaka bawaan board manager ESP8266 pada Arduino IDE, digunakan untuk mengatur, mengonfigurasi, dan memonitor koneksi jaringan WiFi.

- **PubSubClient.h** – Pustaka pihak ketiga (by Nick O'Leary) untuk memfasilitasi komunikasi protokol MQTT (Publish dan Subscribe) pada platform mikrokontroler.

- **ArduinoJson.h** – Pustaka pihak ketiga (by Benoit Blanchon) untuk membuat, mem-parsing (deserialisasi), serta mengolah data berformat JSON pada mikrokontroler.

---

## Percobaan 4A: Subscribe dan Deserialisasi Data JSON untuk Kendali Aktuator

### Konfigurasi Rangkaian

ESP8266 terhubung ke PC melalui kabel USB untuk daya dan komunikasi Serial Monitor. Pin Digital (GPIO 5 / D1) dihubungkan ke anoda LED melalui resistor 220 Ohm menuju Ground (GND) sebagai simulasi aktuator. Perangkat terhubung secara nirkabel ke jaringan WiFi eksternal yang memiliki akses internet.

### Penjelasan Code

- `PubSubClient client(espClient)` menginisialisasi modul klien MQTT yang ditautkan dengan koneksi TCP WiFi dasar.

- `client.setCallback(callback)` mendaftarkan fungsi *callback* utama yang akan secara otomatis dieksekusi oleh sistem setiap kali ada pesan baru masuk dari topik yang di-subscribe.

- `void callback(char* topic, byte* payload, unsigned int length)` membaca *payload* mentah berbentuk larik *byte* per karakter, lalu mengonversinya menjadi tipe data `String`.

- `JsonDocument doc;` dan `DeserializationError error = deserializeJson(doc, pesan);` melakukan proses deserialisasi (ekstraksi) dari teks berformat JSON menjadi objek JSON yang nilai variabelnya dapat diakses secara langsung.

- `const char* perintah = doc["perintah"];` mengambil nilai dari kunci/field `"perintah"` di dalam data JSON yang diterima.

- Pengondisian `if (String(perintah) == "ON")` mengeksekusi perintah `digitalWrite(ledPin, HIGH)` untuk menyalakan LED, sedangkan `else if (String(perintah) == "OFF")` mengeksekusi `digitalWrite(ledPin, LOW)` untuk mematikan LED.

- `client.subscribe(topicPerintah)` dipanggil di dalam fungsi `hubungkanMQTT()` tepat setelah koneksi ke broker MQTT berhasil terbuat, sehingga memastikan ESP8266 secara otomatis melakukan *re-subscribe* jika koneksi sempat terputus.

- `client.loop()` dipanggil secara kontinu di dalam perulangan `loop()` utama tanpa dihalangi oleh `delay()` agar ESP8266 selalu siap mendengarkan dan memproses pesan masuk secara *real-time*.

### Penjelasan Code Modifikasi (Pengaturan Kecerahan LED via PWM)

- Di dalam pembentukan dan pembacaan struktur JSON, ditambahkan ekstraksi parameter kunci baru yaitu `int intensitas = doc["intensitas"] | 255;`.

- Penggunaan nilai *fallback* `| 255` memastikan bahwa jika *field* `"intensitas"` tidak dikirimkan dalam *payload*, nilai kecerahan LED akan otomatis secara default diatur pada tingkat maksimum (255).

- Perintah `analogWrite(ledPin, intensitas)` menggantikan `digitalWrite()` saat sinyal `"ON"` diterima, yang berfungsi mengirimkan sinyal Pulse Width Modulation (PWM) ke pin LED untuk mengatur tingkat redup/terangnya cahaya LED sesuai nilai numerik (0–255) yang dikirimkan via MQTT.

---

## Percobaan 4B: Pertukaran Data Dua Arah (Publish dan Subscribe Secara Bersamaan)

### Konfigurasi Rangkaian

ESP8266 terhubung ke PC via USB. Rangkaian mempertahankan koneksi LED indikator pada pin digital output, serta terhubung ke broker MQTT publik secara *full duplex* melalui jaringan WiFi.

### Penjelasan Code Original

- Variabel `waktuTerakhirPublish` dan `intervalPublish = 5000` diinisialisasi untuk menerapkan mekanisme pemrograman *non-blocking* menggunakan fungsi `millis()`.

- Fungsi `millis()` mengembalikan nilai milidetik sejak board pertama kali dinyalakan. Logika `if (millis() - waktuTerakhirPublish > intervalPublish)` mengecek apakah interval 5 detik telah terlampaui tanpa pernah menghentikan (*blocking*) eksekusi baris kode lainnya.

- Di dalam blok pemrosesan waktu, data dummy suhu dihasilkan secara otomatis, dibungkus ke dalam objek JSON `doc["suhu"] = suhuDummy;`, dan dipublikasikan ke `topicData` menggunakan `client.publish(topicData, buffer)`.

- Pemrosesan *subscribe* perintah kendali LED di dalam fungsi `callback()` dan pemanggilan `client.loop()` tetap berjalan secara kontinu di setiap iterasi tanpa terhenti, sehingga penerimaan perintah dari subscriber lain tidak akan terhambat oleh proses *publish* data berkala.

### Penjelasan Code Modifikasi (Penambahan Topik Perintah Kedua / Buzzer)

- Ditambahkan variabel topik perintah baru `const char* topicBuzzer` dan pendaftaran pin aktuator kedua `const int buzzerPin`.

- Fungsi `hubungkanMQTT()` dimodifikasi dengan menambahkan pemanggilan `client.subscribe(topicBuzzer)` agar ESP8266 mendaftarkan diri pada dua topik penerimaan perintah yang berbeda secara simultan.

- Di dalam fungsi `callback()`, ditambahkan logika pemilah topik menggunakan pengondisian `if (String(topic) == topicPerintah)` dan `else if (String(topic) == topicBuzzer)`.

- Struktur ini memungkinkan fungsi *callback* membedakan asal topik pesan MQTT yang masuk, sehingga instruksi kendali untuk LED dan instruksi kendali untuk aktuator kedua (Buzzer) dapat diproses secara terpisah dan independen tanpa memicu bentrok *command*.