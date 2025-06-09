#include <Arduino.h>
#include <DHT.h>
#include <Servo.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// ====== WiFi Hotspot HP ======
const char* ssid = "Laniisa";
const char* password = "lanii220703";

// ====== Pin Sensor & Aktuator ======
#define DHTPIN D2
#define DHTTYPE DHT11
#define HUJAN_PIN D4      // sensor hujan digital
#define LDR_PIN A0        // sensor LDR analog
#define SERVO_PIN D1

DHT dht(DHTPIN, DHTTYPE);
Servo kanopi;
ESP8266WebServer server(80);

// ====== Variabel Status ======
bool isClosed = true;
String suhuStr, cahayaStr, hujanStr, statusKanopiStr;
float suhu = NAN;

// ====== Fungsi Buka & Tutup Kanopi ======
void bukaKanopi() {
  Serial.println("→ Buka kanopi");
  kanopi.writeMicroseconds(700);
  isClosed = false;
}

void tutupKanopi() {
  Serial.println("→ Tutup kanopi");
  kanopi.writeMicroseconds(2500);
  isClosed = true;
}

// ====== Fungsi Baca Sensor & Logika ======
void bacaSensor() {
  suhu = dht.readTemperature();
  int ldrValue = analogRead(LDR_PIN);        // baca LDR analog
  int hujanValue = digitalRead(HUJAN_PIN);   // baca hujan digital

  // Interpretasi suhu
  String kondisiSuhu = "Tidak diketahui";
  if (isnan(suhu)) {
    suhuStr = "Error";
  } else {
    suhuStr = String(suhu, 1) + " °C";
    if (suhu < 29) kondisiSuhu = "Rendah";
    else if (suhu <= 32) kondisiSuhu = "Sedang";
    else kondisiSuhu = "Tinggi";
  }

  // Interpretasi cahaya (LDR analog)
  String kondisiCahaya = (ldrValue >= 700) ? "Gelap" : "Cerah";
  cahayaStr = kondisiCahaya + " (Value: " + String(ldrValue) + ")";

  // Interpretasi hujan (digital)
  // Asumsi: LOW = hujan, HIGH = tidak hujan, bisa dicek sesuai sensor
  bool hujan = (hujanValue == LOW);
  hujanStr = hujan ? "Hujan" : "Tidak Hujan";

  // Logika keputusan 
  if (hujan) {
    // Semua kondisi hujan -> kanopi tutup
    tutupKanopi();
  } else {
    // Non hujan
    // Buka jika cahaya cerah sedang, atau gelap sedang, else tutup
    if ((kondisiCahaya == "Cerah" && kondisiSuhu == "Sedang") ||
        (kondisiCahaya == "Gelap" && kondisiSuhu == "Sedang")) {
      bukaKanopi();
    } else {
      tutupKanopi();
    }
  }

  statusKanopiStr = isClosed ? "Tertutup" : "Terbuka";

  // Debug di Serial Monitor
  Serial.println("============");
  Serial.println("Suhu: " + suhuStr + " (" + kondisiSuhu + ")");
  Serial.println("Cahaya: " + cahayaStr);
  Serial.println("Hujan: " + hujanStr + " (Digital: " + String(hujanValue) + ")");
  Serial.println("Kanopi: " + statusKanopiStr);
  Serial.println("============");
}

