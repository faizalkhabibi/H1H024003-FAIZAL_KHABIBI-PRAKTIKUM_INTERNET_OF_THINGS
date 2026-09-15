# Pertemuan 2 Konfigurasi Jaringan

## Alat dan Bahan

- Board ESP32 DevKit
- Kabel USB Micro-USB
- Laptop/PC dengan Arduino IDE (sudah terpasang board manager ESP32)
- Jaringan WiFi (router/hotspot smartphone) beserta SSID dan password
- Smartphone/laptop untuk menguji koneksi ke Access Point ESP32
- LED (1 buah) dan Resistor 220 Ω (opsional, untuk indikator status koneksi)

## Library / Dependencies

- `WiFi.h`  pustaka bawaan board manager ESP32 pada Arduino IDE, digunakan untuk mengatur dan memonitor koneksi jaringan (mode Station, Access Point, maupun AP+STA). Tidak perlu instalasi terpisah, otomatis tersedia setelah board manager ESP32 terpasang.

---

## Percobaan 2A: Konfigurasi Mode Station (STA)

### Konfigurasi Rangkaian

LED indikator (opsional) dihubungkan ke pin GPIO 2 melalui resistor 220 Ω, kaki LED lainnya dihubungkan ke GND.

### Penjelasan Code

- `WiFi.mode(WIFI_STA)`  menetapkan mode operasi WiFi ESP32 menjadi Station (klien).
- `WiFi.begin(ssid, password)`  memulai proses koneksi ke jaringan WiFi yang dituju.
- `while (WiFi.status() != WL_CONNECTED)`  loop yang menunggu hingga status koneksi berhasil, mencetak "." setiap 500 ms sebagai indikator progres.
- Setelah terhubung, `WiFi.localIP()`, `WiFi.macAddress()`, dan `WiFi.RSSI()` dipanggil untuk menampilkan IP Address, MAC Address, dan kekuatan sinyal ke Serial Monitor, lalu LED indikator dinyalakan.
- Di dalam `loop()`, status koneksi dicek ulang setiap 5 detik; jika terputus, LED dimatikan dan pesan "Status: Terputus" dicetak.

### Penjelasan Code Modifikasi (reconnect otomatis)

- Saat status koneksi terdeteksi `!= WL_CONNECTED` di dalam `loop()`, program memanggil `WiFi.disconnect()` lalu `WiFi.begin()` ulang untuk memicu proses reconnect.
- Ditambahkan batas waktu tunggu menggunakan `millis() - waktuMulai < 10000` (maksimal ~10 detik) agar program tidak menggantung tanpa batas seperti pada koneksi awal.
- Jika reconnect berhasil dalam batas waktu tersebut, LED dinyalakan kembali; jika gagal, program tetap lanjut ke siklus `loop()` berikutnya dan akan mencoba lagi otomatis 5 detik kemudian.

---

## Percobaan 2B: Konfigurasi Mode Access Point (AP)

### Parameter Konfigurasi Access Point

| No | Parameter | Nilai |
|----|-----------|-------|
| 1 | SSID Access Point | ESP32_AccessPoint |
| 2 | Password | 12345678 |
| 3 | IP Address default | 192.168.4.1 |

### Penjelasan Code Original

- `WiFi.mode(WIFI_AP)`  menetapkan mode operasi WiFi ESP32 menjadi Access Point.
- `WiFi.softAP(ap_ssid, ap_password)`  mengaktifkan ESP32 sebagai Access Point dengan SSID dan password yang ditentukan.
- `WiFi.softAPIP()`  mengambil alamat IP dari Access Point yang dibuat (default 192.168.4.1), lalu ditampilkan ke Serial Monitor.
- Di dalam `loop()`, `WiFi.softAPgetStationNum()` dipanggil setiap 5 detik untuk memantau jumlah perangkat yang sedang terhubung ke Access Point.

### Penjelasan Code Modifikasi (mode AP+STA)

- `WiFi.mode(WIFI_AP_STA)` mengaktifkan kedua mode sekaligus dalam satu waktu, berbeda dari kode original yang hanya menggunakan `WIFI_AP`.
- `WiFi.softAP()` tetap dipanggil untuk mengaktifkan Access Point ESP32 sendiri, sementara `WiFi.begin()` dipanggil terpisah untuk menyambungkan ESP32 sebagai Station ke jaringan WiFi rumah.
- Karena kedua mode berjalan bersamaan, `loop()` memantau kedua sisi: jumlah client yang terhubung ke Access Point ESP32, sekaligus status koneksi ESP32 sebagai klien ke jaringan rumah.