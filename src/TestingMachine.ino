#include <ArduinoJson.h>
#include <SPI.h>
#include <DFPlayerMini_Fast.h>
#include <Adafruit_Thermal.h>
#include <LiquidCrystal_I2C.h>
#include <SD.h>
#include <PinChangeInterrupt.h>
#include <SoftwareSerial.h>
#include <AnalogButtons.h>

#define MP3_RX 17
#define MP3_TX 16
#define PRINTER_RX 19
#define PRINTER_TX 18
#define BTN_INT 34

#define BTN_TIME 300

#define PROBABILITY 10 

#define USE_PRINTER 0

#define BUSY_PIN 53 

String json; 

bool inTest; 
bool lessonSelect; 
bool newInput; 
bool shouldPrintResults; 

String input; 

int correctAnswersNum = 0;

int questionAmount; 
int counter = 0;

LiquidCrystal_I2C lcd(0x27, 16, 2);

SoftwareSerial mp3Serial(MP3_RX, MP3_TX);
Adafruit_Thermal printer(&Serial1);

DFPlayerMini_Fast mp3;

void loadQuestions();
void formQuestionsList();
void playSoundForQuestion(int i);
void results();
void _generateTestFile();
bool playerBusy();

enum TestingMode{
	MODE_RANDOM,
	MODE_REGULAR
};


char buf; 
String numStr; 
bool qfound; 
bool didPrint; 
volatile char btn;
volatile bool st;
bool btnPressed; 
int sig; 
bool listReady; 

int audioCorrect, audioIncorrect;

unsigned long btnTime = 0; 

const 	int folderRootNum = 1;
const 	int fileCorrectNum = 1;
const 	int fileIncorrectNum = 2;
const 	int fileAskNum = 1;
		int folderSelectedNum = 2;

void setup() 
{
	Serial.begin(9600);
	Serial1.begin(9600);

#ifdef USE_PRINTER == 1
	printer.begin();
#endif

	mp3Serial.begin(9600);
	mp3.begin(mp3Serial);
	delay(100);
	mp3.volume(15);

	randomSeed(analogRead(0));

	pinMode(53, INPUT);

	pinMode(2, INPUT);
	pinMode(3, INPUT);
	pinMode(4, INPUT);
	pinMode(5, INPUT);
	pinMode(6, INPUT);
	pinMode(7, INPUT);
	pinMode(8, INPUT);
	pinMode(9, INPUT);
	pinMode(10, INPUT);
	pinMode(11, INPUT);
	pinMode(12, INPUT);
	pinMode(13, INPUT);
	pinMode(36, INPUT);
	pinMode(38, INPUT);
	pinMode(40, INPUT);
	pinMode(42, INPUT);
	pinMode(44, INPUT);
	pinMode(46, INPUT);

	Serial.println("Starting QA");

	lcd.init();
	lcd.backlight();
	lcd.clear();
	lcd.setCursor(0, 0);
	lcd.print("TEST MACHINE");
	lcd.setCursor(0, 1);
	Serial.println("MP3 Info: " + String(mp3.currentVersion()));
	delay(10);
	Serial.println("Volume: " + String(mp3.currentVolume()));
	countFiles();
	mp3.playFromMP3Folder(1);
	mp3.playFolder(folderSelectedNum, 2);
	// mp3.playAdvertisement(1);
	delay(900);
	Serial.println("Busy: " + String(playerBusy()));
	Serial.println("Tracks: " + String(mp3.numSdTracks()));

	

}

int countFiles()
{
	int n = 0;
	mp3.volume(1);
	for(int i=0; i<100; i++){
	    mp3.playFolder(folderSelectedNum, i);
	    Serial.println("i");
	    delay(900);
	    if (!playerBusy()) {
	    	n = i;
	    }
	}
	if (n>=1) 
	{
		listReady = true;
	}
	mp3.stop();
	mp3.volume(15);
	Serial.println("Counted files: " + String(n));
	return n;
}

bool playerBusy()
{
	delay(1);
	if (digitalRead(BUSY_PIN) == LOW) {return true;}
	else {return false;}
}