// ====== Tampilan Web ======
void handleRoot() {
  bacaSensor();
  String warnaStatus = isClosed ? "red" : "green";
  
  String iconSuhu = "fas fa-temperature-empty";  // default
  if (!isnan(suhu)) {
    if (suhu >= 29 && suhu <= 32) iconSuhu = "fas fa-temperature-half";
    else if (suhu > 32) iconSuhu = "fas fa-temperature-full";
  } else {
    suhuStr = "Error";
  }

  // Gunakan String untuk seluruh HTML supaya bisa gabung-gabung dengan mudah
  String html = R"rawliteral(
    <!DOCTYPE html>
    <html lang='id'>
    <head>
      <meta charset='UTF-8'>
      <meta name='viewport' content='width=device-width, initial-scale=1'>
      <title>Kanopi Otomatis</title>
      <link href='https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.4.0/css/all.min.css' rel='stylesheet'>
      <meta http-equiv='refresh' content='5'>
      <style>
        body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; background: #e8f0fe; margin:0; padding:20px; color:#222; }
        header { text-align:center; margin-bottom: 30px; }
        h1 { color: #2a52be; }
        .sensor-row { display:flex; justify-content:center; gap:40px; margin-bottom: 30px; flex-wrap: wrap; }
        .sensor { background: white; padding:20px; border-radius: 10px; box-shadow: 0 4px 8px rgba(0,0,0,0.1); width: 180px; text-align:center; }
        .sensor i { font-size: 40px; color: #2a52be; margin-bottom: 10px; }
        .sensor .value { font-size: 18px; font-weight: bold; margin-top: 5px; }
  )rawliteral";

  // Masukkan warna status di CSS .status-kanopi dengan string biasa
  html += ".status-kanopi { font-size: 22px; font-weight: 700; margin-top: 10px; color: " + warnaStatus + "; }";
  html += R"rawliteral(
        .status-kanopi i { margin-right: 10px; }
        table { border-collapse: collapse; width: 100%; max-width: 800px; margin: 0 auto 40px auto; background: white; border-radius: 10px; overflow: hidden; box-shadow: 0 4px 12px rgba(0,0,0,0.1); }
        th, td { border: 1px solid #ddd; padding: 12px 15px; text-align: center; }
        th { background-color: #2a52be; color: white; }
        tbody tr:nth-child(even) { background: #f5faff; }
        .open { color: green; font-weight: bold; }
        .closed { color: red; font-weight: bold; }
        .section { max-width: 800px; margin: 0 auto; padding: 10px 20px; background: white; border-radius: 10px; box-shadow: 0 4px 8px rgba(0,0,0,0.1); }
        .section h2 { text-align:center; color: #2a52be; margin-bottom: 15px; }
        .sensor-desc { font-size: 15px; line-height: 1.5; max-width: 600px; margin: 10px auto 0 auto; color: #444; }
        @media(max-width:700px) {
          .sensor-row { flex-direction: column; gap: 20px; }
          .sensor { width: 100%; max-width: 300px; margin: 0 auto; }
        }
      </style>
    </head>
    <body>
      <header><h1>Kanopi Otomatis</h1></header>
      <div class='sensor-row'>
        
  )rawliteral";
  
  // Icon suhu dinamis
  html += "<div class='sensor'>";
  html += "<i class='" + iconSuhu + "'></i>";
  html += "<div>Suhu</div>";
  html += "<div class='value'>" + suhuStr + "</div>";
  html += "</div>";

  // Icon cahaya
  String iconCahaya = (cahayaStr.indexOf("Cerah") >= 0) ? "fas fa-sun" : "fas fa-cloud-moon";
  html += "<div class='sensor'>";
  html += "<i class='" + iconCahaya + "'></i>";
  html += "<div>Cahaya</div>";
  html += "<div class='value'>" + cahayaStr + "</div>";
  html += "</div>";

  // Icon hujan
  String iconHujan = (hujanStr == "Hujan") ? "fas fa-cloud-showers-heavy" : "fas fa-cloud-sun";
  html += "<div class='sensor'>";
  html += "<i class='" + iconHujan + "'></i>";
  html += "<div>Hujan</div>";
  html += "<div class='value'>" + hujanStr + "</div>";
  html += "</div>";

  // Status kanopi
  html += "<div class='sensor'>";
  html += "<i class='fas fa-door-" + String(isClosed ? "closed" : "open") + "'></i>";
  html += "<div>Status Kanopi</div>";
  html += "<div class='value status-kanopi'>" + statusKanopiStr + "</div>";
  html += "</div>";

  html += "</div>"; // tutup .sensor-row

  // Tabel logika keputusan buka/tutup
  html += "<section class='section'>";
  html += "<h2>Logika Keputusan Kanopi</h2>";
  html += "<table>";
  html += "<thead><tr><th>Kondisi</th><th>Status Kanopi</th></tr></thead><tbody>";

  const char* rules[] = {
    "Hujan, Cerah, Tinggi", "Tutup",
    "Hujan, Cerah, Sedang", "Tutup",
    "Hujan, Cerah, Rendah", "Tutup",
    "Hujan, Gelap, Tinggi", "Tutup",
    "Hujan, Gelap, Sedang", "Tutup",
    "Hujan, Gelap, Rendah", "Tutup",

    "Nonhujan, Cerah, Tinggi", "Tutup",
    "Nonhujan, Cerah, Sedang", "Buka",
    "Nonhujan, Cerah, Rendah", "Tutup",
    "Nonhujan, Gelap, Tinggi", "Tutup",
    "Nonhujan, Gelap, Sedang", "Buka",
    "Nonhujan, Gelap, Rendah", "Tutup"
  };

  int rulePairs = sizeof(rules) / sizeof(rules[0]) / 2;
  for (int i = 0; i < rulePairs; i++) {
    String kondisi = String(rules[i*2]);
    String status = String(rules[i*2+1]);
    html += "<tr><td>" + kondisi + "</td><td class='" + (status=="Buka" ? "open" : "closed") + "'>" + status + "</td></tr>";
  }

  html += "</tbody></table>";
  html += "</section>";

  // Keterangan sensor
  html += R"rawliteral(
    <section class='section'>
      <h2>Keterangan Sensor & Ambang Batas</h2>
      <p class='sensor-desc'><b>Sensor DHT11 (Suhu):</b><br>
      Suhu rendah &lt; 29°C (kanopi tutup)<br>
      Suhu sedang 29°C ≤ x ≤ 32°C (kanopi buka)<br>
      Suhu tinggi &gt; 32°C (kanopi tutup)</p>

      <p class='sensor-desc'><b>Sensor LDR (Cahaya):</b><br>
      Nilai analog ≥ 700 = Gelap<br>
      Nilai analog &lt; 700 = Cerah</p>

      <p class='sensor-desc'><b>Sensor Hujan:</b><br>
      Nilai ADC ≤ 900 = Hujan<br>
      Nilai ADC &gt; 900 = Tidak Hujan</p>
    </section>
  )rawliteral";

  // Footer
  html += "<footer style='text-align:center; color:#666; font-size:0.9em; margin-top:40px;'>";
  html += "<i>Halaman otomatis refresh setiap 3 detik</i>";
  html += "</footer>";

  html += "</body></html>";

  server.send(200, "text/html", html);
}


// ====== Setup Awal ======
void setup() {
  Serial.begin(9600);
  dht.begin();

  pinMode(HUJAN_PIN, INPUT);
  // LDR_PIN analog otomatis sudah di-set input

  kanopi.attach(SERVO_PIN, 500, 2500);
  tutupKanopi();  // posisi awal

  WiFi.begin(ssid, password);
  Serial.print("Menghubungkan ke WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi terhubung!");
  Serial.print("Alamat IP: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.begin();
  Serial.println("Web server aktif...");
}

// ====== Loop ======
void loop() {
  server.handleClient();  

  // Update pembacaan sensor dan kendali servo secara berkala
  static unsigned long lastRead = 0;
  if (millis() - lastRead > 3000) { // setiap 3 detik baca sensor dan update servo
    bacaSensor();
    lastRead = millis();
  }
}
