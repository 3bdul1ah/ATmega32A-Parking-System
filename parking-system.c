#include <avr/io.h>          // Include AVR IO definitions for handling input/output
#include <avr/interrupt.h>   // Include AVR interrupt definitions for handling interrupts

#define F_CPU 1000000UL      // Define CPU frequency as 1 MHz for timer calculations

#define TOTAL_PARKING_SPACES 200  // Total number of parking spaces available

#define ENTRY_IR PD2            // Define entry IR sensor connected to pin PD2
#define EXIT_IR PD3             // Define exit IR sensor connected to pin PD3
#define SEGMENT_1 PA7           // Define first segment of the display connected to pin PA7
#define SEGMENT_2 PA6           // Define second segment of the display connected to pin PA6
#define SEGMENT_3 PA5           // Define third segment of the display connected to pin PA5
#define SEGMENT_4 PA4           // Define fourth segment of the display connected to pin PA4
#define RED_LED PC0             // Define red LED connected to pin PC0
#define GREEN_LED PC1           // Define green LED connected to pin PC1

#define DELAY_5_MS 5            // Define a constant for a delay of 5 milliseconds
#define DELAY_500_MS 500        // Define a constant for a delay of 500 milliseconds

volatile uint8_t availableSpaces = TOTAL_PARKING_SPACES; // Declare a global variable to track available parking spaces

void setup() {
    initializeIO();           // Call function to initialize IO settings
    initializeInterrupts();   // Call function to set up interrupts
    initializeTimer();        // Call function to initialize the timer
    sei();                    // Enable global interrupts
}

int main(void) {
    setup();                  // Perform initial setup

    while (1) {               // Enter an infinite loop
        if (availableSpaces == 0) {
            displayFull();    // Display "FULL" if no spaces are available
        } else {
            displayNumber(availableSpaces);  // Otherwise, show number of available spaces
        }
    }
}

void displayNumber(uint8_t number) {
    // Array mapping numbers to their 7-segment display representation
    const uint8_t segment09values[] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};
    uint8_t hundreds = (number / 100) % 10;  // Calculate the hundreds place
    uint8_t tens = (number / 10) % 10;       // Calculate the tens place
    uint8_t ones = number % 10;              // Calculate the ones place

    // Display the ones place
    PORTA = ~(1 << SEGMENT_4);               // Activate SEGMENT_4
    PORTB = segment09values[ones];           // Send value to display
    Timer1_Delay(DELAY_5_MS);                // Delay to stabilize display

    // Display the tens place
    PORTA = ~(1 << SEGMENT_3);               // Activate SEGMENT_3
    PORTB = segment09values[tens];           // Send value to display
    Timer1_Delay(DELAY_5_MS);                // Delay to stabilize display

    // Display the hundreds place
    PORTA = ~(1 << SEGMENT_2);               // Activate SEGMENT_2
    PORTB = segment09values[hundreds];       // Send value to display
    Timer1_Delay(DELAY_5_MS);                // Delay to stabilize display
}

void displayFull() {
    // Array to display the word "FULL"
    const uint8_t segmentFullValues[] = {0x71, 0x3E, 0x38};

    // Display 'F'
    PORTA = ~(1 << SEGMENT_1);               // Activate SEGMENT_1
    PORTB = segmentFullValues[0];            // Send 'F' to display
    Timer1_Delay(DELAY_5_MS);                // Delay to stabilize display

    // Display 'U'
    PORTA = ~(1 << SEGMENT_2);               // Activate SEGMENT_2
    PORTB = segmentFullValues[1];            // Send 'U' to display
    Timer1_Delay(DELAY_5_MS);                // Delay to stabilize display

    // Display 'L'
    PORTA = ~(1 << SEGMENT_3);               // Activate SEGMENT_3
    PORTB = segmentFullValues[2];            // Send 'L' to display
    Timer1_Delay(DELAY_5_MS);                // Delay to stabilize display

    // Display 'L'
    PORTA = ~(1 << SEGMENT_4);               // Activate SEGMENT_4
    PORTB = segmentFullValues[2];            // Send 'L' to display
    Timer1_Delay(DELAY_5_MS);                // Delay to stabilize display
}

void initializeIO() {
    DDRA = 0xF0;                            // Set the higher 4 bits of PORTA as output
    DDRB = 0xFF;                            // Set all bits of PORTB as output
    DDRC = 0xFF;                            // Set all bits of PORTC as output
    DDRD &= ~((1 << ENTRY_IR) | (1 << EXIT_IR));  // Set ENTRY_IR and EXIT_IR as input
    PORTD |= (1 << ENTRY_IR) | (1 << EXIT_IR);    // Enable pull-up resistors for ENTRY_IR and EXIT_IR
}

void initializeInterrupts() {
    GICR |= (1 << INT0) | (1 << INT1);      // Enable external interrupts INT0 and INT1
    MCUCR |= (1 << ISC01) | (1 << ISC11);   // Set INT0 and INT1 to trigger on the falling edge
}

void initializeTimer() {
    TCCR1B = (1 << WGM12) | (1 << CS12) | (1 << CS10);  // Set Timer1 to CTC mode with prescaler of 1024
}

void Timer1_Delay(uint16_t delay_ms) {
    TCNT1 = 0;                              // Reset Timer1 counter
    OCR1A = ((delay_ms / 1000.0) * (F_CPU / 1024) - 1); // Calculate and set compare match value
    TIFR |= (1 << OCF1A);                   // Clear Timer1 compare match flag

    while (!(TIFR & (1 << OCF1A)));         // Wait until Timer1 compare match flag is set

    TIFR |= (1 << OCF1A);                   // Clear the compare match flag again
}

ISR(INT0_vect) {                            // Interrupt service routine for INT0
    if (availableSpaces > 0) {             // Check if there are available spaces
        availableSpaces--;                 // Decrease available spaces
        PORTC |= (1 << GREEN_LED);         // Turn on green LED
        Timer1_Delay(DELAY_500_MS);        // Delay for visibility
        PORTC &= ~(1 << GREEN_LED);        // Turn off green LED
        Timer1_Delay(DELAY_500_MS);        // Delay before next action
    }
    GIFR |= (1 << INTF0);                  // Clear the INT0 interrupt flag
}

ISR(INT1_vect) {                            // Interrupt service routine for INT1
    if (availableSpaces < TOTAL_PARKING_SPACES) {  // Check if all spaces are not occupied
        availableSpaces++;                 // Increase available spaces
        PORTC |= (1 << RED_LED);           // Turn on red LED
        Timer1_Delay(DELAY_500_MS);        // Delay for visibility
        PORTC &= ~(1 << RED_LED);          // Turn off red LED
        Timer1_Delay(DELAY_500_MS);        // Delay before next action
    }
    GIFR |= (1 << INTF1);                  // Clear the INT1 interrupt flag
}