// ----
void start()
{
	inTest = !inTest;
	digitalWrite(LED_BUILTIN, inTest);
	lcd.clear();
	if (!inTest) counter = 0;
}

void one()
{
	input = input + "1";

	lcd.setCursor(0, 1); lcd.print("     ");
}

void two()
{
	input = input + "2";
	lcd.setCursor(0, 1); lcd.print("     ");

}

void three()
{
	input = input + "3";
	lcd.setCursor(0, 1); lcd.print("     ");
}

void four()
{
	input = input + "4";
	lcd.setCursor(0, 1); lcd.print("     ");
}

void five()
{
	input = input + "5";
	lcd.setCursor(0, 1); lcd.print("     ");
}

void six()
{
	input = input + "6";
	lcd.setCursor(0, 1); lcd.print("     ");
}

void seven()
{
	input = input + "7";
	lcd.setCursor(0, 1); lcd.print("     ");
}

void eight()
{
	input = input + "8";
	lcd.setCursor(0, 1); lcd.print("     ");
}

void nine()
{
	input = input + "9";
	lcd.setCursor(0, 1); lcd.print("     ");
}

void zero()
{
	input = input + "0";
	lcd.setCursor(0, 1); lcd.print("     ");
}

void yes()
{
	newInput = true;
}

void no()
{
	input.remove(input.length()-1, 1);
	lcd.setCursor(0, 1); lcd.print("     ");
}

void stats()
{
	Serial.println("No action specified");
}

void faster()
{
	Serial.println("No action specified");
}

void slower()
{
	Serial.println("No action specified");
}

void normal()
{
	Serial.println("No action specified");
}
// ----

void checkButtons()
{
	if(digitalRead(2) == HIGH && millis() - btnTime>=BTN_TIME) {lesson(); btnTime = millis(); Serial.println("lesson");}
	if(digitalRead(4) == HIGH && millis() - btnTime>=BTN_TIME) {start(); btnTime = millis(); Serial.println("start");}
	if(digitalRead(6) == HIGH && millis() - btnTime>=BTN_TIME) {stats(); btnTime = millis(); Serial.println("stats");}
	if(digitalRead(8) == HIGH && millis() - btnTime>=BTN_TIME) {faster(); btnTime = millis(); Serial.println("faster");}
	if(digitalRead(10) == HIGH && millis() - btnTime>=BTN_TIME) {slower(); btnTime = millis(); Serial.println("slower");}
	if(digitalRead(12) == HIGH && millis() - btnTime>=BTN_TIME) {normal(); btnTime = millis(); Serial.println("normal");}

	if(digitalRead(3) == HIGH && millis() - btnTime>=BTN_TIME) {seven(); btnTime = millis(); Serial.println("seven");}
	if(digitalRead(5) == HIGH && millis() - btnTime>=BTN_TIME) {eight(); btnTime = millis(); Serial.println("eight");}
	if(digitalRead(7) == HIGH && millis() - btnTime>=BTN_TIME) {nine(); btnTime = millis(); Serial.println("nine");}
	if(digitalRead(9) == HIGH && millis() - btnTime>=BTN_TIME) {zero(); btnTime = millis(); Serial.println("zero");}
	if(digitalRead(11) == HIGH && millis() - btnTime>=BTN_TIME) {no(); btnTime = millis(); Serial.println("no");}
	if(digitalRead(13) == HIGH && millis() - btnTime>=BTN_TIME) {yes(); btnTime = millis(); Serial.println("yes");}

	if(digitalRead(36) == HIGH && millis() - btnTime>=BTN_TIME) {one(); btnTime = millis(); Serial.println("one");}
	if(digitalRead(38) == HIGH && millis() - btnTime>=BTN_TIME) {two(); btnTime = millis(); Serial.println("two");}
	if(digitalRead(40) == HIGH && millis() - btnTime>=BTN_TIME) {three(); btnTime = millis(); Serial.println("three");}
	if(digitalRead(42) == HIGH && millis() - btnTime>=BTN_TIME) {four(); btnTime = millis(); Serial.println("four");}
	if(digitalRead(44) == HIGH && millis() - btnTime>=BTN_TIME) {five(); btnTime = millis(); Serial.println("five");}
	if(digitalRead(46) == HIGH && millis() - btnTime>=BTN_TIME) {six(); btnTime = millis(); Serial.println("six");}
}

