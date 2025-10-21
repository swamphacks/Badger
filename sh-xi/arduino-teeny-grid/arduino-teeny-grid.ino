/*
 * ATtiny414 LED Matrix - Graphic display.
 * 
 * Revision History:
 *  Oct-13  First authored by Robert Conde for SwampHack XI.
 *           Made with ❤️!
 *  Oct-14  First checkerboard on October 14th w/ Jason from
 *           Toronto.
 *  Oct-21  First graphic on October 21st (thinking of Jason
 *           from Toronto).
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
#include <avr/pgmspace.h>

// Pin definitions
#define S_R_PIN     PIN1_bm    // PA1 - Row data
#define CLK_R_PIN   PIN2_bm    // PA2 - Row clock
#define S_C_PIN     PIN3_bm    // PA3 - Column data
#define CLK_C_PIN   PIN4_bm    // PA4 - Column clock
#define OE_PIN      PIN5_bm    // PA5 - Output Enable

#define ROWS 16
#define COLS 32

// Graphic pattern
// Each row is 4 bytes (32 bits), stored in program memory
const uint8_t graphic[ROWS][4] PROGMEM = {
    {0b00000000, 0b00000000, 0b00000000, 0b00000000}, // Row 00
    {0b01000001, 0b00000000, 0b00001001, 0b00111110}, // Row 01
    {0b01000001, 0b00000000, 0b00001001, 0b00111110}, // Row 02
    {0b01000001, 0b00001001, 0b00001001, 0b00111110}, // Row 03
    {0b01000001, 0b00000000, 0b00001001, 0b00011100}, // Row 04
    {0b01000001, 0b00000000, 0b00001001, 0b00011100}, // Row 05
    {0b01000001, 0b00001111, 0b00001001, 0b00011100}, // Row 06
    {0b01111111, 0b00010000, 0b10001001, 0b00001000}, // Row 07
    {0b01000001, 0b00100000, 0b01001001, 0b00001000}, // Row 08
    {0b01000001, 0b00100000, 0b01001001, 0b00000000}, // Row 09
    {0b01000001, 0b00111111, 0b10001001, 0b00011100}, // Row 10
    {0b01000001, 0b00100000, 0b00001001, 0b00101010}, // Row 11
    {0b01000001, 0b00100000, 0b00001001, 0b00110110}, // Row 12
    {0b01000001, 0b00010000, 0b00001001, 0b00101010}, // Row 13
    {0b01000001, 0b00001111, 0b10000100, 0b10011100}, // Row 14
    {0b00000000, 0b00000000, 0b00000000, 0b00000000}, // Row 15
};

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
    
    _delay_us(1);
    
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

// Get pixel value from heart pattern
// Current byte and bit position for shifting
uint8_t current_byte;
uint8_t current_bit_mask;

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
    // Load last byte and initialize bit mask (shift in reverse order)
    uint8_t byte_idx = 3;
    current_byte = pgm_read_byte(&graphic[row_select][byte_idx]);
    current_bit_mask = 0x01; // Start with LSB

    for (uint8_t i = 0; i < COLS; i++) {
        // Shift out current bit
        shift_bit(current_byte & current_bit_mask, S_C_PIN, CLK_C_PIN);
        
        // Move to next bit
        current_bit_mask <<= 1;
        
        // If we've exhausted this byte, load the previous one
        if (current_bit_mask == 0) {
            if (byte_idx > 0) {
                byte_idx--;
                current_byte = pgm_read_byte(&graphic[row_select][byte_idx]);
            }
            current_bit_mask = 0x01; // Reset to LSB
        }
    }
    
    shift_bit(0, S_C_PIN, CLK_C_PIN); // extra shift since we tied clocks together!

    /* Flash Row */
    // Enable output
    PORTA.OUTCLR = OE_PIN;
    // Wait some time...
    _delay_us(25); // Plenty of time
}

int main(void) {
    init_gpio();

    reset_registers();

    shift_bit(0, S_R_PIN, CLK_R_PIN); // TODO: consider if needed
    while (true) {
        display_next_row();
    }
    
    return 0;
}
