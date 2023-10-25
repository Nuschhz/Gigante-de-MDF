#define F_CPU 16000000UL;
#include <avr/io.h>
#include <avr/interrupt.h>

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
	}else{
		LIMPA_BIT(PORTB, PB4);
	}
}
ISR(ADC_vect){
	adcValue = ADC;
	Serial.println(adcValue);
	if(adcValue >= 800){
		dano();
	}
}
void desliga(){//desliga o robô após ser atingido 3 vezes
	cli();//desabilita interrupções globais
	PORTB |= 0;//desliga as portas no PORTB
	PORTD |= 0;//desliga as portas no PORTD
	flag = 0;//desabilita a flag de funcionamento
}
void giro(){//faz o robô girar 180 graus ao ser atingido e desativa por 5s antes que possa ser utilizado
	PORTB &= ~0x06;//desliga as rodas da esquerda
	PORTD &= ~0x40;//desliga o bit PD6
	PORTD |= _BV(PD7);//Gira pra direita
	_delay_ms(1000);
	PORTD &= 0x0C;//cancela o giro
	_delay_ms(4000);
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
		desliga();
	}
}
void acelera(){
	while(!(PINC & _BV(PC2))){//curva esquerda
		PORTB |= _BV(PB2) ;
	}
	while(!(PINC & _BV(PC3))){//curva direita
		PORTD |= _BV(PD6);
	}
	while(LINHA_RETA_AC){//linha reta
		PORTB |= _BV(PB2);
		PORTD |= _BV(PD6);
	}
		PORTB &= ~0x06;
		PORTD &= ~0xC0;
}
void re(){
	while(!(PINC & _BV(PC2))){//curva esquerda
		PORTB |= _BV(PB1);
	}
	while(!(PINC & _BV(PC3))){//curva direita
		PORTD |= _BV(PD7);
	}
	while(LINHA_RETA_RE){//linha reta
		PORTB |= _BV(PB1);
		PORTD |= _BV(PD7);
	}
		PORTB &= ~0x06;
		PORTD &= ~0xC0;
}
void piscaLaser(){
	TCCR1A = 0;
	TCCR1A |= (1<<WGM12);//toggle CTC
	TCCR1B = 0;
	TCCR1B |= (1<<CS12);//prescaler 256
	TIMSK1 = 0;
	TIMSK1 |= (1<<OCIE1A);//ativa as interrupções do timer1
	OCR1A = 62500;//valor que sera comparado 
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
	OCR0B = 191;
	TCCR0A = 0;
	TCCR0A |= _BV(COM0B1) | _BV(WGM01)| _BV(WGM00);
	TCCR0B = 0;
	TCCR0B |= _BV(CS00);
}
void enableB_PWM(){
	OCR2A = 191;
	TCCR2A = 0;
	TCCR2A |= _BV(COM2A1) | _BV(WGM21)| _BV(WGM20);
	TCCR2B = 0;
	TCCR2B |= _BV(CS20);
}
void leituraADC(){
	ADCSRA |= _BV(ADSC);
	while(!(ADCSRA & _BV(ADIF)));
	ADCSRA |= _BV(ADIF);
}
int main(void){
	//	    	 D12
	DDRB |= _BV(PB4);//define o led atirador como saída
	//		 D9		  D10		  D11
	DDRB |= _BV(PB1) | _BV(PB2) | _BV(PB3);//define os bits Pb3~EnA, Pb2-In1, Pb1-In2 como saída
	//		 D5		  D6		  D7
	DDRD |= _BV(PD5) | _BV(PD6) | _BV(PD7);//define os bits Pd5~EnB, Pd6-In3, Pd7-In4 como saída
	//		 D2		  D3		  D4
	DDRD |= _BV(PD2) | _BV(PD3) | _BV(PD4);//define os Leds de vida como saída
	//					    D16, D17, D18, D19
	DDRC |= 0x00;//define os bits Pc0, Pc2-esquerda, Pc3-direita, Pc4-acelera, Pc5-ré como entrada
	PORTC |= _BV(PC2) | _BV(PC3) | _BV(PC4) | _BV(PC5);//ativa os resistores de pull-up do PORTC
	flag = 1;
	
	piscaLaser();
	inicializaVidas();
	inicializaAnalog();
	enableA_PWM();
	enableB_PWM();
	sei();
	
	Serial.begin(9600);
	
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