#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
#include "hardware/structs/scb.h"
#include "nanoberry.h"

#define NANO_PENDSV_SET (1 << 28)

static tcb_t tasks[NANO_MAX_TASKS];
static uint32_t task_count = 0;
static uint32_t current_task = 0;

// Aplicado volatile para evitar problemas de sincronizacao no RP2040
volatile uint32_t *current_sp = NULL;
volatile uint32_t *next_sp = NULL;

static bool kernel_started = false;

static nano_file_t fs[NANO_FS_MAX_FILES];
static char cli_buffer[NANO_CLI_BUF_SIZE];

void nano_init(void) {
    task_count = 0;
    current_task = 0;
    kernel_started = false;
    memset(tasks, 0, sizeof(tasks));
    memset(fs, 0, sizeof(fs));
    
    // Carrega a tabela de arquivos armazenada na Flash SPI
    const uint8_t *flash_target_contents = (const uint8_t *) (XIP_BASE + NANO_FS_FLASH_OFFSET);
    memcpy(fs, flash_target_contents, sizeof(fs));
}

bool nano_task_create(const char *name, task_function_t task_func, void *param, uint32_t priority) {
    if (task_count >= NANO_MAX_TASKS) return false;

    tcb_t *tcb = &tasks[task_count];
    tcb->id = task_count;
    tcb->state = TASK_READY;
    tcb->priority = priority;
    strncpy(tcb->name, name, sizeof(tcb->name) - 1);

    uint32_t *sp = &tcb->stack[NANO_STACK_SIZE];

    // Alinhamento da pilha em 8 bytes
    sp = (uint32_t *)((uint32_t)sp & ~7);

    // Contexto empilhado pelo hardware Cortex-M0+
    *(--sp) = 0x01000000;                // xPSR (Modo Thumb)
    *(--sp) = (uint32_t)task_func;       // PC
    *(--sp) = 0;                         // LR
    *(--sp) = 0;                         // R12
    *(--sp) = 0;                         // R3
    *(--sp) = 0;                         // R2
    *(--sp) = 0;                         // R1
    *(--sp) = (uint32_t)param;           // R0

    // Contexto empilhado via software pelo PendSV_Handler (R4-R7)
    *(--sp) = 0;                         // R7
    *(--sp) = 0;                         // R6
    *(--sp) = 0;                         // R5
    *(--sp) = 0;                         // R4

    tcb->sp = sp;
    task_count++;
    return true;
}

void nano_delay_ms(uint32_t ms) {
    uint64_t target = time_us_64() + (ms * 1000ULL);
    while (time_us_64() < target) {
        nano_yield();
    }
}

void nano_mutex_init(nano_mutex_t *mutex) {
    if (!mutex) return;
    mutex->locked = false;
    mutex->owner = NULL;
}

void nano_mutex_lock(nano_mutex_t *mutex) {
    if (!mutex) return;
    while (mutex->locked) {
        nano_yield();
    }
    mutex->locked = true;
    mutex->owner = &tasks[current_task];
}

void nano_mutex_unlock(nano_mutex_t *mutex) {
    if (!mutex || mutex->owner != &tasks[current_task]) return;
    mutex->locked = false;
    mutex->owner = NULL;
}

static void sync_fs_to_flash(void) {
    uint8_t buffer[FLASH_SECTOR_SIZE];
    memset(buffer, 0, sizeof(buffer));
    memcpy(buffer, fs, sizeof(fs));

    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(NANO_FS_FLASH_OFFSET, FLASH_SECTOR_SIZE);
    flash_range_program(NANO_FS_FLASH_OFFSET, buffer, FLASH_SECTOR_SIZE);
    restore_interrupts(ints);
}

bool nano_fs_write(const char *filename, const char *content) {
    if (!filename || !content) return false;
    bool success = false;

    for (int i = 0; i < NANO_FS_MAX_FILES; i++) {
        if (fs[i].used && strcmp(fs[i].filename, filename) == 0) {
            strncpy(fs[i].data, content, NANO_FS_FILE_SIZE - 1);
            fs[i].data[NANO_FS_FILE_SIZE - 1] = '\0';
            fs[i].size = strnlen(fs[i].data, NANO_FS_FILE_SIZE);
            success = true;
            break;
        }
    }

    if (!success) {
        for (int i = 0; i < NANO_FS_MAX_FILES; i++) {
            if (!fs[i].used) {
                fs[i].used = true;
                strncpy(fs[i].filename, filename, sizeof(fs[i].filename) - 1);
                fs[i].filename[sizeof(fs[i].filename) - 1] = '\0';
                strncpy(fs[i].data, content, NANO_FS_FILE_SIZE - 1);
                fs[i].data[NANO_FS_FILE_SIZE - 1] = '\0';
                fs[i].size = strnlen(fs[i].data, NANO_FS_FILE_SIZE);
                success = true;
                break;
            }
        }
    }

    if (success) {
        sync_fs_to_flash();
    }
    return success;
}

