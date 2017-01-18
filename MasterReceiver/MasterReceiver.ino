/*
 Name:		MasterReceiver.ino
 Created:	2016-10-09 03:11:14
 Author:	jeda
*/

// the setup function runs once when you press reset or power the board




#include <SimpleTimer.h>
#include <Judging.h>
#include <Timer.h>
#include <Event.h>
#include <SPI.h>
#include "RF24.h"


#define CE_PIN 7
#define CS_PIN 8
#define JUDGE_WAKEUP 1000UL
#define JUDGE_WAKEUP_TIMEOUT 10000UL
#define SLEEP_CHECK_INTERVAL 100UL
#define JUDGE_THINK_TIME 3000UL
/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 7 & 8 */
RF24 radio(CE_PIN, CS_PIN);

/**********************************************************/
bool HasDecided[] = { false,false,false };
byte addresses[][6] = { "1Jeda", "2Jeda","3Jeda" };
int8_t SleepingJudge = 0;

uint8_t judge = 1;


SimpleTimer st;
bool JudgingComplete = false;
bool ShouldDelayNotify = true;
bool wakeUpTimoutEnabled = false;
bool NotifyTimer = false;

int wakeupEvent = NO_TIMER;

uint32_t lastDecisionTime;
uint32_t SleepCheck_t0;
uint32_t NotifyTimer_t0;
uint32_t LastCompleteDecisionTime;


void setup() {

	Serial.begin(115200);

	
	radio.begin();
	radio.setPALevel(RF24_PA_MAX);
	radio.setAutoAck(true);
	radio.setDataRate(RF24_250KBPS);
	radio.setPayloadSize(1);
	radio.setRetries(15, 15);
	radio.setChannel(110);
	radio.openReadingPipe(1, addresses[0]);
	radio.openReadingPipe(2, addresses[1]);
	radio.openReadingPipe(3, addresses[2]);
	radio.startListening();

	NotifyTimer_t0 = millis();
	lastDecisionTime = millis();
	SleepCheck_t0 = millis();
	LastCompleteDecisionTime = millis();

}

void loop() {

	

	st.run();	

	if (radio.available()) {
		//Serial.print(F("Judge: "));
		Serial.println("Got something");
		uint8_t recByte;
		radio.read(&recByte, 1);
		Serial.print(F("Got: "));
		Serial.println(recByte);
		if (DECISION_MIN <= recByte && recByte <= DECISION_MAX) {

			Serial.print(F("Decision received: "));
			Serial.println(recByte);

			judge = GetJudge(recByte);
			int8_t SleepingJudgeBefore = GetSleepingJudge(HasDecided, sizeof(HasDecided));				
			HasDecided[judge - 1] = true;
			SleepingJudge = GetSleepingJudge(HasDecided, sizeof(HasDecided));
			NotifyTimer = false;
			if (SleepingJudgeBefore != 0 && SleepingJudge == 0) {
				LastCompleteDecisionTime = millis();
			}
			
			Keyboard.write('0' + recByte);
			lastDecisionTime = millis();
			wakeUpTimoutEnabled = true;

		}
	}
	if (millis() - SleepCheck_t0 >= SLEEP_CHECK_INTERVAL) {
		
		SleepCheck();
		SleepCheck_t0 = millis();

	}

	if (NotifyTimer &&  millis() - NotifyTimer_t0 >= JUDGE_WAKEUP) {
		Serial.println(F("Hello"));
		NotifyJudge();
	}
}
void SleepCheck() {
	SleepingJudge = GetSleepingJudge(HasDecided, sizeof(HasDecided));

	if (SleepingJudge == 0 && (millis() - LastCompleteDecisionTime >=  JUDGE_THINK_TIME)) {

		Serial.println(F("No sleeping"));
		removeWakeUp();
		ClearDecisions(HasDecided, sizeof(HasDecided));
		NotifyAll();

	}
	else if (wakeUpTimoutEnabled && millis() - lastDecisionTime >= JUDGE_WAKEUP_TIMEOUT) {
		ClearDecisions(HasDecided, sizeof(HasDecided));
		wakeUpTimoutEnabled = false;
		NotifyTimer = false;
	}
	else if (SleepingJudge >= 1 && SleepingJudge <= 3) {
		Serial.print(F("Sleeping: "));
		Serial.println(SleepingJudge);
		if (ShouldDelayNotify && !NotifyTimer) {
			NotifyTimer_t0 = millis();
			NotifyTimer = true;
		}
		else if(!ShouldDelayNotify){
			NotifyJudge();
		}
		//if (ShouldDelayNotify && !st.isEnabled(wakeupEvent)) {
		//	wakeupEvent = st.setTimeout(JUDGE_WAKEUP, NotifyJudge);
		//	//t.after(JUDGE_WAKEUP, NotifyJudge);
		//}
		//else if (!ShouldDelayNotify) {
		//	NotifyJudge();
		//}
	}
}
void removeWakeUp() {
	Serial.println(F("Remove Wakeup"));
	NotifyTimer = false;
	/*st.deleteTimer(wakeupEvent);
	wakeupEvent = NO_TIMER;*/
}
void NotifyJudge() {

	if (SleepingJudge == 0 || SleepingJudge == -1) {
		NotifyAll();
		return;
	}

	radio.stopListening();
	radio.openWritingPipe(addresses[SleepingJudge - 1]);
	Serial.print(F("Open Writing pipe: "));
	Serial.println(SleepingJudge - 1);
	uint8_t v = VIB_ON;
	ShouldDelayNotify = radio.write(&v, 1);
	/*if (!ShouldDelayNotify) {
		NotifyTimer = false;
	}
	*/
	NotifyTimer = !ShouldDelayNotify;
	Serial.print(F("Notify: "));
	Serial.println(ShouldDelayNotify ? "Success" : "Failed");
	radio.startListening();
}
void NotifyAll() {
	radio.stopListening();
	uint8_t o = VIB_OFF;
	for (int i = 0; i < 3; ++i) {
		radio.openWritingPipe(addresses[i]);
		radio.write(&o, 1);
		delayMicroseconds(500);
	}
	ShouldDelayNotify = true;
	radio.startListening();
}
uint8_t GetJudge(uint8_t Decision) {
	return (Decision + 1) / 2;
}
void ClearDecisions(bool * decision, uint8_t len) {

	for (int i = 0; i < len; i++) {
		*(decision + i) = false;
	}
	Serial.println(F("Decisions cleared"));
}
int8_t GetSleepingJudge(const bool * decision, uint8_t len) {
	int8_t numDecisions = 0;
	for (int i = 0; i < len; i++) {
		if (*(decision + i) == true) {
			++numDecisions;
		}
	}
	if (numDecisions == 3) {
		return 0;
	}
	if (numDecisions == 2) {
		for (int i = 0; i < len; i++) {
			if (*(decision + i) == false) {
				return i + 1;
			}
		}
	}
	return -1;
}
