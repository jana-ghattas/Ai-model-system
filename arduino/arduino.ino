#include <DHT.h>
#include <Servo.h>

#define DHT_PIN 2
#define SOIL_PIN A0
#define LDR_PIN A1
#define VALVE_RELAY 3
#define FAN_RELAY 5
#define LAMP_RELAY 6
#define SERVO_PIN 4

#define RELAY_ON LOW
#define RELAY_OFF HIGH

#define DHT_TYPE DHT11
DHT dht(DHT_PIN, DHT_TYPE);

Servo servo;

enum PlantProfile {
  STEVIA,
  BASIL,
  SPINACH
};

float temperature;
float humidity;
int soilMoisture;
int lightIntensity;
bool sensorOK = true;

float tempSetpoint;
float humSetpoint;
int soilSetpoint;
int lightSetpoint;

PlantProfile currentPlant;

void loadPlantSettings(PlantProfile plant) {
  switch (plant) {
    case STEVIA:
      tempSetpoint = 23.5;
      humSetpoint = 50;
      soilSetpoint = 60;
      lightSetpoint = 6000;
      break;

    case BASIL:
      tempSetpoint = 22;
      humSetpoint = 65;
      soilSetpoint = 60;
      lightSetpoint = 15400;
      break;

    case SPINACH:
      tempSetpoint = 20;
      humSetpoint = 70;
      soilSetpoint = 70;
      lightSetpoint = 5555;
      break;
  }
}

void readSensors() {
  sensorOK = true;
  delay(1000);
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("DHT ERROR");
    sensorOK = false;
  }

  soilMoisture = map(analogRead(SOIL_PIN), 0, 1023, 0, 100);
  lightIntensity = map(analogRead(LDR_PIN), 0, 1023, 0, 20000);

  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" °C");

  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");

  Serial.print("Soil Moisture: ");
  Serial.print(soilMoisture);
  Serial.println(" %");

  Serial.print("Light Intensity: ");
  Serial.print(lightIntensity);
  Serial.println(" lux");

  // Format for ESP32: SENSORS:temp,hum,soil,light
  Serial.print("SENSORS:");
  Serial.print(temperature);
  Serial.print(",");
  Serial.print(humidity);
  Serial.print(",");
  Serial.print(soilMoisture);
  Serial.print(",");
  Serial.println(lightIntensity);
}

void controlSystem() {
  if (temperature > tempSetpoint) {
    digitalWrite(FAN_RELAY, RELAY_ON); //low
    servo.write(90);
  } else {
    digitalWrite(FAN_RELAY, RELAY_OFF); //high
    servo.write(0);
  }

  if (lightIntensity < lightSetpoint) {
    digitalWrite(LAMP_RELAY, RELAY_OFF);
  } else {
    digitalWrite(LAMP_RELAY, RELAY_ON);
  }

  if (soilMoisture < soilSetpoint) {
    digitalWrite(VALVE_RELAY, RELAY_ON);
  } else {
    digitalWrite(VALVE_RELAY, RELAY_OFF);
  }

  Serial.println("System Status:");
  Serial.print("Fan: ");
  Serial.println(digitalRead(FAN_RELAY) == LOW ? "ON" : "OFF");

  Serial.print("Vents: ");
  Serial.println(servo.read() == 90 ? "OPEN" : "CLOSED");

  Serial.print("Lamp: ");
  Serial.println(digitalRead(LAMP_RELAY) == LOW ? "ON" : "OFF");

  Serial.print("Temp: "); Serial.println(temperature);
  Serial.print("Light: "); Serial.println(lightIntensity);
  Serial.print("Soil: "); Serial.println(soilMoisture);
}

void setup() {
  Serial.begin(115200);

  pinMode(VALVE_RELAY, OUTPUT);
  pinMode(FAN_RELAY, OUTPUT);
  pinMode(LAMP_RELAY, OUTPUT);

  servo.attach(SERVO_PIN);
  servo.write(0);

  delay(500);
  dht.begin();

  currentPlant = STEVIA;
  loadPlantSettings(currentPlant);
}

void loop() {
  readSensors();

  if (sensorOK) {
    controlSystem();
  }

  delay(2000);

  if (Serial.available()) {
    String msg = Serial.readStringUntil('\n');

    if (msg.startsWith("SET_PLANT:")) {
      String plant = msg.substring(10);
      plant.trim();
      if (plant == "Stevia" || plant == "STEVIA") loadPlantSettings(STEVIA);
      else if (plant == "Basil" || plant == "BASIL") loadPlantSettings(BASIL);
      else if (plant == "Spinach" || plant == "SPINACH") loadPlantSettings(SPINACH);
      Serial.println("Updated Plant to: " + plant);
    }

    if (msg == "PLANT_HEALTH:DRY") {
      digitalWrite(VALVE_RELAY, RELAY_ON);
    }

    if (msg == "PLANT_HEALTH:HEALTHY") {
      digitalWrite(VALVE_RELAY, RELAY_OFF);
    }

    if (msg == "PLANT_HEALTH:STRESSED") {
      digitalWrite(FAN_RELAY, RELAY_ON);
      servo.write(90);
    }
  }
}