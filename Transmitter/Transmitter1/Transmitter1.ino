/*
 Name:		Transmitter1.ino
 Created:	2016-10-18 09:33:28
 Author:	jeda
*/



#include <Judging.h>
#include <Timer.h>
#include <Event.h>
#include <Bounce2.h>


#include <SPI.h>
#include <RF24_config.h>
#include <RF24.h>
#include <printf.h>
#include <nRF24L01.h>
#include <stdio.h>

#define JUDGE_1_PIN 2 
#define JUDGE_3_PIN 3
#define WHITE_PIN 7
#define RED_PIN 8
#define CE_PIN 9
#define CS_PIN 10
#define WRITING_ENABLED_DELAY 1000UL
#define VIBRATE_PIN A0
#define VIBRATE_OSCILLIATION  400UL
#define VIBRATE_OSCILLIATION_COUNT 10
#define MANAGE_JUDGES_INTERVAL 1000

/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 7 & 8 */
RF24 radio(CE_PIN, CS_PIN);
Timer t;

Bounce WB = Bounce();
Bounce RB = Bounce();

//
/**********************************************************/

uint8_t addresses[][6] = { "1Jeda", "2Jeda", "3Jeda" };
uint8_t judge = 0;
uint8_t lastSentDecision = 0;
uint32_t lastVibrateCommand = 0;
uint8_t vibrateEvent = NO_TIMER;
uint8_t enableWritingEvent = NO_TIMER;
bool WritingIsEnabled = true;

void setup() {
	
	Serial.begin(115200);
	while (!Serial) {}

	pinMode(VIBRATE_PIN, OUTPUT);
	pinMode(JUDGE_1_PIN, INPUT_PULLUP);
	pinMode(JUDGE_3_PIN, INPUT_PULLUP);
	pinMode(VIBRATE_PIN, OUTPUT);
	WB.attach(WHITE_PIN, INPUT_PULLUP);
	WB.interval(50);
	RB.attach(RED_PIN, INPUT_PULLUP);
	RB.interval(50);
	
	judge = GetJudge();
	
	Serial.print(F("Judge: "));
	Serial.println(judge);

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
	radio.closeReadingPipe(1);
	radio.closeReadingPipe(2);
	radio.closeReadingPipe(3);
	radio.openWritingPipe(addresses[0]);
	radio.openWritingPipe(addresses[1]);
	radio.openWritingPipe(addresses[2]);

	radio.openReadingPipe(judge, addresses[judge-1]);
	radio.openWritingPipe(addresses[judge-1]);
	
	
	
	radio.startListening();
	
}
void loop() {
	t.update();
	
	WB.update();
	RB.update();
	ManageJudgeSettings();
	
	if (radio.available(&judge)) {
		//Serial.print(F("Judge: "));
		//Serial.println(j);
		Serial.println(F("Something was read"));

		uint8_t v = 0;
		radio.read(&v, 1);
		Serial.println(v);

		if (v == VIB_ON) {

			Serial.println(F("Vibrate ON"));

			if (vibrateEvent == NO_TIMER) {
				
				Serial.println(F("New Timer"));				
				vibrateEvent = t.oscillate(VIBRATE_PIN, VIBRATE_OSCILLIATION, LOW, VIBRATE_OSCILLIATION_COUNT);
			}
			/*else {
				t.stop(vibrateEvent);
				vibrateEvent = t.oscillate(VIBRATE_PIN, VIBRATE_OSCILLIATION, LOW, VIBRATE_OSCILLIATION_COUNT);
			}*/
		}
		else if(v = VIB_OFF){	
			Serial.println(F("Vibrate OFF"));
			VibrateOff();
		}
	}

	uint8_t newDecision = lastSentDecision;
	bool white = WB.read();
	bool red = RB.read();

	if (white != red)
	{
		newDecision = getDecisionNumber(!white, judge);
		
		if (WritingIsEnabled || newDecision != lastSentDecision) {

			Serial.print(F("New decision: "));
			Serial.println(newDecision);

			Serial.print(F("Last sent decision: "));
			Serial.println(lastSentDecision);

			if (white) {
				Serial.println(F("White was pressed"));
			}
			else {
				Serial.println(F("Red was pressed"));
			}
			radio.stopListening();
			radio.flush_tx();

			Serial.println(F("Stop listen"));

			bool OK = radio.write(&newDecision, 1);			

			Serial.println("Write");

			radio.startListening();

			Serial.println("Start listen");

			if (!OK) {
				WritingIsEnabled = true;
				Serial.println(F("Writing failed"));
			}
			else{ 
				Serial.println(F("Writing successful"));

				lastSentDecision = newDecision;
				VibrateOff();
				WritingIsEnabled = false;
				if (enableWritingEvent != NO_TIMER) {
					t.stop(enableWritingEvent);
				}				
				enableWritingEvent = t.every(WRITING_ENABLED_DELAY, EnableWriting, 1);
			}
		}
	}
}
void ManageJudgeSettings() {
	uint8_t j = GetJudge();
	
	if (j != judge) {



		Serial.print(F("New Judge: "));
		Serial.println(j);

		radio.stopListening();
		radio.closeReadingPipe(1);
		radio.closeReadingPipe(2);
		radio.closeReadingPipe(3);
		judge = j;
		radio.openReadingPipe(judge, addresses[judge - 1]);
		radio.openWritingPipe(addresses[judge - 1]);

		radio.startListening();
	}
}
uint8_t GetJudge() {
	if (digitalRead(JUDGE_1_PIN) == LOW && digitalRead(JUDGE_3_PIN) == HIGH) {
		return 3;
	}
	else if (digitalRead(JUDGE_1_PIN) == HIGH && digitalRead(JUDGE_3_PIN) == LOW) {
		return 1;
	}
	else if (digitalRead(JUDGE_1_PIN) == HIGH && digitalRead(JUDGE_3_PIN) == HIGH) {
		return 2;
	}
	return 0;	
}

void toggleVibrate() {

	static bool state = true;
	digitalWrite(VIBRATE_PIN, state);
	state = !state;

}
void VibrateOff() {
	t.stop(vibrateEvent);	
	vibrateEvent = NO_TIMER;
	digitalWrite(VIBRATE_PIN, LOW);
}
void EnableWriting() {

	enableWritingEvent = NO_TIMER;
	WritingIsEnabled = true;

	Serial.println(F("Writing Is Enabled"));
}
uint8_t getDecisionNumber(bool isWhite, uint8_t judge) {
	return  (judge * 2) - isWhite;
}

