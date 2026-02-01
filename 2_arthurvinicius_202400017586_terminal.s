.data
    .eqv UART_BASE, 0xFFFF0000
    array: .space 4000
    msg_break: .string ","

.text
.globl main

main:
    jal ra, read_int
    mv s1, a0
    blez s1, end_program

    la s2, array
    li t0, 0

read_loop:
    bge t0, s1, start_sort
    mv s3, t0
    jal ra, read_int
    slli t1, s3, 2
    add t2, s2, t1
    sw a0, 0(t2)
    addi t0, s3, 1
    j read_loop

start_sort:
    li t0, 0

outer_loop:
    addi t1, s1, -1
    bge t0, t1, print_all
    li t2, 0
    sub t3, t1, t0

inner_loop:
    bge t2, t3, next_outer
    slli t4, t2, 2
    add t5, s2, t4
    lw t6, 0(t5)
    lw a0, 4(t5)
    ble t6, a0, no_swap
    sw a0, 0(t5)
    sw t6, 4(t5)

no_swap:
    addi t2, t2, 1
    j inner_loop

next_outer:
    addi t0, t0, 1
    j outer_loop

print_all:
    li t0, 0

print_loop:
    bge t0, s1, end_program
    slli t1, t0, 2
    add t2, s2, t1
    lw a0, 0(t2)
    mv s3, t0
    jal ra, print_int
    
    addi t0, s1, -1
    beq s3, t0, skip_comma
    li a0, 44
    jal ra, put_char

skip_comma:
    addi t0, s3, 1
    j print_loop

end_program:
    li a7, 10
    ecall

read_int:
    addi sp, sp, -4
    sw ra, 0(sp)
    li t0, 0
    li t1, 1
    li t2, 0

skip_whitespace:
    jal ra, get_char
    li t3, 45
    beq a0, t3, set_negative
    li t3, 48
    blt a0, t3, check_end
    li t3, 57
    bgt a0, t3, check_end
    
    li t2, 1
    addi a0, a0, -48
    
    # CORRECAO AQUI: Carrega 10 em t4 antes de multiplicar
    li t4, 10
    mul t0, t0, t4
    
    add t0, t0, a0
    j skip_whitespace

check_end:
    bnez t2, finish_read
    j skip_whitespace

set_negative:
    li t1, -1
    li t2, 1
    j skip_whitespace

finish_read:
    mul a0, t0, t1
    lw ra, 0(sp)
    addi sp, sp, 4
    ret

print_int:
    addi sp, sp, -8
    sw ra, 0(sp)
    sw s0, 4(sp)
    mv t0, a0
    
    bnez t0, check_sign
    li a0, 48
    jal ra, put_char
    j end_print_int

check_sign:
    bgez t0, print_digits
    li a0, 45
    mv s0, t0
    jal ra, put_char
    mv t0, s0
    neg t0, t0

print_digits:
    li t1, 0
    li t2, 10

convert_loop:
    beqz t0, pop_loop
    rem t3, t0, t2
    div t0, t0, t2
    addi t3, t3, 48
    addi sp, sp, -4
    sw t3, 0(sp)
    addi t1, t1, 1
    j convert_loop

pop_loop:
    beqz t1, end_print_int
    lw a0, 0(sp)
    addi sp, sp, 4
    mv s0, t1
    jal ra, put_char
    mv t1, s0
    addi t1, t1, -1
    j pop_loop

end_print_int:
    lw s0, 4(sp)
    lw ra, 0(sp)
    addi sp, sp, 8
    ret

get_char:
    li t0, UART_BASE
wait_rx:
    lw t1, 0(t0)
    andi t1, t1, 1
    beqz t1, wait_rx
    lw a0, 4(t0)
    ret

put_char:
    li t0, UART_BASE
wait_tx:
    lw t1, 8(t0)
    andi t1, t1, 1
    beqz t1, wait_tx
    sw a0, 12(t0)
    ret