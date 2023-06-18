
/*


int main() {
  int b[2] = {1230,22324};
  int a = b[0] + b[1];
  if (a < 12945) {
    return 1;
  } else if (a == 324871) {
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

  li a5,1230          # thought 'd get "doesn't fit in 12 bits" error, but it's fine
  li a6,22324       
  add a5,a5,a6        # b[0] + b[1]
  li a6,12945
  blt a5,a6,.L1      # if (a < 12945) goto .L1
  li a6,324871
  beq a5,a6,.L2      # if (a == 324871) goto .L2
  li a5,2
  j .L0

.L0:
  # epilogue
  mv a0,a5            # move result to the return register
  ld s0,92(sp)        # restore frame ptr
  addi sp,sp,100      # reallocate the stack space
  # end

  jr ra

.L1:
  li a5,1
  j .L0

.L2:
  li a5,0
  j .L0




