
#include <ESP32Servo.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

// ================= PIN CONNECTIONS =================

#define TRIG_PIN 32
#define ECHO_PIN 33
#define SERVO_PIN 27
#define SOIL_PIN 34

// ================= EXISTING SETTINGS =================

#define DETECTION_DISTANCE 10

#define NEUTRAL_ANGLE 90
#define DRY_ANGLE 0
#define WET_ANGLE 180

#define SORTING_DELAY 3000

Servo myServo;

// ================= WIFI SETTINGS =================

const char* ssid = "Waste_Sorter_ESP32";
const char* password = "waste1234";

AsyncWebServer server(80);

// ================= DASHBOARD DATA =================

// 0 = No classification yet
// 1 = Dry
// 2 = Wet

volatile float dashboardDistance = -1;
volatile bool dashboardWasteDetected = false;
volatile int dashboardWasteType = 0;
volatile int dashboardServoAngle = 90;

volatile int dryCount = 0;
volatile int wetCount = 0;

// ================= DASHBOARD WEBPAGE =================

const char index_html[] PROGMEM = R"rawliteral(

<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">

<title>Smart Waste Segregation</title>

<style>

body {
  font-family: Arial;
  background: #101827;
  color: white;
  text-align: center;
  margin: 0;
  padding: 20px;
}

h1 {
  color: #38bdf8;
}

.container {
  max-width: 700px;
  margin: auto;
}

.grid {
  display: grid;
  grid-template-columns: repeat(2, 1fr);
  gap: 15px;
}

.card {
  background: #1e293b;
  padding: 25px 10px;
  border-radius: 15px;
}

.value {
  font-size: 30px;
  font-weight: bold;
  margin-top: 12px;
}

.label {
  color: #cbd5e1;
}

.status {
  margin-top: 20px;
  padding: 20px;
  background: #1e293b;
  border-radius: 15px;
}

.dry {
  color: #4ade80;
}

.wet {
  color: #38bdf8;
}

.waiting {
  color: #facc15;
}

</style>
</head>

<body>

<div class="container">

<h1>Smart Waste Segregation</h1>

<p id="connection" class="dry">ESP32 Connected</p>

<div class="grid">

<div class="card">
<div class="label">Ultrasonic Distance</div>
<div class="value" id="distance">-- cm</div>
</div>

<div class="card">
<div class="label">Waste Detection</div>
<div class="value" id="detection">Waiting</div>
</div>

<div class="card">
<div class="label">Dry Waste Count</div>
<div class="value dry" id="dryCount">0</div>
</div>

<div class="card">
<div class="label">Wet Waste Count</div>
<div class="value wet" id="wetCount">0</div>
</div>

</div>

<div class="status">

<h2>Latest Classification</h2>
<div class="value" id="wasteType">Waiting</div>

<h2>Servo Position</h2>
<div class="value" id="servo">90° - Neutral</div>

</div>

<p class="label">Live ESP32 monitoring dashboard</p>

</div>

<script>

async function updateDashboard() {

  try {

    const response = await fetch('/data');
    const data = await response.json();

    document.getElementById("connection").innerText =
      "ESP32 Connected";

    document.getElementById("connection").className = "dry";

    if (data.distance > 0) {
      document.getElementById("distance").innerText =
        data.distance.toFixed(1) + " cm";
    } else {
      document.getElementById("distance").innerText =
        "-- cm";
    }

    document.getElementById("detection").innerText =
      data.detected ? "Waste Detected" : "No Waste";

    document.getElementById("dryCount").innerText =
      data.dryCount;

    document.getElementById("wetCount").innerText =
      data.wetCount;

    const typeElement =
      document.getElementById("wasteType");

    typeElement.innerText = data.type;

    typeElement.className = "value";

    if (data.type === "DRY") {
      typeElement.classList.add("dry");
    }
    else if (data.type === "WET") {
      typeElement.classList.add("wet");
    }
    else {
      typeElement.classList.add("waiting");
    }

    let servoText = data.servo + "°";

    if (data.servo === 0) {
      servoText += " - Left (Dry)";
    }
    else if (data.servo === 180) {
      servoText += " - Right (Wet)";
    }
    else {
      servoText += " - Neutral";
    }

    document.getElementById("servo").innerText =
      servoText;

  }

  catch(error) {

    document.getElementById("connection").innerText =
      "Connection Lost";

    document.getElementById("connection").className =
      "wet";

  }

}

