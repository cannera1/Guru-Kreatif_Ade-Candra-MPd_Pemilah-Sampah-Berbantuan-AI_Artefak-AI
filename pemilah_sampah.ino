#include <ESP32Servo.h>

// developed by Ade candra=====================================================
// PIN ESP32
// =====================================================

#define SERVO1_PIN 18
#define SERVO2_PIN 19

#define SENSOR_LOGAM 27

#define TRIG_PIN 5
#define ECHO_PIN 4

#define SENSOR_IR 25

#define SENSOR_KAPASITIF 34


// =====================================================
// OBJECT SERVO
// =====================================================

Servo servo1;
Servo servo2;


// =====================================================
// POSISI SERVO
// =====================================================

// SERVO 1
const int S1_KIRI   = 0;
const int S1_TENGAH = 90;
const int S1_KANAN  = 180;

// SERVO 2
const int S2_KIRI   = 0;
const int S2_TENGAH = 90;
const int S2_KANAN  = 180;


// =====================================================
// SENSOR KAPASITIF
// =====================================================

const int BATAS_KAPASITIF = 2400;


// =====================================================
// WAKTU
// =====================================================

// SR04 mendeteksi sampah
// kemudian tunggu sebelum membaca LJ12A3
const unsigned long DELAY_LJ12A3 = 2500;

// IR mendeteksi sampah
// kemudian tunggu sebelum membaca kapasitif
const unsigned long DELAY_IR = 3000;

// Lama servo berada pada posisi sortir
const unsigned long WAKTU_SERVO = 1500;

// Waktu sensor harus kosong sebelum sistem siap lagi
const unsigned long WAKTU_SENSOR_KOSONG = 1000;


// =====================================================
// STATE SERVO 1
// =====================================================

enum StateServo1
{
  S1_IDLE,
  S1_TUNGGU_LJ12A3,
  S1_SERVO_GERAK,
  S1_TUNGGU_KOSONG
};

StateServo1 stateServo1 = S1_IDLE;


// =====================================================
// STATE SERVO 2
// =====================================================

enum StateServo2
{
  S2_IDLE,
  S2_TUNGGU_KAPASITIF,
  S2_SERVO_GERAK,
  S2_TUNGGU_KOSONG
};

StateServo2 stateServo2 = S2_IDLE;


// =====================================================
// TIMER
// =====================================================

unsigned long waktuSR04 = 0;
unsigned long waktuServo1 = 0;

unsigned long waktuIR = 0;
unsigned long waktuServo2 = 0;

unsigned long waktuKosongSR04 = 0;
unsigned long waktuKosongIR = 0;


// =====================================================
// STATUS SAMPAH
// =====================================================

// true  = sampah NONLOGAM
// false = sampah LOGAM / belum diproses
bool sampahNonLogam = false;


// =====================================================
// BACA JARAK SR04
// =====================================================

long bacaJarak()
{
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);

  long durasi = pulseIn(
    ECHO_PIN,
    HIGH,
    30000
  );

  if (durasi == 0)
  {
    return 999;
  }

  long jarak = durasi * 0.0343 / 2;

  return jarak;
}


// =====================================================
// URUTKAN DATA
// =====================================================

void urutkanData(int data[], int jumlah)
{
  for (int i = 0; i < jumlah - 1; i++)
  {
    for (int j = i + 1; j < jumlah; j++)
    {
      if (data[j] < data[i])
      {
        int temp = data[i];

        data[i] = data[j];

        data[j] = temp;
      }
    }
  }
}


// =====================================================
// BACA SENSOR KAPASITIF
// 10 DATA
// MENGGUNAKAN MEDIAN
// =====================================================

int bacaKapasitifMedian()
{
  const int JUMLAH_DATA = 10;

  int data[JUMLAH_DATA];

  Serial.println();
  Serial.println("MEMBACA SENSOR KAPASITIF");

  for (int i = 0; i < JUMLAH_DATA; i++)
  {
    data[i] = analogRead(SENSOR_KAPASITIF);

    Serial.print("Data ");
    Serial.print(i + 1);
    Serial.print(" = ");
    Serial.println(data[i]);

    delay(50);
  }

  urutkanData(
    data,
    JUMLAH_DATA
  );

  int median =
    (data[4] + data[5]) / 2;

  Serial.print("Median = ");
  Serial.println(median);

  return median;
}


// =====================================================
// SERVO 1 KE 180 DERAJAT
// ADA LOGAM
// =====================================================

