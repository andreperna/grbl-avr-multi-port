#include "st7920.h"
#include <util/delay.h>
#include <stdio.h> // Necessário para snprintf
#include "system.h" // Necessário para variáveis globais

// Macros internas de controle
#define SET_SCK  LCD_PORT_A |= (1 << SCK_PIN)
#define CLR_SCK  LCD_PORT_A &= ~(1 << SCK_PIN)
#define SET_MOSI LCD_PORT_C |= (1 << MOSI_PIN)
#define CLR_MOSI LCD_PORT_C &= ~(1 << MOSI_PIN)
#define SET_CS   LCD_PORT_A |= (1 << CS_PIN)
#define CLR_CS   LCD_PORT_A &= ~(1 << CS_PIN)

// Função auxiliar (estática pois só é usada aqui dentro)
static void spi_send_byte(uint8_t byte) {
    for (int i = 0; i < 8; i++) {
        if (byte & 0x80) SET_MOSI;
        else CLR_MOSI;
        
        SET_SCK; 
        _delay_us(1); 
        CLR_SCK;
        byte <<= 1;
    }
}

void st7920_send(uint8_t type, uint8_t data) {
    SET_CS;
    spi_send_byte(type);           
    spi_send_byte(data & 0xF0);    
    spi_send_byte(data << 4);      
    CLR_CS;
    // _delay_us(50);
}

void lcd_init() {
    LCD_DDR_A |= (1 << SCK_PIN) | (1 << CS_PIN);
    LCD_DDR_C |= (1 << MOSI_PIN);
    
    _delay_ms(50);
    st7920_send(ST7920_CMD, 0x30); 
    _delay_ms(1);
    st7920_send(ST7920_CMD, 0x30); 
    _delay_ms(1);
    st7920_send(ST7920_CMD, 0x0C); 
    _delay_ms(1);
    st7920_send(ST7920_CMD, 0x01); 
    _delay_ms(10);

    // Posiciona na primeira linha
    st7920_send(ST7920_CMD, 0x80); 
    lcd_write_string("GRBL v1.1-Ender3");

    // Preenche linhas iniciais
    st7920_send(ST7920_CMD, 0x90); 
    lcd_write_string("X:---.--");
    st7920_send(ST7920_CMD, 0x88); 
    lcd_write_string("Y:---.--");
    st7920_send(ST7920_CMD, 0x98); 
    lcd_write_string("Z:---.--");
}

void lcd_write_string(char* str) {
    while (*str) {
        st7920_send(ST7920_DATA, *str++);
    }
}

void lcd_write_coords(char* eixo, float valor){
    char buffer[17]; // 16 caracteres + 1 para o terminador nulo '\0'

    // arredonda terceira casa decimal
    float valor_arredondado;
    if (valor >= 0){
        valor_arredondado = valor + 0.005f;
    } else {
        valor_arredondado = valor - 0.005f;
    }

    valor_arredondado = valor;

    int inteiro = (int)valor_arredondado;
    int decimal = (int)((valor_arredondado - inteiro) * 100);
    if (decimal < 0) {
      decimal = decimal * -1;
    }

    // char eixo[1] = "X";
    
    // snprintf(onde guardar, tamanho, "formato", variáveis...)
    snprintf(buffer, sizeof(buffer), "%s%4d.%03d", eixo, inteiro, decimal);

    // Envia a string formatada
    lcd_write_string(buffer);
}


void get_wpos(float *wpos) {
    float mpos[N_AXIS];
    float wco[N_AXIS];

    // 1. Converte a posição atual de steps para Milímetros (MPos)
    system_convert_array_steps_to_mpos(mpos, sys_position);

    // 2. Obtém o Work Coordinate Offset atual (WCO)
    // O WCO é a soma do sistema de coordenadas ativo (ex: G54) + G92 + offset da sonda
    for (uint8_t idx=0; idx<N_AXIS; idx++) {
        wco[idx] = gc_state.coord_system[idx] + gc_state.coord_offset[idx];
        
        // 3. Calcula a WPos: Work = Machine - Offset
        wpos[idx] = mpos[idx] - wco[idx];
    }
}



void lcd_update(void){
    // Contador estático que permanece na memória entre as chamadas
    static uint32_t lcd_loop_counter = 0;
    
    // Incrementa a cada vez que a função é chamada
    lcd_loop_counter++;

    // Ajuste este número (ex: 2000) conforme a velocidade que deseja
    // Se o LCD atualizar muito rápido, aumente o valor.
    if (lcd_loop_counter < 3000) { 
        return; 
    }
    
    // Se chegou aqui, resetamos o contador e executamos a atualização
    lcd_loop_counter = 0;


    float current_wpos[N_AXIS];
    get_wpos(current_wpos);

    // Linha 1
    st7920_send(ST7920_CMD, 0x90);
    lcd_write_coords("X", current_wpos[X_AXIS]);

    // Linha 2
    st7920_send(ST7920_CMD, 0x88);
    lcd_write_coords("Y", current_wpos[Y_AXIS]);

    // Linha 3
    st7920_send(ST7920_CMD, 0x98);
    lcd_write_coords("Z", current_wpos[Z_AXIS]);
}