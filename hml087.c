#include "ch32fun.h"

//select the coder DATA table
#if CODER_ID == 1377368
#include "1377368.h"
#elif CODER_ID == 1380873
#include "1380873.h"
#elif CODER_ID == 1385364
#include "1385364.h"
#elif CODER_ID == 1385468
#include "1385468.h"
#elif CODER_ID == 1394321
#include "1394321.h"
#endif

//define GPIO pins
#define PIN_CLOCK PD6
#define PIN_DATA PC2
#define PIN_CS PC1

//define a wakeup interrupt AFIO_EXTICR register bit for PIN_CS
#define WAKEUP_IRQ_BIT AFIO_EXTICR_EXTI1_PC

//define the time period in seconds after which the chip may go to sleep
#define TIME_MAY_SLEEP 120

//the flag is set when a SysTick interrupt is fired
volatile uint8_t systick_irq_fired = 0;


/**
 * @desc Initialises the SysTick to trigger an IRQ after the time period
 */
void init_systick(void)
{
    //resets any pre-existing configuration
    SysTick->CTLR = 0x0000;

    //sets the compare register to trigger once per millisecond
    SysTick->CMP = TIME_MAY_SLEEP * 1000 * DELAY_MS_TIME;

    //resets the Count Register
    SysTick->CNT = 0x00000000;

    //sets the SysTick Configuration
    //enable the counter
    SysTick->CTLR |= SYSTICK_CTLR_STE
                  //enable interrupts
                  | SYSTICK_CTLR_STIE;

    //enables the SysTick IRQ
    NVIC_EnableIRQ(SysTicK_IRQn);
}

/**
 * @desc The SysTick ISR is fired after the time period has passed
 */
void SysTick_Handler(void) __attribute__((interrupt));
void SysTick_Handler(void)
{

    //incrementa the Compare Register for the next trigger
    SysTick->CMP += TIME_MAY_SLEEP * 1000 * DELAY_MS_TIME;

    //clears the trigger state for the next IRQ
    SysTick->SR = 0x00000000;

    //sets the flag
    systick_irq_fired = 1;
}

void EXTI7_0_IRQHandler( void ) __attribute__((interrupt));
void EXTI7_0_IRQHandler( void ) 
{
    // Acknowledge the interrupt
    EXTI->INTFR = EXTI_Line1;
}


/**
 * @desc Inits the wakeup interrupt that wakes up the system from sleep
 */
void init_wakeup_irq(void)
{

    //AFIO is needed for EXTI
    RCC->APB2PCENR |= RCC_AFIOEN;

    //assigns the interrupt bit for the CS pin
    AFIO->EXTICR |= (uint32_t)WAKEUP_IRQ_BIT;

    //enables line 1 (PIN_CS) interrupt event
    EXTI->EVENR |= EXTI_Line1;

    //enables the rising edge interrupt trigger
    EXTI->RTENR |= EXTI_Line1;

}

/**
 * @desc Inits the sleep mode
 */
void init_sleep_mode(void)
{
    //selects standby on power-down
    PWR->CTLR |= PWR_CTLR_PDDS;
    
    // peripheral interrupt controller is sent to deep sleep
    PFIC->SCTLR |= (1 << 2);

}

int main() 
{
    //the current bit number in the coder array that is sent to the DATA pin
    uint8_t current_bit = 0;
    //the previous read value of the CLOCK pin
    uint8_t previous_clock_value = 1;

    SystemInit();

    // This delay gives us some time to reprogram the device. 
    // Otherwise if the device enters standby mode we can't 
    // program it any more.
    //Delay_Ms(5000);


    // Enable GPIOs
    funGpioInitAll();
    
    //sets the CS pin as input
    funPinMode(PIN_CS, GPIO_CFGLR_IN_PUPD);

    //sets it as pull down
    funDigitalWrite(PIN_CS, FUN_LOW);

    //sets the DATA pin as output
    funPinMode(PIN_DATA, GPIO_Speed_50MHz | GPIO_CNF_OUT_PP);

    //initially set it to 1
    funDigitalWrite(PIN_DATA, FUN_HIGH);

    //sets the CLOCK pin as input
    funPinMode(PIN_CLOCK, GPIO_CFGLR_IN_PUPD);

    //sets it as pull UP
    funDigitalWrite(PIN_CLOCK, FUN_HIGH);

    //set unused pins as output
    funPinMode(PA2, GPIO_Speed_50MHz | GPIO_CNF_OUT_PP);
    //initially set it to 0
    funDigitalWrite(PA2, FUN_LOW);

    funPinMode(PC4, GPIO_Speed_50MHz | GPIO_CNF_OUT_PP);
    //initially set it to 0
    funDigitalWrite(PC4, FUN_LOW);

    //sets the time period timer
    init_systick();

    //inits the wakeup irq
    init_wakeup_irq();

    //inits the sleep mode
    init_sleep_mode();

    funPinMode(PC3, GPIO_Speed_50MHz | GPIO_CNF_OUT_PP);
    funDigitalWrite(PC3, FUN_HIGH);


    while (1) {
        //CS is active
        if (funDigitalRead(PIN_CS) == FUN_HIGH) {
            //the CLOCK pin is 0
            if (funDigitalRead(PIN_CLOCK) == FUN_LOW) {
                //send DATA when CLOCK changes from 1 to 0
                if (previous_clock_value == 1) { 
                    if (coder[current_bit] == 1) {
                        //send 1
                        funDigitalWrite(PIN_DATA, FUN_HIGH);
                    } else {
                        //send 0
                        funDigitalWrite(PIN_DATA, FUN_LOW);
                    }

                    current_bit++;
                    if (current_bit >= sizeof(coder)/sizeof(coder[0])) {
                        current_bit = 0;
                    }
                } 

                previous_clock_value = 0;
            } else {
                previous_clock_value = 1;
            }
        } 
        //CS is inactive
        else {
            current_bit = 0;
            previous_clock_value = 1;

            //the DATA pin is 1 in the inactive state
            funDigitalWrite(PIN_DATA, FUN_HIGH);

            //if the time period has passed
            if (systick_irq_fired == 1) {
                systick_irq_fired = 0;

                //go to sleep
                __WFE();

                //restore the clock to the full speed
                SystemInit();

                funDigitalWrite(PC3, FUN_HIGH);
            }
        }
    }
}

