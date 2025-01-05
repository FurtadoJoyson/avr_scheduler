#include <avr/io.h>
#include <avr/interrupt.h>

volatile uint16_t timer0Duration; // Variable for Timer 0 duration
volatile uint16_t timer1Duration; // Variable for Timer 1 duration
volatile uint16_t timer2Duration; // Variable for Timer 2 duration

volatile uint8_t timer0Flag = 0; // Flag for Timer 0 interrupt (highest priority)
volatile uint8_t timer1Flag = 0; // Flag for Timer 1 interrupt (medium priority)
volatile uint8_t timer2Flag = 0; // Flag for Timer 2 interrupt (lowest priority)

void UART_init(unsigned int baud) {
    unsigned int ubrr = F_CPU / 16 / baud - 1;
    UBRR0H = (unsigned char)(ubrr >> 8);
    UBRR0L = (unsigned char)ubrr;
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void UART_transmit(unsigned char data) {
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = data;
}

void UART_sendString(const char* str) {
    while (*str) {
        UART_transmit(*str++);
    }
}

void setupTimer0(uint16_t x) {
    timer0Duration = x;
    TCCR0A = (1 << WGM01); //Clear Timer on Compare Match (CTC) mode
    OCR0A = timer0Duration; //(F_CPU / 1024) * timer0Duration / 1000 - 1;  // The Output Compare Register A contains an 8-bit value that is continuously compared with the counter value(TCNT0).
                            // A match generates an Output Compare interrupt. one tick 
    TIMSK0 |= (1 << OCIE0A);  //  Timer/Counter0 Compare Match A interrupt is enabled. The corresponding interrupt is executed if a Compare Match in Timer/Counter0
    TCCR0B |= (1 << CS02) | (1 << CS00);    // clock input frrom 1024 prescalar
}

void setupTimer1(uint16_t x) {
    timer1Duration = x;
    TCCR1A = 0;
    TCCR1B = (1 << WGM12);
    OCR1A = timer1Duration; //(F_CPU / 1024) * timer1Duration / 100000;
    TIMSK1 |= (1 << OCIE1A);
    TCCR1B |= (1 << CS12) | (1 << CS10);
}

void setupTimer2(uint16_t x) {
    timer2Duration = x;
    TCCR2A = (1 << WGM21);
    OCR2A = timer2Duration; //(F_CPU / 1024) * timer2Duration / 100000;
    TIMSK2 |= (1 << OCIE2A);
    TCCR2B |= (1 << CS22) | (1 << CS20);
}

// Timer 0 interrupt service routine (highest priority)
ISR(TIMER0_COMPA_vect) {
    timer0Flag = 1; // Set Timer 0 flag
//    UART_sendString("In Timer 0 Interrupt\n");
    sei(); // Enable global interrupts
}

// Timer 1 interrupt service routine (medium priority)
ISR(TIMER1_COMPA_vect) {
    timer1Flag = 1; // Set Timer 1 flag
//    UART_sendString("In Timer 1 Interrupt\n");
    sei(); // Enable global interrupts
}

// Timer 2 interrupt service routine (lowest priority)
ISR(TIMER2_COMPA_vect) {
    timer2Flag = 1; // Set Timer 2 flag
//    UART_sendString("In Timer 2 Interrupt\n");
    sei(); // Enable global interrupts
}

void handleTimers() {
    // Check Timer 0 first (highest priority)
    if (timer0Flag) {
        UART_sendString("Timer 0 Interrupt\n");
        timer0Flag = 0; // Clear the flag
    }
    // Check Timer 1 next (medium priority)
    else if (timer1Flag) {
        UART_sendString("Timer 1 Interrupt\n");
        timer1Flag = 0; // Clear the flag
    }
    // Check Timer 2 last (lowest priority)
    else if (timer2Flag) {
        UART_sendString("Timer 2 Interrupt\n");
        timer2Flag = 0; // Clear the flag
    }
}

int main(void) {
    UART_init(9600);
    setupTimer0(50); // Set Timer 0 duration to 1000 ms
    setupTimer1(100);  // Set Timer 1 duration to 500 ms
    setupTimer2(200);  // Set Timer 2 duration to 250 ms

    sei(); // Enable global interrupts

    while (1) {
        handleTimers(); // Handle timer interrupts based on priority
    }
}
