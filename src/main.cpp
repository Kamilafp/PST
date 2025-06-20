#include <Arduino.h>
#include <DHT.h>
#include <Servo.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// ====== WiFi Hotspot HP ======
const char* ssid = "Belilah kuota";
const char* password = "pusingkuotaabis";

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
        (kondisiCahaya == "Cerah" && kondisiSuhu == "Rendah") ||
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
    body {
      font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
      background: #e8f0fe;
      margin: 0;
      padding: 20px;
      color: #222;
    }
    header {
      text-align: center;
      margin-bottom: 30px;
    }
    h1 {
      color: #2a52be;
    }
    .sensor-row {
      display: flex;
      justify-content: center;
      gap: 40px;
      margin-bottom: 30px;
      flex-wrap: wrap;
    }
    .sensor {
      background: white;
      padding: 20px;
      border-radius: 10px;
      box-shadow: 0 4px 8px rgba(0,0,0,0.1);
      width: 200px;
      text-align: center;
    }
    .sensor i {
      font-size: 40px;
      color: #2a52be;
      margin-bottom: 10px;
    }
    .sensor .value {
      font-size: 18px;
      font-weight: bold;
      margin-top: 5px;
    }
    .status-kanopi {
      font-size: 22px;
      font-weight: 700;
      margin-top: 10px;
    }
    .status-kanopi i {
      margin-right: 10px;
    }
    table {
      border-collapse: collapse;
      width: 100%;
      max-width: 800px;
      margin: 0 auto 40px auto;
      background: white;
      border-radius: 10px;
      overflow: hidden;
      box-shadow: 0 4px 12px rgba(0,0,0,0.1);
    }
    th, td {
      border: 1px solid #ddd;
      padding: 12px 15px;
      text-align: center;
    }
    th {
      background-color: #2a52be;
      color: white;
    }
    tbody tr:nth-child(even) {
      background: #f5faff;
    }
    .open {
      color: green;
      font-weight: bold;
    }
    .closed {
      color: red;
      font-weight: bold;
    }
    .section {
      max-width: 800px;
      margin: 0 auto;
      padding: 10px 20px;
      background: white;
      border-radius: 10px;
      box-shadow: 0 4px 8px rgba(0,0,0,0.1);
    }
    .section h2 {
      text-align: center;
      color: #2a52be;
      margin-bottom: 15px;
    }
    .keterangan-container {
      display: flex;
      justify-content: center;
      gap: 20px;
      flex-wrap: wrap;
      margin-top: 20px;
    }
    .keterangan-card {
      flex: 1;
      min-width: 250px;
      background: #f9fbff;
      border: 1px solid #d6e0f5;
      border-radius: 10px;
      padding: 20px;
      box-shadow: 0 3px 6px rgba(0,0,0,0.05);
    }
    .keterangan-card h3 {
      color: #2a52be;
      margin-bottom: 10px;
      font-size: 18px;
      border-bottom: 1px solid #cfdaf0;
      padding-bottom: 5px;
    }
    .keterangan-card ul {
      padding-left: 20px;
      color: #444;
      font-size: 15px;
      line-height: 1.6;
    }
    .keterangan-card ul li b {
      color: #222;
    }
    @media(max-width:700px) {
      .sensor-row {
        flex-direction: column;
        gap: 20px;
      }
      .sensor {
        width: 100%;
        max-width: 300px;
        margin: 0 auto;
      }
    }
  </style>
</head>
<body>
  <header>
    <h1>Kanopi Otomatis</h1>
  </header>
  <div class='sensor-row'>
)rawliteral";

// Sensor Suhu
html += "<div class='sensor'>";
html += "<i class='" + iconSuhu + "'></i>";
html += "<div>Suhu</div>";
html += "<div class='value'>" + suhuStr + "</div>";
html += "</div>";

// Sensor Cahaya
String iconCahaya = (cahayaStr.indexOf("Cerah") >= 0) ? "fas fa-sun" : "fas fa-cloud-moon";
html += "<div class='sensor'>";
html += "<i class='" + iconCahaya + "'></i>";
html += "<div>Cahaya</div>";
html += "<div class='value'>" + cahayaStr + "</div>";
html += "</div>";

