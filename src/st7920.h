#ifndef ST7920_H
#define ST7920_H

#include <avr/io.h>

// Definições de Hardware
#define LCD_PORT_A PORTA
#define LCD_DDR_A  DDRA
#define LCD_PORT_C PORTC
#define LCD_DDR_C  DDRC

#define SCK_PIN  PA1
#define CS_PIN   PA3
#define MOSI_PIN PC1

// Constantes do ST7920
#define ST7920_CMD  0xF8
#define ST7920_DATA 0xFA

// Protótipos das Funções
void lcd_init(void);
void st7920_send(uint8_t type, uint8_t data);
void lcd_write_string(char* str);
void lcd_write_coords(char* eixo, float valor);
void get_wpos(float *wpos);
void lcd_update(void);

#endif