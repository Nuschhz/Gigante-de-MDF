/**
 * @file giganteMDF.ino
 * @brief Gigante-De-MDF Main
 * @author Gustavo Lucio | Gustavo Nusch | Lucas Faria | Rafaela Pinheiro
 * @see https://github.com/Nuschhz/Gigante-de-MDF
 * @date 23-11-2023
 */
#define F_CPU 16000000UL;/**< Definição da CPU para 16MHz.*/
#include <avr/io.h>/**< Inclusão da biblioteca avr/io.*/
#include <avr/interrupt.h>/**< Inclusão da biblioteca avr/interrupt.*/

#define _BV(n) (1<<n)/**< Macro que insere o valor em um determinado BIT.*/
#define ALTERNA_BIT(PORT,BIT) PORT ^= (1<<BIT)/**< Macro que alterna o valor em um determinado BIT entre 0 e 1.*/
#define LIMPA_BIT(PORT, BIT) PORT &= ~(1<<BIT)/**< Macro que limpa o determinado BIT.*/

#define LINHA_RETA_AC_1 (!(PINC & _BV(PC4)) && (PINC & _BV(PC2)) && (PINC & _BV(PC3)))
#define LINHA_RETA_AC_2 (!(PINC & _BV(PC4)) && !(PINC & _BV(PC2)) && !(PINC & _BV(PC3)))
#define LINHA_RETA_AC LINHA_RETA_AC_1 || LINHA_RETA_AC_2/**< Definição da condição para linha reta durante a aceleração caso dois botões, ou nenhum, estejam pressionados.*/

#define LINHA_RETA_RE_1 (!(PINC & _BV(PC5)) && (PINC & _BV(PC2)) && (PINC & _BV(PC3)))
#define LINHA_RETA_RE_2 (!(PINC & _BV(PC5)) && !(PINC & _BV(PC2)) && !(PINC & _BV(PC3)))
#define LINHA_RETA_RE LINHA_RETA_RE_1 || LINHA_RETA_RE_2/**< Definição da condição para linha reta durante a ré caso dois botões, ou nenhum, estejam pressionados.*/

uint16_t adcValue = 500;/**< Variável de comparação ADC inicializada em 500 para evitar que um dano seja tomado na primeira chamada.*/
volatile uint8_t danoRecebido;/**< Variável correspondente ao dano recebido pelo Gigante de MDF.*/
volatile uint8_t vidasTotais;/**< Váriavel responsável pelas vidas máximas do Gigante de MDF.*/
char flag;/**< Variável que controla se o Gigante está funcionando ou não.*/

