
// the setup function runs once when you press reset or power the board



#include <Judging.h>
#include <Timer.h>
#include <Event.h>
#include <SPI.h>
#include "RF24.h"

/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 7 & 8 */
RF24 radio(7, 8);

/**********************************************************/

byte addresses[][6] = { "1Jeda", "2Jeda","3Jeda","4Jeda" };

bool progMode = true;


void setup() {

	Serial.begin(115200);
	while (!Serial) {}
	pinMode(4, INPUT_PULLUP);
	radio.begin();
	radio.setPALevel(RF24_PA_MAX);
	radio.setAutoAck(true);
	radio.setDataRate(RF24_250KBPS);
	radio.setPayloadSize(1);
	radio.setRetries(4, 15);
	radio.setChannel(110);
	radio.openReadingPipe(1, addresses[0]);
	radio.openReadingPipe(2, addresses[1]);
	radio.openReadingPipe(3, addresses[2]);

	radio.startListening();
	//Serial.print(F(""));
	//Serial.println("");

}

void loop() {
	
	
	/*if (Serial.available() >0) {
		Serial.println(F("Something was received"));
		char c = Serial.read();
		char j = Serial.read();
		if (j >= 1 && j <= 3 && (c== 'v' || c == 'o')) {
			radio.stopListening();
			radio.openWritingPipe(addresses[j - 1]);
			uint8_t send = c == 'v' ? VIB_ON : VIB_OFF;
			Serial.print(F("Judge: "));
			Serial.println(j);
			Serial.print(F("Command: "));
			Serial.println(c == 'v' ? "On" : "Off");
			
			if (radio.write(&send, 1)) {
				Serial.println("Success");
			}
			else {
				Serial.println("Failed");
			}

			radio.startListening();
		}
	}*/
		
		if (radio.available()) {
			uint8_t recUL = 0;
			
			radio.read(&recUL, sizeof(recUL));
			 
				Serial.println(F("\n\n\n\n\n\n"));
				
			
				//Keyboard.write('0'+recUL);
				Serial.println(recUL);
				if (recUL >= 1 || recUL <= 6) {
					uint8_t judge = GetJudge(recUL);
					radio.stopListening();
					radio.openWritingPipe(addresses[judge - 1]);
					uint8_t decision = recUL % 2;
					uint8_t send= VIB_OFF - decision;
					bool OK = radio.write(&send,1);				
					radio.startListening();
					if (!OK) {
						Serial.println(F("Response failed"));
					}
					else {
						Serial.println(F("Response successful"));
					}
				}
				
				
			
			}

	}

uint8_t GetJudge(uint8_t Decision) {
	return (Decision + 1) / 2;
}


