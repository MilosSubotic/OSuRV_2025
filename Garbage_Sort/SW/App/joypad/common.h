#pragma once
#include <stdint.h>
#include <stdio.h>

// Povećali smo na 4 jer ti je najveći ID dugmeta 3
#define N_BUTTONS 4 

// Tvoje mapiranje sa džojstika:
#define BTN_IDI_DESNO  1  // Tvoje "dugme 2" (fizički) koje daje ID 1
#define BTN_IDI_LEVO   2  // Tvoje "dugme 4" (fizički) koje daje ID 2
#define BTN_IDI_SREDINA 3 // Tvoje "dugme 1" (fizički) koje daje ID 3

static uint8_t buttons[N_BUTTONS];

static void print_buttons(const char* msg) {
    // Samo za debug
    printf("%s: [%d %d %d %d]\n", msg, buttons[0], buttons[1], buttons[2], buttons[3]);
}