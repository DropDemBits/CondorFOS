/*
 * Copyright (C) 2017 DropDemBits <r3usrlnd@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

.globl idle_process, dummy_process, tss_set_esp0
.extern p_current, p_last, tss_start

idle_process:
    hlt
    jmp idle_process

dummy_process:
    jmp dummy_process

tss_set_esp0:
    push %ebp
    movl %esp, %ebp
    push %eax

    # Current Stack state (EBP relative)
    #  ___________
    # |           |
    # |    EBP    |    0
    # |___________|
    # |           |
    # |  RET ADDR |    4
    # |___________|
    # |           |
    # |   ESP0    |    8
    # |___________|
    # |           |

    movl 8(%ebp), %eax
    movl %eax, (tss_start + 4)

    pop %eax
    pop %ebp
    ret

get_eip:
    popl %eax
    jmp *%eax

switch_process:
    push %eax
    push %edx
    push %ecx

    # TODO: Complete this

    popl %ecx
    popl %edx
    popl %ecx
