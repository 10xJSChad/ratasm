```
REGISTERS:
r1, r2, r3, r4, r5

INSTRUCTIONS:
- mov [dest] [src]
- add [dest] [src]
- sub [dest] [src]
- mul [x] [y] # Result stored in r5
- div [x] [y] # Result stored in r5
- cmp [x] [y]

- jmp [label]
- je  [label]
- jg  [label]
- jl  [label]
- jne [label]
- jge [label]
- jle [label]

Sample program, prints every number from 1 to 10:
  mov r1 0
loop:
  add r1 1
  printint r1
  printchar 10
  cmp r1 10
  jne loop
```