void servo1Kanan()
{
  Serial.println("SERVO 1 -> 180 DERAJAT");

  servo1.write(S1_KANAN);

  waktuServo1 = millis();

  stateServo1 = S1_SERVO_GERAK;
}


// =====================================================
// SERVO 1 KE 0 DERAJAT
// TIDAK ADA LOGAM
// =====================================================

void servo1Kiri()
{
  Serial.println("SERVO 1 -> 0 DERAJAT");

  servo1.write(S1_KIRI);

  waktuServo1 = millis();

  stateServo1 = S1_SERVO_GERAK;
}


// =====================================================
// SERVO 2 KE 180 DERAJAT
// BASAH
// =====================================================

void servo2Kanan()
{
  Serial.println("SERVO 2 -> 180 DERAJAT");

  servo2.write(S2_KANAN);

  waktuServo2 = millis();

  stateServo2 = S2_SERVO_GERAK;
}


// =====================================================
// SERVO 2 KE 0 DERAJAT
// KERING
// =====================================================

void servo2Kiri()
{
  Serial.println("SERVO 2 -> 0 DERAJAT");

  servo2.write(S2_KIRI);

  waktuServo2 = millis();

  stateServo2 = S2_SERVO_GERAK;
}


// =====================================================
// PROSES SENSOR SR04
// =====================================================
//
// SR04 < 20 cm
//       ↓
// Tunggu 2,5 detik
//       ↓
// Baca LJ12A3
//
// LJ12A3 = 1 → LOGAM → Servo 1 = 180°
// LJ12A3 = 0 → NONLOGAM → Servo 1 = 0°
// =====================================================

void prosesSensorSR04()
{
  long jarak = bacaJarak();

  Serial.print("SR04 = ");
  Serial.print(jarak);
  Serial.println(" cm");


  // ===================================================
  // 1. IDLE
  // MENUNGGU SR04
  // ===================================================

  if (stateServo1 == S1_IDLE)
  {
    if (
      jarak > 0 &&
      jarak < 20
    )
    {
      Serial.println();
      Serial.println("==============================");
      Serial.println("SR04 DETEKSI SAMPAH");
      Serial.println("JARAK < 20 CM");
      Serial.println();
      Serial.println("SERVO 1 BELUM BERGERAK");
      Serial.println("MENUNGGU 2,5 DETIK");
      Serial.println("MENUNGGU SAMPAH KE LJ12A3");
      Serial.println("==============================");

      waktuSR04 = millis();

      stateServo1 = S1_TUNGGU_LJ12A3;
    }
  }


  // ===================================================
  // 2. TUNGGU SAMPAH SAMPAI LJ12A3
  // ===================================================

  else if (
    stateServo1 == S1_TUNGGU_LJ12A3
  )
  {
    if (
      millis() - waktuSR04 >= DELAY_LJ12A3
    )
    {
      Serial.println();
      Serial.println("==============================");
      Serial.println("MEMBACA LJ12A3");
      Serial.println("==============================");


      int nilaiLogam =
        digitalRead(SENSOR_LOGAM);


      Serial.print("NILAI LJ12A3 = ");
      Serial.println(nilaiLogam);


      // =================================================
      // LJ12A3 = 1
      // ADA LOGAM
      // SERVO 1 = 180
      // =================================================

      if (nilaiLogam == HIGH)
      {
        Serial.println();
        Serial.println("LJ12A3 = 1");
        Serial.println("ADA LOGAM");
        Serial.println("HASIL = LOGAM");
        Serial.println("SERVO 1 -> 180 DERAJAT");

        // Logam tidak diteruskan ke proses IR
        sampahNonLogam = false;

        servo1Kanan();
      }


      // =================================================
      // LJ12A3 = 0
      // TIDAK ADA LOGAM
      // SERVO 1 = 0
      // =================================================

      else
      {
        Serial.println();
        Serial.println("LJ12A3 = 0");
        Serial.println("TIDAK ADA LOGAM");
        Serial.println("HASIL = NONLOGAM");
        Serial.println("SERVO 1 -> 0 DERAJAT");

        // Nonlogam akan diteruskan ke IR
        sampahNonLogam = true;

        servo1Kiri();
      }
    }
  }


  // ===================================================
  // 3. SERVO 1 SEDANG BERGERAK
  // ===================================================

  else if (
    stateServo1 == S1_SERVO_GERAK
  )
  {
    if (
      millis() - waktuServo1 >= WAKTU_SERVO
    )
    {
      servo1.write(S1_TENGAH);

      Serial.println();
      Serial.println("SERVO 1 -> 90 DERAJAT");
      Serial.println("SERVO 1 KEMBALI KE TENGAH");

      stateServo1 = S1_TUNGGU_KOSONG;
    }
  }


  // ===================================================
  // 4. MENUNGGU SR04 KOSONG
  // ===================================================

  else if (
    stateServo1 == S1_TUNGGU_KOSONG
  )
  {
    if (jarak > 20)
    {
      if (waktuKosongSR04 == 0)
      {
        waktuKosongSR04 = millis();
      }

      if (
        millis() - waktuKosongSR04 >=
        WAKTU_SENSOR_KOSONG
      )
      {
        Serial.println();
        Serial.println("SR04 KOSONG");
        Serial.println("SERVO 1 SIAP");

        stateServo1 = S1_IDLE;

        waktuKosongSR04 = 0;
      }
    }
    else
    {
      waktuKosongSR04 = 0;
    }
  }
}


