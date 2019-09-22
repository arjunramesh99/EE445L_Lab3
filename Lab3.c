// Lab1.c
// Runs on TM4C123
// Uses ST7735.c LCD.
// Jonathan Valvano
// August 28, 2019
// Possible main program to test the lab
// Feel free to edit this to match your specifications

// Backlight (pin 10) connected to +3.3 V
// MISO (pin 9) unconnected 
// SCK (pin 8) connected to PA2 (SSI0Clk)
// MOSI (pin 7) connected to PA5 (SSI0Tx)
// TFT_CS (pin 6) connected to PA3 (SSI0Fss)
// CARD_CS (pin 5) unconnected
// Data/Command (pin 4) connected to PA6 (GPIO)
// RESET (pin 3) connected to PA7 (GPIO)
// VCC (pin 2) connected to +3.3 V
// Gnd (pin 1) connected to ground

#include <stdint.h>
#include <string.h>

#include "ST7735.h"
#include "PLL.h"
#include "../inc/tm4c123gh6pm.h"
#include "ClockBmp.h"
#include "Timers.h"

#define PB0   (*((volatile uint32_t *)0x40005004))
#define PB1   (*((volatile uint32_t *)0x40005008))
#define PB2   (*((volatile uint32_t *)0x40005010))
#define PB3   (*((volatile uint32_t *)0x40005020))
#define PB4   (*((volatile uint32_t *)0x40005040))
#define PB5   (*((volatile uint32_t *)0x40005080))
#define PB6   (*((volatile uint32_t *)0x40005100))
#define PB7   (*((volatile uint32_t *)0x40005200))

#define PF0   (*((volatile uint32_t *)0x40025004))
#define PF2   (*((volatile uint32_t *)0x40025010))
#define PF3   (*((volatile uint32_t *)0x40025020))
#define PF4   (*((volatile uint32_t *)0x40025040))

typedef struct Clock_Hand {
	int length;
	int angle_index, set_time_angle_index, set_alarm_angle_index;
	int angle_inc;
	short color;
} ClockHand;

ClockHand minute_hand, hour_hand;


void PortF_Init(void);
void displayNumTime(int hour_index, int minute_index, int msg_flag);
void displayHands(int hour_index, int minute_index);
// PF4 is input
// Make PF2 an output, enable digital I/O, ensure alt. functions off
void PortF_Init(void){ 
  SYSCTL_RCGCGPIO_R |= 0x20;        // 1) activate clock for Port F
  while((SYSCTL_PRGPIO_R&0x20)==0){}; // allow time for clock to start
                                    // 2) no need to unlock PF2, PF4
  GPIO_PORTF_PCTL_R &= ~0x000F0F00; // 3) regular GPIO
  GPIO_PORTF_AMSEL_R &= ~0x14;      // 4) disable analog function on PF2, PF4
  GPIO_PORTF_PUR_R |= 0x10;         // 5) pullup for PF4
  GPIO_PORTF_DIR_R |= 0x04;         // 5) set PF2 direction to output
	GPIO_PORTF_DIR_R &= ~(0x10);			// PF4 input
  GPIO_PORTF_AFSEL_R &= ~0x14;      // 6) regular port function
  GPIO_PORTF_DEN_R |= 0x14;         // 7) enable digital port
}

void PortB_Init(void){ 
  SYSCTL_RCGCGPIO_R |= 0x02;        // 1) activate clock for Port B
  while((SYSCTL_PRGPIO_R&0x02)==0){}; // allow time for clock to start
                                    // 2) no need to unlock PF2, PF4
  GPIO_PORTB_PCTL_R &= ~0x000F00FF; // 3) regular GPIO
  GPIO_PORTB_AMSEL_R &= ~0x13;      // 4) disable analog function on PF2, PF4
  GPIO_PORTB_PUR_R |= 0x13;         // 5) pullup for PF4
  GPIO_PORTB_DIR_R &= ~(0x13);         // 5) set direction to output
  GPIO_PORTB_AFSEL_R &= ~0x13;      // 6) regular port function
  GPIO_PORTB_DEN_R |= 0x13;         // 7) enable digital port
}


int isDisplayed = 0;
int BM_X = 7;
int BM_Y = 150;//138;
int CENTER_X = 64;
int CENTER_Y = 92;//80;
int secondCount = 0;
int prevPF4 = 1, prevPF0 = 1, prevPB0 = 1, prevPB1 = 1, prevPB4 = 1;

