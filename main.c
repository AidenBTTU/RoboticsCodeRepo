#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include "pico/stdlib.h"
#include "pico/bootrom.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"

#ifndef GPIO_FUNC_PWM
#define GPIO_FUNC_PWM 2
#endif

// ── Pin definitions ───────────────────────────────────────────────────────────
#define LFWD_PIN     2
#define LREV_PIN     3
#define RFWD_PIN     4
#define RREV_PIN     5
#define LSIDE_INT_PIN 15
#define RSIDE_INT_PIN 14

// ── State machine variables ─────────────────────────────────────────────────
void init_pins() {
    gpio_set_function(LREV_PIN, GPIO_FUNC_PWM);
    gpio_set_function(LFWD_PIN, GPIO_FUNC_PWM);
    gpio_set_function(RFWD_PIN, GPIO_FUNC_PWM);
    gpio_set_function(RREV_PIN, GPIO_FUNC_PWM);
}


volatile int x_pos = 105; // our starting value is in the middle of the field 
volatile int y_pos = 0; // our starting value is 0
int heading = 0;
int max_x = 205;    //we can move roughly 205cm to the right from 0,0
int min_x = 10;      
int max_y = 245;    //we can move roughly 245cm towards the enemy
int min_y = 0;      
    
void turn(bool direction, short angleChange, int speed) { // 0 == LEFT, 1 == RIGHT, angleChange in DEGREES, speed [0 - 100]
    int duty_cycle = (speed * 65535) / 100;
    int pastHeading = heading;
    if(direction == 0) {
        int futureHeading = pastHeading - angleChange;
        if (futureHeading < -180) {
            futureHeading += 360;
        }
        while(heading < futureHeading + 2 || heading > futureHeading - 2) {
            //UPDATE HEADING HERE IF NOT AUTOMATIC
            pwm_set_gpio_level(LREV_PIN, duty_cycle);
            pwm_set_gpio_level(LFWD_PIN, 0);
            pwm_set_gpio_level(RFWD_PIN, duty_cycle);
            pwm_set_gpio_level(RREV_PIN, 0);
            sleep_ms(5);
        }
        pwm_set_gpio_level(LREV_PIN, 0);
        pwm_set_gpio_level(LFWD_PIN, 0);
        pwm_set_gpio_level(RFWD_PIN, 0);
        pwm_set_gpio_level(RREV_PIN, 0);
        return;
    }
    else {
        int futureHeading = pastHeading + angleChange;
        if (futureHeading > 180) {
            futureHeading -= 360;
        }
        while(heading < futureHeading + 2 || heading > futureHeading - 2) {
            //UPDATE HEADING HERE IF NOT AUTOMATIC
            pwm_set_gpio_level(LREV_PIN, 0);
            pwm_set_gpio_level(LFWD_PIN, duty_cycle);
            pwm_set_gpio_level(RFWD_PIN, 0);
            pwm_set_gpio_level(RREV_PIN, duty_cycle);
            sleep_ms(5);
        }
        pwm_set_gpio_level(LREV_PIN, 0);
        pwm_set_gpio_level(LFWD_PIN, 0);
        pwm_set_gpio_level(RFWD_PIN, 0);
        pwm_set_gpio_level(RREV_PIN, 0);
        return;
    }
}

void move(bool DIR, int distance, int speed) { //DIR 1 = FWD, 0 = REV, distance in cm, speed [0 - 100]
    int start_pos_x = x_pos;
    int start_pos_y = y_pos;
    int leftCount = 0; 
    int rightCount = 0;
    int heading = 0; //IMU HEADING NUMBER?
    int x_distance = distance * cos(heading * M_PI / 180);
    int y_distance = distance * sin(heading * M_PI / 180);

    // Set direction pins
    while((x_pos < start_pos_x + x_distance) && (x_pos < max_x) && (y_pos < max_y) && (y_pos > min_y) && (x_pos > min_x)) {
        //UPDATE ENCODER COUNTS AND HEADING  
        leftCount = 0; //encoder_delta(&left_enc);
        rightCount = 0; //encoder_delta(&lright_enc);
        x_pos += ((leftCount + rightCount) / 2) * cos(heading * M_PI / 180) * (8*M_PI/1920); // 8cm wheel diameter, 1920 counts per revolution
        y_pos += ((leftCount + rightCount) / 2) * sin(heading * M_PI / 180) * (8*M_PI/1920); 
        
        
        // Set PWM duty cycle based on speed
        int duty_cycle = (speed * 65535) / 100; // Convert percentage to 16-bit value
        if (DIR) { // Forward PWM
            pwm_set_gpio_level(LFWD_PIN, duty_cycle);
            pwm_set_gpio_level(LREV_PIN, 0); } 
        else {
            pwm_set_gpio_level(LFWD_PIN, 0);
            pwm_set_gpio_level(LREV_PIN, duty_cycle); }
        if (DIR) {
            pwm_set_gpio_level(RFWD_PIN, duty_cycle);
            pwm_set_gpio_level(RREV_PIN, 0); } 
        else {
            pwm_set_gpio_level(RFWD_PIN, 0);
            pwm_set_gpio_level(RREV_PIN, duty_cycle); }
        //go direction for at least 10ms.
        sleep_ms(10);
        
    }
    //When we have reached the target position, 
    //or hit a boundary, stop the motors
    pwm_set_gpio_level(LFWD_PIN, 0);
    pwm_set_gpio_level(LREV_PIN, 0);
    pwm_set_gpio_level(RFWD_PIN, 0);
    pwm_set_gpio_level(RREV_PIN, 0);
    return;
}

typedef void (*StateFunc)(void);
StateFunc current_state;

void state_initial(void) {
    // move(y_dist to middle line ish)
    // transfer to captu
}
void state_goalie(void) {
    if (/* trigger */) current_state = state_search;
}
void state_capture(void) { // Search and capture
    // Poll camera
    // IF(Ball/blob Detected)
        // ball_x, ball_y = find_blobs
        // if ball_x < (middle_x + ((max_x) / 6)) { //left third of field
            // turn(0, 10); //turn left for 10 degrees
        // }
        // if ball_x > (middle_x + ((max_x) / 6)) {
        
        // }
    // ELSE {
    //      
    //      Turn right __ increment?}
    
}
void state_deposit(void) { // SCORE

}
// In main loop:
while (current_state) {
    current_state();
}


