
.globl idle_process

idle_process:
    hlt
    jmp idle_process
