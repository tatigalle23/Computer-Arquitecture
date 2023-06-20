#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAXLINELENGTH 1000

char *rTypeOpcodes = "add|nor";
char *iTypeOpcodes = "lw|sw|beq";
char *jTypeOpcodes = "jalr";
char *oTypeOpcodes = "halt|noop";

typedef struct LABEL
{
    char name[7];
    char value[MAXLINELENGTH];
    int address;
    struct LABEL *next;
} LABEL;

LABEL labels;
int labelCount;

int readAndParse(FILE *, char *, char *, char *, char *, char *);

void buildLabels(FILE *readFilePtr);
int getValueInLabels(char *);
int isNumber(char *);
int isAlphabet(char *);
int isValidLabel(char *);
int isValidRegister(char *, int);
int checkOpCode(char *, char *);
void errorMessage(char *string);
int instR(char *opcode, char *reg0, char *reg1, char *destReg);
int instI(char *opcode, char *reg0, char *reg1, char *offset, int pc);
int instJ(char *reg0, char *reg1);
int instO(char *opcode);

int main(int argc, char *argv[])
{
    int line = 0;
    char *inFileString, *outFileString;
    FILE *inFilePtr, *outFilePtr;
    char label[MAXLINELENGTH], opcode[MAXLINELENGTH], arg0[MAXLINELENGTH], arg1[MAXLINELENGTH], arg2[MAXLINELENGTH];

    if (argc != 3)
    {
        printf("error: usage: %s <assembly-code-file> <machine-code-file>\n", argv[0]);
        exit(1);
    }

    inFileString = argv[1];
    outFileString = argv[2];

    inFilePtr = fopen(inFileString, "r");
    if (inFilePtr == NULL)
    {

        printf("error in opening %s\n", inFileString);

        exit(1);
    }
    outFilePtr = fopen(outFileString, "w");
    if (outFilePtr == NULL)
    {

        printf("error in opening %s\n", outFileString);

        exit(1);
    }

     buildLabels(inFilePtr);
 
    while (readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2))
    {
        int x = 0;

        if (checkOpCode(opcode, rTypeOpcodes))
        {
            x = instR(opcode, arg0, arg1, arg2);
        }
        else if (checkOpCode(opcode, iTypeOpcodes))
        {
            x = instI(opcode, arg0, arg1, arg2, line);
        }
        else if (checkOpCode(opcode, jTypeOpcodes))
        {
            x = instJ(arg0, arg1);
        }
        else if (checkOpCode(opcode, oTypeOpcodes))
        {
            x = instO(opcode);
        }
        else if (!strcmp(opcode, ".fill"))
        {
            if (isNumber(arg0))
            {
                x = atoi(arg0);
            }
            else
            {
                x = getValueInLabels(arg0);
            }
        }
        else
        {
            errorMessage("Do not support its opcode.");
        }

        fprintf(outFilePtr, "%d\n", x);
        printf("(Address %d): %d\n", line, x);

        line++;
    }

    fclose(inFilePtr);
    fclose(outFilePtr);
    exit(0);
}

int readAndParse(FILE *inFilePtr, char *label, char *opcode, char *arg0, char *arg1, char *arg2)
{
    char line[MAXLINELENGTH];
    char *ptr = line;


    label[0] = opcode[0] = arg0[0] = arg1[0] = arg2[0] = '\0';


    if (fgets(line, MAXLINELENGTH, inFilePtr) == NULL)
    {

        return (0);
    }


    if (strchr(line, '\n') == NULL)
    {
 
        printf("error: line too long\n");
        exit(1);
    }

    ptr = line;
    if (sscanf(ptr, "%[^\t\n\r ]", label))
    {
 
        ptr += strlen(label);
    }


    sscanf(ptr, "%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]", opcode, arg0, arg1, arg2);
    return (1);
}

void buildLabels(FILE *readFilePtr)
{
    LABEL *current;
    int line = 0;
    char label[MAXLINELENGTH], opcode[MAXLINELENGTH], arg0[MAXLINELENGTH], arg1[MAXLINELENGTH], arg2[MAXLINELENGTH];

    labelCount = 0;
    while (readAndParse(readFilePtr, label, opcode, arg0, arg1, arg2))
    {
        
        if (!strcmp(label, ""))
        {
            line++;
            continue;
        }

        if (!isValidLabel(label))
        {
            errorMessage("Label is not Valid.");
        }

        for (current = &labels; current; current = current->next)
        {
            if (!strcmp(current->name, label))
            {
                errorMessage("Label is duplicated.");
            }
        }

        for (current = &labels; current->next; current = current->next)
            ;

        current->next = malloc(sizeof(LABEL));
        current = current->next;

        strcpy(current->name, label);
        current->address = line++;

        if (!strcmp(opcode, ".fill"))
        {
            if (isNumber(arg0))
            {
                
                long long temp = atoll(arg0);
     
                if (temp > 2147483647 || temp < -2147483648)
                {
                    errorMessage(".fill value overflow.");
                }
            }

            strcpy(current->value, arg0);
        }
        else
        {
            current->value[0] = '\0';
        }

        labelCount++;
    }

    rewind(readFilePtr);
}

