/*
int main() {
    int a = 32;
    char *str = "Hello World";
    prinf("String: %s and int: %d\n", str, a);
    return 0;
}
*/

.globl main
.type main, @function

.LCO:
    .string "String: %s and int: %d\n"

.LC1:
    .string "Hello World"

main:
  # prologue
  addi sp,sp,-32     # allocate space in stack
  sd ra,24(sp)       # save return address at first 8 bytes of the newly alloc space
  sd s0,16(sp)       # save frame ptr at the next 8 bytes of the newly alloc space
  addi s0,sp,32      # move frame ptr to the start of the newly alloc space
  # end

  li a2,32              # load 32 to a2
  lui a5,%hi(.LC1)
  addi a1,a5,%lo(.LC1)  # load addr of 'Hello World' to a1
  lui a5,%hi(.LCO)
  addi a0,a5,%lo(.LCO)  # load addr of format string to a0
  call printf

  li a5,0

  # epilogue
  mv a0,a5            # move result to the return register
  ld ra,24(sp)        # restore return address
  ld s0,16(sp)        # restore frame ptr
  addi sp,sp,32      # reallocate the stack space
  # end
  jr ra
