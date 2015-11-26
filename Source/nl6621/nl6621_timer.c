/*
 * ====================================================================
 *     Copyright: (c) 2015 GuangDong  Nufront SOC Chip Co., Ltd.
 *     All rights reserved.
 *
 *       Filename:  nl6621_spi.c
 *
 *    Description:  This file provides all the TIMER firmware functions.
 *
 *        Version:  0.0.1
 *        Created:  2015/11/27 14:47:43
 *       Revision:  none
 *
 *         Author:  Hui Lou (Link), louis_zhl@foxmail.com
 *   Organization:  Nufront
 *
 *--------------------------------------------------------------------------------------          
 *       ChangLog:
 *        version    Author      Date        Purpose
 *        0.0.1      Hui Lou    2015/11/27    Create and initialize
 *
 * ====================================================================
 */
#include "nl6621_timer.h"

extern UINT32 CpuClkFreq;
extern UINT32 ApbClkFreq;

//
/* NL6621 TIMER DEMO */
//
/* 滴答时钟初始化 */
void BSP_TickInit(void)	
{

    int         ClkFreq; //Hz
    int         cnts;

    ClkFreq = CpuClkFreq;
    cnts = ClkFreq/OS_TICKS_PER_SEC;
    *SysTickLoad = cnts;
    
    /* Start, with interrupts enabled */
    *SysTickCtrl = SysTickEnable | SysTickInterrupt | SysTickClkSource;

    *(volatile UINT8*)(0xE000ED23) = (SYS_TICK_PRIO << 5); // b7..b5有效    
}

void Timerx_Init(TIM_TypeDef TIMx, uint32_t timerus)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	TIM_TimeBaseStructure.TIM_Period = timerus; 
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_UserDown; /* TIM_CounterMode_FreeRun不支持中断功能 */
	TIM_TimeBaseInit(TIMx, &TIM_TimeBaseStructure);
 
	/* TIM IT enable */
	TIM_ITConfig(TIMx, IRQ_ENABLE);

	/* Enable the TIM3 global Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = ((TIMx == TIMER0)? TMR0_IRQn : TMR1_IRQn); 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3; 
	NVIC_InitStructure.NVIC_IRQChannelCmd = IRQ_ENABLE; 
	NVIC_Init(&NVIC_InitStructure);  

	TIM_Cmd(TIMx, IRQ_ENABLE);  //ê1?üTIMxíaéè							 
}

void TIM0_IRQHandler(void)   
{
    NVIC_DisableIRQ(TMR1_IRQn);
  
	if (TIM_GetITStatus(TIMER0) != RESET)
    {
		TIM_ClearITPendingBit(TIMER0); 
		//ADD UERS CODE...
		//...  
	}

    NVIC_EnableIRQ(TMR1_IRQn);
}

void TIM1_IRQHandler(void)   
{
    NVIC_DisableIRQ(TMR1_IRQn);
  
	if (TIM_GetITStatus(TIMER1) != RESET)  
	{
		TIM_ClearITPendingBit(TIMER1); 
		//ADD UERS CODE...
		//... 
	}

    NVIC_EnableIRQ(TMR1_IRQn);
}
//
/* NL6621 TIMER DEMO */
//


void TIM_Cmd(TIM_TypeDef TIMx, FunctionalState NewState)
{
	/* Check the parameters */
	assert_param(IS_TIM_ALL_PERIPH(TIMx));
	assert_param(IS_FUNCTIONAL_STATE(NewState));
	
	
	if (NewState != IRQ_DISABLE)
	{
		/* Enable the TIM Counter */
		if(TIMx == TIMER0)
		   *Tmr0Ctl |= TMR_ENA;
		else 
		   *Tmr1Ctl |= TMR_ENA;
	}
	else
	{
		/* Disable the TIM Counter */
		if(TIMx == TIMER0)
		   *Tmr0Ctl &= ~TMR_ENA;
		else 
		   *Tmr1Ctl &= ~TMR_ENA;
	}
}
/* NL6621 TIMER DEMO */




void TIM_ClearITPendingBit(TIM_TypeDef TIMx)
{
    uint32_t RegEoi;

    /* Check the parameters */
    assert_param(IS_TIM_ALL_PERIPH(TIMx));
    /* Clear the IT pending Bit */

    // clear timer interrupt
	if(TIMx == TIMER1)
       RegEoi = *Tmr1Eoi; //timer1 interrupt flag clear.
	else
	   RegEoi = *Tmr0Eoi; //timer0 interrupt flag clear. 

    RegEoi = *TmrsEoi; //all timer interrupt flag clear.
    RegEoi = RegEoi;
}

void TIM_ITConfig(TIM_TypeDef TIMx, FunctionalState NewState)
{  
    /* Check the parameters */
    assert_param(IS_TIM_ALL_PERIPH(TIMx));
    assert_param(IS_FUNCTIONAL_STATE(NewState));
  
    if (NewState != IRQ_DISABLE)
    {
        /* Enable the Interrupt sources */
		if(TIMx == TIMER0)
		   *Tmr0Ctl &= (~TMR_INT_MASK);
		else 
		   *Tmr1Ctl &= (~TMR_INT_MASK);
    }
    else
    {
        /* Disable the Interrupt sources */
	  	if(TIMx == TIMER0)
		   *Tmr0Ctl |= (TMR_INT_MASK);
		else 
		   *Tmr1Ctl |= (TMR_INT_MASK);
    }
}

ITStatus TIM_GetITStatus(TIM_TypeDef TIMx)
{
	ITStatus bitstatus = RESET;  
	uint8_t itstatus = 0x0;
	/* Check the parameters */
	assert_param(IS_TIM_ALL_PERIPH(TIMx));
	
	itstatus = *TmrsIntStatus;
	
	if (((itstatus&0x03)>>TIMx) == SET)
	{
		bitstatus = SET;
	}
	else
	{
		bitstatus = RESET;
	}
	return bitstatus;
}

void TIM_TimeBaseInit(TIM_TypeDef TIMx, TIM_TimeBaseInitTypeDef* TIM_TimeBaseInitStruct)
{
	
	/* Check the parameters */
	assert_param(IS_TIM_ALL_PERIPH(TIMx)); 
	assert_param(IS_TIM_COUNTER_MODE(TIM_TimeBaseInitStruct->TIM_CounterMode));

	if(TIMx == TIMER1)
	{
        *Tmr1Load = ApbClkFreq/1000000*TIM_TimeBaseInitStruct->TIM_Period;
    
        *Tmr1Ctl |= TIM_TimeBaseInitStruct->TIM_CounterMode;	
	}
	else 
	{
        *Tmr0Load = ApbClkFreq/1000000*TIM_TimeBaseInitStruct->TIM_Period;
    
        *Tmr0Ctl |= TIM_TimeBaseInitStruct->TIM_CounterMode;		
	}       
}

/*
******************************************************************************
**                            End Of File                                    **
******************************************************************************
*/