// =====================================================
// PROSES SENSOR IR
// =====================================================
//
// IR = 0
//   ↓
// ADA SAMPAH
//   ↓
// TUNGGU 3 DETIK
//   ↓
// BACA KAPASITIF
//
// KAPASITIF < 2400
//   ↓
// BASAH
//   ↓
// SERVO 2 = 180°
//
// KAPASITIF >= 2400
//   ↓
// KERING
//   ↓
// SERVO 2 = 0°
// =====================================================

void prosesSensorIR()
{
  int statusIR =
    digitalRead(SENSOR_IR);


  // ===================================================
  // 1. IDLE
  // ===================================================

  if (stateServo2 == S2_IDLE)
  {
    // IR = 0 = ADA OBJEK
    //
    // Hanya sampah nonlogam yang diteruskan
    // ke sensor IR.

    if (
      statusIR == LOW &&
      sampahNonLogam == true
    )
    {
      Serial.println();
      Serial.println("==============================");
      Serial.println("IR = 0");
      Serial.println("SAMPAH NONLOGAM TERDETEKSI");
      Serial.println();
      Serial.println("TUNGGU 3 DETIK");
      Serial.println("==============================");

      waktuIR = millis();

      stateServo2 = S2_TUNGGU_KAPASITIF;
    }
  }


  // ===================================================
  // 2. TUNGGU 3 DETIK
  // ===================================================

  else if (
    stateServo2 == S2_TUNGGU_KAPASITIF
  )
  {
    if (
      millis() - waktuIR >= DELAY_IR
    )
    {
      Serial.println();
      Serial.println("==============================");
      Serial.println("3 DETIK SELESAI");
      Serial.println("MEMBACA SENSOR KAPASITIF");
      Serial.println("==============================");


      int nilaiKapasitif =
        bacaKapasitifMedian();


      Serial.println();
      Serial.print("NILAI KAPASITIF = ");
      Serial.println(nilaiKapasitif);


      // =================================================
      // NILAI < 2400
      // BASAH
      // SERVO 2 = 180
      // =================================================

      if (
        nilaiKapasitif < BATAS_KAPASITIF
      )
      {
        Serial.println();
        Serial.println("KAPASITIF < 2400");
        Serial.println("HASIL = BASAH");
        Serial.println("SERVO 2 -> 180 DERAJAT");

        servo2Kanan();
      }


      // =================================================
      // NILAI >= 2400
      // KERING
      // SERVO 2 = 0
      // =================================================

      else
      {
        Serial.println();
        Serial.println("KAPASITIF >= 2400");
        Serial.println("HASIL = KERING");
        Serial.println("SERVO 2 -> 0 DERAJAT");

        servo2Kiri();
      }


      // Sampah sudah selesai diproses
      sampahNonLogam = false;
    }
  }


  // ===================================================
  // 3. SERVO 2 SEDANG BERGERAK
  // ===================================================

  else if (
    stateServo2 == S2_SERVO_GERAK
  )
  {
    if (
      millis() - waktuServo2 >= WAKTU_SERVO
    )
    {
      servo2.write(S2_TENGAH);

      Serial.println();
      Serial.println("SERVO 2 -> 90 DERAJAT");
      Serial.println("SERVO 2 KEMBALI KE TENGAH");

      stateServo2 = S2_TUNGGU_KOSONG;
    }
  }


  // ===================================================
  // 4. MENUNGGU IR KOSONG
  // ===================================================

  else if (
    stateServo2 == S2_TUNGGU_KOSONG
  )
  {
    // IR = 1 = TIDAK ADA OBJEK
    if (statusIR == HIGH)
    {
      if (waktuKosongIR == 0)
      {
        waktuKosongIR = millis();
      }

      if (
        millis() - waktuKosongIR >=
        WAKTU_SENSOR_KOSONG
      )
      {
        Serial.println();
        Serial.println("IR = 1");
        Serial.println("IR KOSONG");
        Serial.println("SERVO 2 SIAP");

        stateServo2 = S2_IDLE;

        waktuKosongIR = 0;
      }
    }
    else
    {
      waktuKosongIR = 0;
    }
  }
}


