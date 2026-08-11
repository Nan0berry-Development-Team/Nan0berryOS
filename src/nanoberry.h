#ifndef NANOBERRY_H
#define NANOBERRY_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define NANO_MAX_TASKS       8
#define NANO_STACK_SIZE      256
#define NANO_FS_MAX_FILES    4
#define NANO_FS_FILE_SIZE    128
#define NANO_CLI_BUF_SIZE    64

// Deslocamento de 1MB na Flash para salvar arquivos de forma permanente
#define NANO_FS_FLASH_OFFSET (1024 * 1024)

// Ponteiros globais de pilha com volatile para seguranca no hardware real
extern volatile uint32_t *current_sp;
extern volatile uint32_t *next_sp;

typedef void (*task_function_t)(void *param);

typedef enum {
    TASK_READY,
    TASK_RUNNING,
    TASK_BLOCKED,
    TASK_SLEEPING
} task_state_t;

typedef struct {
    uint32_t *sp;
    uint32_t stack[NANO_STACK_SIZE] __attribute__((aligned(8)));
    uint32_t id;
    char name[16];
    task_state_t state;
    uint32_t priority;
    uint32_t sleep_ticks;
} tcb_t;

typedef struct {
    bool locked;
    tcb_t *owner;
} nano_mutex_t;

typedef struct {
    bool used;
    char filename[16];
    char data[NANO_FS_FILE_SIZE];
    size_t size;
} nano_file_t;

void nano_init(void);
bool nano_task_create(const char *name, task_function_t task_func, void *param, uint32_t priority);
void nano_start(void);
void nano_yield(void);
void nano_delay_ms(uint32_t ms);

void nano_mutex_init(nano_mutex_t *mutex);
void nano_mutex_lock(nano_mutex_t *mutex);
void nano_mutex_unlock(nano_mutex_t *mutex);

bool nano_fs_write(const char *filename, const char *content);
bool nano_fs_read(const char *filename, char *buffer, size_t max_len);
void nano_fs_list(void);

void nano_cli_process(void);

#endif // NANOBERRY_H
