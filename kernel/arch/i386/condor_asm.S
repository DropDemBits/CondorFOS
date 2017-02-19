
.macro readCR creg_num
.globl readCR\creg_num\()
readCR\creg_num\():
    movl %cr\creg_num\(), %eax
    ret
.endm

readCR 0
readCR 1
readCR 2
readCR 3
readCR 4
readCR 8
