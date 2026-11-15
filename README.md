# ♻️ Sistem Pemilah Sampah Otomatis Berbasis ESP32 developed By ade candra

Projek ini adalah purwarupa (*prototype*) tempat sampah pintar berbasis mikrokontroler **ESP32** yang mampu mendeteksi dan memilah sampah secara otomatis ke dalam kategori **Logam**, **Organik**, dan **Anorganik** menggunakan kombinasi sensor proximity dan motor servo.

---

## ✨ Fitur Utama
- **Deteksi Kehadiran Objek:** Membaca keberadaan sampah yang masuk secara otomatis menggunakan sensor ultrasonik.
- **Klasifikasi Jenis Sampah:** Membedakan sampah logam dan non-logam (serta sampah basah/kering) menggunakan sensor proximity induktif dan kapasitif.
- **Pemilahan Mekanis Otomatis:** Menggerakkan sudut motor servo untuk mengarahkan sampah ke kompartemen wadah yang sesuai.
- **Monitoring Kapasitas (Opsional IoT):** Mengirimkan notifikasi ke aplikasi Blynk / Telegram ketika kapasitas tempat sampah sudah penuh.

---

## 🛠️ Komponen Perangkat Keras (Hardware)

| No | Nama Komponen | Jumlah | Fungsi |
| :--- | :--- | :---: | :--- |
| 1 | ESP32 DevKit V1 | 1 | Mikrokontroler utama & konektivitas Wi-Fi |
| 2 | Sensor Ultrasonik HC-SR04 | 2 | Mendeteksi objek masuk & mengukur kapasitas penuh wadah |
| 3 | Sensor Proximity Induktif (`LJ12A3-4-Z/BX`) | 1 | Mendeteksi sampah berbahan logam |
| 4 | Sensor Proximity Kapasitif (`LJC18A3-H-Z/BX`) | 1 | Mendeteksi sampah organik / berkadar air tinggi |
| 5 | Motor Servo (`MG996R` / `SG90`) | 1 | Menggerakkan katup/pintu pemilah sampah |
| 6 | Step-Down Buck Converter (`LM2596`) | 1 | Menurunkan tegangan adaptor ke 5V stabil untuk servo & sensor |
| 7 | Adaptor Power Supply 12V 2A | 1 | Sumber daya utama rangkaian |

---

## 🔌 Skema Rangkaian (Pinout ESP32)

| Komponen | Pin Komponen | Pin ESP32 | Keterangan |
| :--- | :--- | :--- | :--- |
| **Ultrasonik 1 (Deteksi Masuk)** | `TRIG` / `ECHO` | `GPIO 5` / `GPIO 18` | Trigger & Echo pembacaan objek |
| **Ultrasonik 2 (Kapasitas)** | `TRIG` / `ECHO` | `GPIO 19` / `GPIO 21` | Mengukur ketinggian sampah di wadah |
| **Proximity Induktif** | `OUT` (Signal) | `GPIO 34` | Gunakan *voltage divider* jika output sensor > 3.3V |
| **Proximity Kapasitif** | `OUT` (Signal) | `GPIO 35` | Gunakan *voltage divider* jika output sensor > 3.3V |
| **Motor Servo** | `PWM` (Kabel Oranye/Kuning) | `GPIO 13` | Sinyal kontrol sudut servo |

> **Catatan Daya:** Hubungkan pin `VCC` motor servo ke output `5V` eksternal dari modul Step-Down (bukan dari pin `3V3` ESP32) dan pastikan seluruh jalur `GND` terhubung menjadi satu (*common ground*).

---

## 📚 Library yang Dibutuhkan
Pastikan library berikut sudah terinstal di **Arduino IDE** atau **PlatformIO** sebelum melakukan *compile*:
- `ESP32Servo` oleh Kevin Harrington
- `WiFi` (bawaan board package ESP32)
- `Blynk` atau `UniversalTelegramBot` *(opsional, jika menggunakan fitur notifikasi IoT)*

---

## 🚀 Cara Menggunakan
1. **Clone atau Unduh Repositori**
   
       git clone https://github.com/USERNAME/pemilah-sampah-esp32.git

2. **Konfigurasi Kode**
   - Buka file `.ino` di Arduino IDE.
   - Sesuaikan kredensial Wi-Fi dan token API (jika fitur IoT diaktifkan):
   
         const char* ssid = "NAMA_WIFI_KAMU";
         const char* password = "PASSWORD_WIFI_KAMU";

3. **Upload ke Board ESP32**
   - Pilih board **DOIT ESP32 DEVKIT V1** pada menu *Tools > Board*.
   - Pilih port COM yang sesuai, lalu klik **Upload**.

---

## 📁 Struktur Folder

    pemilah-sampah-esp32/
    ├── src/
    │   └── pemilah_sampah.ino    # Kode utama ESP32
    ├── images/
    │   ├── wiring-diagram.png    # Gambar skema rangkaian
    │   └── foto-alat.jpg         # Dokumentasi fisik purwarupa
    └── README.md                 # Dokumentasi projek

---

## 📸 Dokumentasi Alat
*(Tambahkan foto purwarupa dan diagram rangkaianmu di folder `images/`, lalu tampilkan di bawah ini)*

- **Skema Rangkaian:** `images/wiring-diagram.png`
- **Hasil Purwarupa:** `images/foto-alat.jpg`

---

## 📄 Lisensi
Projek ini bersifat *open-source* di bawah lisensi [MIT License](LICENSE).
