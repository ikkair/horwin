#include QMK_KEYBOARD_H
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "digitizer.h"

// Uncomment below to use predefined keycode
// #define ENUM_KEYCODE


// Profile Struct
typedef struct Profile{
    uint8_t posX;
    uint8_t posY;
    uint16_t numeratorX;
    uint16_t denominatorX;
    uint16_t numeratorY;
    uint16_t denominatorY;
    struct Profile *next;
    struct Profile *prev;
}Profile;

// Global Variable Declaration
bool fast_horwin = false;
bool click_horwin = false;
Profile *Active = NULL;

// Functions Declaration Horwin
void horwin_point(double, double);
void horwin_coordinate(uint8_t, uint8_t);
void horwin_reset(void);
void horwin_in(void);
void horwin_out(void);
void horwin_fast_toggle(void);
void horwin_save_fast(void);
void horwin_click(void);
void horwin_loop_out(uint16_t*, uint16_t*);
void horwin_adder(uint16_t*, uint16_t*, uint8_t*);

// Profile
Profile* initProfile(void);
void movRight(void);
void movLeft(void);
void addProfile(void);
void delProfile(void);

// Pre defined keycode
#ifdef ENUM_KEYCODE
enum custom_keycodes{
    // Horwin Button
    HWLT = SAFE_RANGE, //Left Top
    HWMT, //Middle Top
    HWRT, //Right Top
    HWLM, //Left Middle
    HWMM, //Middle Middle
    HWRM, //Right Middle
    HWLB, //Left Bottom
    HWMB, //Middle Bottom
    HWRB, //Right Bottom
    HWRES, //Restart
    HWIN, //Going Inside Window
    HWOUT, //Going Outside Window
    HWNEXT, //Change to Next Profile
    HWPREV, //Change to Previous Profile
    HWADDR, //Add Profile to the Right
    HWCLICK, //Click
    HWDEL, //Delete Profile
    HWFAST //Toggle Fast Horwin
};

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode){
        // Horwin Keys
        // TOP
        case HWLT:
            if (record->event.pressed) {
                horwin_coordinate(0,0);
            }
        break;
        case HWMT:
            if (record->event.pressed) {
                horwin_coordinate(1,0);
            }
        break;
        case HWRT:
            if (record->event.pressed) {
                horwin_coordinate(2,0);
            }
        break;
        // MID
        case HWLM:
            if (record->event.pressed) {
                horwin_coordinate(0,1);
            }
        break;
        case HWMM:
            if (record->event.pressed) {
                horwin_coordinate(1,1);
            }
        break;
        case HWRM:
            if (record->event.pressed) {
                horwin_coordinate(2,1);
            }
        break;
        // BOTTOM
        case HWLB:
            if (record->event.pressed) {
                horwin_coordinate(0,2);
            }
        break;
        case HWMB:
            if (record->event.pressed) {
                horwin_coordinate(1,2);
            }
        break;
        case HWRB:
            if (record->event.pressed) {
                horwin_coordinate(2,2);
            }
        break;
        // RESET
        case HWRES:
            if (record->event.pressed) {
                horwin_reset();
            }
        break;
        // IN
        case HWIN:
            if (record->event.pressed) {
                horwin_in();
            }
        break;
        // OUT
        case HWOUT:
            if (record->event.pressed) {
                horwin_out();
            }
        break;
        // Profile Previous 
        case HWPREV:
            if (record->event.pressed) {
                movLeft();
            }
        break;
        // Profile Next
        case HWNEXT:
            if (record->event.pressed) {
                movRight();
            }
        break;
        // Profile Add
        case HWADDR:
            if (record->event.pressed) {
                addProfile();
            }
        break;
        // Click
        case HWCLICK:
            if (record->event.pressed) {
                horwin_click();
            }
        break;
        // Delete Current Profile
        case HWDEL:
            if (record->event.pressed) {
                delProfile();
            }
        break;
        // TOGGLE FAST
        case HWFAST:
            if (record->event.pressed) {
                horwin_fast_toggle();
            }
        break;
    }
    return true;   
}
#endif