const short cosine[720] = {-1000, -1000, -1000, -1000, -999, -999, -999, -998, -998, -997, -996, -995, -995, -994, -993, -991, -990, -989, -988, -986, -985, -983, -982, -980, -978, -976, -974, -972, -970, -968, -966, -964, -961, -959, -956, -954, -951, -948, -946, -943, -940, -937, -934, -930, -927, -924, -921, -917, -914, -910, -906, -903, -899, -895, -891, -887, -883, -879, -875, -870, -866, -862, -857, -853, -848, -843, -839, -834, -829, -824, -819, -814, -809, -804, -799, -793, -788, -783, -777, -772, -766, -760, -755, -749, -743, -737, -731, -725, -719, -713, -707, -701, -695, -688, -682, -676, -669, -663, -656, -649, -643, -636, -629, -623, -616, -609, -602, -595, -588, -581, -574, -566, -559, -552, -545, -537, -530, -522, -515, -508, -500, -492, -485, -477, -469, -462, -454, -446, -438, -431, -423, -415, -407, -399, -391, -383, -375, -367, -358, -350, -342, -334, -326, -317, -309, -301, -292, -284, -276, -267, -259, -250, -242, -233, -225, -216, -208, -199, -191, -182, -174, -165, -156, -148, -139, -131, -122, -113, -105, -96, -87, -78, -70, -61, -52, -44, -35, -26, -17, -9, 0, 9, 17, 26, 35, 44, 52, 61, 70, 78, 87, 96, 105, 113, 122, 131, 139, 148, 156, 165, 174, 182, 191, 199, 208, 216, 225, 233, 242, 250, 259, 267, 276, 284, 292, 301, 309, 317, 326, 334, 342, 350, 358, 367, 375, 383, 391, 399, 407, 415, 423, 431, 438, 446, 454, 462, 469, 477, 485, 492, 500, 508, 515, 522, 530, 537, 545, 552, 559, 566, 574, 581, 588, 595, 602, 609, 616, 623, 629, 636, 643, 649, 656, 663, 669, 676, 682, 688, 695, 701, 707, 713, 719, 725, 731, 737, 743, 749, 755, 760, 766, 772, 777, 783, 788, 793, 799, 804, 809, 814, 819, 824, 829, 834, 839, 843, 848, 853, 857, 862, 866, 870, 875, 879, 883, 887, 891, 895, 899, 903, 906, 910, 914, 917, 921, 924, 927, 930, 934, 937, 940, 943, 946, 948, 951, 954, 956, 959, 961, 964, 966, 968, 970, 972, 974, 976, 978, 980, 982, 983, 985, 986, 988, 989, 990, 991, 993, 994, 995, 995, 996, 997, 998, 998, 999, 999, 999, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 999, 999, 999, 998, 998, 997, 996, 995, 995, 994, 993, 991, 990, 989, 988, 986, 985, 983, 982, 980, 978, 976, 974, 972, 970, 968, 966, 964, 961, 959, 956, 954, 951, 948, 946, 943, 940, 937, 934, 930, 927, 924, 921, 917, 914, 910, 906, 903, 899, 895, 891, 887, 883, 879, 875, 870, 866, 862, 857, 853, 848, 843, 839, 834, 829, 824, 819, 814, 809, 804, 799, 793, 788, 783, 777, 772, 766, 760, 755, 749, 743, 737, 731, 725, 719, 713, 707, 701, 695, 688, 682, 676, 669, 663, 656, 649, 643, 636, 629, 623, 616, 609, 602, 595, 588, 581, 574, 566, 559, 552, 545, 537, 530, 522, 515, 508, 500, 492, 485, 477, 469, 462, 454, 446, 438, 431, 423, 415, 407, 399, 391, 383, 375, 367, 358, 350, 342, 334, 326, 317, 309, 301, 292, 284, 276, 267, 259, 250, 242, 233, 225, 216, 208, 199, 191, 182, 174, 165, 156, 148, 139, 131, 122, 113, 105, 96, 87, 78, 70, 61, 52, 44, 35, 26, 17, 9, 0, -9, -17, -26, -35, -44, -52, -61, -70, -78, -87, -96, -105, -113, -122, -131, -139, -148, -156, -165, -174, -182, -191, -199, -208, -216, -225, -233, -242, -250, -259, -267, -276, -284, -292, -301, -309, -317, -326, -334, -342, -350, -358, -367, -375, -383, -391, -399, -407, -415, -423, -431, -438, -446, -454, -462, -469, -477, -485, -492, -500, -508, -515, -522, -530, -537, -545, -552, -559, -566, -574, -581, -588, -595, -602, -609, -616, -623, -629, -636, -643, -649, -656, -663, -669, -676, -682, -688, -695, -701, -707, -713, -719, -725, -731, -737, -743, -749, -755, -760, -766, -772, -777, -783, -788, -793, -799, -804, -809, -814, -819, -824, -829, -834, -839, -843, -848, -853, -857, -862, -866, -870, -875, -879, -883, -887, -891, -895, -899, -903, -906, -910, -914, -917, -921, -924, -927, -930, -934, -937, -940, -943, -946, -948, -951, -954, -956, -959, -961, -964, -966, -968, -970, -972, -974, -976, -978, -980, -982, -983, -985, -986, -988, -989, -990, -991, -993, -994, -995, -995, -996, -997, -998, -998, -999, -999, -999, -1000, -1000, -1000};
const short sine[720] = {0, 9, 17, 26, 35, 44, 52, 61, 70, 78, 87, 96, 105, 113, 122, 131, 139, 148, 156, 165, 174, 182, 191, 199, 208, 216, 225, 233, 242, 250, 259, 267, 276, 284, 292, 301, 309, 317, 326, 334, 342, 350, 358, 367, 375, 383, 391, 399, 407, 415, 423, 431, 438, 446, 454, 462, 469, 477, 485, 492, 500, 508, 515, 522, 530, 537, 545, 552, 559, 566, 574, 581, 588, 595, 602, 609, 616, 623, 629, 636, 643, 649, 656, 663, 669, 676, 682, 688, 695, 701, 707, 713, 719, 725, 731, 737, 743, 749, 755, 760, 766, 772, 777, 783, 788, 793, 799, 804, 809, 814, 819, 824, 829, 834, 839, 843, 848, 853, 857, 862, 866, 870, 875, 879, 883, 887, 891, 895, 899, 903, 906, 910, 914, 917, 921, 924, 927, 930, 934, 937, 940, 943, 946, 948, 951, 954, 956, 959, 961, 964, 966, 968, 970, 972, 974, 976, 978, 980, 982, 983, 985, 986, 988, 989, 990, 991, 993, 994, 995, 995, 996, 997, 998, 998, 999, 999, 999, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 999, 999, 999, 998, 998, 997, 996, 995, 995, 994, 993, 991, 990, 989, 988, 986, 985, 983, 982, 980, 978, 976, 974, 972, 970, 968, 966, 964, 961, 959, 956, 954, 951, 948, 946, 943, 940, 937, 934, 930, 927, 924, 921, 917, 914, 910, 906, 903, 899, 895, 891, 887, 883, 879, 875, 870, 866, 862, 857, 853, 848, 843, 839, 834, 829, 824, 819, 814, 809, 804, 799, 793, 788, 783, 777, 772, 766, 760, 755, 749, 743, 737, 731, 725, 719, 713, 707, 701, 695, 688, 682, 676, 669, 663, 656, 649, 643, 636, 629, 623, 616, 609, 602, 595, 588, 581, 574, 566, 559, 552, 545, 537, 530, 522, 515, 508, 500, 492, 485, 477, 469, 462, 454, 446, 438, 431, 423, 415, 407, 399, 391, 383, 375, 367, 358, 350, 342, 334, 326, 317, 309, 301, 292, 284, 276, 267, 259, 250, 242, 233, 225, 216, 208, 199, 191, 182, 174, 165, 156, 148, 139, 131, 122, 113, 105, 96, 87, 78, 70, 61, 52, 44, 35, 26, 17, 9, 0, -9, -17, -26, -35, -44, -52, -61, -70, -78, -87, -96, -105, -113, -122, -131, -139, -148, -156, -165, -174, -182, -191, -199, -208, -216, -225, -233, -242, -250, -259, -267, -276, -284, -292, -301, -309, -317, -326, -334, -342, -350, -358, -367, -375, -383, -391, -399, -407, -415, -423, -431, -438, -446, -454, -462, -469, -477, -485, -492, -500, -508, -515, -522, -530, -537, -545, -552, -559, -566, -574, -581, -588, -595, -602, -609, -616, -623, -629, -636, -643, -649, -656, -663, -669, -676, -682, -688, -695, -701, -707, -713, -719, -725, -731, -737, -743, -749, -755, -760, -766, -772, -777, -783, -788, -793, -799, -804, -809, -814, -819, -824, -829, -834, -839, -843, -848, -853, -857, -862, -866, -870, -875, -879, -883, -887, -891, -895, -899, -903, -906, -910, -914, -917, -921, -924, -927, -930, -934, -937, -940, -943, -946, -948, -951, -954, -956, -959, -961, -964, -966, -968, -970, -972, -974, -976, -978, -980, -982, -983, -985, -986, -988, -989, -990, -991, -993, -994, -995, -995, -996, -997, -998, -998, -999, -999, -999, -1000, -1000, -1000, -1000, -1000, -1000, -1000, -999, -999, -999, -998, -998, -997, -996, -995, -995, -994, -993, -991, -990, -989, -988, -986, -985, -983, -982, -980, -978, -976, -974, -972, -970, -968, -966, -964, -961, -959, -956, -954, -951, -948, -946, -943, -940, -937, -934, -930, -927, -924, -921, -917, -914, -910, -906, -903, -899, -895, -891, -887, -883, -879, -875, -870, -866, -862, -857, -853, -848, -843, -839, -834, -829, -824, -819, -814, -809, -804, -799, -793, -788, -783, -777, -772, -766, -760, -755, -749, -743, -737, -731, -725, -719, -713, -707, -701, -695, -688, -682, -676, -669, -663, -656, -649, -643, -636, -629, -623, -616, -609, -602, -595, -588, -581, -574, -566, -559, -552, -545, -537, -530, -522, -515, -508, -500, -492, -485, -477, -469, -462, -454, -446, -438, -431, -423, -415, -407, -399, -391, -383, -375, -367, -358, -350, -342, -334, -326, -317, -309, -301, -292, -284, -276, -267, -259, -250, -242, -233, -225, -216, -208, -199, -191, -182, -174, -165, -156, -148, -139, -131, -122, -113, -105, -96, -87, -78, -70, -61, -52, -44, -35, -26, -17, -9};		
int clockMode = 0;

