; Sequential LC-3 integration program.
;
; The program calculates the running sums, final sum, and maximum value of
; ARRAY. It then exercises indirect memory access, direct and indirect
; subroutine calls, RET, and JMP before halting.
;
; Expected final state:
;   RUNNING_SUMS   = [3, 1, 6, 7]
;   FINAL_SUM      = 7       (x0007)
;   FINAL_MAX      = 5       (x0005)
;   INDIRECT_RESULT= 7       (x0007)
;   MASKED_RESULT  = 7       (x0007)
;   NEGATED_DOUBLE = -14     (xFFF2)
;   ERROR_MARKER   = 0       (x0000)

.ORIG x3000

    ; Initialize the array pointers, element count, sum, and maximum.
    LEA R1, ARRAY
    LEA R2, RUNNING_SUMS
    LD  R3, COUNT
    AND R0, R0, #0
    LDR R4, R1, #0

LOOP
    ; Load the current value, add it to the sum, and save the running sum.
    LDR R5, R1, #0
    ADD R0, R0, R5
    STR R0, R2, #0

    ; Compute value - maximum. Update maximum only when the result is positive.
    NOT R6, R4
    ADD R6, R6, #1
    ADD R6, R5, R6
    BRnz KEEP_MAXIMUM
    ADD R4, R5, #0

KEEP_MAXIMUM
    ; Advance both pointers and continue while elements remain.
    ADD R1, R1, #1
    ADD R2, R2, #1
    ADD R3, R3, #-1
    BRp LOOP

    ; Store the final results using PC-relative and indirect addressing.
    ST  R0, FINAL_SUM
    ST  R4, FINAL_MAX
    STI R0, RESULT_POINTER
    LDI R5, RESULT_POINTER

    ; Mask the indirectly loaded result using the register form of AND.
    LD  R6, MASK
    AND R5, R5, R6
    ST  R5, MASKED_RESULT

    ; Call one subroutine directly and another through a register.
    JSR NEGATE
    LEA R4, DOUBLE
    JSRR R4
    ST R0, NEGATED_DOUBLE

    ; Jump over code that would set ERROR_MARKER if JMP failed.
    AND R6, R6, #0
    LEA R5, DONE
    JMP R5
    ADD R6, R6, #1

DONE
    ST R6, ERROR_MARKER
    HALT

NEGATE
    NOT R0, R0
    ADD R0, R0, #1
    RET

DOUBLE
    ADD R0, R0, R0
    RET

COUNT
    .FILL x0004

MASK
    .FILL x000F

RESULT_POINTER
    .FILL INDIRECT_RESULT

ARRAY
    .FILL x0003
    .FILL xFFFE
    .FILL x0005
    .FILL x0001

RUNNING_SUMS
    .BLKW #4

FINAL_SUM
    .FILL #0

FINAL_MAX
    .FILL #0

INDIRECT_RESULT
    .FILL #0

MASKED_RESULT
    .FILL #0

NEGATED_DOUBLE
    .FILL #0

ERROR_MARKER
    .FILL #0

.END