// =====================================================
// SETUP
// =====================================================

void setup()
{
  Serial.begin(115200);

  delay(1000);


  // ===================================================
  // LJ12A3
  // ===================================================

  pinMode(
    SENSOR_LOGAM,
    INPUT_PULLUP
  );


  // ===================================================
  // SR04
  // ===================================================

  pinMode(
    TRIG_PIN,
    OUTPUT
  );

  pinMode(
    ECHO_PIN,
    INPUT
  );


  // ===================================================
  // IR
  // ===================================================

  pinMode(
    SENSOR_IR,
    INPUT
  );


  // ===================================================
  // SENSOR KAPASITIF
  // ===================================================

  pinMode(
    SENSOR_KAPASITIF,
    INPUT
  );


  // ===================================================
  // SERVO
  // ===================================================

  servo1.setPeriodHertz(50);
  servo2.setPeriodHertz(50);

  servo1.attach(
    SERVO1_PIN,
    500,
    2500
  );

  servo2.attach(
    SERVO2_PIN,
    500,
    2500
  );


  // ===================================================
  // POSISI AWAL
  // ===================================================

  servo1.write(S1_TENGAH);
  servo2.write(S2_TENGAH);

  delay(1000);


  // ===================================================
  // INFORMASI SERIAL
  // ===================================================

  Serial.println();
  Serial.println("======================================");
  Serial.println("       PEMILAH SAMPAH ESP32");
  Serial.println("======================================");

  Serial.println();
  Serial.println("PIN SENSOR:");
  Serial.println("------------------------------");
  Serial.println("Servo 1       = GPIO 18");
  Serial.println("Servo 2       = GPIO 19");
  Serial.println("LJ12A3        = GPIO 27");
  Serial.println("SR04 TRIG     = GPIO 5");
  Serial.println("SR04 ECHO     = GPIO 4");
  Serial.println("IR            = GPIO 25");
  Serial.println("Kapasitif     = GPIO 34");

  Serial.println();
  Serial.println("LOGIKA SR04:");
  Serial.println("------------------------------");
  Serial.println("< 20 cm = ADA SAMPAH");

  Serial.println();
  Serial.println("LOGIKA LJ12A3:");
  Serial.println("------------------------------");
  Serial.println("0 = TIDAK ADA LOGAM");
  Serial.println("    SERVO 1 = 0 DERAJAT");
  Serial.println();
  Serial.println("1 = ADA LOGAM");
  Serial.println("    SERVO 1 = 180 DERAJAT");

  Serial.println();
  Serial.println("LOGIKA IR:");
  Serial.println("------------------------------");
  Serial.println("0 = ADA SAMPAH");
  Serial.println("1 = KOSONG");
  Serial.println("IR = 0 -> TUNGGU 3 DETIK");

  Serial.println();
  Serial.println("LOGIKA KAPASITIF:");
  Serial.println("------------------------------");
  Serial.println("< 2400 = BASAH");
  Serial.println("    SERVO 2 = 180 DERAJAT");
  Serial.println();
  Serial.println(">= 2400 = KERING");
  Serial.println("    SERVO 2 = 0 DERAJAT");

  Serial.println();
  Serial.println("POSISI AWAL:");
  Serial.println("------------------------------");
  Serial.println("SERVO 1 = 90 DERAJAT");
  Serial.println("SERVO 2 = 90 DERAJAT");

  Serial.println();
  Serial.println("======================================");
  Serial.println("SISTEM SIAP");
  Serial.println("======================================");
  Serial.println();
}


// =====================================================
// LOOP
// =====================================================

void loop()
{
  // Proses SR04 dan Servo 1
  prosesSensorSR04();

  // Proses IR, kapasitif dan Servo 2
  prosesSensorIR();

  delay(50);
}