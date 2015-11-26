/*
 * ====================================================================
 *     Copyright: (c) 2015 GuangDong  Nufront SOC Chip Co., Ltd.
 *     All rights reserved.
 *
 *       Filename:  nl6621_exti.c
 *
 *    Description:  This file provides all the EXTI firmware functions.
 *
 *        Version:  0.0.1
 *        Created:  2015/11/25 15:47:43
 *       Revision:  none
 *
 *         Author:  Hui Lou (Link), louis_zhl@foxmail.com
 *   Organization:  Nufront
 *
 *--------------------------------------------------------------------------------------          
 *       ChangLog:
 *        version    Author      Date        Purpose
 *        0.0.1      Hui Lou    2015/11/25    Create and initialize
 *        0.0.2      Hui Lou    2015/11/25    Modify webserver framework to fix serialnet
 *
 * ====================================================================
 */
#include "nl6621_exti.h"

/* NL6621  GPIO 中断 Demo */
void GPIO_Interrupt_Demo(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
 	EXTI_InitTypeDef EXTI_InitStructure;
 	NVIC_InitTypeDef NVIC_InitStructure;

    /* 初始化GPIO10,GPIO21 输入 */
  	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_10|GPIO_Pin_21;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_In;
  	GPIO_Init(&GPIO_InitStructure);

    GPIO_EXTILineConfig(GPIO_Pin_22,IRQ_ENABLE);
    /* 初始化GPIO10低电平触发 */
   	EXTI_InitStructure.EXTI_Line = EXTI_Line10;
  	EXTI_InitStructure.EXTI_Mode = EXTI_LEVEL_SENSITIVE;	
  	EXTI_InitStructure.EXTI_Trigger = EXTI_ACTIVE_LOW;
  	EXTI_InitStructure.EXTI_LineCmd = IRQ_ENABLE;
  	EXTI_Init(&EXTI_InitStructure);	 

    GPIO_EXTILineConfig(GPIO_Pin_22,IRQ_ENABLE);
    /* 初始化GPIO21上升沿触发 */
  	EXTI_InitStructure.EXTI_Line = EXTI_Line21;
  	EXTI_InitStructure.EXTI_Mode = EXTI_EDGE_SENSITIVE;	
  	EXTI_InitStructure.EXTI_Trigger = EXTI_ACTIVE_HIGH;
  	EXTI_InitStructure.EXTI_LineCmd = IRQ_ENABLE;
  	EXTI_Init(&EXTI_InitStructure);	  	

  
  	NVIC_InitStructure.NVIC_IRQChannel = EXTI10_IRQn;			
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;			
  	NVIC_InitStructure.NVIC_IRQChannelCmd = IRQ_ENABLE;							
  	NVIC_Init(&NVIC_InitStructure); 

  	NVIC_InitStructure.NVIC_IRQChannel = EXTI16_31_IRQn;			
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;			
  	NVIC_InitStructure.NVIC_IRQChannelCmd = IRQ_ENABLE;							
  	NVIC_Init(&NVIC_InitStructure); 
}

VOID  BSP_GPIOIntISR (uint32_t EXTI_Line)
{
    int irq_num,portNum;

     //NL6621 gpio irq index
     //GPIO: 0 1 2 3 4 5 6 7  8  9 10 11 12 13 14 15 16~31
     //IRQ:  8 7 6 5 4 3 2 1 25 24 23 22 21 20 19 18  29
	irq_num = EXTI_Line; 
	for(portNum=0;portNum<32;portNum++) {
	   if(irq_num&0x01) {
		   break;
	   } 
	   irq_num = irq_num >> 1; 
	}
    if((portNum >= 0)&&(portNum < 8))
        irq_num = 8 - portNum;
    else if((portNum >=8)&&(portNum < 16))
        irq_num = 33 -portNum;
    else
        irq_num = 29;
    
    //disable interrupt
    NVIC_DisableIRQ(irq_num);

	if(EXTI_GetITStatus(EXTI_Line) != RESET)	
	{
	
	  //add user code

	}
    
    EXTI_ClearITPendingBit(EXTI_Line);  
    //TODO: Add what you want to do
	
    //enable interrupt
    NVIC_EnableIRQ(irq_num);
}
/* NL6621  GPIO 中断 Demo */


