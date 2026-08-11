#include <stdio.h>
#include "pico/stdlib.h"
#include "nanoberry.h"

void task_blink(void *param) {
    (void)param;
#ifdef PICO_DEFAULT_LED_PIN
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
#endif

    while (1) {
#ifdef PICO_DEFAULT_LED_PIN
        gpio_put(LED_PIN, 1);
        nano_delay_ms(250);
        gpio_put(LED_PIN, 0);
        nano_delay_ms(250);
#else
        nano_delay_ms(500);
#endif
    }
}

void task_shell(void *param) {
    (void)param;
    while (1) {
        nano_cli_process();
        nano_delay_ms(10);
    }
}

int main(void) {
    stdio_init_all();
    nano_init();

    // Cria arquivo inicial se nao existir na Flash
    char read_buffer[64];
    if (!nano_fs_read("welcome.txt", read_buffer, sizeof(read_buffer))) {
        nano_fs_write("welcome.txt", "Nan0berryOS em Flash Permanente");
    }

    // Registra as tarefas
    nano_task_create("blink", task_blink, NULL, 1);
    nano_task_create("shell", task_shell, NULL, 2);

    // Inicia o Kernel
    nano_start();

    while (1);
    return 0;
}
