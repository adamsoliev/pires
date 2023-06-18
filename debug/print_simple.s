/*
int main() {
    int a = 32;
    prinf("%d\n", a);
    return 0;
}
*/

.globl main
.type main, @function

.LCO:
    .string "%d\n"

main:
  # prologue
  addi sp,sp,-32     # allocate space in stack
  sd ra,24(sp)       # save return address at first 8 bytes of the newly alloc space
  sd s0,16(sp)       # save frame ptr at the next 8 bytes of the newly alloc space
  addi s0,sp,32      # move frame ptr to the start of the newly alloc space
  # end

  li a5,32
  mv a1,a5
  lui a5,%hi(.LCO)
  addi a0,a5,%lo(.LCO)
  call printf

  li a5,0
  # epilogue
  mv a0,a5            # move result to the return register
  ld ra,24(sp)        # restore return address
  ld s0,16(sp)        # restore frame ptr
  addi sp,sp,32      # reallocate the stack space
  # end
  jr ra
