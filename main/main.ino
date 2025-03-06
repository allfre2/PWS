#define PUMP_1_PIN 10
#define PUMP_2_PIN 11
#define PUMP_3_PIN 12

#define STOP_BUTTON_PIN 2
#define SELECT_BUTTON_PIN 3
#define START_BUTTON_PIN 4

#define STOP_LED_PIN 5
#define SELECT_LED_PIN 6
#define START_LED_PIN 7

#define HUMIDITY_SENSOR_1_PIN A1
#define HUMIDITY_SENSOR_2_PIN A2
#define HUMIDITY_SENSOR_3_PIN A3

#define PUMP_DEFAULT_ML_PER_MIN 2500

#define PLANT_NUMBER 3

struct Plant {
  char Name[20];
  double Volume; // Cubic Centimeters
  double RequiredMiliLiters; // Per Unit of volume
  double RequiredHumidity;
};

struct Pump {
  byte Pin;
  double MiliLiters; // Per unit of time (Minutes)
};

struct HumiditySensor {
  byte Pin;
};

struct Channel {
  Pump * Pump;
  Plant * Plant;
  HumiditySensor HumiditySensor;
};

Pump Pumps[PLANT_NUMBER];
Plant Plants[PLANT_NUMBER];
HumiditySensor HumiditySensors[PLANT_NUMBER];
Channel Channels[PLANT_NUMBER];

void SetupPumps() {
  
  Pumps[0] = (struct Pump) {
    PUMP_1_PIN,
    PUMP_DEFAULT_ML_PER_MIN
  };

  Pumps[1] = (struct Pump) {
    PUMP_2_PIN,
    PUMP_DEFAULT_ML_PER_MIN
  };

  Pumps[2] = (struct Pump) {
    PUMP_3_PIN,
    PUMP_DEFAULT_ML_PER_MIN
  };

}

void SetupPlants() {

  Plants[0] = (struct Plant) {
    "Cactus",
    100,
    1000,
    0.4
  };

  Plants[1] = (struct Plant) {
    "Orquidea",
    100,
    1000,
    0.7
  };

  Plants[2] = (struct Plant) {
    "Gardenia",
    100,
    1000,
    0.85
  };

}

void SetupLCDScreen() {

}

void SetupButtons() {
  pinMode(STOP_BUTTON_PIN, INPUT);
  pinMode(START_BUTTON_PIN, INPUT);
  pinMode(SELECT_BUTTON_PIN, INPUT);
}

void SetupLEDs() {
  pinMode(STOP_LED_PIN, OUTPUT);
  pinMode(SELECT_LED_PIN, OUTPUT);
  pinMode(START_LED_PIN, OUTPUT);
}

void SetupSensors() {
  
  HumiditySensors[0] = (struct HumiditySensor) {
    HUMIDITY_SENSOR_1_PIN
  };
  
  HumiditySensors[1] = (struct HumiditySensor) {
    HUMIDITY_SENSOR_2_PIN
  };
  
  HumiditySensors[2] = (struct HumiditySensor) {
    HUMIDITY_SENSOR_3_PIN
  };

}

void TogglePin(byte Pin) {
  digitalWrite(Pin, !digitalRead(Pin));
}

void setup() {
  SetupPumps();
  SetupPlants();
  SetupButtons();
  SetupLEDs();
  SetupLCDScreen();
  SetupSensors();
}

void loop() {
  if (digitalRead(STOP_BUTTON_PIN) == HIGH) {
    digitalWrite(LED_BUILTIN, HIGH);
    TogglePin(STOP_LED_PIN);
    delay(500);
    digitalWrite(LED_BUILTIN, LOW);
  }
  if (digitalRead(SELECT_BUTTON_PIN) == HIGH) {
    digitalWrite(LED_BUILTIN, HIGH);
    TogglePin(SELECT_LED_PIN);
    delay(500);
    digitalWrite(LED_BUILTIN, LOW);
  }
  if (digitalRead(START_BUTTON_PIN) == HIGH) {
    digitalWrite(LED_BUILTIN, HIGH);
    TogglePin(START_LED_PIN);
    delay(500);
    digitalWrite(LED_BUILTIN, LOW);
  }
}
