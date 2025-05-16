#include <OneWire.h>
#include <DallasTemperature.h>

// DS18B20 Temp sensor setup
#define ONE_WIRE_BUS 2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
float temperatureC = 25.0; // Default fallback

// Turbidity sensor
#define TURBIDITY_PIN A1
float turbidityVoltage, turbidityNTU;

// pH Sensor
#define PH_PIN A0
float voltagePH, pHValue;

// DO Sensor
#define DO_PIN A3
#define VREF 5000    // in mV
#define ADC_RES 1024
#define CAL1_V (1455)
#define CAL1_T (25)
const uint16_t DO_Table[41] = {
  14460, 14220, 13820, 13440, 13090, 12740, 12420, 12110, 11810, 11530,
  11260, 11010, 10770, 10530, 10300, 10080, 9860, 9660, 9460, 9270,
  9080, 8900, 8730, 8570, 8410, 8250, 8110, 7960, 7820, 7690,
  7560, 7430, 7300, 7180, 7070, 6950, 6840, 6730, 6630, 6530, 6410
};

int16_t readDO(uint32_t voltage_mv, uint8_t temperature_c) {
  uint16_t V_saturation = (uint32_t)CAL1_V + 35 * temperature_c - CAL1_T * 35;
  return (voltage_mv * DO_Table[temperature_c] / V_saturation);
}

void setup() {
  Serial.begin(9600);
  sensors.begin();
}

void loop() {
  // Get temperature
  sensors.requestTemperatures();
  temperatureC = sensors.getTempCByIndex(0);
  if (temperatureC < 0 || temperatureC > 50) temperatureC = 25; // safe fallback

  // Read turbidity
  int turbidityRaw = analogRead(TURBIDITY_PIN);
  turbidityVoltage = turbidityRaw * (5.0 / 1024.0);
  turbidityNTU = -1120.4 * sq(turbidityVoltage) + 5742.3 * turbidityVoltage - 4352.9;

  // Read pH
  long sumPH = 0;
  for (int i = 0; i < 10; i++) {
    sumPH += analogRead(PH_PIN);
    delay(10);
  }
  float avgPH = sumPH / 10.0;
  voltagePH = avgPH * (5.0 / 1024.0);
  float m = (7.0 - 4.0) / (2.0 - 1.5);
  float c = 7.0 - m * 2.0;
  pHValue = m * voltagePH + c;

  // Read DO
  int doRaw = analogRead(DO_PIN);
  int adc_mv = (VREF * doRaw) / ADC_RES;
  int DOValue = readDO(adc_mv, (uint8_t)temperatureC);

  // Display all values
  Serial.println("---------- Water Quality Monitoring ----------");
  Serial.print("Temperature: ");
  Serial.print(temperatureC);
  Serial.println(" °C");

  Serial.print("Turbidity Voltage: ");
  Serial.print(turbidityVoltage, 2);
  Serial.print(" V | NTU: ");
  Serial.println(turbidityNTU);

  Serial.print("pH Voltage: ");
  Serial.print(voltagePH, 2);
  Serial.print(" V | pH Value: ");
  Serial.println(pHValue);

  Serial.print("DO Voltage: ");
  Serial.print(adc_mv);
  Serial.print(" mV | DO (mg/L): ");
  Serial.println(DOValue / 1000.0, 2); // since values are in µg/L

  Serial.println("------------------------------------------------\n");

  delay(2000);
}
