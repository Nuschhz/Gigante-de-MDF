#include <avr/io.h>
#include <avr/interrupt.h>

#define _BV(n) (1<<n)
#define ALTERNA_BIT(PORT,BIT) PORT ^= (1<<BIT)

volatile uint8_t tot_overflow0;

ISR(TIMER0_OVF_vect){//alterna laser de 1 em 1 segundo
	tot_overflow0++;
	if(tot_overflow0 >= 61){
		ALTERNA_BIT(PORTB, PB4);
		tot_overflow0 = 0;
	}
}

void acerto(){
//provavelmente vai virar uma interrupção
//ajustar ponteH e ativar os enables
}

void acelera(){
//while pro botao da direita e dps da esquerda
}
void re(){
//while pro botao da direita e dps da esquerda
}

void piscaLaser(){
	TCCR0A |= 0;
	TCCR0B |= 0;
	TCCR0B |= _BV(CS00) | _BV(CS02);//prescaler 1024
	TIMSK0 |= 0;
	TIMSK0 |= _BV(TOIE0);//habilidad interrupção em overflow
	TCNT0 = 0;//inicializa o contador
	tot_overflow0 = 0;//inicializa variável de overflow
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
	//		 D18		  D19
	DDRC |= _BV(PC4) | _BV(PC5);//define os bits Pc4, Pc5 como entrada
	piscaLaser();
	
	sei();
	
	while(1){
		while(PINC4){
		//acelera
		}
		while(PINC5){
		//da ré
		}
	}
}