bool nano_fs_read(const char *filename, char *buffer, size_t max_len) {
    if (!filename || !buffer || max_len == 0) return false;

    for (int i = 0; i < NANO_FS_MAX_FILES; i++) {
        if (fs[i].used && strcmp(fs[i].filename, filename) == 0) {
            strncpy(buffer, fs[i].data, max_len - 1);
            buffer[max_len - 1] = '\0';
            return true;
        }
    }
    return false;
}

void nano_fs_list(void) {
    for (int i = 0; i < NANO_FS_MAX_FILES; i++) {
        if (fs[i].used) {
            printf("%s (%lu bytes)\n", fs[i].filename, (unsigned long)fs[i].size);
        }
    }
}

static void handle_cli_command(const char *cmd) {
    char buffer[128];

    if (strcmp(cmd, "ls") == 0) {
        printf("\n--- Arquivos no Nan0berryOS ---\n");
        nano_fs_list();
        printf("-------------------------------\n");
    } else if (strncmp(cmd, "cat ", 4) == 0) {
        const char *filename = cmd + 4;
        if (nano_fs_read(filename, buffer, sizeof(buffer))) {
            printf("\n[%s]: %s\n", filename, buffer);
        } else {
            printf("\nErro: Arquivo '%s' nao encontrado.\n", filename);
        }
    } else if (strncmp(cmd, "write ", 6) == 0) {
        char filename[16] = {0};
        char content[NANO_FS_FILE_SIZE] = {0};

        if (sscanf(cmd + 6, "%15s %[^\n]", filename, content) == 2) {
            if (nano_fs_write(filename, content)) {
                printf("\nSucesso: Arquivo '%s' salvo na Flash.\n", filename);
            } else {
                printf("\nErro: Sem espaco para gravar o arquivo '%s'.\n", filename);
            }
        } else {
            printf("\nUso correto: write <nome_do_arquivo> <conteudo>\n");
        }
    } else if (strcmp(cmd, "help") == 0) {
        printf("\nComandos disponiveis:\n");
        printf("  ls                     - Lista os arquivos salvos\n");
        printf("  cat <nome>             - Le o conteudo de um arquivo\n");
        printf("  write <nome> <texto>   - Grava um arquivo na Flash\n");
        printf("  help                   - Mostra esta lista\n");
    } else {
        printf("\nComando desconhecido: '%s'. Digite 'help'.\n", cmd);
    }
}

void nano_cli_process(void) {
    if (!stdio_usb_connected()) return;

    int bytes = getchar_timeout_us(0);
    if (bytes < 0) return;

    static size_t idx = 0;
    char c = (char)bytes;

    putchar(c);

    if (c == '\r' || c == '\n') {
        cli_buffer[idx] = '\0';
        if (idx > 0) {
            handle_cli_command(cli_buffer);
            idx = 0;
        }
        printf("\nNan0berry> ");
        return;
    }

    if (c == '\b' || c == 0x7F) {
        if (idx > 0) {
            idx--;
            printf("\b \b");
        }
        return;
    }

    if (idx < NANO_CLI_BUF_SIZE - 1) {
        cli_buffer[idx++] = c;
    }
}

static void nano_scheduler(void) {
    if (task_count == 0) return;

    uint32_t best_task = current_task;
    uint32_t highest_prio = 0;

    for (uint32_t i = 0; i < task_count; i++) {
        uint32_t idx = (current_task + 1 + i) % task_count;
        if (tasks[idx].state == TASK_READY && tasks[idx].priority > highest_prio) {
            highest_prio = tasks[idx].priority;
            best_task = idx;
        }
    }

    current_sp = (volatile uint32_t *)&tasks[current_task].sp;
    next_sp = (volatile uint32_t *)&tasks[best_task].sp;
    current_task = best_task;
}

void nano_yield(void) {
    if (task_count <= 1 || !kernel_started) return;

    nano_scheduler();
    scb_hw->icsr = NANO_PENDSV_SET;
    __asm volatile ("isb");
}

void nano_start(void) {
    if (task_count == 0) return;

    kernel_started = true;
    current_task = 0;
    current_sp = NULL;
    next_sp = (volatile uint32_t *)&tasks[0].sp;

    __asm volatile ("msr psp, %0" :: "r" (0));
    scb_hw->icsr = NANO_PENDSV_SET;
    __asm volatile ("isb");
}
