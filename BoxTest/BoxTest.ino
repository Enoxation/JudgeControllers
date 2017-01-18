
#include <SimpleTimer.h>
#include <EasyButton.h>
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
#define WHITE_PIN 4
#define RED_PIN 5
#define CE_PIN 7
#define CS_PIN 8


/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 7 & 8 */

Timer t;
SimpleTimer st;

Bounce WB = Bounce();
Bounce RB = Bounce();
//
/**********************************************************/


uint8_t judge = 3;
uint8_t lastSentDecision = 0;
int  vibrateOnEvent = -1;
int  vibrateOffEvent = -1;

bool WritingIsEnabled = true;

void setup() {

	Serial.begin(115200);
	while (!Serial) {}


	pinMode(JUDGE_1_PIN, INPUT_PULLUP);
	pinMode(JUDGE_3_PIN, INPUT_PULLUP);

	WB.attach(WHITE_PIN, INPUT_PULLUP);
	WB.interval(50);
	RB.attach(RED_PIN, INPUT_PULLUP);
	RB.interval(50);

	judge = GetJudge();
	
	
}
void loop() {
	t.update();
	st.run();
	WB.update();
	RB.update();
	if (WB.fell() || RB.fell()) {
		PrintStatus();
	}

}

uint8_t GetJudge() {
	if (digitalRead(JUDGE_1_PIN) == HIGH && digitalRead(JUDGE_3_PIN) == LOW) {
		return 1;
	}
	else if (digitalRead(JUDGE_1_PIN) == LOW && digitalRead(JUDGE_3_PIN) == HIGH) {
		return 3;
	}
	return 2;

}


void PrintStatus() {
	Serial.println(F("Some Button was pushed"));
	Serial.print(F("White: "));
	Serial.println(!WB.read());
	Serial.print(F("Red: "));
	Serial.println(!RB.read());
	Serial.print(F("Judge: "));
	Serial.println(GetJudge());
	Serial.print(F("Decision: "));
	Serial.println(getDecisionNumber(!WB.read(), GetJudge()));
}
uint8_t getDecisionNumber(bool isWhite, uint8_t judge) {
	return  (judge * 2) - isWhite;
}