int getValueInLabels(char *name)
{
    LABEL *current;
    for (current = &labels; current; current = current->next)
    {
        if (!strcmp(current->name, name))
        {
            return current->address;
        }
    }

    errorMessage("undefined label");

    return 0;
}


int isNumber(char *string)
{
    int i;
    return ((sscanf(string, "%d", &i)) == 1);
}

int isAlphabet(char *string)
{
    return (int)((*string >= 'a' && *string <= 'z') || (*string >= 'A' && *string <= 'Z'));
}

int isValidLabel(char *string)
{
    int i, strLength = 0;

    strLength = strlen(string);

    if (strLength > 6)
    {
        return 0;
    }
    if (isNumber(&string[0]))
    {
        return 0;
    }
    for (i = 0; i < strLength; i++)
    {
        if (isAlphabet(&string[i]) || isNumber(&string[i]))
        {
            continue;
        }

        return 0;
    }
    return 1;
}


int isValidRegister(char *reg, int isDestReg)
{
    int regValue = 0;

    if (!isNumber(reg))
    {
        return 0;
    }

    regValue = atoi(reg);
    if (regValue < 0 || regValue >= 8)
    {
        return 0;
    }

    if (isDestReg && regValue == 0)
    {
        return 0;
    }

    return 1;
}


void errorMessage(char *string)
{
    fprintf(stderr, "error: %s\n", string);
    exit(1);
}


int checkOpCode(char *needle, char *opcodes)
{
    if (strstr(opcodes, needle) != NULL)
    {
        return 1;
    }

    return 0;
}

//Returns R-Type Format (ADD, NOR)

int instR(char *opcode, char *reg0, char *reg1, char *destReg)
{
    int x = 0;
    if (!isValidRegister(reg0, 0) || !isValidRegister(reg1, 0) || !isValidRegister(destReg, 1))
    {
        errorMessage("Registers are not valid.");
    }
    if (!strcmp(opcode, "add"))
    {
        x = (0 << 22);
    }
    else if (!strcmp(opcode, "nor"))
    {
        x = (1 << 22);
    }
    else
    {
        errorMessage("Wrong Opcode for R Type Inst Formatting");
    }
    x |= (atoi(reg0) << 19);
    x |= (atoi(reg1) << 16);
    x |= (atoi(destReg) << 0);

    return x;
}
  
 // Returns I-Type Format (with check offsetField logic) (LW, SW, BEQ)

int instI(char *opcode, char *reg0, char *reg1, char *offset, int pc)
{
    int x = 0, address = 0;

    if (!isValidRegister(reg0, 0) || !isValidRegister(reg1, 0))
    {
        errorMessage("Registers are not valid.");
    }

    if (!strcmp(opcode, "lw"))
    {
        x |= (2 << 22);
    }
    else if (!strcmp(opcode, "sw"))
    {
        x |= (3 << 22);
    }
    else if (!strcmp(opcode, "beq"))
    {
        x |= (4 << 22);
    }
    else
    {
        errorMessage("Wrong opcode for I Type Inst Formatting");
    }
    x |= (atoi(reg0) << 19);
    x |= (atoi(reg1) << 16);

    if (isNumber(offset))
    {
        address = atoi(offset);
        if (address > 32767 || address < -32768)
        {
            errorMessage("Offset out of boundary");
        }
    }
    else
    {
        address = getValueInLabels(offset);
    }

    if (!strcmp(opcode, "beq") && !isNumber(offset))
    {
        int dest = address;
        address = dest - pc - 1;
        if (address > 32767 || address < -32768)
        {
            errorMessage("Offset out of boundary");
        }
    }

    address &= 0xFFFF;
    x |= address;

    return x;
}

//Returns J-Type Format (JALR)
int instJ(char *reg0, char *reg1)
{
    int x = 0;

    if (!isValidRegister(reg0, 0) || !isValidRegister(reg1, 0))
    {
        errorMessage("Registers are not valid.");
    }

    x = (5 << 22);

    x |= (atoi(reg0) << 19);
    x |= (atoi(reg1) << 16);

    return x;
}

//Returns O-Type Format (HALT, NOOP)
int instO(char *opcode)
{
    int x = 0;
    if (!strcmp(opcode, "halt"))
    {
        x = (6 << 22);
    }
    else if (!strcmp(opcode, "noop"))
    {
        x = (7 << 22);
    }
    else
    {
        errorMessage("Wrong Opcode for O Type Inst Formatting");
    }

    return x;
}