setInterval(updateDashboard, 500);

updateDashboard();

</script>

</body>
</html>

)rawliteral";

// ================= ULTRASONIC DISTANCE =================

float getDistance() {

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);

  if (duration == 0) {
    return -1;
  }

  return duration * 0.0343 / 2;
}

// ================= SETUP =================

void setup() {

  Serial.begin(9600);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(SOIL_PIN, INPUT);

  myServo.setPeriodHertz(50);
  myServo.attach(SERVO_PIN, 500, 2400);

  myServo.write(NEUTRAL_ANGLE);

  // Start ESP32 Wi-Fi Access Point

  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);

  Serial.println();
  Serial.println("SMART WASTE SEGREGATION SYSTEM");
  Serial.println("System Ready");

  Serial.print("Wi-Fi IP Address: ");
  Serial.println(WiFi.softAPIP());

  // Dashboard homepage

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html);
  });

  // Dashboard live data

  server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request) {

    float distance = dashboardDistance;
    bool detected = dashboardWasteDetected;
    int type = dashboardWasteType;
    int angle = dashboardServoAngle;
    int dry = dryCount;
    int wet = wetCount;

    const char* classification =
      type == 1 ? "DRY" :
      type == 2 ? "WET" : "WAITING";

    char json[200];

    snprintf(
      json,
      sizeof(json),
      "{\"distance\":%.1f,\"detected\":%s,\"type\":\"%s\",\"servo\":%d,\"dryCount\":%d,\"wetCount\":%d}",
      distance,
      detected ? "true" : "false",
      classification,
      angle,
      dry,
      wet
    );

    request->send(200, "application/json", json);

  });

  server.begin();

  Serial.println("Dashboard Server Started");
  Serial.println("Open http://192.168.4.1");

}

// ================= MAIN SORTING LOGIC =================

void loop() {

  float distance = getDistance();

  dashboardDistance = distance;

  if (distance > 0 && distance <= DETECTION_DISTANCE) {

    dashboardWasteDetected = true;

    Serial.println("Waste Detected!");

    // Read digital moisture output

    int moistureState = digitalRead(SOIL_PIN);

    Serial.print("Soil Sensor Digital State: ");
    Serial.println(moistureState);

    // LOW = WET

    if (moistureState == LOW) {

      dashboardWasteType = 2;
      wetCount++;

      Serial.println("WET WASTE DETECTED");
      Serial.println("Servo Moving RIGHT");

      myServo.write(WET_ANGLE);
      dashboardServoAngle = WET_ANGLE;

    }

    // HIGH = DRY

    else {

      dashboardWasteType = 1;
      dryCount++;

      Serial.println("DRY WASTE DETECTED");
      Serial.println("Servo Moving LEFT");

      myServo.write(DRY_ANGLE);
      dashboardServoAngle = DRY_ANGLE;

    }

    delay(SORTING_DELAY);

    myServo.write(NEUTRAL_ANGLE);
    dashboardServoAngle = NEUTRAL_ANGLE;

    Serial.println("Servo Returned to 90 Degrees");

    // Wait until waste is removed

    while (true) {

      float newDistance = getDistance();

      dashboardDistance = newDistance;

      if (newDistance < 0 || newDistance > DETECTION_DISTANCE) {
        break;
      }

      delay(150);

    }

    dashboardWasteDetected = false;

    Serial.println("Ready for Next Waste");

  }

  else {

    dashboardWasteDetected = false;

    myServo.write(NEUTRAL_ANGLE);
    dashboardServoAngle = NEUTRAL_ANGLE;

    Serial.println("No Waste - Servo at 90 Degrees");

  }

  delay(3000);

}
