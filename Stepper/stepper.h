#ifndef _STEPPER_H_
#define _STEPPER_H_

#include "mbed.h"
// phisics constants
float s2d = 1.8;             //step2deg [deg]/[step]
float d2s = 1.0/1.8;         //step2deg [step]/[deg]
float Ts = (1e-4);             // [s]/[tick] = 1/[Hz] = [s]
float p_true2p =d2s;
float v_true2v =d2s * Ts;
float a_true2a =d2s * Ts*Ts;
float t_true2t =1/Ts;

//Max velocity constraint
//v_max =1 => v_true_max = 1/v_true2v;

// Initialize a pins to perform analog and digital output fucntions
class StepperDriver{

private:
    float pos;
    float vel;
    float V;
    float A;
    
    int last_pos;
    int tick_counter ;
    int step_counter;
    int P;
    int P1 ;
    int P2 ;
    bool enabled;

    Ticker timer;
    DigitalOut _step_out;
    DigitalOut _dir_out;
    
    void isr() {
        tick_counter++;
        if(step_counter<P1)
            vel = vel +A;
        else if(step_counter<P2)
            vel = vel;
        else if(step_counter<P)
            vel = vel -A;
        else{
            vel = 0;
            enabled = false;
        }
        pos = pos+vel;
        if(enabled){
            if(floor(pos) != last_pos){
                _step_out = !_step_out;
                step_counter++;
                last_pos = floor(pos);
            }    
        }
    }
    
public:
    StepperDriver(PinName step_pin, PinName dir_pin) :
    _step_out(step_pin),
    _dir_out(dir_pin)
    {
        //initialization isr var
        _step_out = 0;
        _dir_out = 0;
        pos =0;
        last_pos = pos;
        tick_counter =0;
        step_counter =0;
        enabled = false;
        
        A = 0.01;
        V = 0.9;
        P = 0;
        P1 = 0;
        P2 = 0;
                
        vel=0;
        pos =0;
        last_pos = pos;
        step_counter=0;
        
        //timer = new Ticker();
        
        timer.attach_us(this, &StepperDriver::isr, Ts/1e-6);
    }
    
    int startMotion(float p_des ){
        
        if(p_des>=0) _dir_out =1;
        else _dir_out =0;
        //inputs 
        //float feed = 1.0;
        A = 0.01;
        V = 0.8;
        
        P = fabs(p_des)*p_true2p;
        P1 = V*V /(2*A);
        P2 = P-P1;
        //float T1 = V/A;
        
        vel=0;
        pos =0;
        last_pos = pos;
        step_counter=0;
        enabled = true;
        return 1;
    }
    
    bool isBusy(void){
        return enabled;
    }
    

};

#endif