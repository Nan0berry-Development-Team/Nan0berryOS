.syntax unified
.cpu cortex-m0plus
.thumb

.global PendSV_Handler
.type PendSV_Handler, %function

PendSV_Handler:
    cpsid i                     /* Desabilita interrupcoes globais */

    /* 1. Verifica se e o primeiro boot do kernel (PSP == 0) */
    mrs r0, psp
    cmp r0, #0
    beq skip_save               /* Se for 0, nao salva contexto anterior */

    /* 2. Salva R4-R7 na pilha da tarefa saindo */
    subs r0, r0, #16
    ldr r1, =current_sp
    ldr r1, [r1]
    cmp r1, #0
    beq skip_save

    str r4, [r0, #0]
    str r5, [r0, #4]
    str r6, [r0, #8]
    str r7, [r0, #12]
    
    /* Atualiza o SP no TCB da tarefa atual */
    str r0, [r1]

skip_save:
    /* 3. Carrega o SP da proxima tarefa (next_sp) */
    ldr r1, =next_sp
    ldr r1, [r1]
    ldr r0, [r1]                /* r0 recebe o SP da nova tarefa */

    /* 4. Restaura R4-R7 da nova tarefa */
    ldr r4, [r0, #0]
    ldr r5, [r0, #4]
    ldr r6, [r0, #8]
    ldr r7, [r0, #12]
    adds r0, r0, #16            /* Ajusta o SP apos restaurar R4-R7 */

    /* 5. Atualiza o registrador PSP */
    msr psp, r0

    cpsie i                     /* Reabilita interrupcoes */

    /* 6. Retorna ao Thread Mode usando PSP (EXC_RETURN = 0xFFFFFFFD) */
    ldr r1, =0xFFFFFFFD
    mov lr, r1
    bx lr
