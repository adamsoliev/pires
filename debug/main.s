.globl main
main:
  addi sp, sp, -32
  sd s0, 24(sp)
  addi s0, sp, 32

  li a0, 3
  sw a0, -28(s0)
  addi a5, s0, -28
  mv a1, a5
  sw a1, -24(s0)
  ld a5, -24(s0)
  lw a5, 0(a5)

  mv a0,a5
  ld s0,24(sp)
  addi sp, sp, 32
  jr ra 
