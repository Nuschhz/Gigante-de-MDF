/**
 * @file giganteMDF.ino
 * @brief Gigante-De-MDF Main
 * @author nusch05
 * @details https://github.com/Nuschhz/Gigante-de-MDF
 * @date 11-2023
 */
#define F_CPU 16000000UL;
#include <avr/io.h> /**< Exemplo de explicação de uma struct #a. */
#include <avr/interrupt.h> /**< Exemplo de explicação de uma struct #a. */

#define _BV(n) (1<<n)
#define ALTERNA_BIT(PORT,BIT) PORT ^= (1<<BIT)
#define LIMPA_BIT(PORT, BIT) PORT &= ~(1<<BIT)

#define LINHA_RETA_AC_1 (!(PINC & _BV(PC4)) && (PINC & _BV(PC2)) && (PINC & _BV(PC3)))
#define LINHA_RETA_AC_2 (!(PINC & _BV(PC4)) && !(PINC & _BV(PC2)) && !(PINC & _BV(PC3)))
#define LINHA_RETA_AC LINHA_RETA_AC_1 || LINHA_RETA_AC_2

#define LINHA_RETA_RE_1 (!(PINC & _BV(PC5)) && (PINC & _BV(PC2)) && (PINC & _BV(PC3)))
#define LINHA_RETA_RE_2 (!(PINC & _BV(PC5)) && !(PINC & _BV(PC2)) && !(PINC & _BV(PC3)))
#define LINHA_RETA_RE LINHA_RETA_RE_1 || LINHA_RETA_RE_2

uint16_t adcValue;
volatile uint8_t danoRecebido;
volatile uint8_t vidasTotais;
char flag;

ISR(TIMER1_COMPA_vect){//alterna laser de 1 em 1 segundo
	if(flag == 1){
		ALTERNA_BIT(PORTB, PB4);
		TCNT1 = 0;
	}else{
		LIMPA_BIT(PORTB, PB4);
	}
}
ISR(ADC_vect){
	adcValue = ADC;
}
void desliga(){//desliga o robô após ser atingido 3 vezes
	cli();//desabilita interrupções globais
	adcValue = 0;
	PORTB |= 0;//desliga as portas no PORTB
	PORTD |= 0;//desliga as portas no PORTD
	flag = 0;//desabilita a flag de funcionamento
}
void giro(){//faz o robô girar 180 graus ao ser atingido e desativa por 5s antes que possa ser utilizado
	PORTB &= ~0x06;//desliga as rodas da esquerda
	PORTD &= ~0x40;//desliga o bit PD6
	PORTD |= _BV(PD7);//Gira pra direita
	TCCR1A = 0;
	TCNT1 = 0;
	while(TCNT1 <= 15625);
	PORTD &= 0x0C;//cancela o giro
	while(TCNT1 <= 62500);
	piscaLaser();
	sei();//ativa as interrupções globais
}
void dano(){//Apaga um ods leds de vida sempre que for acertado no LDR
	flag = 0;//desabilita a flag de ativação do circuito
	danoRecebido++;//incrementa a variável de dano recebido
	char vidasAtuais = vidasTotais - danoRecebido;//cria a variavel local para vidas atuais
	if(vidasAtuais >= 2){
		ALTERNA_BIT(PORTD,PD4);
		LIMPA_BIT(PORTB, PB4);
		giro();
		flag = 1;
	}else if(vidasAtuais >= 1){
		ALTERNA_BIT(PORTD,PD3);
		LIMPA_BIT(PORTB, PB4);
		giro();
		flag = 1;
	}else if(vidasAtuais >= 0){
		ALTERNA_BIT(PORTD,PD2);
		LIMPA_BIT(PORTB, PB4);
    giro();
    desliga();
	}
}
void acelera(){
	while(!(PINC & _BV(PC2))){//curva esquerda
		leituraADC();
		PORTB |= _BV(PB1) ;
	}
	while(!(PINC & _BV(PC3))){//curva direita
		leituraADC();
		PORTD |= _BV(PD6);
	}
	while(LINHA_RETA_AC){//linha reta
		leituraADC();
		PORTB |= _BV(PB1);
		PORTD |= _BV(PD6);
	}
  LIMPA_BIT(PORTB, PB1);
  LIMPA_BIT(PORTB, PB2);
  LIMPA_BIT(PORTD, PD6);
  LIMPA_BIT(PORTD, PD7);
}
void re(){
	while(!(PINC & _BV(PC2))){//curva esquerda
		leituraADC();
		PORTB |= _BV(PB2);
	}
	while(!(PINC & _BV(PC3))){//curva direita
		leituraADC();
		PORTD |= _BV(PD7);
	}
	while(LINHA_RETA_RE){//linha reta
		leituraADC();
		PORTB |= _BV(PB2);
		PORTD |= _BV(PD7);
	}
  LIMPA_BIT(PORTB, PB1);
  LIMPA_BIT(PORTB, PB2);
  LIMPA_BIT(PORTD, PD6);
  LIMPA_BIT(PORTD, PD7);
}
void piscaLaser(){
	TCCR1A = 0;
	TCCR1A |= _BV(WGM12);//CTC
	TCCR1B = 0;
	TCCR1B |= _BV(CS10) | _BV(CS12);//prescaler 1024
	TIMSK1 = 0;
	TIMSK1 |= _BV(OCIE1A);//ativa as interrupções do timer1
	OCR1A = 15625;//valor que sera comparado 
	TCNT1 = 0;
}

