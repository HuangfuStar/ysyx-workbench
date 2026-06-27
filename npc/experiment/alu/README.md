
# 4-bit Signed ALU Experiment

Directory layout follows the `nvboard/example` style.

Input mapping:
- `SW7..SW4`: operand `A[3:0]`
- `SW3..SW0`: operand `B[3:0]`
- `BTNL..BTNC`: operation select `op[2:0]`

Output mapping:
- `LED3..LED0`: result `C[3:0]`
- `LED4`: zero flag `ZF`
- `LED5`: overflow flag `OF`
- `LED6`: carry flag `CF`
- `HEX0`: result decoded as hexadecimal digit

ALU functions:
- `000`: `A + B`
- `001`: `A - B`
- `010`: `~A`
- `011`: `A & B`
- `100`: `A | B`
- `101`: `A ^ B`
- `110`: signed compare `A < B`
- `111`: equality compare `A == B`
