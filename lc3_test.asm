.ORIG x3000

; Test ADD immediate
ADD R0, R0, #5        ; R0 = 5

; Test NOT
NOT R1, R0            ; R1 = bitwise NOT of R0

; Test ST / LD
ST R0, VALUE           ; memory[VALUE] = R0
LD R2, VALUE           ; R2 = memory[VALUE] = 5

; Test BR
ADD R2, R2, #-5        ; R2 = 0, sets Z flag
BRz ZERO_CASE          ; should branch

; This should be skipped
ADD R3, R3, #1

ZERO_CASE:
ADD R3, R3, #7         ; R3 = 7

; Test JSR
JSR SUBROUTINE

HALT                   ; TRAP x25

SUBROUTINE:
ADD R4, R4, #3         ; R4 = 3

; Return manually using JMP R7 would normally be needed,
; but JMP is not implemented in this reduced emulator.
; So we branch directly to AFTER_SUB instead.
BRnzp AFTER_SUB

AFTER_SUB:
HALT

VALUE:
.FILL x0000

.END
