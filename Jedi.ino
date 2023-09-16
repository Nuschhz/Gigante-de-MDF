#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>

#define ldr PC5

#define laserAtivo PD0
#define ledVida PD1

#define viraEsquerda PD2
#define viraDireita PD3
#define botaoLaser PD4
#define acelera PD5
#define re PD6

#define in1 PB0
#define enableA PB1
#define enableB PB2
#define in2 PB3
#define in3 PB4
#define in4 PB5

void timer_init(){
	//timer do laser funcionando de 1s em 1s
}

void desliga(){
	//se o sensor LDR for acertado, desliga o circuito por 1s
}

void direcao(/*re,acelera*/){
	if(1/*re == 1*/){}
	else{}/*acelera == 1*/
}

int main(void){
	//C5 é o input analógico do LDR
	DDRC |= (0<<ldr);
	//Outputs dos leds e Inputs dos controladores
	DDRD |= (1<<laserAtivo)|(1<<ledVida)|(0<<viraEsquerda)|(0<<viraDireita)|(0<<botaoLaser)|(0<<acelera)|(0<<re);
	//Controle dos motores
	DDRB |= (1<<enableA)|(1<<enableB)|(1<<in1)|(1<<in2)|(1<<in3)|(1<<in4);
	
	sei();
	
	while(1){
		
		if(1/*botao viraDireita configura in1 2 3 4 e enableA B...*/){
			direcao(/*re,acelera*/);
		}else if(2/*botao viraEsquerda configura in1 2 3 4 e enableA B...*/){
			direcao(/*re,acelera*/);
		}else{
			direcao(/*re,acelera*/);
		}
		
		if(1/*recebe botao laser*/);
			timer_init();
		if(1/*recebe LDR*/);
			desliga();
	}
	
}