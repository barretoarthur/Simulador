
# Code section
.section .text
# Factorial recursive function
factorial:
    # begin
    begin:
        # Prologue
        addi sp, sp, -16
        sw ra, 0(sp)
    # Base case
    base:
        # Check if n != 0
        bnez a0, recursive
        # Returning 1
        li t0, 1
        j end
    # Recurrence
    recursive:
        # Saving n in stack
        sw a0, 4(sp)
        # factorial(n - 1)
        addi a0, a0, -1
        call factorial
        # Restoring n from stack
        lw a0, 4(sp)
        # Calculating n * fatorial(n - 1)
        mul t0, t0, a0
    # end
    end:
        # Epilogue
        lw ra, 0(sp)
        addi sp, sp, 16
        # Returning value
        mv a0, t0
        ret

# Main function
.global main
main:
    # Prologue
    addi sp, sp, -16
    sw ra, 0(sp)
    # factorial(5)
    li a0, 5
    call factorial
    # Epilogue
    lw ra, 0(sp)
    addi sp, sp, 16
    # Setting return value to zero (success)
    addi a0, zero, 0
    # Returning from call
    ret

    # ta massa pai