#include <EEPROM.h>

#define WATERPUMP_PIN 7
#define MOISTURESENSOR_PIN A0

#define MOISTURE_DRY_THRESHOLD 350
#define PUMP_FREQUENCY 60 * 5
#define SLEEP_TIME (PUMP_FREQUENCY / 3)

// End of configuration

#define STATISTICS_MAGIC 0xae

struct statistics_t {
  unsigned int version;
  unsigned int cycles;
  unsigned int waterCycles;
  unsigned char magic;
};

unsigned long lastTimePumpOn = 0;
unsigned long sleepTime = (unsigned long)SLEEP_TIME * 1000;

statistics_t statistics;

void setup() {
  serialInit();
  statusLedInit();
  pumpInit();
  moistureSensorInit();
  statistics = statisticsLoad();
}

void loop() {
  statistics.cycles++;

  serialPrintf("[*] Awaken, running one loop [Watered %ld out of %ld cycles]", statistics.waterCycles, statistics.cycles);

  unsigned long currentTime = millis() / 1000;
  unsigned long secondsElapsedSincePumpOn = currentTime - lastTimePumpOn;

//  serialPrintf("[D] Last time: %ld, Current time: %ld, Elapse time: %ld", lastTimePumpOn, currentTime, secondsElapsedSincePumpOn);
  // Only switch at most once  every 5 minutes
  if (secondsElapsedSincePumpOn > PUMP_FREQUENCY) {

    Serial.println("[*] Enough time elapsed since last time pump was on, checking if we need some water");

    // Check if moisture is below threshold, and switch pump on for a while
    unsigned int moistureLevel = moistureSensorRead();

    if(moistureLevel > MOISTURE_DRY_THRESHOLD) {
      serialPrintf("[!] Moisture level (%d) is above threshold (%d), switching on pump", moistureLevel, MOISTURE_DRY_THRESHOLD);
      pumpStart();
      delay(10000);
      pumpStop();
      statistics.waterCycles++;
      statisticsStore(statistics);
    } else {
      serialPrintf("[*] Moisture level (%d) is below threshold (%d), no need to water", moistureLevel, MOISTURE_DRY_THRESHOLD);
    }
  } else {
    Serial.println("[*] The pump was used too recently, do not use it right now");
  }

  serialPrintf("[*] Sleeping for %d seconds", sleepTime / 1000);

  pumpStop(); // Just in case the pump was on
  delay(sleepTime);
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

void pumpInit() {
  pinMode(WATERPUMP_PIN, OUTPUT);
  pumpStop();
}

void pumpStart() {
  statusLedOn();
  lastTimePumpOn = millis() / 1000;
  digitalWrite(WATERPUMP_PIN, LOW);
}

void pumpStop() {
  digitalWrite(WATERPUMP_PIN, HIGH);
  statusLedOff();
}

void moistureSensorInit() {
  pinMode(MOISTURESENSOR_PIN, INPUT);
}

#define MOISTURE_SAMPLES 5

unsigned int moistureSensorRead() {
  unsigned int samples[5];

  for(int i=0; i < MOISTURE_SAMPLES; i++) {
    samples[i] = analogRead(MOISTURESENSOR_PIN);
  }

  return average(samples, MOISTURE_SAMPLES);
}

unsigned int average(unsigned int samples[], unsigned int sample_count) {
  unsigned int ret = 0;

  for(int i=0; i < sample_count; i++) {
    ret += samples[i] / sample_count;
  }

  return ret;
}

void statisticsStore(statistics_t statistics) {

  statistics.magic = STATISTICS_MAGIC;
  statistics.version = 1;

  EEPROM.put(0, statistics);
}

statistics_t statisticsLoad() {
  statistics_t ret;

  EEPROM.get(0, ret);

  if(ret.magic != STATISTICS_MAGIC) {
    ret.magic = STATISTICS_MAGIC;
    ret.version = 1;
    ret.cycles = 0;
    ret.waterCycles = 0;

    statisticsStore(ret);
  }

  return ret;
}
