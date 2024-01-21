#include <avr/io.h> // Include AVR Input/Output header for port definitions
#include <avr/interrupt.h> // Include AVR interrupt handling header

#define F_CPU 1000000UL // Define CPU frequency as 1 MHz for delay calculations
#define TOTAL_PARKING_SPACES 200 // Define the total number of parking spaces

// Define pin mappings for various components
#define ENTRY_IR PD2 // Infrared sensor for entry on pin PD2
#define EXIT_IR PD3 // Infrared sensor for exit on pin PD3
#define SEGMENT_1 PA7 // First segment of display on pin PA7
#define SEGMENT_2 PA6 // Second segment of display on pin PA6
#define SEGMENT_3 PA5 // Third segment of display on pin PA5
#define SEGMENT_4 PA4 // Fourth segment of display on pin PA4
#define RED_LED PC0 // Red LED on pin PC0
#define GREEN_LED PC1 // Green LED on pin PC1

// Define constants for delay times
#define DELAY_5_MS 5 // Delay of 5 milliseconds
#define DELAY_500_MS 500 // Delay of 500 milliseconds

volatile uint8_t availableSpaces = TOTAL_PARKING_SPACES; // Variable for tracking available parking spaces

void setup() {
    initializeIO(); // Initialize Input/Output ports
    initializeInterrupts(); // Setup interrupts
    initializeTimer(); // Initialize timer for delays
    sei(); // Enable global interrupts
}

int main(void) {
    setup(); // Perform initial setup

    while (1) { // Infinite loop
        if (availableSpaces == 0) {
            displayFull(); // Display 'full' if no spaces are available
        } else {
            displayNumber(availableSpaces); // Otherwise, display number of available spaces
        }
    }
}

void displayNumber(uint8_t number) {
    // Define segment values for displaying numbers 0-9
    const uint8_t segment09values[] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};
    uint8_t hundreds = (number / 100) % 10; // Calculate hundreds place
    uint8_t tens = (number / 10) % 10; // Calculate tens place
    uint8_t ones = number % 10; // Calculate ones place

    // Display each digit with a delay
    PORTA = ~(1 << SEGMENT_4); // Activate 4th segment
    PORTB = segment09values[ones]; // Display ones digit
    Timer1_Delay(DELAY_5_MS);

    PORTA = ~(1 << SEGMENT_3); // Activate 3rd segment
    PORTB = segment09values[tens]; // Display tens digit
    Timer1_Delay(DELAY_5_MS);

    PORTA = ~(1 << SEGMENT_2); // Activate 2nd segment
    PORTB = segment09values[hundreds]; // Display hundreds digit
    Timer1_Delay(DELAY_5_MS);
}

void displayFull() {
    // Define segment values for displaying 'FULL'
    const uint8_t segmentFullValues[] = {0x71, 0x3E, 0x38};

    // Display 'FULL' across the segments
    PORTA = ~(1 << SEGMENT_1); // Activate 1st segment
    PORTB = segmentFullValues[0]; // Display 'F'
    Timer1_Delay(DELAY_5_MS);

    PORTA = ~(1 << SEGMENT_2); // Activate 2nd segment
    PORTB = segmentFullValues[1]; // Display 'U'
    Timer1_Delay(DELAY_5_MS);

    PORTA = ~(1 << SEGMENT_3); // Activate 3rd segment
    PORTB = segmentFullValues[2]; // Display 'L'
    Timer1_Delay(DELAY_5_MS);

    PORTA = ~(1 << SEGMENT_4); // Activate 4th segment
    PORTB = segmentFullValues[2]; // Display 'L' again
    Timer1_Delay(DELAY_5_MS);
}

void initializeIO() {
    DDRA = 0xF0; // Set PA4-PA7 as output for the display segments
    DDRB = 0xFF; // Set PORTB as output
    DDRC = 0xFF; // Set PORTC as output
    DDRD &= ~((1 << ENTRY_IR) | (1 << EXIT_IR)); // Set ENTRY_IR and EXIT_IR as input
    PORTD |= (1 << ENTRY_IR) | (1 << EXIT_IR); // Enable pull-up resistors for ENTRY_IR and EXIT_IR
}

void initializeInterrupts() {
    GICR |= (1 << INT0) | (1 << INT1); // Enable external interrupts INT0 and INT1
    MCUCR |= (1 << ISC01) | (1 << ISC11); // Set interrupt sense control to falling edge
}

void initializeTimer() {
    TCCR1B = (1 << WGM12) | (1 << CS12) | (1 << CS10); // Configure timer 1 for CTC mode with prescaler 1024
}

void Timer1_Delay(uint16_t delay_ms) {
    TCNT1 = 0; // Reset timer count
    OCR1A = (int)((delay_ms / 1000.0) * (F_CPU / 1024) - 1); // Calculate output compare register value for delay

    while ((TIFR & (1 << OCF1A))==0); // Wait until the compare match flag sets

    TIFR |= (1 << OCF1A); // Clear the compare match flag
}

ISR(INT0_vect) { // Interrupt Service Routine for INT0
    PORTA = 0xFF; // Disable display
    if (availableSpaces > 0) {
        availableSpaces--; // Decrement available spaces
        PORTC|= (1 << GREEN_LED); // Turn on green LED
        Timer1_Delay(DELAY_500_MS); // Wait for 500 ms
        PORTC &= ~(1 << GREEN_LED); // Turn off green LED
    }
    
    GIFR |= (1 << INTF0); // Clear the external interrupt flag for INT0
}

ISR(INT1_vect) { // Interrupt Service Routine for INT1
    PORTA = 0xFF; // Disable display
    if (availableSpaces < TOTAL_PARKING_SPACES) {
        availableSpaces++; // Increment available spaces
        PORTC |= (1 << RED_LED); // Turn on red LED
        Timer1_Delay(DELAY_500_MS); // Wait for 500 ms
        PORTC &= ~(1 << RED_LED); // Turn off red LED
    }
    GIFR |= (1 << INTF1); // Clear the external interrupt flag for INT1
}

