# Pertemuan 5 Pengolahan dan Analisis Data IoT (Filtering, Analisis, dan Visualisasi Data)

## Alat dan Bahan

- Board ESP32 DevKit
- Sensor Suhu dan Kelembapan DHT22 *(Pada praktikum ini digantikan dengan fungsi dummy/mock karena keterbatasan alat)*
- Kabel USB Micro
- Laptop dengan Arduino IDE (terpasang board manager ESP32, pustaka PubSubClient)
- Lingkungan Python 3 pada komputer/laptop
- Jaringan WiFi yang terhubung ke internet
- Broker MQTT publik `broker.hivemq.com` (port 1883)

## Library / Dependencies

**Pada sisi Edge (ESP32 / Arduino IDE):**
- **WiFi.h** – Pustaka bawaan board ESP32 untuk mengatur koneksi jaringan WiFi.
- **PubSubClient.h** – Pustaka untuk memfasilitasi komunikasi protokol MQTT (Publish dan Subscribe) dari mikrokontroler.

**Pada sisi Server (Python 3):**
- **paho-mqtt** – Pustaka klien MQTT berbasis Python untuk terhubung, mem-publish, dan me-subscribe topik dari broker.
- **pandas** – Pustaka Python untuk memanipulasi, menganalisis, dan menyimpan data terstruktur dalam bentuk tabular (*DataFrame*) serta menyimpannya menjadi CSV.
- **matplotlib** – Pustaka Python komprehensif untuk membuat visualisasi grafik/plot dari data yang telah diolah.
- **json & time** – Pustaka bawaan Python untuk melakukan *parsing* data format JSON dari payload MQTT dan merekam *timestamp* saat data diterima.

---

## Percobaan 5A: Pengolahan Data Sensor pada ESP32 (Moving Average dan Statistik)

### Konfigurasi Rangkaian

ESP32 terhubung ke PC melalui kabel USB untuk daya dan pemantauan via *Serial Monitor*. Berdasarkan kondisi praktikum, sensor DHT22 fisik tidak digunakan. Sebagai gantinya, pembacaan sensor digantikan oleh sebuah *mock function* (fungsi simulasi nilai) yang dijalankan secara virtual di dalam ESP32. ESP32 terhubung secara nirkabel ke jaringan WiFi.

### Penjelasan Code

- `float bacaSuhuMock()` merupakan fungsi buatan pengganti sensor DHT22. Fungsi ini membangkitkan nilai acak berdasarkan gelombang sinusoidal `sin()` yang dipadukan dengan fungsi `random()` untuk menyimulasikan derau (*noise*) fluktuasi suhu. Terdapat juga skenario lonjakan nilai paksa setiap 25 iterasi pembacaan.
- `float hitungMovingAverage(float nilaiBaru)` adalah algoritma utama (*edge processing*) yang mengimplementasikan filter *moving average* dengan panjang jendela `N = 5`.
- `indeks = (indeks + 1) % N` menciptakan logika *circular buffer* (buffer melingkar). Nilai pembacaan paling baru akan menimpa posisi nilai yang paling usang (*oldest data*) di dalam memori laci *array*, sehingga rata-rata selalu dihitung berdasarkan 5 data terakhir secara efisien.
- Pengondisian `if (suhu < suhuMin) suhuMin = suhu;` dan `if (suhu > suhuMax) suhuMax = suhu;` berfungsi mencatat riwayat nilai suhu ekstrem (paling rendah dan paling tinggi) selama alat beroperasi.
- `String payload = "{\"suhu\":" + String(suhu) + ...` menyatukan data mentah, data hasil filter, dan data statistik batas ke dalam format *string* JSON, yang kemudian dipublikasikan melalui `client.publish(topicData, payload.c_str())`.

### Penjelasan Code Modifikasi (Penambahan Perhitungan Simpangan Baku)

- Dilakukan penambahan baris kode dan fungsi baru untuk mengalkulasi nilai simpangan baku (standar deviasi) secara langsung di dalam ESP32.
- Fungsi ini bekerja dengan melakukan *looping* untuk menjumlahkan selisih kuadrat dari masing-masing nilai di dalam *buffer* terhadap nilai rata-ratanya (moving average).
- Hasil penjumlahan tersebut dibagi dengan total sampel `N`, lalu diakarkan (menggunakan fungsi `sqrt()`) untuk mendapatkan variansi penyimpangan data dari titik pusatnya pada sisi pinggiran (*edge processing*).

---

## Percobaan 5B: Akuisisi, Analisis, dan Visualisasi Data (Server Processing)

### Konfigurasi Rangkaian

Skrip Python dieksekusi di lingkungan komputer yang terhubung ke jaringan internet. Program ini bertindak murni sebagai klien *subscriber* (*server processing*) yang mendengarkan pesan dari broker MQTT HiveMQ. Komputer merespons *payload* yang diterbitkan oleh ESP32 dari Percobaan 5A.

### Penjelasan Code Original

- `client.subscribe(TOPIC)` diletakkan di dalam fungsi `on_connect()` agar klien Python mendaftarkan diri (*subscribe*) ke topik ESP32 begitu koneksi TCP MQTT berhasil dibuat.
- `try-except (ValueError, KeyError):` membungkus baris ekstraksi `suhu = float(payload["suhu"])` di dalam fungsi `on_message()`. Logika ini bertindak sebagai pelindung agar program Python tidak mengalami *crash* dan tertutup sendiri jika ESP32 tak sengaja mengirimkan format teks cacat atau kehilangan atribut kunci `"suhu"`.
- `df = pd.DataFrame(data)` dan `df.to_csv("data_suhu.csv", index=False)` dijalankan sesaat setelah 30 iterasi pengumpulan `JUMLAH_SAMPEL` selesai, mengubah *list dictionary* di memori RAM menjadi struktur baris-kolom baku (DataFrame) lalu mengekspornya ke penyimpanan lokal dalam ekstensi `.csv`.
- `df["zscore"] = (df["suhu"] - rata2) / std` menjalankan operasi aritmatika statistik ke seluruh baris kolom secara paralel, menghitung seberapa jauh letak data saat ini dari nilai rata-rata keseluruhan.
- `df["anomali"] = df["zscore"].abs() > 2` mendefinisikan regulasi toleransi. Sistem mendeteksi suatu baris sebagai data yang mencurigakan atau *outlier* (anomali) bila besaran penyimpangannya secara absolut melewati angka 2 (batas simpangan baku konvensional).
- Blok visualisasi `plt` dari Matplotlib menggambarkan garis biru berpenanda (*marker*) dari DataFrame. Data anomali diisolasi dengan `anom = df[df["anomali"]]` lalu digambar secara bertumpuk menggunakan `plt.scatter(color="red")` guna memberikan sorotan visual penanda titik ekstrem di hasil render gambar `grafik_suhu.png`.

### Penjelasan Code Modifikasi (Penggunaan Moving Average di sisi Python)

- Pada skrip Python, sebelum tahapan penghitungan nilai rata-rata dan perhitungan statistik deviasi (Z-Score) dilakukan, DataFrame terlebih dahulu diberi filter penghalusan menggunakan metode *rolling window* milik pustaka pandas.
- Instruksi `df["suhu_ma"] = df["suhu"].rolling(window=5).mean()` ditambahkan untuk menghitung *moving average* langsung dari sisi *dataset* Python.
- Nilai dari kolom `suhu_ma` inilah yang kemudian digunakan untuk menghitung batas Z-Score, membandingkan efektivitas tangkapan deteksi anomali jika analisis dilakukan pada data yang mentah (*raw*) vs data yang telah difilter polanya (*smoothed*).
