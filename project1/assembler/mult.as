        lw 0 2 mplier	Load mplier into reg 2
        lw 0 3 mcand	Load mcand into reg 3
        lw 0 4 maskb	Load maskb into reg 4
        lw 0 5 maxbit	Load maxbit into reg 5
        noop           	No-op instruction to separate the setup from the loop
        beq 0 0 loop	Branch to loop if reg 0 equals reg 0 (always true)
loop    nor 2 4 7    	Bitwise NOR of reg 2 and reg 4, store result in reg 7   
        beq 0 7 calc	If reg 7 equals 0, go to calc
        beq 0 0 proc    If reg 7 equals -1, go to proc
calc    add 1 3 1       Add reg 3 to itself and store result in reg 1
proc    lw 0 7 sub1     Load -1 into reg 7
        add 5 7 5       Subtract 1 from reg 5 (maxbit)
        beq 0 5 done    If reg 5 equals 0, exit the loop and go to done
        add 3 3 3       Add reg 3 to itself (shift left)
        add 4 4 4       Add reg 4 to itself (shift left)
        lw 0 7 add1     Load 1 into reg 7 (temporary register)
        add 4 7 4       Add 1 to reg 4 (maskb)
        beq 0 0 loop    Go back to loop
done    noop            No-op instruction to separate the loop from the halt
        halt		Halt the program
mcand   .fill   32766	Memory location for mcand
mplier  .fill   10383	Memory location for mplier
maskb   .fill   -2      Memory location for maskb
add1    .fill   1
sub1    .fill   -1
maxbit  .fill   15     Memory location for max bit (15 for 15-bit multiplication) 
