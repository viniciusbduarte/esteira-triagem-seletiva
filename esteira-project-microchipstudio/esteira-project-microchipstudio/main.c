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

#define TIMEOUT_US 30000UL   // ~30ms ? 5m (HC-SR04)

// ===== Definições de pinos =====
#define TRIG_PIN   PB0
#define ECHO_PIN   PB1

#define LED_G_PIN  PB5
#define LED_M_PIN  PB4
#define LED_P_PIN  PB2

#define PWM_PIN PB3   // OC2B

#define DIG_PINS   ((1 << PC1) | (1 << PC2) | (1 << PC3) | (1 << PC4))
#define DIG_MASK   ((1 << PC1) | (1 << PC2) | (1 << PC3) | (1 << PC4))

// ===== Botões =====
#define BTN_PAUSE_PIN   PC0
#define BTN_RESUME_PIN  PC5
#define BTN_PWM_UP_PIN  PB6
#define BTN_PWM_DOWN_PIN PC6

volatile bool sistemaAtivo = true;
volatile uint8_t dutyPWM = 100;


// ===== Definições dos segmentos para formar caracteres =====
const uint8_t seg[13] = {
	// 0..gfedcba (catodo comum)
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

void hardware_init(void);
void classificarCaixa(char categoria);
ISR(TIMER0_OVF_vect);
ISR(PCINT1_vect);
ISR(PCINT0_vect);
float readDistance(void);
void setPWM(uint8_t duty);

int main(void) {
	hardware_init();
	setPWM(100);
	
	while (1) {

		// ===== Controle geral do sistema (PAUSE / RESUME) =====
		if (!sistemaAtivo) {
			setPWM(0);       // para a esteira
			continue;        // mantém display e interrupções funcionando
		}
		else {
			setPWM(dutyPWM); // aplica duty atual
		}

		// ===== Leitura do sensor ultrassônico =====
		distancia = readDistance();

		// ===== Detecção de caixa =====
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

		// ===== Caixa saiu da área do sensor =====
		if (distancia > 450.0) {
			caixaPresente = false;
		}

		// ===== Atualização do display =====
		valor[0] = categoria;

		if (categoria == 10) numero = countG;
		if (categoria == 11) numero = countM;
		if (categoria == 12) numero = countP;

		valor[1] = (numero / 100) % 10;
		valor[2] = (numero / 10) % 10;
		valor[3] = numero % 10;
	}

}

void hardware_init(void) {
	cli(); // desabilita interrupções durante configuração

	// ===== HC-SR04 =====
	DDRB |= (1 << TRIG_PIN);      // Trigger como saída
	DDRB &= ~(1 << ECHO_PIN);     // Echo como entrada
	PORTB &= ~(1 << TRIG_PIN);    // Trigger em nível baixo inicial

	// ===== LEDs / Saídas de classificação =====
	DDRB |= (1 << LED_G_PIN) | (1 << LED_M_PIN) | (1 << LED_P_PIN);
	PORTB &= ~((1 << LED_G_PIN) | (1 << LED_M_PIN) | (1 << LED_P_PIN)); // LEDs desligados

	// ===== Timer1 - Medição HC-SR04 =====
	TCCR1A = 0;
	TCCR1B = 0;        // Timer parado inicialmente
	TCNT1  = 0;        // Zera contador

	// ===== Timer0 - Multiplexação display 7 segmentos =====
	TCCR0A = 0;
	TCCR0B = (1 << CS01);   // prescaler = 8
	TCNT0  = 0;
	TIMSK0 = (1 << TOIE0);  // habilita interrupção overflow

	// ===== Timer2 - PWM para controle da esteira (motor) =====
	DDRB |= (1 << PWM_PIN); // PB3 (OC2A) como saída
	TCCR2A = (1 << WGM20) | (1 << WGM21) | (1 << COM2A1); // OC2A
	TCCR2B = (1 << CS21);  // prescaler = 8
	OCR2A = 0; // PWM inicial = 0%

	
	// ===== Display 7 segmentos =====
	DDRD = 0xFF;           // segmentos como saída (a-g + dp)
	DDRC |= DIG_PINS;      // dígitos como saída (transistores)

	PORTD = 0x00;          // segmentos desligados
	PORTC &= ~DIG_PINS;    // todos dígitos desligados

	// ===== Botões (entrada com pull-up) =====
	DDRC &= ~((1 << BTN_PAUSE_PIN) | (1 << BTN_RESUME_PIN) | (1 << BTN_PWM_DOWN_PIN));
	PORTC |= (1 << BTN_PAUSE_PIN) | (1 << BTN_RESUME_PIN) | (1 << BTN_PWM_DOWN_PIN);

	DDRB &= ~(1 << BTN_PWM_UP_PIN);
	PORTB |= (1 << BTN_PWM_UP_PIN);

	// ===== Interrupções por mudança de pino (PCINT) =====
	PCICR |= (1 << PCIE0) | (1 << PCIE1);  // PORTB e PORTC

	PCMSK0 |= (1 << PCINT6);              // PB6 (PWM +)
	PCMSK1 |= (1 << PCINT8) | (1 << PCINT13) | (1 << PCINT14);
	// PC0 (pause), PC5 (resume), PC6 (PWM -)


	sei(); // habilita interrupções
}

void setPWM(uint8_t duty) {
	if (duty > 100) duty = 100; // limita 0..100%
	OCR2A = (uint8_t)((duty * 255) / 100);
}

void classificarCaixa(char c) {
	// apaga todos LEDs
	PORTB &= ~((1 << LED_G_PIN) | (1 << LED_M_PIN) | (1 << LED_P_PIN));

	switch (c) {
		case 'G':
		countG++;
		PORTB |= (1 << LED_P_PIN);
		break;

		case 'M':
		countM++;
		PORTB |= (1 << LED_M_PIN);
		break;

		case 'P':
		countP++;
		PORTB |= (1 << LED_G_PIN);
		break;
	}
}


ISR(TIMER0_OVF_vect) {

	// 1) Desliga todos os dígitos
	PORTC &= ~DIG_MASK;

	// 2) Atualiza segmentos (lookup table)
	PORTD = seg[ valor[digito] ];

	// 3) Liga o dígito atual 
	PORTC |= (1 << (PC1 + digito));

	// 4) Próximo dígito (cíclico 0..3)
	digito = (digito + 1) & 0x03;
}

float readDistance(void) {
	uint16_t ticks = 0;
	uint32_t timeout = 0;

	// === 1) Trigger pulse (10 µs) ===
	PORTB &= ~(1 << TRIG_PIN);
	_delay_us(2);
	PORTB |= (1 << TRIG_PIN);
	_delay_us(10);
	PORTB &= ~(1 << TRIG_PIN);

	// === 2) Wait Echo HIGH (with timeout) ===
	timeout = 0;
	while (!(PINB & (1 << ECHO_PIN))) {
		if (timeout++ > TIMEOUT_US) return -1.0f; // sem resposta
		_delay_us(1);
	}

	// === 3) Start Timer1 (prescaler = 8 ? 0.5 µs per tick) ===
	TCNT1 = 0;
	TCCR1A = 0;
	TCCR1B = (1 << CS11);  // prescaler 8

	// === 4) Wait Echo LOW (with timeout) ===
	timeout = 0;
	while (PINB & (1 << ECHO_PIN)) {
		if (timeout++ > TIMEOUT_US) {
			TCCR1B = 0;
			return -1.0f; // echo travado em HIGH
		}
		_delay_us(1);
	}

	// === 5) Stop timer and read ticks ===
	ticks = TCNT1;
	TCCR1B = 0;

	// === 6) Convert ticks ? distance (cm) ===
	// tick = 0.5 µs (F_CPU=16MHz, prescaler=8)
	// distance = (time_us * speed_of_sound) / 2
	// speed_of_sound ? 0.0343 cm/µs
	float time_us = ticks * 0.5f;
	return (time_us * 0.0343f) / 2.0f;
}


ISR(PCINT1_vect) {

	// ===== PAUSE =====
	if (!(PINC & (1 << BTN_PAUSE_PIN))) {
		sistemaAtivo = false;
	}

	// ===== RESUME =====
	if (!(PINC & (1 << BTN_RESUME_PIN))) {
		sistemaAtivo = true;
	}

	// ===== PWM - =====
	if (!(PINC & (1 << BTN_PWM_DOWN_PIN))) {
		if (dutyPWM >= 5) dutyPWM -= 5;
		else dutyPWM = 0;
		setPWM(dutyPWM);
	}
}

// ===== Botão PWM + (PORTB) =====
ISR(PCINT0_vect) {
	if (!(PINB & (1 << BTN_PWM_UP_PIN))) {
		if (dutyPWM < 100) dutyPWM += 5;
		setPWM(dutyPWM);
	}
}

