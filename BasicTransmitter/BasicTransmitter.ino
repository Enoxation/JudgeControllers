/*
Name:		Transmitter1.ino
Created:	2016-10-18 09:33:28
Author:	jeda
*/


/*
Name:		MasterReceiver.ino
Created:	2016-10-09 03:11:14
Author:	jeda
*/

#include <EasyButton.h>
#include <SimpleTimer.h>
#include <Judging.h>
#include <Bounce2.h>
#include <Timer.h>
#include <Event.h>
#include <SPI.h>
#include <RF24_config.h>
#include <RF24.h>
#include <printf.h>
#include <nRF24L01.h>

#define JUDGE_1_PIN 2 
#define JUDGE_3_PIN 3
#define WHITE_PIN 7
#define RED_PIN 8
#define CE_PIN 9
#define CS_PIN 10
#define BOUNCE_DELAY 50
#define VIBRATE_PIN A0
#define VIBRATE_OSCILLIATION 300UL
#define VIBRATE_OSCILLIATION_COUNT 10
/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 7 & 8 */
RF24 radio(CE_PIN, CS_PIN);

//
//
/**********************************************************/

uint8_t addresses[][6] = { "1Jeda", "2Jeda", "3Jeda" };
Bounce WB = Bounce();
Bounce RB = Bounce();
EasyButton wb(WHITE_PIN, NULL, CALL_NONE, true);

uint8_t lastSentDecision = 0;
bool WritingIsEnabled = true;
uint32_t WritingIsEnabledDelay;
SimpleTimer st;
Timer t;
int timerOnEvent = -1;
int timerOffEvent = -1;
uint32_t timerOnPeriod = 100UL;
uint32_t timerOffPeriod = 50UL;
uint8_t cycle = 0;

void setup() {

	Serial.begin(115200);
	while (!Serial) {}


	pinMode(VIBRATE_PIN, OUTPUT);
	pinMode(13, OUTPUT);
	//WB.attach(WHITE_PIN, INPUT_PULLUP);



	radio.begin();
	radio.setPALevel(RF24_PA_MAX);
	radio.setAutoAck(true);
	radio.setDataRate(RF24_250KBPS);
	radio.setPayloadSize(4);
	radio.setRetries(15, 15);
	radio.setChannel(110);
	radio.openWritingPipe(addresses[0]);
	//t.every(1000, send);
	//st.setInterval(100, toggleVIb);
	wb.SetThreshold(500);
	//digitalWrite(VIBRATE_PIN, LOW);
}

void loop() {
	//digitalWrite(A0, HIGH);
	st.run();
	wb.update();

	bool r, w;
	r = w = false;



	if (wb.IsHold()) {
		digitalWrite(VIBRATE_PIN, LOW);
		st.deleteTimer(timerOnEvent);
		st.deleteTimer(timerOffEvent);
		timerOnEvent, timerOffEvent = -1;
		timerOnPeriod = 100UL;
		timerOffPeriod = 50UL;
		Serial.println("Shutoff");
	}
	else if (wb.IsPushed()) {
		st.deleteTimer(timerOffEvent);
		st.deleteTimer(timerOnEvent);
		timerOnEvent = -1;
		timerOffEvent = -1;
		timerOnPeriod += 100;
		timerOffPeriod += 50;
		printDetails();
		OnEvent();
	}
	
	
	

}
void printDetails() {
	Serial.print("On Period: ");
	Serial.println(timerOnPeriod);
	Serial.print("Off Period: ");
	Serial.println(timerOffPeriod);
}
void OnEvent() {
	timerOffEvent = st.setTimeout(timerOnPeriod, OffEvent);
	digitalWrite(VIBRATE_PIN, HIGH);

}
void OffEvent() {
	timerOnEvent = st.setTimeout(timerOffPeriod, OnEvent);
	digitalWrite(VIBRATE_PIN, LOW);

}
void toggleLED() {
	static bool state = false;
	digitalWrite(13, state);
	state = !state;
}
void toggleVIb() {
	static bool state = true;
	digitalWrite(A0, state);
	state = !state;
}
void send() {
	static uint32_t sendUL = 0UL;
	Serial.println("Before print");

	bool OK = false;
	sendUL++;
	OK = radio.write(&sendUL, sizeof(sendUL));
	Serial.print("OK: ");
	Serial.println(OK == 1 ? "True" : "false");
}
