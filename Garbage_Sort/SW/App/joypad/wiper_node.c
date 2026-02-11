#include <stdio.h>
#include <zmq.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdint.h>

#include "common.h"

#define ZMQ_ENDPOINT "tcp://rpi-controls.local:5555" // Ili IP adresa
#define DEV_FN "/dev/gpio_stream"

// Stanja u kojima se sistem moze naci
enum State {
    ST_IDLE,        // Motor stoji
    ST_GOING_LEFT,  // Putuje ka levom (23)
    ST_GOING_RIGHT, // Putuje ka desnom (24)
    ST_GOING_MIDDLE // Putuje ka srednjem (22)
};

int current_state = ST_IDLE;
int last_known_side = 0; // 0 = bili smo levo, 1 = bili smo desno (da znamo kako do sredine)

// --- Funkcije za hardver ---

// Ocitava vrednost pina (1 ili 0)
int gpio_read(int fd, int pin) {
    uint8_t req[2] = {'r', (uint8_t)pin};
    uint8_t val = 0;
    if(write(fd, req, 2) != 2) return 0;
    usleep(100); // Kratka pauza da drajver stigne da obradi
    if(read(fd, &val, 1) != 1) return 0;
    return val; // Vraca 1 ako je pritisnut, 0 ako nije
}

// Upisuje vrednost na pin
void gpio_write(int fd, int pin, int val) {
    uint8_t pkg[3] = {'w', (uint8_t)pin, (uint8_t)val};
    write(fd, pkg, 3);
}

void motor_stop(int fd) {
    gpio_write(fd, 2, 0); // Enable = 0
}

void motor_move_left(int fd) {
    // Podesi smer (CCW)
    gpio_write(fd, 3, 1);
    gpio_write(fd, 4, 0);
    // Start (Enable = 1)
    gpio_write(fd, 2, 1);
}

void motor_move_right(int fd) {
    // Podesi smer (CW)
    gpio_write(fd, 3, 0);
    gpio_write(fd, 4, 1);
    // Start (Enable = 1)
    gpio_write(fd, 2, 1);
}

// --- Main ---

int main() {
    // 1. Otvaranje drajvera
    int gpio_fd = open(DEV_FN, O_RDWR);
    if(gpio_fd < 0){
        perror("Failed to open GPIO driver");
        return 1;
    }

    // 2. ZeroMQ Setup
    void* context = zmq_ctx_new();
    void* subscriber = zmq_socket(context, ZMQ_SUB);
    zmq_connect(subscriber, ZMQ_ENDPOINT);
    zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, "", 0);

    printf("Wiper Node spreman. Cekam komande...\n");

    while(1){
        // --- A. Prijem komandi (Non-blocking) ---
        zmq_msg_t msg;
        zmq_msg_init(&msg);
        int bytes = zmq_msg_recv(&msg, subscriber, ZMQ_DONTWAIT);
        
        if(bytes == N_BUTTONS){
            memcpy(buttons, zmq_msg_data(&msg), bytes);
            
            // Logika za promenu stanja na osnovu dugmeta
            // Koristimo "else if" da ne bi primio dve komande odjednom
            
            if(buttons[BTN_IDI_LEVO]) { 
                printf("Komanda: IDI LEVO (ka pinu 23)\n");
                current_state = ST_GOING_LEFT;
            }
            else if(buttons[BTN_IDI_DESNO]) {
                printf("Komanda: IDI DESNO (ka pinu 24)\n");
                current_state = ST_GOING_RIGHT;
            }
            else if(buttons[BTN_IDI_SREDINA]) {
                printf("Komanda: IDI NA SREDINU (ka pinu 22)\n");
                current_state = ST_GOING_MIDDLE;
            }
        }
        zmq_msg_close(&msg);

        // --- B. Citanje Senzora ---
        int sw_levo = gpio_read(gpio_fd, 23);
        int sw_sred = gpio_read(gpio_fd, 22);
        int sw_desno = gpio_read(gpio_fd, 24);

        // Azuriranje "poslednje poznate strane" (bitno za sredinu)
        if (sw_levo) last_known_side = 0; // Leva strana
        if (sw_desno) last_known_side = 1; // Desna strana

        // --- C. Izvrsavanje (State Machine) ---
        
        switch(current_state) {
            case ST_IDLE:
                motor_stop(gpio_fd);
                break;

            case ST_GOING_LEFT:
                if(sw_levo) {
                    printf("Stigao na LEVI cilj (23). Stajem.\n");
                    current_state = ST_IDLE;
                } else {
                    motor_move_left(gpio_fd);
                }
                break;

            case ST_GOING_RIGHT:
                if(sw_desno) {
                    printf("Stigao na DESNI cilj (24). Stajem.\n");
                    current_state = ST_IDLE;
                } else {
                    motor_move_right(gpio_fd);
                }
                break;

            case ST_GOING_MIDDLE:
                if(sw_sred) {
                    printf("Stigao na SREDNJI cilj (22). Stajem.\n");
                    current_state = ST_IDLE;
                } 
                else {
                    // Ako nismo na sredini, moramo znati odakle dolazimo
                    if(last_known_side == 0) {
                        // Bili smo levo, znaci moramo desno da nadjemo sredinu
                        motor_move_right(gpio_fd);
                    } else {
                        // Bili smo desno, znaci moramo levo da nadjemo sredinu
                        motor_move_left(gpio_fd);
                    }
                }
                break;
        }

        usleep(10000); // 10ms loop delay
    }

    close(gpio_fd);
    zmq_close(subscriber);
    zmq_ctx_destroy(context);
    return 0;
}