/*
 * Moisture sensor calibration - use this script to measure the sensor reading in various environments, to find the thresholds for your watercan controller
 * 
 * In our case, the perfect soil us at a measurement of 310, but this depends on a lot of different factors: soil type, container size, temperature, relative humidity, etc. Hence this script :-)
 * 
 * Raw results from our calibration:
 * - Air: 610
 * - In water: 265
 * - Small pot (750cc) dry soil: 360
 * - Small pot (750cc) wet soil: 280
 * - Bigger pot (3500cc) perfect watered soil:  310
 * 
 */


#define MOISTURESENSOR_PIN A0

void setup() {
  serialInit();
  statusLedInit();
  moistureSensorInit();
}

void loop() {
  unsigned int moistureLevel = moistureSensorRead();

  serialPrintf("[*] Moisture level: %d\n", moistureLevel);
  delay(5000);
}

void serialInit() {
  Serial.begin(9600);
}

#define PRINTF_BUF 1024

void serialPrintf(const char *format, ...) {

      char buf[PRINTF_BUF];
      va_list ap;
      va_start(ap, format);
      vsnprintf(buf, sizeof(buf), format, ap);

      for (char *p = &buf[0]; *p; p++) {
        Serial.write(*p);
      }

      Serial.write("\r\n");
      va_end(ap);
}

void statusLedInit() {
  pinMode(LED_BUILTIN, OUTPUT);

  for(int i=0; i < 10; i++) {
    statusLedOn();
    delay(100);
    statusLedOff();
    delay(100);
  }
}

void statusLedOn() {
  digitalWrite(LED_BUILTIN, HIGH);
}

void statusLedOff() {
  digitalWrite(LED_BUILTIN, LOW);
}

void moistureSensorInit() {
  pinMode(MOISTURESENSOR_PIN, INPUT);
}

#define MOISTURE_SAMPLES 5

unsigned int moistureSensorRead() {
  unsigned int samples[5];

  for(int i=0; i < MOISTURE_SAMPLES; i++) {
    samples[i] = analogRead(MOISTURESENSOR_PIN);
    serialPrintf("%d ", samples[i]);
  }

  serialPrintf("\n");

  return average(samples, MOISTURE_SAMPLES);
}

unsigned int average(unsigned int samples[], unsigned int sample_count) {
  unsigned int ret = 0;

  for(int i=0; i < sample_count; i++) {
    ret += samples[i] / sample_count;
  }

  return ret;
}
