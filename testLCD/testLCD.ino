#include <LiquidCrystal_I2C.h>

#define PUMP_1_PIN 10
#define PUMP_2_PIN 11
#define PUMP_3_PIN 12

#define STOP_BUTTON_PIN 2
#define SELECT_BUTTON_PIN 3
#define START_BUTTON_PIN 4

#define STOP_LED_PIN 5
#define SELECT_LED_PIN 6
#define START_LED_PIN 7

#define LCD_SDA_PIN A4
#define LCD_SCL_PIN A5

#define HUMIDITY_SENSOR_1_PIN A1
#define HUMIDITY_SENSOR_2_PIN A2
#define HUMIDITY_SENSOR_3_PIN A3

#define PUMP_DEFAULT_ML_PER_MIN 2500

#define PLANT_NUMBER 3

struct Plant {
  char Name[10];
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

LiquidCrystal_I2C LCD(0x27,16,2);

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

char * InitMessage = "P.W.S";
char * PumpNumberMessage = "Bomba ";
char * RunningMessage = "Ejecutando ...";
char * EmptyLine = "                ";

int CenterMessage(char * msg, uint8_t line) {
  uint8_t len = strlen(msg);
  uint8_t padding = (16 - len) / 2;
  LCD.setCursor(padding, line);
  LCD.print(msg);
}

void ClearLine(uint8_t line) {
  LCD.setCursor(0, line);
  LCD.print(EmptyLine);
}

void SetupLCDScreen() {
  LCD.init();
  LCD.clear();         
  LCD.backlight();
  CenterMessage(InitMessage, 0);
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

void SetDefaultConfiguration() {
  Channels[0] = (struct Channel) {
    &(Pumps[0]),
    &(Plants[0]),
    &(HumiditySensors[0])
  };
  Channels[1] = (struct Channel) {
    &(Pumps[1]),
    &(Plants[1]),
    &(HumiditySensors[1])
  };
  Channels[2] = (struct Channel) {
    &(Pumps[2]),
    &(Plants[2]),
    &(HumiditySensors[2])
  };
}

void TogglePin(byte Pin) {
  digitalWrite(Pin, !digitalRead(Pin));
}

void SetConfigMode() {

  digitalWrite(START_LED_PIN, LOW);
  digitalWrite(SELECT_LED_PIN, HIGH);
  digitalWrite(STOP_LED_PIN, LOW);

}

void Configure() {

  int channel = 0;

  while (channel < PLANT_NUMBER) {
    LCD.clear();  

    char pump[10];
    strcpy(pump, PumpNumberMessage);
    char number = (channel+1)+'0';
    strncat(pump, &number, 1);
  
    CenterMessage(pump, 0);
  
    delay(500);
  
    int plant = 0;
  
    while (true) {

      bool plantSelected = false;
      
      ClearLine(1);
      CenterMessage(Plants[plant].Name, 1);
      
      delay(700);
      
      while (true) {
      
        if (digitalRead(SELECT_BUTTON_PIN) == HIGH) {
          Channels[channel].Plant = & (Plants[plant]);
          plantSelected = true;
          break;
        }
      
        if (digitalRead(STOP_BUTTON_PIN) == HIGH) {
          plant = (plant + 1) % PLANT_NUMBER;
          break;
        }
      
        if (digitalRead(START_BUTTON_PIN) == HIGH) {
          plantSelected = true;
          channel = PLANT_NUMBER;
          break;
        }
      }

      if (plantSelected) {
        break;
      }
    }

    ++channel;
  }

  delay(500);
  LCD.clear();
  CenterMessage(InitMessage, 0);
}

void SetRunMode() {
  digitalWrite(START_LED_PIN, HIGH);
  digitalWrite(SELECT_LED_PIN, LOW);
  digitalWrite(STOP_LED_PIN, LOW);
}

void Run() {
  LCD.clear();
  CenterMessage(RunningMessage, 0);
  delay(5000);
  LCD.clear();
  CenterMessage(InitMessage, 0);
}

void SetStopMode() {
  digitalWrite(START_LED_PIN, LOW);
  digitalWrite(SELECT_LED_PIN, LOW);
  digitalWrite(STOP_LED_PIN, HIGH);
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
  SetStopMode();

  delay(500);
  
  if (digitalRead(SELECT_BUTTON_PIN) == HIGH) {
    SetConfigMode();
    Configure();
  }

  if (digitalRead(START_BUTTON_PIN) == HIGH) {
    SetRunMode();
    Run();
  }
}
