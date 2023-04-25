/*
This file is part of CanFestival, a library implementing CanOpen Stack.

Copyright (C): Edouard TISSERANT and Francis DUPIN
mbed Port: sgrove

See COPYING file for copyrights details.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

// Includes for the Canfestival driver
#include "canfestival.h"
#include "canopen_timer.h"
#include "mbed.h"

// make the system speed available for timer configuration
extern uint32_t SystemCoreClock;

#define TIM_USR      TIM7
#define TIM_USR_IRQ  TIM7_IRQn
 
//DigitalOut myled(LED1);
TIM_HandleTypeDef mTimUserHandle;

// Define the timer registers
#define TimerAlarm        TIM7->ARR    // compare register to genterate an interrupt
#define TimerCounter      TIM7->CNT    // free running timer counter register


/************************** Module variables **********************************/
// Store the last timer value to calculate the elapsed time
static TIMEVAL last_time_set = TIMEVAL_MAX;     

void initTimer(void)
/******************************************************************************
Initializes the timer, turn on the interrupt and put the interrupt time to zero
INPUT    void
OUTPUT    void
******************************************************************************/
{
    TimerAlarm = 0;
    
    __HAL_RCC_TIM7_CLK_ENABLE();
 // set up the prescaler to create an 8us incrementing tick using Timer1
  mTimUserHandle.Instance               = TIM_USR;
  mTimUserHandle.Init.Prescaler         = 640; //640 8us timebase  //1280 16 us timebase //NB L432Kc 80MHz
  mTimUserHandle.Init.CounterMode       = TIM_COUNTERMODE_UP;
  mTimUserHandle.Init.Period            = 65536; 
  mTimUserHandle.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
  mTimUserHandle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  
  NVIC_SetVector(TIM_USR_IRQ, (uint32_t)M_TIM_USR_Handler);
  NVIC_EnableIRQ(TIM_USR_IRQ);
  
  HAL_TIM_Base_Init(&mTimUserHandle);
  HAL_TIM_Base_Start_IT(&mTimUserHandle);
 
    // Set timer for CANopen operation tick 8us , max time is 9+ hrs( co cazz so 16 bit, 0.5 s
    TimeDispatch();
}

void setTimer(TIMEVAL value)
/******************************************************************************
Set the timer for the next alarm.
INPUT    value TIMEVAL (unsigned long)
OUTPUT    void
******************************************************************************/
{
  //uint32_t x = TimerAlarm +((uint32_t)value);
  //TimerAlarm = (x%((uint32_t)65536));    // Add the desired time to timer interrupt time
  
  TimerAlarm = ((uint32_t)value);
}

TIMEVAL getElapsedTime(void)
/******************************************************************************
Return the elapsed time to tell the Stack how much time is spent since last call.
INPUT    void
OUTPUT    value TIMEVAL (unsigned long) the elapsed time
******************************************************************************/
{
    uint32_t timer = TimerCounter;    // Copy the value of the running timer
    // Calculate the time difference and return it
    return timer > last_time_set ? timer - last_time_set : last_time_set - timer;
}

///******************************************************************************
//Interruptserviceroutine Timer Compare for the stack CAN timer
//******************************************************************************/

extern "C" void M_TIM_USR_Handler(void) {
   last_time_set = TimerCounter;
   HAL_TIM_IRQHandler(&mTimUserHandle);
   TimeDispatch();
  //if (__HAL_TIM_GET_FLAG(&mTimUserHandle, TIM_FLAG_UPDATE) == SET) {
    //__HAL_TIM_CLEAR_FLAG(&mTimUserHandle, TIM_FLAG_UPDATE);
    // store the time of the last interrupt occurance
    
    //myled != myled;
    // Call the time handler of the stack to adapt the elapsed time
    //  
  //}
  
}