void results()
{
	lcd.clear();
	lcd.setCursor(0, 0);
	lcd.print("Result:");
	float grade = 100.0 / (float)questionAmount * (float)correctAnswersNum;
	lcd.setCursor(0, 1);
	lcd.print(String(grade) + "%");

	printer.justify('C');
	printer.boldOn();
	printer.println(F("Results"));

	printer.justify('R');
	printer.boldOff();

	printer.println("Total: " + String(grade) + "%");

	printer.println(F(""));
	printer.println(F(""));
	printer.println(F(""));
	printer.println(F(""));
}

void loop() 
{
	int amount;
	bool shouldAsk, waitingForInput; 
	checkButtons();
	if (!inTest)
	{
		lcd.setCursor(0, 1);
		lcd.print("Press start...");
	}
	if (inTest)
	{
		if (!listReady)
		{
			amount = 100;
			Serial.println("Files in folder: " + String(amount));
			listReady = true;
		}
		if(counter == 0) 
		{
			next();
			printer.justify('C');
			printer.boldOn();
			printer.println(F("Test"));
			printer.println("");
			printer.justify('R');
			printer.boldOff();
		}

		if (counter == amount + 1) 
		{
			inTest = false;
			didPrint = true;
			results();
		}

		if (shouldAsk)
		{
			Serial.println("Asking question");
			mp3.playFolder(folderRootNum, fileAskNum);
			shouldAsk = false;
			waitingForInput = true;
		}

		if (!shouldAsk & !waitingForInput) 
		{
			if(!playerBusy()) 
			{
				Serial.println("Playing: " + String(folderSelectedNum) + String(counter));
				mp3.playFolder(folderSelectedNum, counter);
				delay(100);
				if (!playerBusy())
				{
					Serial.println("No more files");
					amount = counter;
				}

			}
			if (checkForStop()) shouldAsk = true;
		}
		

		if (waitingForInput & newInput)
		{
			Serial.println("New input: " + String(input));
			waitingForInput = false;
			if (input.compareTo(String(counter)) == 0)
			{
				input = "";
				correct();
				next();
			}
			else
			{
				input = "";
				incorrect();
				next();
			}
			newInput = false;
		}
		if (waitingForInput)
		{
			lcd.setCursor(0, 1);
			lcd.print(input);
			lcd.setCursor(0, 0);
			lcd.print("What number of sentence are we on?");
		}
	}
}

void next()
{
	lcd.clear();
	counter++;
	didPrint = false;
}

void correct()
{
	lcd.clear();
	lcd.setCursor(0, 1);
	lcd.print("Correct");

	printer.justify('L');
	printer.boldOff();
	printer.setSize('S');
	printer.println("Answer: " + input + " - CORRECT");
	printer.println(F(""));

	Serial.println("Playing audio");
	mp3.playFolder(folderRootNum, fileCorrectNum);

	delay(2000);
	newInput = false;

	correctAnswersNum++;
	Serial.println("Correct total: " + String(correctAnswersNum));
}

void incorrect()
{
	lcd.clear();
	lcd.setCursor(0, 1);
	lcd.print("Incorrect");

	printer.justify('L');
	printer.boldOff();
	printer.setSize('S');
	printer.println("Answer: " + input + " - INCORRECT");
	printer.println(F(""));

	Serial.println("Playing audio");
	mp3.playFolder(folderRootNum, fileIncorrectNum);

	delay(2000);
	newInput = false;
}

void printResults()
{
	Serial.println("No action specified");
}

void printInput(int num)
{
	Serial.println("Printing input");

	printer.justify('R');
	printer.boldOff();
	printer.setSize('S');
	String qt;

	qt = "Answer: " + input + " - CORRECT";
	printer.println(qt);
	printer.println(F(""));
}

bool checkForStop()
{
	int a = random(0, 100);
	Serial.print("Checking to stop \t");
	Serial.println(a);
	if (a <= PROBABILITY) {
		return true;
	}
	return false;
}

void askQuestion()
{
	Serial.println("No action specified");
}
