#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define NUMMEMORY 65536 /* maximum number of words in memory */
#define NUMREGS 8       /* number of machine registers */
#define MAXLINELENGTH 1000

typedef struct stateStruct
{
    int pc;
    int mem[NUMMEMORY];
    int reg[NUMREGS];
    int numMemory;
} stateType;

void printState(stateType *);
void errorMessage(char *string);
int isValidRegister(int reg);
int convertNum(int num);
void divideInfo(stateType *statePtr, int *opcode, int *arg0, int *arg1, int *arg2);
void instR(stateType *statePtr, int opcode, int arg0, int arg1, int dest);
void instI(stateType *statePtr, int opcode, int arg0, int arg1, int offset);
void instJ(stateType *statePtr, int opcode, int arg0, int arg1);

int main(int argc, char *argv[])
{
    char line[MAXLINELENGTH];
    stateType state;
    FILE *filePtr;
    int executionCount = 0;

    if (argc != 2)
    {
        printf("error: usage: %s <machine-code file>\n", argv[0]);
        exit(1);
    }

    filePtr = fopen(argv[1], "r");
    if (filePtr == NULL)
    {

        printf("error: can't open file %s", argv[1]);

        perror("fopen");

        exit(1);
    }

    for (state.numMemory = 0; fgets(line, MAXLINELENGTH, filePtr) != NULL; state.numMemory++)
    {

        if (sscanf(line, "%d", state.mem + state.numMemory) != 1)
        {
            printf("error in reading address %d\n", state.numMemory);
            exit(1);
        }
        printf("memory[%d]=%d\n", state.numMemory, state.mem[state.numMemory]);
    }

    // INITIALIZE REG TO 0 IF NOR INFINITE LOOP
    for (int i = 0; i < NUMREGS; ++i) {
        state.reg[i] = 0;
    }
    printState(&state);
    while (1)
    {
        int isHalt = 0;
        int opcode, arg0, arg1, arg2;
        divideInfo(&state, &opcode, &arg0, &arg1, &arg2);

        state.pc++;
        executionCount++;

        if (state.pc < 0 || state.pc >= NUMMEMORY)
        {
            errorMessage("PC out of memory");
        }

        /**
         * OPCODES
         * 
         * 0 - add
         * 1 - nor
         * 2 - lw
         * 3 - sw
         * 4 - beq
         * 5 - jalr
         * 6 - halt
         * 7 - noop
         */
        switch (opcode)
        {
        case 0: 
        case 1: 
            instR(&state, opcode, arg0, arg1, arg2);
            break;
        case 2: 
        case 3: 
        case 4: 
            instI(&state, opcode, arg0, arg1, arg2);
            break;
        case 5: 
            instJ(&state, opcode, arg0, arg1);
            break;        
        case 6: 
            isHalt = 1;
            break;
        case 7: 
            break;
        default:
            errorMessage("Do not support its opcode.");
            break;
        }        
        if (isHalt)
        {
            break;
        }
        printState(&state);
    }

    printf("machine halted\n");
    printf("total of %d instructions executed\n", executionCount);
    printf("final state of machine:\n");

    printState(&state);

    fclose(filePtr);
    exit(0);
}

void printState(stateType *statePtr)
{

    int i;
    printf("\n@@@\nstate:\n");
    printf("\tpc %d\n", statePtr->pc);
    printf("\tmemory:\n");
    for (i = 0; i < statePtr->numMemory; i++)
    {

        printf("\t\tmem[ %d ] %d\n", i, statePtr->mem[i]);
    }
    printf("\tregisters:\n");
    for (i = 0; i < NUMREGS; i++)
    {

        printf("\t\treg[ %d ] %d\n", i, statePtr->reg[i]);
    }
    printf("end state\n");
}

void errorMessage(char *string)
{
    fprintf(stderr, "error: %s\n", string);
    exit(1);
}
int isValidRegister(int reg)
{
    return (int)(reg >= 0 && reg < NUMREGS);
}
int convertNum(int num)
{
    
    if (num & (1 << 15))
    {
        num -= (1 << 16);
    }

    return (num);
}


void divideInfo(stateType *statePtr, int *opcode, int *arg0, int *arg1, int *arg2)
{
    int memValue = statePtr->mem[statePtr->pc];

    *opcode = (memValue >> 22) & 0b111;
    *arg0 = (memValue >> 19) & 0b111;
    *arg1 = (memValue >> 16) & 0b111;
    *arg2 = (memValue & 0xFFFF);
}

//ADD NOR
void instR(stateType *statePtr, int opcode, int arg0, int arg1, int dest)
{

    if (!isValidRegister(arg0) || !isValidRegister(arg1) || !isValidRegister(dest))
    {
        errorMessage("Register is not valid.");
    }

    switch (opcode)
    {
    case 0: 
        statePtr->reg[dest] = statePtr->reg[arg0] + statePtr->reg[arg1];
        break;
    case 1: 
        statePtr->reg[dest] = ~(statePtr->reg[arg0] | statePtr->reg[arg1]);
        break;
    default:
        errorMessage("Do not support its opcode");
        break;
    }
}

//LW SW BEQ
void instI(stateType *statePtr, int opcode, int arg0, int arg1, int offset)
{
    offset = convertNum(offset);

    if (!isValidRegister(arg0) || !isValidRegister(arg1))
    {
        errorMessage("Register is not valid.");
    }

    if (offset > 32767 || offset < -32768)
    {
        errorMessage("Offset out of range");
    }

    switch (opcode)
    {
    case 2:
        statePtr->reg[arg1] = statePtr->mem[statePtr->reg[arg0] + offset];
        break;
    case 3:
        statePtr->mem[statePtr->reg[arg0] + offset] = statePtr->reg[arg1];
        break;
    case 4:
        if (statePtr->reg[arg0] == statePtr->reg[arg1])
        {
            statePtr->pc += offset;
        }
        break;
    default:
        errorMessage("Do not support its opcode");
        break;
    }
}

//JALR
void instJ(stateType *statePtr, int opcode, int arg0, int arg1)
{
    if (!isValidRegister(arg0) || !isValidRegister(arg1))
    {
        errorMessage("Register is not valid.");
    }

    switch (opcode)
    {
    case 5:
        statePtr->reg[arg1] = statePtr->pc;
        statePtr->pc = statePtr->reg[arg0];
        break;
    default:
        errorMessage("Do not support its opcode");
        break;
    }
}
