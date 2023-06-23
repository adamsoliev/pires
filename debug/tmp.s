  .globl main
main:
  addi sp, sp, -32
  sd s0, 24(sp)
  addi s0, sp, 32
  call ret32
  mv a5, a0
  j .L.return.main
.L.return.main:
  mv a0,a5
  ld s0,24(sp)
  addi sp, sp, 32
  jr ra
  .globl ret32
ret32:
  addi sp, sp, -32
  sd s0, 24(sp)
  addi s0, sp, 32
  li a5, 32
  j .L.return.ret32
.L.return.ret32:
  mv a0,a5
  ld s0,24(sp)
  addi sp, sp, 32
  jr ra