// Creating Nodes
Profile* initProfile(void){
    Profile *temp;
    temp = (Profile*)malloc(sizeof(Profile));
    if (temp != NULL){
        temp->posX = 0;
        temp->posY = 0;
        temp->numeratorX = 1;
        temp->denominatorX = 1;
        temp->numeratorY = 1;
        temp->denominatorY = 1;
        temp->next = temp;
        temp->prev = temp;
    } else {
        horwin_point(0,0);
    }
    return temp;
}

// Move to Next Profile
void movRight(void){
    Active = Active->next;
    horwin_save_fast();
}

// Move to Prev Profile
void movLeft(void){
    Active = Active->prev;
    horwin_save_fast();
}

// Add Profile to the Right
void addProfile(void){
    Profile *temp = initProfile();
    if (temp != NULL){
        temp->next = Active->next;
        temp->prev = Active;
        Active->next->prev = temp;
        Active->next = temp;
        Active = temp;
        horwin_save_fast();
    } else {
        horwin_point(0,0);
    }
}

// Delete Current Profile
void delProfile(void){
    if (Active != NULL){
        Profile *temp = Active;
        Active = Active->prev;
        if (Active != temp){
            temp->next->prev = Active;
            Active->next = temp->next;
            free(temp);
        } else {
            horwin_reset();
        }
    }
    horwin_save_fast();
}

// To Click
void horwin_click(void){
    click_horwin = true;
    horwin_coordinate(Active->posX, Active->posY);
    click_horwin = false;
}

// To use digitizer
void horwin_point(double x, double y){
    digitizer_t digitizer;
    digitizer.x = x;
    digitizer.y = y;
    digitizer.tipswitch = click_horwin;
    digitizer.inrange = 1;
    digitizer_set_report(digitizer);
}

// To use digitizer as coordinate
void horwin_coordinate(uint8_t x, uint8_t y){
    if (Active == NULL)
        Active = initProfile();
    Active->posX = x;
    Active->posY = y;
    if(fast_horwin != true){
        horwin_point(
            ((Active->denominatorX >= 3?((double)Active->numeratorX/Active->denominatorX):0)+((((double)1/(Active->denominatorX*3)*x))+(((double)1/(Active->denominatorX*3))/2))),
            ((Active->denominatorY >= 3?((double)Active->numeratorY/Active->denominatorY):0)+((((double)1/(Active->denominatorY*3)*y))+(((double)1/(Active->denominatorY*3))/2)))
        );
    } else {
        horwin_in();
    }
}

// To multiply the adder
void horwin_adder( uint16_t *adderNumerator, uint16_t *adderDenominator, uint8_t *pos){
    if(*adderDenominator >= 3){
        *adderNumerator *= 3;
        *adderNumerator += *pos;
    } else {
        *adderNumerator = *pos;
    }
    *adderDenominator *= 3;
}

// to get inside a window
void horwin_in(){
    // if to prevent variable overflow
    if (Active->denominatorX * 3 > Active->denominatorX && Active->denominatorY * 3 > Active->denominatorY){
        horwin_adder(&Active->numeratorX, &Active->denominatorX, &Active->posX);
        horwin_adder(&Active->numeratorY, &Active->denominatorY, &Active->posY);
    }
    horwin_save_fast();
}

// To reset coordinate
void horwin_reset(){
    Active->numeratorX = 1;
    Active->denominatorX = 1;
    Active->numeratorY = 1;
    Active->denominatorY = 1;
    horwin_save_fast();
}

// Logic to get out of a window
void horwin_loop_out(uint16_t *numerator,  uint16_t *denominator){
    if(*denominator/3 > 1){
        while (*numerator % 3 != 0){
            *numerator -= 1;
        }
        *numerator /= 3;
    }
    *denominator /= *denominator/3 >= 1 ? 3 : 1;
}

// To get out of a window
void horwin_out(){
    horwin_loop_out(&Active->numeratorX, &Active->denominatorX);
    horwin_loop_out(&Active->numeratorY, &Active->denominatorY);
}

// Toggle fast horwin
void horwin_fast_toggle(){
    fast_horwin = !fast_horwin;
}

// To safely refresh coordinate
void horwin_save_fast(){
    if(fast_horwin == true){
        horwin_fast_toggle();
        horwin_coordinate(1,1);
        horwin_fast_toggle();
    } else {
        horwin_coordinate(1,1);
    }
}
