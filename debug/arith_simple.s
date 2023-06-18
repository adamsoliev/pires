/*

# 1 + 2 + 3 + 4

int main() {
  int a = 1 + 2 + 3 + 4;
  if (a < 10) {
    return 1;
  } else if (a == 10) {
    return 0;
  }
  return 2;
*/

.globl main
.type main, @function

main:
  addi sp,sp,-32     # allocate space in stack
  sd s0,24(sp)        # save frame ptr at the beginning of the newly alloc space
  addi s0,sp,32      # move frame ptr to the start of the newly alloc space
  # end

  li a2,10
  li t0, 10           # 10
  blt a0, t0, .L1     # if result < 10, goto .L1
  beq a0, t0, .L2     # if result == 10, goto .L2

  mv a0, a1
  j .L0

.L0:
  ld s0,24(sp)        # restore frame ptr
  addi sp,sp,32      # reallocate the stack space
  jr ra

.L1:
  mv a0, a0
  j .L0

.L2:
  mv a0, zero
  j .L0



