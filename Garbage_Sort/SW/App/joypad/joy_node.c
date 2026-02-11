#include <stdio.h>
#include <linux/joystick.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <zmq.h>
#include <errno.h>
#include <string.h>

#include "common.h"

// 0.0.0.0 znaci da prihvata konekcije sa bilo koje IP adrese u mrezi
#define ZMQ_ENDPOINT "tcp://0.0.0.0:5555"

int main() {
    // --- 1. ZeroMQ Setup ---
    void* context = zmq_ctx_new();
    void* publisher = zmq_socket(context, ZMQ_PUB);
    if(zmq_bind(publisher, ZMQ_ENDPOINT) != 0){
        perror("ZMQ Bind failed");
        return 1;
    }

    // --- 2. Otvaranje Dzojstika ---
    int js_fd = open("/dev/input/js0", O_RDONLY);
    if(js_fd == -1){
        perror("Nema dzojstika na /dev/input/js0");
        return 1;
    }

    printf("Joy Node pokrenut. Cekam pritisak na tastere...\n");

    while(1){
        struct js_event js;

        // Citanje dogadjaja sa dzojstika
        if (read(js_fd, &js, sizeof(struct js_event)) != sizeof(struct js_event)) {
            perror("Greska pri citanju dzojstika");
            break;
        }

        // Proveravamo da li je dogadjaj "pritisak dugmeta"
        // js.type & ~JS_EVENT_INIT sluzi da preskocimo pocetno stanje dzojstika
        if ((js.type & ~JS_EVENT_INIT) == JS_EVENT_BUTTON) {
            
            // Provera da li je indeks dugmeta u opsegu niza (N_BUTTONS je 4)
            if (js.number < N_BUTTONS) {
                // Ako je js.value == 1, dugme je pritisnuto. Ako je 0, pusteno je.
                buttons[js.number] = js.value;
                
                // Debug ispis da vidis u terminalu sta se desava
                if (js.value == 1) {
                    printf("Pritisnuto dugme ID: %d\n", js.number);
                }

                // --- 3. Slanje stanja preko mreze ---
                // Saljemo ceo niz 'buttons' (svih 4 bajta)
                zmq_send(publisher, buttons, N_BUTTONS, 0);
            }
        }
    }

    close(js_fd);
    zmq_close(publisher);
    zmq_ctx_destroy(context);
    return 0;
}