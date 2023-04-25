/**************************************************************
     Koala crowler periperials handlers on STM32L432KC 
**************************************************************/
/*

Copyright (C): Alessandro De Crescenzo - ale-dc@live.it

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

#include "koalaPeriph.h"
#include "DigitalOut.h"
#include "koala_OD.h"
#include "objdictdef.h"
#include <cstdint>

#include "stepper.h"
#include "PwmIn.h"

#define DEBUG_PRINT 0
/* Stepper*/
/*L432KC pinout for custom PCB */
#define ENABLE_PIN PA_2
#define A_STEP_PIN PA_6  // A: 4x pipe clamp
#define A_DIR_PIN PA_7
#define B_STEP_PIN PA_4  // B: 2x crawler clamp
#define B_DIR_PIN PA_5
#define C_STEP_PIN PA_3  // C: 1x crawler lock
#define C_DIR_PIN PA_1
#define PWM_1_PIN PB_0   // pwm/digital inputs 
#define PWM_2_PIN PB_7
#define PWM_3_PIN PB_6
#define BUTTON_PIN PB_4

/* stepper */
#define STEPS_CLOSE_CLAMP  400000        //#regolare
#define STEPS_OPEN_CLAMP -STEPS_CLOSE_CLAMP
#define STEPS_CLOSE_CRAWLER  400000        //#regolare
#define STEPS_OPEN_CRAWLER -STEPS_CLOSE_CLAMP
#define STEPS_CLOSE_LOCK  200000        //#regolare
#define STEPS_OPEN_LOCK -STEPS_CLOSE_CLAMP
#define STATE_CLOSED 0
#define STATE_OPEN 1
#define STATE_CLOSING 2
#define STATE_OPENING 3
#define CMD_OPEN 1
#define CMD_CLOSE -1
#define PWM_UP_TRESHOLD  0.001700 // > close
#define PWM_DN_TRESHOLD  0.001300 // < open

//stepper object
DigitalOut torqueOff(ENABLE_PIN);
StepperDriver stepper_A_clamp(A_STEP_PIN, A_DIR_PIN);
StepperDriver stepper_B_crawler(B_STEP_PIN, B_DIR_PIN);
StepperDriver stepper_C_lock(C_STEP_PIN, C_DIR_PIN);

bool first_run;

/*
//PWM object
PwmIn PWM_1_clamp(PWM_1_PIN);
PwmIn PWM_2_crawler(PWM_2_PIN);
PwmIn PWM_3_lock(PWM_3_PIN);
//digital in object
DigitalIn IN_1_clamp(PWM_1_PIN,PullDown);    //USED ONLY FOR DEBUG SWITCHES TO TEST Movement
DigitalIn IN_2_crawler(PWM_2_PIN,PullDown);
DigitalIn IN_3_lock(PWM_3_PIN,PullDown);
*/

/* hw objects*/
Serial main_pc(USBTX, USBRX);


/*debug*/
// make the system speed available for timer configuration
extern uint32_t SystemCoreClock;
void printSYSCLK(void){
    //printf("system clock: %d \n",SystemCoreClock);
}


/* utility functions */

