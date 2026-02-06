/*
 * esteira-project-microchipstudio.c
 *
 * Created: 04/02/2026 17:49:10
 * Author : Vinicius Duarte
 */ 

#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <math.h>
#include <util/delay.h>
#include <stdbool.h>

// .gfedcba (catodo comum)
const uint8_t seg[13] = {
	0b00111111, // 0
	0b00000110, // 1
	0b01011011, // 2
	0b01001111, // 3
	0b01100110, // 4
	0b01101101, // 5
	0b01111101, // 6
	0b00000111, // 7
	0b01111111, // 8
	0b01101111,  // 9
	0b01110111, // A (10)
	0b01111100, // b (11)
	0b00111001  // C (12)
};

volatile uint8_t digito = 0;
volatile uint8_t valor[4] = {1, 2, 3, 4};
uint16_t countG, countM, countP, countTotal;
uint8_t categoria;
int numero;


bool caixaPresente = false;
float distancia, ultimaDistancia;

void hardware_init() {
	// ===== HC-SR04 =====
	DDRB |= (1 << PB0);   // Trigger (PB0) saída
	DDRB &= ~(1 << PB1);  // Echo (PB1) entrada

	// ===== LEDs / Saídas P, M, G =====
	DDRB |= (1 << PB5);
	DDRB |= (1 << PB4);
	DDRB |= (1 << PB3);
	
	// ===== Timer1 - Medição de tempo (HC-SR04) =====
	TCCR1A = 0;
	TCCR1B = 0;  // Timer parado inicialmente
	
	TCCR0A = 0;
	TCCR0B = (1 << CS01);   // prescaler 8
	TIMSK0 = (1 << TOIE0);  // interrupção overflow
	
	DDRD = 0xFF; // segmentos como saída
	DDRC |= (1 << PC1) | (1 << PC2) | (1 << PC3) | (1 << PC4); // dígitos

	PORTD = 0x00;
	PORTC = 0x00; // todos transistores desligados

	sei(); // habilita interrupções
}

void classificarCaixa(char categoria){
	if (categoria == 'G'){
		countG++;
		PORTB &= ~(1 << PB5);
		PORTB &= ~(1 << PB4);
		PORTB |= (1 << PB3);
	}
	else if (categoria == 'M'){
		countM++;
		PORTB &= ~(1 << PB5);
		PORTB |= (1 << PB4);
		PORTB &= ~(1 << PB3);
	}
	else if (categoria == 'P'){
		countP++;
		PORTB |= (1 << PB5);
		PORTB &= ~(1 << PB4);
		PORTB &= ~(1 << PB3);
	}
}

ISR(TIMER0_OVF_vect) {

	// 1?? Desliga todos os dígitos (transistores OFF)
	PORTC &= ~((1 << PC1) | (1 << PC2) | (1 << PC3) | (1 << PC4));

	// 2?? Atualiza segmentos
	PORTD = seg[ valor[digito] ];

	// 3?? Liga apenas o dígito atual (transistor ON)
	switch (digito) {
		case 0: PORTC |= (1 << PC1); break;
		case 1: PORTC |= (1 << PC2); break;
		case 2: PORTC |= (1 << PC3); break;
		case 3: PORTC |= (1 << PC4); break;
	}

	// 4?? Próximo dígito
	digito++;
	if (digito > 3) digito = 0;
}

float readDistance() {
    uint16_t tempo_echo = 0;

    // 1. Gera o pulso de Trigger (10 microsegundos)
    PORTB &= ~(1 << PB0); 
    _delay_us(2);
    PORTB |= (1 << PB0);
    _delay_us(10);
    PORTB &= ~(1 << PB0);

    // 2. Espera o início do pulso Echo (ficar em nível alto)
    // Adicionar um timeout aqui seria ideal para evitar travamentos
    while (!(PINB & (1 << PB1)));

    // 3. Inicia o Timer1 (Prescaler de 8) 
    // Com 16MHz / 8 = 2MHz, cada "tick" do timer vale 0.5us
    TCNT1 = 0;           // Zera o contador
    TCCR1B |= (1 << CS11); 

    // 4. Espera o fim do pulso Echo (voltar para nível baixo)
    while (PINB & (1 << PB1));

    // 5. Para o Timer1
    tempo_echo = TCNT1;
    TCCR1B = 0; 

    /* * Cálculo da Distância:
     * Distância = (Tempo * Velocidade do Som) / 2
     * Com prescaler de 8, tempo em segundos = $TCNT1 \cdot \frac{8}{F\_CPU}$
     * $Distancia(cm) = \frac{TCNT1 \cdot 0.5 \cdot 0.0343}{2}$
     */
    return (float)tempo_echo * 0.008575;
}

int main(void) {
	hardware_init();
	
	while (1) {
		
		distancia = readDistance();

		if (distancia < 450.0 && !caixaPresente) {
			caixaPresente = true;

			if (distancia < 200.0) {
				classificarCaixa('G');
				categoria = 10;
			}
			else if (distancia < 300.0) {
				classificarCaixa('M');
				categoria = 11;
			}
			else if (distancia < 450.0) {
				classificarCaixa('P');
				categoria = 12;
			}
		}

		if (distancia > 450.0) {
			caixaPresente = false;
		}

		// ===== Atualiza display SEM travar =====

		valor[0] = categoria;
		
		if(categoria == 10) numero = countG;
		if(categoria == 11) numero = countM;
		if(categoria == 12) numero = countP;

		valor[1] = (numero / 100) % 10;
		valor[2] = (numero / 10) % 10;
		valor[3] = numero % 10;
	}
}
