#include <LiquidCrystal_I2C.h>

#define PUMP_1_PIN 12
#define PUMP_2_PIN 11
#define PUMP_3_PIN 10

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

#define HUMIDITY_SENSOR_MIN 520 // Dry
#define HUMIDITY_SENSOR_MAX 200 // Inside Water

#define TEMPERATURE_SENSOR_PIN A7

#define PUMP_DEFAULT_ML_PER_MIN 2500
#define MILLISECONDS_PER_MINUTE 60000

#define CHANNEL_NUMBER 2
#define PLANT_NUMBER 3

#define PLANT_MAX_VOLUME 5400
#define PLANT_MIN_VOLUME 300
#define VOLUME_INCREASE_STEP PLANT_MIN_VOLUME

bool RunMode = false;

struct Plant {
  char Name[10];
  double RequiredHumidity; // Percentage
};

struct Pump {
  byte Pin;
  double Limit;
  double CCPerMinute; // Per unit of time (Minutes)
};

struct HumiditySensor {
  byte Pin;
  int LastValue;
  double Humidity;
};

struct TemperatureSensor {
  byte Pin;
  int LastValue;
};

struct Channel {
  Pump * Pump;
  Plant * Plant;
  HumiditySensor * HumiditySensor;
};

Pump Pumps[PLANT_NUMBER];
Plant Plants[PLANT_NUMBER];
HumiditySensor HumiditySensors[PLANT_NUMBER];
Channel Channels[PLANT_NUMBER];

LiquidCrystal_I2C LCD(0x27,16,2);

void Join(char * str1, char * str2, char * dest, int length) {
    strcpy(dest, str1);
    strncat(dest, str2, length);
}

void Debug(char * str) {
  Serial.print(str);
}

void DebugConfiguration() {
  for (int i = 0; i < CHANNEL_NUMBER; ++i) {
    char number = (i+1)+'0';

    Debug("> Canal ");
    Debug(&number);
    Debug("\n");
    Debug(Channels[i].Plant -> Name);
    Debug("\n");
    Debug("Required Humidity: ");
    char humidity[10];
    dtostrf(Channels[i].Plant -> RequiredHumidity, 3, 2, humidity);
    Debug(humidity);
    Debug("\n");
    Debug("Limit (cc): ");
    char limit[10];
    dtostrf(Channels[i].Pump -> Limit, 1, 1, limit);
    Debug(limit);

    Debug("\n\n");
  }
}

double GetHumidityPercentage(int x) {
  double range = HUMIDITY_SENSOR_MIN - HUMIDITY_SENSOR_MAX;
  double value = HUMIDITY_SENSOR_MIN - x;
  double percent = (value/range);
  return percent;
}

void ReadHumidity(Channel * channel, int number) {
  Debug("\n");
  Debug("> Humidity for channel ");
  char channelNumber[8];
  sprintf(channelNumber, "%d", number+1);
  Debug(channelNumber);
  Debug(": ");
  
  HumiditySensor * sensor = channel -> HumiditySensor;
  sensor -> LastValue = analogRead(sensor -> Pin);
  sensor -> Humidity = GetHumidityPercentage(sensor -> LastValue);

  char reading[10];
  sprintf(reading, "%d", sensor -> LastValue);
  Debug(reading);

  char percentReading[10];
  dtostrf(sensor -> Humidity, 1, 2, percentReading);
  Debug(", ");
  Debug(percentReading);

  Plant * plant = channel -> Plant;

  Debug(" -> required: ");

  char requiredHumidity[10];
  dtostrf(plant -> RequiredHumidity, 1, 2, requiredHumidity);
  Debug(requiredHumidity);

  delay(200);
}

bool PlantIsOk(Channel * channel) {
  // TODO: Take into account temperature ?
  if (channel -> HumiditySensor -> Humidity < channel -> Plant -> RequiredHumidity) {
    return false;
  }

  return true;
}

void PumpWater(Channel * channel) {
  Plant * plant = channel -> Plant;
  HumiditySensor * humiditySensor = channel -> HumiditySensor;
  Pump * pump = channel -> Pump;

  double overflowLimit = 0.50 * channel -> Pump -> Limit;
  double percentageMissing = (channel -> Plant -> RequiredHumidity) - (channel -> HumiditySensor -> Humidity);

  double CCs = percentageMissing * overflowLimit;
  double MLPerMillisecond = pump -> CCPerMinute / MILLISECONDS_PER_MINUTE;
  double time = CCs / MLPerMillisecond;

  long pumpTime = round(time);

  Debug("\n");
  Debug("= Pumping water for ");
  char pumpTimeStr[10];
  sprintf(pumpTimeStr, "%d", pumpTime);
  Debug(pumpTimeStr);
  Debug(" milliseconds");

  digitalWrite(pump -> Pin, HIGH);
  delay(pumpTime);
  digitalWrite(pump -> Pin, LOW);
}