void move(StepperDriver & stepper, int steps){ 
    torqueOff = false;
    stepper.startMotion(steps);
    //while(stepper.isBusy()) wait(0.5); 
    //torqueOff = true;
}
/*
void read_commands_pwm(int &cmd_A, int &cmd_B, int &cmd_C){
   float pw1 = PWM_1_clamp.pulsewidth();
   float pw2 = PWM_2_crawler.pulsewidth();
   float pw3 = PWM_3_lock.pulsewidth();

   if(pw1 > PWM_UP_TRESHOLD) cmd_A = CMD_CLOSE;
   if(pw1 < PWM_DN_TRESHOLD) cmd_A = CMD_OPEN;
   if(pw2 > PWM_UP_TRESHOLD) cmd_B = CMD_CLOSE;
   if(pw2 < PWM_DN_TRESHOLD) cmd_B = CMD_OPEN;
   if(pw3 > PWM_UP_TRESHOLD) cmd_C = CMD_CLOSE;
   if(pw3 < PWM_DN_TRESHOLD) cmd_C = CMD_OPEN;
}

void read_commands_pwm_single(int &cmd_A, int &cmd_B, int &cmd_C){
   float pw1 = PWM_1_clamp.pulsewidth();

   if(pw1 > 1035 && pw1 <1045) cmd_A = CMD_OPEN;
   if(pw1 > 1295 && pw1 <1310) cmd_A = CMD_CLOSE;
   if(pw1 > 1405 && pw1 <1415) cmd_B = CMD_OPEN;
   if(pw1 > 1540 && pw1 <1550) cmd_B = CMD_CLOSE;
   if(pw1 > 1675 && pw1 <1685) cmd_C = CMD_OPEN;
   if(pw1 > 1855 && pw1 <1865) cmd_C = CMD_CLOSE;
}

void read_commands_digital(int &cmd_A, int &cmd_B, int &cmd_C){
   if(IN_1_clamp) cmd_A = CMD_CLOSE;
   else cmd_A = CMD_OPEN;

   if(IN_2_crawler) cmd_B = CMD_CLOSE;
   else cmd_B = CMD_OPEN;

   if(IN_3_lock) cmd_C = CMD_CLOSE;
   else cmd_C = CMD_OPEN;
}
*/

void read_commands_can(int &cmd_A, int &cmd_B, int &cmd_C){
   if(Clamp_cmd_8_bit[0] == 0x00) cmd_A = CMD_CLOSE;
   else cmd_A = CMD_OPEN;

   if(Crawler_cmd_8_bit[0] ==0x00) cmd_B = CMD_CLOSE;
   else cmd_B = CMD_OPEN;

   if(Lock_cmd_8_bit[0] == 0x00) cmd_C = CMD_CLOSE;
   else cmd_C = CMD_OPEN;
}


/*
*  PERIPHERIAL FUNCTIONS 
*/ 
void init_uart(void){
    main_pc.baud(115200);
}

void init_stepper(void){
    first_run = true;
    move(stepper_A_clamp,STEPS_OPEN_CLAMP); 
    Clamp_state_8_Bit[0]   = STATE_OPENING;
    move(stepper_B_crawler,STEPS_OPEN_CLAMP); 
    Crawler_state_8_Bit[0] = STATE_OPENING;
    move(stepper_C_lock,STEPS_OPEN_CLAMP); 
    Lock_state_8_Bit[0]    = STATE_OPENING;

    printf("start homing\n");
    // // TODO not blocking
    // while(stepper_A_clamp.isBusy() && stepper_B_crawler.isBusy() && stepper_C_lock.isBusy() ){
    //     wait(0.1);
    // }
    // torqueOff = true;

    
}

void init_led(void){
    Ir_led_state_8_bit[0]  = Ir_led_cmd_8_bit[0];
}

