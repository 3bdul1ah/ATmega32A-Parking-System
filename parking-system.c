#include <avr/io.h> // Include the AVR input/output header
#include <avr/interrupt.h> // Include the AVR interrupt handling header
#define F_CPU 1000000UL // Define the CPU frequency as 1 MHz
// Define constants for the project
#define TOTAL_PARKING_SPACES 200 // Total number of parking spaces
#define ENTRY_IR PD2 // IR sensor for entry gate connected to PD2
#define EXIT_IR PD3 // IR sensor for exit gate connected to PD3
#define SEGMENT_1 PA7 // Segment 1 of the display connected to PA7
#define SEGMENT_2 PA6 // Segment 2 of the display connected to PA6
#define SEGMENT_3 PA5 // Segment 3 of the display connected to PA5
#define SEGMENT_4 PA4 // Segment 4 of the display connected to PA4
#define RED_LED PC0 // Red LED connected to PC0
#define GREEN_LED PC1 // Green LED connected to PC1
#define DELAY_5_MS 5 // Define a 5 millisecond delay
#define DELAY_500_MS 500 // Define a 500 millisecond delay
volatile uint8_t availableSpaces = TOTAL_PARKING_SPACES; // Variable to track the available parking spaces
void setup() {
initializeIO(); // Initialize the IO ports
initializeInterrupts(); // Setup the interrupts
initializeTimer(); // Setup the timer
sei(); // Enable global interrupts
}
int main(void) {
setup(); // Call the setup function
while (1) {
if (availableSpaces == 0) {
displayFull(); // Display "FULL" when no spaces are available
} else {
displayNumber(availableSpaces); // Otherwise, display the number of available spaces
}
}
}
// Implement the function to display numbers on the LED segments
void displayNumber(uint8_t number) {
// Array to map numbers to the 7‐segment display
const uint8_t segment09values[] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};
uint8_t hundreds = (number / 100) % 10; // Calculate hundreds digit
uint8_t tens = (number / 10) % 10; // Calculate tens digit
uint8_t ones = number % 10; // Calculate ones digit
// Display each digit with a delay
PORTA = ~(1 << SEGMENT_4);
PORTB = segment09values[ones];
Timer1_Delay(DELAY_5_MS);
PORTA = ~(1 << SEGMENT_3);
PORTB = segment09values[tens];
Timer1_Delay(DELAY_5_MS);
PORTA = ~(1 << SEGMENT_2);
PORTB = segment09values[hundreds];
Timer1_Delay(DELAY_5_MS);
}
// Implement the function to display "FULL" on the LED segments
void displayFull() {
// Array to map "FULL" to the 7‐segment display
const uint8_t segmentFullValues[] = {0x71, 0x3E, 0x38};
