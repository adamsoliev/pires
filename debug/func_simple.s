
/*

int sum(int a, int b) {
  return a + b;
}

int main() {
  int a = sum(293843, 42);
  if (a < 92832344) {
    return 1;
  } else if (a == 17345) {
    return 0;
  }
  return 2;
}
*/

.globl sum
.type sum, @function 
sum:
  # prologue
  addi sp,sp,-32     # allocate space in stack
  sd s0,24(sp)        # save frame ptr at the beginning of the newly alloc space
  addi s0,sp,32      # move frame ptr to the start of the newly alloc space
  # end

  mv a5,a0            # move first argument to eval register 
  mv a4,a1            # move second argument to next eval register 
  add a5,a5,a4        # a + b

  # epilogue
  mv a0,a5            # move result to the return register
  ld s0,24(sp)        # restore frame ptr
  addi sp,sp,32      # reallocate the stack space
  # end
  jr ra


.globl main
.type main, @function

main:
  # prologue
  addi sp,sp,-32     # allocate space in stack
  sd ra,24(sp)       # save return address at first 8 bytes of the newly alloc space
  sd s0,16(sp)       # save frame ptr at the next 8 bytes of the newly alloc space
  addi s0,sp,32      # move frame ptr to the start of the newly alloc space
  # end

  li a0,293843      # load first argument
  li a1,42          # load second argument
  call sum 
  mv a5,a0          # move result to eval register
  li a4,92832344    
  blt a5,a4,.L1     # if a < 92832344, jump to .L1
  li a4,17345
  beq a5,a4,.L2     # if a == 17345, jump to .L2
  li a5,2
  j .L0

.L0:
  # epilogue
  mv a0,a5            # move result to the return register
  ld ra,24(sp)        # restore return address
  ld s0,16(sp)        # restore frame ptr
  addi sp,sp,32      # reallocate the stack space
  # end
  jr ra

.L1:
  li a5,1
  j .L0

.L2:
  li a5,0
  j .L0