/**
 *@brief Interrupção para cada comparação do Timer1.
 *
 *Esta interrupção é chamada sempre que o registrador <b> TCNT1 </b> atinge o registrador de topo <b> OCR1A </b>.
 *Caso a <b> flag </b> de funcionamento esteja ativa, o laser irá atirar de 1 em 1 segundo.
*/
ISR(TIMER1_COMPA_vect){
	if(flag == 1){
		ALTERNA_BIT(PORTB, PB4);
		TCNT1 = 0;
	}else{
		LIMPA_BIT(PORTB, PB4);
	}
}
/**
 *@brief Interrupção para cada conversão ADC.
 *
 *Esta interrupção é chamada sempre ocorre uma conversão analógica para digital.
 *Ela armazena o valor do registro <b> ADC </b> na variável <b> adcValue </b>.
*/
ISR(ADC_vect){
	adcValue = ADC;
}
/**
 *@brief Função que desativa as operações do Gigante de MDF.
 *
 *Esta função desativa o robô após ele ter sido atingido 3 vezes.
 *Desativando todas as variáveis de funcionamento.
 *@code {.C}
 * cli();//desabilita as interrupções globais.
 * adcValue = 0;//zera o valor da variável de conversão analógica.
 * PORTB |= 0;//zera todos os bits do PORTB.
 * PORTD |= 0;//zera todos os bits do PORTD.
 * flag = 0;//zera o valor da variável de funcionamento.
 *@endcode
*/
void desliga(){
	cli();
	adcValue = 0;
	PORTB |= 0;
	PORTD |= 0;
	flag = 0;
}
/**
 *@brief Função que realiza o giro de 180° do Gigante de MDF.
 *
 *Esta Função ativa os BITS para sentido anti-horário dos motores.
 *Também espera o tempo de 5 segundos alterando o modo de funcionamento do <b> Timer1 </b>.
 * @code {.C}
 * TCCR1A = 0;//define o Timer1 para modo de operação normal.
 * TCNT1 = 0;//zera o registro contador.
 * while(TCNT1 <= 15625);//delay de 1 segundo
 * //...
 * TCNT1 = 0;//zera o registro contador.
 * while(TCNT1 <= 62500);//delay de 4 segundos
 * //...
 * @endcode
*/
void giro(){
	PORTB &= ~0x02
	PORTD &= ~0x40;
  PORTB |= _BV(PB2);
	PORTD |= _BV(PD7);
	TCCR1A = 0;
	TCNT1 = 0;
	while(TCNT1 <= 15625);
  LIMPA_BIT(PORTB, PB2);
  LIMPA_BIT(PORTD, PD7);
  TCNT1 = 0;
	while(TCNT1 <= 62500);
	piscaLaser();
	sei();
}
/**
 *@brief Função que administra o dano recebido pelo Gigante de MDF.
 *
 *
 *
 * @code {.C}
 * @endcode
*/
void dano(){
	flag = 0;
	danoRecebido++;
	char vidasAtuais = vidasTotais - danoRecebido;
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
/**
 *@brief Função que faz o Gigante de MDF acelerar.
 *
 *
 *
 * @code {.C}
 * 
 * @endcode
*/
void acelera(){
	while(!(PINC & _BV(PC2))){//curva esquerda
		leituraADC();
		PORTD |= _BV(PD6);
	}
	while(!(PINC & _BV(PC3))){//curva direita
		leituraADC();
    PORTB |= _BV(PB2);
	}
	while(LINHA_RETA_AC){//linha reta
		leituraADC();
		PORTB |= _BV(PB2);
		PORTD |= _BV(PD6);
	}
  LIMPA_BIT(PORTB, PB1);
  LIMPA_BIT(PORTB, PB2);
  LIMPA_BIT(PORTD, PD6);
  LIMPA_BIT(PORTD, PD7);
}
/**
 *@brief Função que faz o Gigante de MDF dar ré.
 *
 *
 * @code {.C}
 * 
 * @endcode
*/
void re(){
	while(!(PINC & _BV(PC2))){//curva esquerda
		leituraADC();
    PORTD |= _BV(PD7);
	}
	while(!(PINC & _BV(PC3))){//curva direita
		leituraADC();
    PORTB |= _BV(PB1);
	}
	while(LINHA_RETA_RE){//linha reta
		leituraADC();
		PORTB |= _BV(PB1);
		PORTD |= _BV(PD7);
	}
  LIMPA_BIT(PORTB, PB1);
  LIMPA_BIT(PORTB, PB2);
  LIMPA_BIT(PORTD, PD6);
  LIMPA_BIT(PORTD, PD7);
}
/**
 *@brief Função que inicia o laser do Gigante de MDF.
 *
 *
 *
 * @code {.C}
 * 
 * @endcode
*/
void piscaLaser(){
	TCCR1A = 0;
	TCCR1A |= _BV(WGM12);
	TCCR1B = 0;
	TCCR1B |= _BV(CS10) | _BV(CS12);
	TIMSK1 = 0;
	TIMSK1 |= _BV(OCIE1A);
	OCR1A = 15625;
	TCNT1 = 0;
}
/**
 *@brief Função que Gigante de MDF.
 *
 *
 *
 * @code {.C}
 * 
 * @endcode
*/
void inicializaAnalog(){
	ADMUX = 0;
	ADMUX |= _BV(REFS0);
	ADMUX |= (0<<MUX0) | (0<<MUX1) | (0<<MUX2) | (0<<MUX3);
	ADCSRA = 0;
	ADCSRA |= _BV(ADEN) | _BV(ADPS2) | _BV(ADPS0) | _BV(ADIE);
}
/**
 *@brief Função que Gigante de MDF.
 *
 *
 *
 * @code {.C}
 * 
 * @endcode
*/
void inicializaVidas(){
	vidasTotais = 3;
	danoRecebido = 0;
	PORTD |= _BV(PD2) | _BV(PD3) | _BV(PD4);
}
/**
 *@brief Função que Gigante de MDF.
 *
 *
 *
 * @code {.C}
 * 
 * @endcode
*/
void enableA_PWM(){
	OCR0B = 191;
	TCCR0A = 0;
	TCCR0A |= _BV(COM0B1) | _BV(WGM01)| _BV(WGM00);
	TCCR0B = 0;
	TCCR0B |= _BV(CS00);
}
/**
 *@brief Função que Gigante de MDF.
 *
 *
 *
 * @code {.C}
 * 
 * @endcode
*/
void enableB_PWM(){
	OCR2A = 191;
	TCCR2A = 0;
	TCCR2A |= _BV(COM2A1) | _BV(WGM21)| _BV(WGM20);
	TCCR2B = 0;
	TCCR2B |= _BV(CS20);
}
/**
 *@brief Função que Gigante de MDF.
 *
 *
 *
 * @code {.C}
 * 
 * @endcode
*/
void leituraADC(){
	ADCSRA |= _BV(ADSC);
	while((ADCSRA & _BV(ADIF)));
	ADCSRA |= _BV(ADIF);
	if(adcValue <= 400){
		dano();
    adcValue = 500;
	}
}
/**
 *@brief Função que Gigante de MDF.
 *
 *
 *
 * @code {.C}
 * 
 * @endcode
*/
int main(void){
	DDRB |= _BV(PB4);//define o laser atirador como saída
	DDRB |= _BV(PB1) | _BV(PB2) | _BV(PB3);//define os bits Pb3~EnA, Pb2-In1, Pb1-In2 como saída
	DDRD |= _BV(PD5) | _BV(PD6) | _BV(PD7);//define os bits Pd5~EnB, Pd6-In3, Pd7-In4 como saída
	DDRD |= _BV(PD2) | _BV(PD3) | _BV(PD4);//define os Leds de vida como saída
	DDRC = 0x00;//define os bits Pc0, Pc2-esquerda, Pc3-direita, Pc4-acelera, Pc5-ré como entrada
  DDRC |= _BV(PC6);
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