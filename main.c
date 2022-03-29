#include "MKL46Z4.h"
#include "lcd.h"

// LED (RG)
// LED_GREEN = PTD5 (pin 98)
// LED_RED = PTE29 (pin 26)

// SWICHES
// RIGHT (SW1) = PTC3 (pin 73)
// LEFT (SW2) = PTC12 (pin 88)

// Enable IRCLK (Internal Reference Clock)
// see Chapter 24 in MCU doc

int status_bt1 = 0;
int status_bt2 = 0;
int SYSTEM_STATUS = 0;

void irclk_ini()
{
  MCG->C1 = MCG_C1_IRCLKEN(1) | MCG_C1_IREFSTEN(1);
  MCG->C2 = MCG_C2_IRCS(0); //0 32KHZ internal reference clock; 1= 4MHz irc
}

void delay(void)
{
  volatile int i;

  for (i = 0; i < 1000000; i++);
}

// RIGHT_SWITCH (SW1) = PTC3
void sw1_ini()
{
  SIM->COPC = 0;
  SIM->SCGC5 |= SIM_SCGC5_PORTC_MASK;
  PORTC->PCR[3] |= PORT_PCR_MUX(1) | PORT_PCR_PE(1);
  GPIOC->PDDR &= ~(1 << 3);

  // IRQ
  PORTC->PCR[3] |= PORT_PCR_IRQC(0xA); // IRQ en el flanco de bajada
  NVIC_SetPriority(31, 0); // Prioridad de la interrupcion 31
  NVIC_EnableIRQ(31);   // Activa la interrupcion
}

// LEFT_SWITCH (SW2) = PTC12
void sw2_ini()
{
  SIM->COPC = 0;
  SIM->SCGC5 |= SIM_SCGC5_PORTC_MASK;
  PORTC->PCR[12] |= PORT_PCR_MUX(1) | PORT_PCR_PE(1);
  GPIOC->PDDR &= ~(1 << 12);

  // IRQ
  PORTC->PCR[12] |= PORT_PCR_IRQC(0xA); // IRQ en el flanco de bajada
  NVIC_SetPriority(31, 0); // Prioridad de la interrupcion 31
  NVIC_EnableIRQ(31);   // Activa la interrupcion
}

int sw1_check()
{
  return( !(GPIOC->PDIR & (1 << 3)) );
}

int sw2_check()
{
  return( !(GPIOC->PDIR & (1 << 12)) );
}

// RIGHT_SWITCH (SW1) = PTC3
// LEFT_SWITCH (SW2) = PTC12
void sws_ini()
{
  SIM->COPC = 0;
  SIM->SCGC5 |= SIM_SCGC5_PORTC_MASK;
  PORTC->PCR[3] |= PORT_PCR_MUX(1) | PORT_PCR_PE(1);
  PORTC->PCR[12] |= PORT_PCR_MUX(1) | PORT_PCR_PE(1);
  GPIOC->PDDR &= ~(1 << 3 | 1 << 12);
}

// LED_GREEN = PTD5
void led_green_ini()
{
  SIM->COPC = 0;
  SIM->SCGC5 |= SIM_SCGC5_PORTD_MASK;
  PORTD->PCR[5] = PORT_PCR_MUX(1);
  GPIOD->PDDR |= (1 << 5);
  GPIOD->PSOR = (1 << 5);
}

void led_green_toggle()
{
  GPIOD->PTOR = (1 << 5);
}

// LED_RED = PTE29
void led_red_ini()
{
  SIM->COPC = 0;
  SIM->SCGC5 |= SIM_SCGC5_PORTE_MASK;
  PORTE->PCR[29] = PORT_PCR_MUX(1);
  GPIOE->PDDR |= (1 << 29);
  GPIOE->PSOR = (1 << 29);
}

void led_red_toggle(void)
{
  GPIOE->PTOR = (1 << 29);
}

// LED_RED = PTE29
// LED_GREEN = PTD5
void leds_ini()
{
  SIM->COPC = 0;
  SIM->SCGC5 |= SIM_SCGC5_PORTD_MASK | SIM_SCGC5_PORTE_MASK;
  PORTD->PCR[5] = PORT_PCR_MUX(1);
  PORTE->PCR[29] = PORT_PCR_MUX(1);
  GPIOD->PDDR |= (1 << 5);
  GPIOE->PDDR |= (1 << 29);
  // both LEDS off after init
  GPIOD->PSOR = (1 << 5);
  GPIOE->PSOR = (1 << 29);
}

void PORTDIntHandler(void) {
  int pressed_switch = PORTC->ISFR;
  PORTC->ISFR = 0xFFFFFFFF; // Clear IRQ
  // SW1
  if(pressed_switch == (0x8)) {
    status_bt1 += 1;
    if(status_bt1 > 1) {
	status_bt1 = 0;
    }
  } else
    status_bt1 = 0;

  // SW2
  if(pressed_switch == (0x1000)) {
    status_bt2 += 1;
    if(status_bt2 > 1) {
	status_bt2 = 0;
    }
  } else
    status_bt2 = 0;


  if (status_bt1 == 1)
    SYSTEM_STATUS = 1;
  else if (status_bt2 == 1)
    SYSTEM_STATUS = 2;  
  else
    SYSTEM_STATUS = 0;
}

void led_red_on(void) {
 GPIOE->PCOR |= (1 << 29);
}

void led_red_off(void) {
 GPIOE->PSOR |= (1 << 29);
}

void led_green_on(void)
{
 GPIOD->PCOR |= (1 << 5);
}

void led_green_off(void) {
 GPIOD->PSOR |= (1 << 5);
}

// Hit condition: (else, it is a miss)
// - Left switch matches red light
// - Right switch matches green light

int main(void)
{
  irclk_ini(); // Enable internal ref clk to use by LCD

  int hit = 0;
  int miss = 0;
  lcd_ini();
  leds_ini();
  sw1_ini();
  sw2_ini();

  // 'Random' sequence :-)
  volatile unsigned int sequence = 0x32B14098,
  index = 0;
  int encen = 0;

  while (index < 32) {
    lcd_display_dec(index);
    if (SYSTEM_STATUS == 0) {
      if (sequence & (1 << index)) { //odd
	
        //
        // Switch on green led
        // [...]
        //

        led_green_on();
        led_red_off();
        encen = 1;
      } else { //even
        //
        // Switch on red led
        // [...]
        //
        led_red_on();
        led_green_off();
        encen = 2;
      }
    }
    // [...]
    while (encen != 0) {
      switch(SYSTEM_STATUS) {
        case 1:
          if (encen == 1)
            hit += 1;
          else
	    miss += 1;
          encen = 0;
          led_red_off();
          led_green_off();
          (index)++;
          break;
        case 2:
          if (encen == 2)
            hit += 1;
          else
	    miss += 1;
          encen = 0;
          led_red_off();
          led_green_off();
          (index)++;
          break;
      }
    } 
  }

  // Stop game and show blinking final result in LCD: hits:misses
  // [...]
  //

  while (1){
    lcd_display_time(hit, miss);
    delay();
    lcd_ini();
    delay();
  }

  return 0;
}