void SetupPumps() {
  
  Pumps[0] = (struct Pump) {
    PUMP_1_PIN,
    PLANT_MIN_VOLUME,
    PUMP_DEFAULT_ML_PER_MIN
  };

  Pumps[1] = (struct Pump) {
    PUMP_2_PIN,
    PLANT_MIN_VOLUME,
    PUMP_DEFAULT_ML_PER_MIN
  };

  Pumps[2] = (struct Pump) {
    PUMP_3_PIN,
    PLANT_MIN_VOLUME,
    PUMP_DEFAULT_ML_PER_MIN
  };

  for (int i = 0; i < CHANNEL_NUMBER; ++i) {
    pinMode(Pumps[i].Pin, OUTPUT);
  }
}

void SetupPlants() {

  Plants[0] = (struct Plant) {
    "Cactus",
    0.23
  };

  Plants[1] = (struct Plant) {
    "Orquidea",
    0.50
  };

  Plants[2] = (struct Plant) {
    "Gardenia",
    0.64
  };
}

char * InitMessage = "P.W.S";
char * PumpNumberMessage = "Bomba ";
char * VolumeMessage = "V: ";
char * RunningMessage = "Ejecutando ...";
char * EmptyLine = "                ";

int CenterMessage(char * msg, uint8_t line) {
  uint8_t len = strlen(msg);
  uint8_t padding = (16 - len) / 2;
  LCD.setCursor(padding, line);
  LCD.print(msg);
}

void DisplayPump(int channel, uint8_t line) {
    char pump[10];

    char number = (channel+1)+'0';
  
    Join(PumpNumberMessage, &number, pump, 1);

    CenterMessage(pump, line);
}

void DisplayVolume(int volume, uint8_t line) {
  char volumeString[8];
  sprintf(volumeString, "%d", volume);
  char volumeMsg[12];
  Join(VolumeMessage, volumeString, volumeMsg, strlen(volumeString));
  ClearLine(line);
  CenterMessage(volumeMsg, line);
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
    HUMIDITY_SENSOR_1_PIN,
    0,
    0
  };
  
  HumiditySensors[1] = (struct HumiditySensor) {
    HUMIDITY_SENSOR_2_PIN,
    0,
    0
  };
  
  HumiditySensors[2] = (struct HumiditySensor) {
    HUMIDITY_SENSOR_3_PIN,
    0,
    0
  };

  for (int i = 0; i < CHANNEL_NUMBER; ++i) {
    pinMode(HumiditySensors[i].Pin, INPUT);
  }
}

void SetupChannels() {
  
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
  RunMode = false;
}

void Configure() {

  int channel = 0;

  while (channel < CHANNEL_NUMBER) {
    LCD.clear();  

    DisplayPump(channel, 0);
  
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

          int selectedVolume = Channels[channel].Pump -> Limit;
          
          delay(500);

          DisplayVolume(selectedVolume, 1);
          
          while(true) {

            if (digitalRead(SELECT_BUTTON_PIN) == HIGH) {
              Channels[channel].Pump -> Limit = selectedVolume;
              break;
            }

            if (digitalRead(STOP_BUTTON_PIN) == HIGH) {
              selectedVolume = ((selectedVolume + VOLUME_INCREASE_STEP) % PLANT_MAX_VOLUME);
              DisplayVolume(selectedVolume, 1);
              delay(300);
            }
          }

          break;
        }
      
        if (digitalRead(STOP_BUTTON_PIN) == HIGH) {
          plant = (plant + 1) % PLANT_NUMBER;
          break;
        }
      
        if (digitalRead(START_BUTTON_PIN) == HIGH) {
          plantSelected = true;
          channel = CHANNEL_NUMBER;
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
  DebugConfiguration();
}

void SetRunMode() {
  digitalWrite(START_LED_PIN, HIGH);
  digitalWrite(SELECT_LED_PIN, LOW);
  digitalWrite(STOP_LED_PIN, LOW);
  RunMode = true;
}

void Run() {
  LCD.clear();
  CenterMessage(RunningMessage, 0);

  for (int channel = 0; channel < CHANNEL_NUMBER; ++channel) {

    if (digitalRead(STOP_BUTTON_PIN) == HIGH) {
      RunMode = false;
      break;
    }

    ReadHumidity(&(Channels[channel]), channel);

    if (!PlantIsOk(&(Channels[channel]))) {
      ClearLine(1);

      DisplayPump(channel, 1);

      PumpWater(&(Channels[channel]));

      delay(3000);

      ClearLine(1);
    }

    delay(700);
  }

  LCD.clear();
  CenterMessage(InitMessage, 0);
}

void SetStopMode() {
  digitalWrite(START_LED_PIN, LOW);
  digitalWrite(SELECT_LED_PIN, LOW);
  digitalWrite(STOP_LED_PIN, HIGH);
  RunMode = false;
}

void setup() {
  Serial.begin(9600);
  SetupPumps();
  SetupPlants();
  SetupChannels();
  SetupButtons();
  SetupLEDs();
  SetupLCDScreen();
  SetupSensors();
}

void loop() {
  if (!RunMode) {
    SetStopMode();
  }

  delay(500);
  
  if (digitalRead(SELECT_BUTTON_PIN) == HIGH) {
    SetConfigMode();
    Configure();
  }

  if (RunMode || digitalRead(START_BUTTON_PIN) == HIGH) {
    SetRunMode();
    Run();
  }
}