void EXTI_DeInit(void)
{
	NST_WR_GPIO_REG(G_INT_TYPE_LEVEL, 0); 
	NST_WR_GPIO_REG(G_INT_POLARITY, 0);
    NST_WR_GPIO_REG(G_INT_ENA, 0);  
    NST_WR_GPIO_REG(G_INT_MASK, 0);      
}

void EXTI_Init(EXTI_InitTypeDef* EXTI_InitStruct)
{
    int reg_val;

	/* Check the parameters */
    assert_param(IS_EXTI_MODE(EXTI_InitStruct->EXTI_Mode));
    assert_param(IS_EXTI_TRIGGER(EXTI_InitStruct->EXTI_Trigger));
    assert_param(IS_EXTI_LINE(EXTI_InitStruct->EXTI_Line));  
    assert_param(IS_FUNCTIONAL_STATE(EXTI_InitStruct->EXTI_LineCmd));

    //set int type level
    reg_val = NST_RD_GPIO_REG(G_INT_TYPE_LEVEL);
    if(EXTI_InitStruct->EXTI_Trigger) {
        reg_val |= EXTI_InitStruct->EXTI_Trigger;
    } else {
        reg_val &= ~(EXTI_InitStruct->EXTI_Trigger); 
    }
    NST_WR_GPIO_REG(G_INT_TYPE_LEVEL, reg_val);
	 
    //set int polarity
    reg_val = NST_RD_GPIO_REG(G_INT_POLARITY);	 
    if(EXTI_InitStruct->EXTI_Mode){
        reg_val |= EXTI_InitStruct->EXTI_Mode;
    }
    else{
	    reg_val &= ~(EXTI_InitStruct->EXTI_Mode); 
    }
    NST_WR_GPIO_REG(G_INT_POLARITY, reg_val); 
	 
    //enable interrupt
    reg_val = NST_RD_GPIO_REG(G_INT_ENA);
	if(EXTI_InitStruct->EXTI_LineCmd) {
        reg_val |= EXTI_InitStruct->EXTI_Line;	
	} else {
        reg_val &= ~(EXTI_InitStruct->EXTI_Line);	
	}
	NST_WR_GPIO_REG(G_INT_ENA, reg_val);     
}

void EXTI_StructInit(EXTI_InitTypeDef* EXTI_InitStruct)
{
    EXTI_InitStruct->EXTI_Line = 0;
    EXTI_InitStruct->EXTI_Mode = EXTI_LEVEL_SENSITIVE;
    EXTI_InitStruct->EXTI_Trigger = EXTI_ACTIVE_HIGH;
    EXTI_InitStruct->EXTI_LineCmd = IRQ_DISABLE;
}

ITStatus EXTI_GetITStatus(uint32_t EXTI_Line)
{
    ITStatus bitstatus = RESET;
    uint32_t enablestatus = 0;
    /* Check the parameters */
    assert_param(IS_GET_EXTI_LINE(EXTI_Line));
  
    enablestatus = NST_RD_GPIO_REG(G_INT_STATUS) & EXTI_Line;
											   
    if (((NST_RD_GPIO_REG(G_INT_ENA) & EXTI_Line) != (uint32_t)RESET) && (enablestatus != (uint32_t)RESET)){
        bitstatus = SET;
    } else {
        bitstatus = RESET;
    }

    return bitstatus;
}

void EXTI_ClearITPendingBit(uint32_t EXTI_Line)
{
    int  reg_val = 0;

    /* Check the parameters */
    assert_param(IS_EXTI_LINE(EXTI_Line));
  
    //clear interrupt
    reg_val = NST_RD_GPIO_REG(PORTA_EOI);
    reg_val |= EXTI_Line;
    NST_WR_GPIO_REG(PORTA_EOI, reg_val); 
}