void setCurrentTimeDisplay() {
		ST7735_FillRect(0, 0, 128, 32, ST7735_WHITE);
		displayNumTime(hour_hand.set_time_angle_index, minute_hand.set_time_angle_index, 1);
		ST7735_DrawBitmap(BM_X, BM_Y, ClockImg, 118, 118);
		displayHands(hour_hand.set_time_angle_index, minute_hand.set_time_angle_index);
		isDisplayed = 1;
}

void setAlarmDisplay() {
		ST7735_FillRect(0, 0, 128, 32, ST7735_WHITE);
		displayNumTime(hour_hand.angle_index, minute_hand.angle_index, 2);
		ST7735_DrawBitmap(BM_X, BM_Y, ClockImg, 118, 118);
		displayHands(hour_hand.angle_index, minute_hand.angle_index);
		isDisplayed = 1;
}

void ambientDisplay() {
		ST7735_FillRect(0, 0, 128, 32, ST7735_WHITE);
		displayNumTime(hour_hand.angle_index, minute_hand.angle_index, 0);
		ST7735_DrawBitmap(BM_X, BM_Y, ClockImg, 118, 118);
		displayHands(hour_hand.angle_index, minute_hand.angle_index);
		isDisplayed = 1;
}

void itoa(int val, char* val_str) {
	val_str[0] = val/10 + 48;
	val_str[1] = val%10 + 48;
}


