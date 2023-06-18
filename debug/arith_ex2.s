
/*

# (1 + 2) - 3 * 4

int main() {
  int a = (1 + 12) - 3 * 4;
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
  # prologue
  addi sp,sp,-100     # allocate space in stack
  sd s0,92(sp)        # save frame ptr at the beginning of the newly alloc space
  addi s0,sp,100      # move frame ptr to the start of the newly alloc space
  # end

  li a0,1
  li a1,12
  add a0,a0,a1 # 1 + 2
  li a1,3
  li a2,4
  mul a1,a1,a2 # 3 * 4
  sub a0,a0,a1 # (1 + 12) - 3 * 4
  li a1,10
  blt a0,a1,.L1 # if (a < 10) goto .L1
  beq a0,a1,.L2 # if (a == 10) goto .L2

  li a0,2
  j .L0

.L0:
  # epilogue
  ld s0,92(sp)        # restore frame ptr
  addi sp,sp,100      # reallocate the stack space
  # end
  jr ra

.L1:
  li a0,1
  j .L0

.L2:
  li a0,0
  j .L0



