#include "Sensors_SI7021.h"

#define NETWORKID 100 //the same on all nodes that talk to each other
#define GATEWAYID 1
#define FREQUENCY RF69_915MHZ
#define ENCRYPTKEY "sampleEncryptKey" //exactly the same 16 characters/bytes on all nodes!
#define FLASH_ADDR 1
#define BLINK_MS 50
#define LED_PIN 9
#define INTER 1

#define SLEEP_CYCLES 40

#define NODEID 4 //unique for each node on same network

byte cycle = 0;

//#define DEBUG

#define D(input) {Serial.print(input); Serial.flush();}
#define Dln(input) {Serial.println(input); Serial.flush();}

RFM69 radio;
SI7021 temp;
SPIFlash flash(5);

void blink(int cnt) {
	for (int i = 0; i < cnt; i++) {
		pinMode(LED_PIN, OUTPUT);
		digitalWrite(LED_PIN, HIGH);
		delay(BLINK_MS);
		digitalWrite(LED_PIN, LOW);
		delay(BLINK_MS);
		pinMode(LED_PIN, INPUT);
	}
}

int float_part(int val) {
	return ((float)val/100 - val/100) * 100;
}

void transmit(int temp, int hum) {
	char buff[20];
	sprintf(buff, "%0d.%d,%0d.%d",
			temp/100, float_part(temp),
			hum/100, float_part(hum));
#ifdef DEBUG
	D("Transmit: ");
	Dln(buff);
#endif
	boolean sent = radio.sendWithRetry(GATEWAYID, buff, (byte) strlen(buff));
	radio.sleep();
#ifdef DEBUG
	D("Sent ");
	Dln(sent ? "OK" : "Fail");
#endif

}

void setup() {
#ifdef DEBUG
	Serial.begin(115200);
	D("Start node ");
	Dln(NODEID);
#endif
	temp.begin();
	radio.initialize(FREQUENCY, NODEID, NETWORKID);
	radio.encrypt(ENCRYPTKEY);
	radio.sleep();
    if (flash.initialize()) flash.sleep();
	blink(4);
}

void loop() {

	cycle++;

	if (cycle >= SLEEP_CYCLES) {
		si7021_env data = temp.getHumidityAndTemperature();
		int temp = data.celsiusHundredths;
		int hum = data.humidityBasisPoints;
#ifdef DEBUG
		D("Temp: ");
		D(temp);
		D(" Hum: ")
		Dln(hum);
#endif
		transmit(temp, hum);
		cycle = 0;
	} else {
#ifdef DEBUG
		D(cycle);
		Dln(" - Sleeping");
#endif
	}

	LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
}
