  .globl main
main:
  addi sp,sp,-32
  sd s0, 24(sp)
  addi s0,sp,32

  li a5, 1
  beq a5, x0, .L2
  li a5, 2
  j .L3

.L2:
  li a5, 3

.L3:
  mv a0,a5
  ld s0,24(sp)
  addi sp,sp,32
  jr ra