void displayNumTime(int hour_index, int minute_index, int msg_flag) {
	int hour = hour_index / 60;
	if (hour == 0) 
		hour = 12;
	int minute = minute_index / 12;
	char hour_str[3], minute_str[3];
	itoa(hour, hour_str);
	itoa(minute, minute_str);
	char messages[3][25] = {"Current Time - ", "Set Cur Time - ", "Set Alarm - "};
	char* current_time = messages[msg_flag];
	strcat(current_time, hour_str); 
	strcat(current_time, ":");
	strcat(current_time, minute_str);
	ST7735_DrawString(1, 1, current_time, ST7735_BLACK);
}


void displayHands(int hour_index, int minute_index) { 
	int minute_hand_length_x = (minute_hand.length * sine[minute_index]) / 1000;
	int minute_hand_length_y = (minute_hand.length * cosine[minute_index]) / 1000;
	int hour_hand_length_x = (hour_hand.length * sine[hour_index]) / 1000;
	int hour_hand_length_y = (hour_hand.length * cosine[hour_index]) / 1000;
	
	ST7735_Line(CENTER_X, CENTER_Y, CENTER_X + minute_hand_length_x, CENTER_Y + minute_hand_length_y, minute_hand.color);
	ST7735_Line(CENTER_X, CENTER_Y, CENTER_X + hour_hand_length_x, CENTER_Y + hour_hand_length_y, hour_hand.color);
}

void (*modeRender[])() = {ambientDisplay, setAlarmDisplay, setCurrentTimeDisplay};	

