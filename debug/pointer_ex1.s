/*

int main() {
    int a = 13;
    int *ptr = &a;
    if (*ptr > 13) {
        return 0;
    } else if (*ptr == 13) {
        return 1;
    } 
    return 2;
}
*/

.globl main
.type main, @function

main:
    # prologue
    addi sp, sp, -16
    sd s0, 8(sp)
    addi s0, sp, 16
    # end

    li a5,13           # load 13
    sw a5,-28(s0)      # store 13 to addr
    addi a5,s0, -28    # put addr to a5
    sd a5,-24(s0)      # store addr to ptr 
    ld a5,-24(s0)      # load ptr to a5
    lw a5,0(a5)        # load *ptr to a5
    li a4,13           # load 13
    bgt a5,a4, .L1      # if *ptr > 13, goto L1
    li a4,13
    beq a5,a4, .L2      # if *ptr == 23, goto L2

    li a5,2
    j .L0

.L0:
    # epilogue
    mv a0,a5
    ld s0, 8(sp)
    addi sp, sp, 16
    jr ra
    # end

.L1:
    li a5, 0
    j .L0

.L2:
    li a5, 1
    j .L0

