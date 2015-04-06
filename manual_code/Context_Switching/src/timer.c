/**
 * @brief timer.c - Timer example code. Tiemr IRQ is invoked every 1ms
 * @author T. Reidemeister
 * @author Y. Huang
 * @author NXP Semiconductors
 * @date 2012/02/12
 */

#include <LPC17xx.h>
#include "timer.h"
#include "k_utilities.h"
#include "k_process.h"
#include "k_rtx.h"
#include "k_memory.h"
#include "uart_def.h"
#include "uart_polling.h"
#include "printf.h"


volatile uint32_t g_timer_count = 0; // increment every 1 ms
volatile uint32_t* timing_analysis;
extern MSG_HEADER *pending_delayed_messages;
MSG_HEADER *timeout_queue = NULL;
uint32_t g_timer_switch_flag = 0;
/**
 * @brief: initialize timer. Only timer 0 is supported
 */
uint32_t timer_init(uint8_t n_timer) 
{
	LPC_TIM_TypeDef *pTimer1;
	LPC_TIM_TypeDef *pTimer2;
	if (n_timer == 0) {
		/*
		Steps 1 & 2: system control configuration.
		Under CMSIS, system_LPC17xx.c does these two steps
		
		----------------------------------------------------- 
		Step 1: Power control configuration.
		        See table 46 pg63 in LPC17xx_UM
		-----------------------------------------------------
		Enable UART0 power, this is the default setting
		done in system_LPC17xx.c under CMSIS.
		Enclose the code for your refrence
		//LPC_SC->PCONP |= BIT(1);
	
		-----------------------------------------------------
		Step2: Select the clock source, 
		       default PCLK=CCLK/4 , where CCLK = 100MHZ.
		       See tables 40 & 42 on pg56-57 in LPC17xx_UM.
		-----------------------------------------------------
		Check the PLL0 configuration to see how XTAL=12.0MHZ 
		gets to CCLK=100MHZ in system_LPC17xx.c file.
		PCLK = CCLK/4, default setting in system_LPC17xx.c.
		Enclose the code for your reference
		//LPC_SC->PCLKSEL0 &= ~(BIT(3)|BIT(2));	

		-----------------------------------------------------
		Step 3: Pin Ctrl Block configuration. 
		        Optional, not used in this example
		        See Table 82 on pg110 in LPC17xx_UM 
		-----------------------------------------------------
		*/
		pTimer1 = (LPC_TIM_TypeDef *) LPC_TIM0;
		pTimer2 = (LPC_TIM_TypeDef *) LPC_TIM1;
	} else {
		return 1;
	}

	/*
	-----------------------------------------------------
	Step 4: Interrupts configuration
	-----------------------------------------------------
	*/

	/* Step 4.1: Prescale Register PR setting 
	   CCLK = 100 MHZ, PCLK = CCLK/4 = 25 MHZ
	   2*(12499 + 1)*(1/25) * 10^(-6) s = 10^(-3) s = 1 ms
	   TC (Timer Counter) toggles b/w 0 and 1 every 12500 PCLKs
	   see MR setting below 
	*/
	pTimer1->PR = 12499;  
	pTimer2->PR = 24; //2*(24 + 1)*(1/25) * 10^(-6) s = 10^(-3) s = 2 �s

	/* Step 4.2: MR setting, see section 21.6.7 on pg496 of LPC17xx_UM. */
	pTimer1->MR0 = 1;
	pTimer2->MR0 = 0;

	/* Step 4.3: MCR setting, see table 429 on pg496 of LPC17xx_UM.
	   Interrupt on MR0: when MR0 mathches the value in the TC, 
	                     generate an interrupt.
	   Reset on MR0: Reset TC if MR0 mathches it.
	*/
	pTimer1->MCR = BIT(0) | BIT(1);
	pTimer2->MCR = BIT(0) | BIT(1);

	g_timer_count = 0;

	/* Step 4.4: CSMSIS enable timer0 IRQ */
	NVIC_EnableIRQ(TIMER0_IRQn);
	NVIC_EnableIRQ(TIMER1_IRQn);

	/* Step 4.5: Enable the TCR. See table 427 on pg494 of LPC17xx_UM. */
	pTimer1->TCR = 1;
	pTimer2->TCR = BIT(0);
	
	pTimer2->TC = 1;
	timing_analysis = &pTimer2->TC;

	return 0;
}

/**
 * @brief: use CMSIS ISR for TIMER0 IRQ Handler
 * NOTE: This example shows how to save/restore all registers rather than just
 *       those backed up by the exception stack frame. We add extra
 *       push and pop instructions in the assembly routine. 
 *       The actual c_TIMER0_IRQHandler does the rest of irq handling
 */
__asm void TIMER0_IRQHandler(void)
{
	CPSID I
	PRESERVE8
	IMPORT c_TIMER0_IRQHandler
	IMPORT k_release_processor
	PUSH{r4-r11, lr}
	BL c_TIMER0_IRQHandler
	LDR R4, =__cpp(&g_timer_switch_flag)
	LDR R4, [R4]
	MOV R5, #0     
	CMP R4, R5
	CPSIE I
	BEQ  RESTORE    ; if g_timer_switch_flag == 0, then restore the process that was interrupted
	BL k_release_processor  ; otherwise (i.e g_timer_switch_flag == 1, then switch to the other process)
RESTORE
	POP{r4-r11, pc}
} 

/**
 * @brief: c TIMER0 IRQ Handler
 */
void c_TIMER0_IRQHandler(void)
{
	MSG_HEADER *pending_message;
	/* ack inttrupt, see section  21.6.1 on pg 493 of LPC17XX_UM */
	LPC_TIM0->IR = BIT(0);  
	pending_message = dequeue_pending_queue();
	while(pending_message != NULL) {
		//enqueue into prioirity timeout queue
		enqueue_timeout_queue(pending_message);
		pending_message = dequeue_pending_queue();
	}
	while (timeout_queue != NULL && g_timer_count > timeout_queue->expiry) {
		int target_pid;
		MSG_HEADER *expired_message;
#ifdef DEBUG_0 
	printf("%d expired at %d\n", timeout_queue->expiry, g_timer_count);
#endif /* ! DEBUG_0 */
		expired_message = dequeue_timeout_queue();
		target_pid = expired_message->dest_pid;
	
		k_send_message_header_nb(target_pid, expired_message);
	}
	g_timer_count = g_timer_count + 1;
	g_timer_switch_flag = higher_priority_available();
}
