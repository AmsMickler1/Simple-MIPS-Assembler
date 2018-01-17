// ----------proj1.c-----------
// Amber Mickler
// Assignment 1 - Assembler in C
// Wang COP3101

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int address;
    char str[64];
    int binary;
} Line;

int getRegister(char *reg);

int main() {

    char str[100];
    char temp[50];
    char *token;
    char *reg;
    char *label;
    char *command;
    char *args;
    Line lines[258];
    Line labels[256];
    char d1[2] = "\t";      // Delimeter 1, the tab character
    char d2[5] = "$,()";    // Delimeter 2, for tokening args
    int s=0;                // String Counter for lines array
    int a=0;                // Address Counter
    int l=0;                // Counter for Labels array

    // First pass over input: Read in, store, and address each line
    while (fgets(lines[s].str,64,stdin) != NULL) {

        strcpy(str, lines[s].str);
        token = strtok(str,d1);
        lines[s].binary = 0;

        // Check for label and add to label array
        if (strstr(token, ":") != NULL) {
            strcpy(labels[l].str, str);
            labels[l].address = a;
            lines[s].address = a;
            l++;
            token = strtok(NULL,d1);
        }
        // if line is ".text" set address to -1 and start address counter over at 0
        if (strstr(token, ".text") != NULL) {
            lines[s].address = -1;
            a = 0;
        }
        // If line is ".data" set address to -1 and jump to address 1024
        else if (strstr(token, ".data") != NULL) {
            lines[s].address = -1;
            a = 1024;
        }
        // check for .word
        else if (strstr(token, ".word") != NULL) {
            // .word [value]
            token = strtok(NULL,d1);
            lines[s].binary = atoi(token);
            a += 4;
        }
        // check for .space
        else if (strstr(token, ".space") != NULL) {
            // .space [bytes]
            token = strtok(NULL, d1);
            lines[s].address = a;
            a += atoi(token);
            // initialize those bytes with 0 ?
        }
        // Check for la & separate into lui and ori
        else if (strstr(token, "la") != NULL) {
            token = strtok(NULL,d1);
            // break down args into register and label
            strcpy(temp,token);
            reg = strtok(temp,d2);
            label = strtok(NULL,d2);
            // make lui [args] and ori [args]
            strcpy(lines[s].str,"lui\t$1,");
            strcat(lines[s].str,label);
            lines[s].address = a;
            s++;
            a+=4;
            strcpy(lines[s].str,"ori\t");
            strcat(lines[s].str,reg);
            strcat(lines[s].str,",$1,");
            strcat(lines[s].str,label);
            lines[s].address = a;
            a+=4;
        }
        else {
            lines[s].address = a;
            a += 4;
        }
        s++;
    }   //END OF WHILE LOOP

    //Second pass over input: parse commands into machine code
    for (int i=0; i<s; i++) {
        // char *label declared above
        char type = '.';
        int opcode = 0;     //6 bits
        int rs = 0;         //5 bits
        int rt = 0;         //5 bits
        int rd = 0;         //5 bits
        int shamt = 0;      //5 bits
        int funct = 0;      //6 bits
        int immed = 0;      //16 bits
        int target = 0;     //26 bits

        if (lines[i].address >= 0) {
            token = strtok(lines[i].str, d1);
            // If line has a label, move to next token
            if (strstr(token, ":") != NULL) {
                token = strtok(NULL,d1);
            }
            command = token;
            token = strtok(NULL,d1);
            args = token;
            args = strtok(args,d2);

            if (strstr(command, "j") != NULL) {
                opcode = 2;
                type = 'J';
                label = strtok(args, " \t\n:");
                for (int j=0; j<l;j++) {
                    if (strstr(labels[j].str,label) != NULL) {
                        target = labels[j].address;
                    }
                }
                target = target >> 2;
            }
            else if (strstr(command, "bne") != NULL) {
                opcode = 5;
                type = 'I';
                rs = getRegister(args);
                args = strtok(NULL,d2);
                rt = getRegister(args);
                args = strtok(NULL,d2);
                label = strtok(args, " \t\n:");
                for (int j=0; j<l;j++) {
                    if (strstr(labels[j].str,label) != NULL) {
                        target = labels[j].address;
                    }
                }
                immed = target - lines[i].address - 4;
                immed = immed >> 2;
                immed = immed & 0x0000ffff;
            }
            else if (strstr(command, "lw") != NULL) {
                opcode = 35;
                type = 'I';
                rt = getRegister(args);
                args = strtok(NULL,d2);
                immed = atoi(args);
                args = strtok(NULL,d2);
                rs = getRegister(args);
            }
            else if (strstr(command, "sw") != NULL) {
                opcode = 43;
                type = 'I';
                rt = getRegister(args);
                args = strtok(NULL,d2);
                immed = atoi(args);
                args = strtok(NULL,d2);
                rs = getRegister(args);
            }
            else if (strstr(command, "lui") != NULL) {
                opcode = 15;
                type = 'I';
                rt = getRegister(args);
                args = strtok(NULL,d2);
                label = strtok(args, " \t\n:");
                for (int j=0; j<l;j++) {
                    if (strstr(labels[j].str,label) != NULL) {
                        target = labels[j].address;
                    }
                }
                if (target == 0)
                    immed = atoi(label);
                else
                    immed = (target & 0xffff0000);
            }
            else if (strstr(command, "sll") != NULL) {
                opcode = 0;
                type = 'R';
                funct = 0;
                rd = getRegister(args);
                args = strtok(NULL,d2);
                rt = getRegister(args);
                args = strtok(NULL,d2);
                shamt = atoi(args);
            }
            else if (strstr(command, "ori") != NULL) {
                opcode = 13;
                type = 'I';
                rt = getRegister(args);
                args = strtok(NULL,d2);
                rs = getRegister(args);
                args = strtok(NULL,d2);
                label = strtok(args, " \t\n:");
                for (int j=0; j<l;j++) {
                    if (strstr(labels[j].str,label) != NULL) {
                        target = labels[j].address;
                    }
                }
                if (target == 0)
                    immed = atoi(label);
                else
                    immed = (target & 0x0000ffff);
            }
            else if (strstr(command, "nor") != NULL) {
                opcode = 0;
                type = 'R';
                funct = 39;
                rd = getRegister(args);
                args = strtok(NULL,d2);
                rs = getRegister(args);
                args = strtok(NULL,d2);
                rt = getRegister(args);
            }
            else if (strstr(command, "addi") != NULL) {
                opcode = 8;
                type = 'I';
                rt = getRegister(args);
                args = strtok(NULL,d2);
                rs = getRegister(args);
                args = strtok(NULL,d2);
                immed = atoi(args);
                immed = immed & 0x0000ffff;
            }
            else if (strstr(command, "add") != NULL) {
                opcode = 0;
                type = 'R';
                funct = 32;
                rd = getRegister(args);
                args = strtok(NULL,d2);
                rs = getRegister(args);
                args = strtok(NULL,d2);
                rt = getRegister(args);
            }

            switch (type) {
                case 'J':
                    lines[i].binary = (opcode << 26) | target;
                    break;
                case 'I':
                    lines[i].binary = (opcode << 26) | (rs << 21) | (rt << 16) | immed;
                    break;
                case 'R':
                    lines[i].binary = (opcode << 26) | (rs << 21) | (rt << 16) | (rd << 11) | (shamt << 6) | funct;
                    break;
                case '.':
                    break;
            }
        }
    }   // END OF FOR LOOP

    for (int i=0; i<s; i++) {
        if (lines[i].address >= 0)
            printf("0x%06X: 0x%08X\n", lines[i].address, lines[i].binary);
    }
    return 0;
}

int getRegister(char *reg) {
    //$t0-$t7 = 8-15
    //$s0-$s7 = 16-23
    int num = 0;
    if (strstr(reg,"t") != NULL)
        num += 8;
    else if (strstr(reg,"s") != NULL)
        num += 16;
    else if (strstr(reg, "1") != NULL)
        num = 1;

    reg[0] = '0';
    num += atoi(reg);

    return num;
}