void initHands() {
	minute_hand.angle_index = 0; 
	minute_hand.set_time_angle_index = 0;
	minute_hand.set_alarm_angle_index = 0;
	minute_hand.length = 50;
	minute_hand.color = ST7735_BLUE;
	
	hour_hand.angle_index = 0;
	hour_hand.set_time_angle_index = 0;
	hour_hand.set_alarm_angle_index = 0;
	hour_hand.length = 28;
	hour_hand.color = ST7735_RED;
}


/* Triggers every second to keep track */
void Timer2A_Handler(void) {
	TIMER2_ICR_R = TIMER_ICR_TATOCINT;
	PF2 ^= 0x04;
	volatile int k = 1;
	if (++secondCount == 60) {
		secondCount = 0;
		minute_hand.angle_index = (minute_hand.angle_index + 12) % 720; // Moves 12 times faster than hour hand
		hour_hand.angle_index = (hour_hand.angle_index + 1) % 720;
		isDisplayed = 0;
	}
}

void storePrevButtonVal() {
	prevPF0 = PF0;
	prevPF4 = PF4;
	prevPB0 = PB0;
	prevPB1 = PB1;
	prevPB4 = PB4;
}
int setTimeState = -1;

void decHand(ClockHand* hand) {
				minute_hand.set_time_angle_index = (minute_hand.set_time_angle_index - 12) % 720;
				if (minute_hand.set_time_angle_index < 0) 
					minute_hand.set_time_angle_index += 720;
				
}

void decrementHandler() {
	if(clockMode != 0) {		// Setting time
		if(!setTimeState) {
			hour_hand.set_time_angle_index = (hour_hand.set_time_angle_index - 60) % 720;
			if (hour_hand.set_time_angle_index < 0) 
				hour_hand.set_time_angle_index += 720;
		}
		else {
			minute_hand.set_time_angle_index = (minute_hand.set_time_angle_index - 12) % 720;
			if (minute_hand.set_time_angle_index < 0) 
				minute_hand.set_time_angle_index += 720;
				
			hour_hand.set_time_angle_index = (hour_hand.set_time_angle_index - 60) % 720;
			if (hour_hand.set_time_angle_index < 0) 
				hour_hand.set_time_angle_index += 720;
		}
	}
}


void incrementHandler() {
	if(clockMode != 0) {
		if(!setTimeState) {
			hour_hand.set_time_angle_index = (hour_hand.set_time_angle_index + 60) % 720;
		}
		else {
			minute_hand.set_time_angle_index = (minute_hand.set_time_angle_index + 12) % 720; 
			hour_hand.set_time_angle_index = (hour_hand.set_time_angle_index + 1) % 720;
		}
	}
}


void Timer0A_Handler(void) {
  TIMER0_ICR_R = TIMER_ICR_TATOCINT;// acknowledge timer0A timeout
	if ((PF4 & 0x10) == 0 && (prevPF4 & 0x10) != 0) {	 
		// Set Current Time
		clockMode = 2;
		if (++setTimeState == 2) {
			clockMode = 0;
			minute_hand.angle_index = minute_hand.set_time_angle_index;
			hour_hand.angle_index = hour_hand.set_time_angle_index;
			setTimeState = -1;
		} 
	}
	else if ((PF0 & 0x10) == 0 && (prevPF0 & 0x10) != 0) {
		// Set Alarm Time or Stop Alarm
		clockMode = 1;
		if (++setTimeState == 2) {
			clockMode = 0;
			a = minute_hand.set_alarm_angle_index;
			b = hour_hand.set_alarm_angle_index;
			setTimeState = -1;
		}
	}
	else if ((PB0 & 0x01) == 0) {  // && (prevPB0 & 0x01) != 0) {
		// Increment hand
		incrementHandler();
	}
	else if ((PB1 & 0x02) == 0 && (prevPB1 & 0x02) != 0) {
		// Decrement hand
		decrementHandler();
	}
	else if ((PB4 & 0x10) == 0 && (prevPB4 & 0x10) != 0) {
		// Back to Ambient
		clockMode = 0;
		setTimeState = -1;
	}
	storePrevButtonVal();
}


int main(void){
  PLL_Init(Bus80MHz); 
  PortF_Init();
	PortB_Init();
  ST7735_InitR(INITR_REDTAB);
	initHands();
	Timer2_ClockTick_Init(80000); // 80000000;
	Timer0_ButtonCheck_Init(8000000); // 50ms
	ST7735_FillScreen(ST7735_WHITE);
  while(1){
    PF2 ^= 0x04;
		if (!isDisplayed) {
			(*modeRender[clockMode])();
		}
  } 
} 