unsigned char stepper_handler(CO_Data* d){
    int cmd_A_clamp = 0;
    int cmd_B_crawler = 0;
    int cmd_C_lock = 0;

    //read_commands_digital(cmd_A_clamp, cmd_B_crawler, cmd_C_lock);
    //read_commands_pwm_single(cmd_A_clamp, cmd_B_crawler, cmd_C_lock);
    read_commands_can(cmd_A_clamp, cmd_B_crawler, cmd_C_lock);
    //printf("commands: [%d , %d , %d] \r\n", cmd_A_clamp, cmd_B_crawler, cmd_C_lock);//Print for debug

    if(!stepper_A_clamp.isBusy() && !stepper_B_crawler.isBusy() && !stepper_C_lock.isBusy() ){  //if all stepper have finished their moving
        if(first_run){
            printf("end homing\n");
            first_run = false;
            Clamp_state_8_Bit[0]   = STATE_OPEN;
            Crawler_state_8_Bit[0] = STATE_OPEN;
            Lock_state_8_Bit[0]    = STATE_OPEN;
            torqueOff = true;
        }
        else{
            
            torqueOff = true;
            /* CLAMP */
            if(Clamp_state_8_Bit[0] == STATE_OPENING){
                Clamp_state_8_Bit[0] = STATE_OPEN; 
            }
            else if(Clamp_state_8_Bit[0] == STATE_CLOSING){
                Clamp_state_8_Bit[0] = STATE_CLOSED; 
            }
            else if(cmd_A_clamp == CMD_CLOSE && Clamp_state_8_Bit[0] == STATE_OPEN){
                printf("clamp GO CLOSE\n");
                move(stepper_A_clamp,STEPS_CLOSE_CLAMP);
                Clamp_state_8_Bit[0] = STATE_CLOSING;
            }
            else if(cmd_A_clamp == CMD_OPEN && Clamp_state_8_Bit[0] == STATE_CLOSED){
                printf("clamp GO OPEN\n");
                move(stepper_A_clamp,STEPS_OPEN_CLAMP); 
                Clamp_state_8_Bit[0] = STATE_OPENING;         
            }
            /* CRAWLER */
            if(Crawler_state_8_Bit[0] == STATE_OPENING){
                Crawler_state_8_Bit[0] = STATE_OPEN; 
            }
            else if(Crawler_state_8_Bit[0] == STATE_CLOSING){
                Crawler_state_8_Bit[0] = STATE_CLOSED; 
            }
            else if(cmd_B_crawler == CMD_CLOSE && Crawler_state_8_Bit[0] == STATE_OPEN){
                printf("crawler GO CLOSE\n");
                move(stepper_B_crawler,STEPS_CLOSE_CRAWLER);
                Crawler_state_8_Bit[0] = STATE_CLOSING;
            }
            else if(cmd_B_crawler == CMD_OPEN && Crawler_state_8_Bit[0] == STATE_CLOSED){
                printf("crawler GO OPEN\n");
                move(stepper_B_crawler,STEPS_OPEN_CRAWLER);      
                Crawler_state_8_Bit[0] = STATE_OPENING;       
            }
            /* LOCK */
            if(Lock_state_8_Bit[0] == STATE_OPENING){
                Lock_state_8_Bit[0] = STATE_OPEN; 
            }
            else if(Lock_state_8_Bit[0] == STATE_CLOSING){
                Lock_state_8_Bit[0] = STATE_CLOSED; 
            }
            else if(cmd_C_lock == CMD_CLOSE && Lock_state_8_Bit[0] == STATE_OPEN){
                printf("lock GO CLOSE\n");
                move(stepper_C_lock,STEPS_CLOSE_LOCK);
                Lock_state_8_Bit[0] = STATE_CLOSING;
            }
            else if(cmd_C_lock == CMD_OPEN && Lock_state_8_Bit[0] == STATE_CLOSED){
                printf("lock GO OPEN\n");
                move(stepper_C_lock,STEPS_OPEN_LOCK);      
                Lock_state_8_Bit[0] = STATE_OPENING;       
            }
        }
    }
    else { //else, some stepper is moving
        //printf("stepper busy\n");
    }

    // if(Clamp_state_8_Bit[0] != Clamp_cmd_8_bit[0])
    //     printf("Clamp cmd: %u \n",Clamp_cmd_8_bit[0]);
    // if(Crawler_state_8_Bit[0] != Crawler_cmd_8_bit[0])
    //     printf("Crawler cmd: %u \n",Crawler_cmd_8_bit[0]);
    // if(Lock_state_8_Bit[0] != Lock_cmd_8_bit[0])
    //     printf("Lock cmd: %u \n",Lock_cmd_8_bit[0]);
  
    // Clamp_state_8_Bit[0]   = Clamp_cmd_8_bit[0];
    // Crawler_state_8_Bit[0] = Crawler_cmd_8_bit[0];
    // Lock_state_8_Bit[0]    = Lock_cmd_8_bit[0];    

    return 1;
}


unsigned char led_handler(CO_Data* d){
    if(Ir_led_state_8_bit[0] != Ir_led_cmd_8_bit[0]){
        printf("IR led: %u \n",Ir_led_cmd_8_bit[0]);
        if(Ir_led_cmd_8_bit[0]== 0x00){
            printf("Turn IR led OFF\n");
        }
        else{
            printf("Turn IR led ON\n");
        }
    }
    Ir_led_state_8_bit[0]  = Ir_led_cmd_8_bit[0];
    return 1;
}