// Sensor Hujan
String iconHujan = (hujanStr == "Hujan") ? "fas fa-cloud-showers-heavy" : "fas fa-cloud-sun";
html += "<div class='sensor'>";
html += "<i class='" + iconHujan + "'></i>";
html += "<div>Hujan</div>";
html += "<div class='value'>" + hujanStr + "</div>";
html += "</div>";

// Status Kanopi
html += "<div class='sensor'>";
html += "<i class='fas fa-door-" + String(isClosed ? "closed" : "open") + "'></i>";
html += "<div>Status Kanopi</div>";
html += "<div class='value status-kanopi'>" + statusKanopiStr + "</div>";
html += "</div>";

html += "</div>"; // End of sensor-row

// Section: Tabel Logika Keputusan
html += "<section class='section'>";
html += "<h2>Logika Keputusan Kanopi</h2>";
html += "<table><thead><tr>";
html += "<th>Sensor Hujan</th><th>Sensor LDR</th><th>Sensor Suhu (DHT11)</th><th>Status Kanopi</th>";
html += "</tr></thead><tbody>";

struct Rule {
  const char* hujan;
  const char* cahaya;
  const char* suhu;
  const char* status;
};

Rule rules[] = {
  {"Hujan",    "Cerah", "Tinggi",  "Tutup"},
  {"Hujan",    "Cerah", "Sedang",  "Tutup"},
  {"Hujan",    "Cerah", "Rendah",  "Tutup"},
  {"Hujan",    "Gelap", "Tinggi",  "Tutup"},
  {"Hujan",    "Gelap", "Sedang",  "Tutup"},
  {"Hujan",    "Gelap", "Rendah",  "Tutup"},
  {"Nonhujan", "Cerah", "Tinggi",  "Tutup"},
  {"Nonhujan", "Cerah", "Sedang",  "Buka"},
  {"Nonhujan", "Cerah", "Rendah",  "Buka"},
  {"Nonhujan", "Gelap", "Tinggi",  "Tutup"},
  {"Nonhujan", "Gelap", "Sedang",  "Buka"},
  {"Nonhujan", "Gelap", "Rendah",  "Tutup"},
};

int totalRules = sizeof(rules) / sizeof(rules[0]);
for (int i = 0; i < totalRules; i++) {
  String statusClass = (String(rules[i].status) == "Buka") ? "open" : "closed";
  html += "<tr>";
  html += "<td>" + String(rules[i].hujan) + "</td>";
  html += "<td>" + String(rules[i].cahaya) + "</td>";
  html += "<td>" + String(rules[i].suhu) + "</td>";
  html += "<td class='" + statusClass + "'>" + String(rules[i].status) + "</td>";
  html += "</tr>";
}

html += "</tbody></table>";
html += "</section>";

html += "<div>"; 
html += "</div>"; 

// Section: Keterangan Sensor
html += R"rawliteral(
<section class='section'>
  <h2>Keterangan Sensor & Ambang Batas</h2>
  <div class='keterangan-container'>
    <div class='keterangan-card'>
      <h3>Sensor DHT11 (Suhu)</h3>
      <ul>
        <li><b>Rendah:</b> &lt; 29°C → Kanopi Tutup</li>
        <li><b>Sedang:</b> 29°C - 32°C → Kanopi Buka</li>
        <li><b>Tinggi:</b> &gt; 32°C → Kanopi Tutup</li>
      </ul>
    </div>
    <div class='keterangan-card'>
      <h3>Sensor LDR (Cahaya)</h3>
      <ul>
        <li><b>Cerah:</b> Nilai analog &lt; 700</li>
        <li><b>Gelap:</b> Nilai analog ≥ 700</li>
      </ul>
    </div>
    <div class='keterangan-card'>
      <h3>Sensor Hujan</h3>
      <ul>
        <li><b>Hujan:</b> Nilai ADC ≤ 900</li>
        <li><b>Tidak Hujan:</b> Nilai ADC &gt; 900</li>
      </ul>
    </div>
  </div>
</section>
</body>
</html>
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