void inicializaAnalog(){
	ADMUX = 0;
	ADMUX |= _BV(REFS0);
	ADMUX |= (0<<MUX0) | (0<<MUX1) | (0<<MUX2) | (0<<MUX3);
	ADCSRA = 0;
	ADCSRA |= _BV(ADEN) | _BV(ADPS2) | _BV(ADPS0) | _BV(ADIE);
}
void inicializaVidas(){
	vidasTotais = 3;
	danoRecebido = 0;
	PORTD |= _BV(PD2) | _BV(PD3) | _BV(PD4);
}

void enableA_PWM(){
	OCR0B = 204;
	TCCR0A = 0;
	TCCR0A |= _BV(COM0B1) | _BV(WGM01)| _BV(WGM00);
	TCCR0B = 0;
	TCCR0B |= _BV(CS00);
}
void enableB_PWM(){
	OCR2A = 204;
	TCCR2A = 0;
	TCCR2A |= _BV(COM2A1) | _BV(WGM21)| _BV(WGM20);
	TCCR2B = 0;
	TCCR2B |= _BV(CS20);
}
void leituraADC(){
	ADCSRA |= _BV(ADSC);
	while((ADCSRA & _BV(ADIF)));
	ADCSRA |= _BV(ADIF);
	if(adcValue <= 500){
		dano();
	}
}
/**
 *@brief main loop
*/
int main(void){
	DDRB |= _BV(PB4);//define o laser atirador como saída
	DDRB |= _BV(PB1) | _BV(PB2) | _BV(PB3);//define os bits Pb3~EnA, Pb2-In1, Pb1-In2 como saída
	DDRD |= _BV(PD5) | _BV(PD6) | _BV(PD7);//define os bits Pd5~EnB, Pd6-In3, Pd7-In4 como saída
	DDRD |= _BV(PD2) | _BV(PD3) | _BV(PD4);//define os Leds de vida como saída
	DDRC = 0x00;//define os bits Pc0, Pc2-esquerda, Pc3-direita, Pc4-acelera, Pc5-ré como entrada
	PORTC |= _BV(PC2) | _BV(PC3)| _BV(PC4) | _BV(PC5);//ativa os resistores de pull-up do PORTC
	flag = 1;

	piscaLaser();
	inicializaVidas();
	inicializaAnalog();
	enableA_PWM();
	enableB_PWM();

	sei();
	
	while(1){
		while(flag == 1){
			leituraADC();
			if(!(PINC & _BV(PC4))){
				acelera();
			}
			else if(!(PINC & _BV(PC5))){
				re();
			}
		}
	}
	
}