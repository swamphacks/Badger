/*
 * ATtiny414 LED Matrix - SIMPLE DEBUG VERSION
 * 
 * 
 * First authored by Robert Conde on October 13th, 2025
 *  for SwampHack XI. Made with ❤️!
 *
 * First checkerboard on October 14th w/ Jason from Toronto.
 * 
 * Pin Assignment:
 * - PA1: ~S_R  (Row data - active low)
 * - PA2: CLK_R (Row clock)
 * - PA3: S_C   (Column data)
 * - PA4: CLK_C (Column clock)
 * - PA5: ~OE   (Output Enable - active LOW)
 * 
 */

#include <avr/io.h>
#include <util/delay.h>

// Pin definitions
#define S_R_PIN     PIN1_bm    // PA1 - Row data
#define CLK_R_PIN   PIN2_bm    // PA2 - Row clock
#define S_C_PIN     PIN3_bm    // PA3 - Column data
#define CLK_C_PIN   PIN4_bm    // PA4 - Column clock
#define OE_PIN      PIN5_bm    // PA5 - Output Enable

#define ROWS 16
#define COLS 32

// Current bit position (0-31)
uint8_t row_select = ROWS;

void init_gpio(void) {
    // Set outputs
    PORTA.DIRSET = S_R_PIN | CLK_R_PIN | S_C_PIN | CLK_C_PIN | OE_PIN;
    
    // Initialize: data HIGH, clocks LOW, OE disabled (HIGH)
    PORTA.OUTSET = S_R_PIN | S_C_PIN | OE_PIN;
    PORTA.OUTCLR = CLK_R_PIN | CLK_C_PIN;
}

// Shift out a single bit
void shift_bit(uint8_t bit, uint8_t data_pin, uint8_t clock_pin) {
    // Set data
    if (bit) {
        PORTA.OUTSET = data_pin;
    } else {
        PORTA.OUTCLR = data_pin;
    }
    
    _delay_us(1);  // Slow for debugging
    
    // Clock HIGH
    PORTA.OUTSET = clock_pin;
    _delay_us(1);
    
    // Clock LOW
    PORTA.OUTCLR = clock_pin;
    _delay_us(1);
}

void reset_registers(void) {
    // Deselect all rows
    for (uint8_t i = 0; i < ROWS + 1; i++) {
        shift_bit(1, S_R_PIN, CLK_R_PIN);
    }

    // Deselect all columns
    for (uint8_t i = 0; i < COLS + 1; i++) {
        shift_bit(0, S_C_PIN, CLK_C_PIN);
    }
}

void display_next_row(void) {
    /* Hide */
    // Disable output
    PORTA.OUTSET = OE_PIN;

    /* Select the row*/
    // Shift (perhaps in) the row select bit
    uint8_t loopback = ++row_select >= ROWS;
    shift_bit(!loopback, S_R_PIN, CLK_R_PIN);
    if (loopback) {
        shift_bit(1, S_R_PIN, CLK_R_PIN);
        row_select %= ROWS; // fix 'da counter
    }

    /* Assemble the row */
    for (uint8_t i = 0; i < COLS; i++)
        shift_bit((i + row_select) % 2, S_C_PIN, CLK_C_PIN);
    shift_bit(0, S_C_PIN, CLK_C_PIN); // extra shift since we tied clocks together!

    /* Flash Row */
    // Enable output
    PORTA.OUTCLR = OE_PIN;
    // Wait some time...
    _delay_us(25); // plenty sufficient
}

int main(void) {
    init_gpio();

    reset_registers();

    shift_bit(0, S_R_PIN, CLK_R_PIN);
    while (true) {
        display_next_row();
    }
    
    return 0;
}
