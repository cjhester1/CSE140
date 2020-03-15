//
//  computer.c
//
//I recieved help on this from my tutor sundeep kumar sundeep@sundeepkumar.com
//  Created by cj hester on 10/09/19.
//due to my weekend schedule and other factors I decided doing this project solo was the best option
//
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include "computer.h"
#undef mips            /* gcc already has a def for mips */

unsigned int endianSwap(unsigned int);

void PrintInfo (int changedReg, int changedMem);
unsigned int Fetch (int);
void Decode (unsigned int, DecodedInstr*, RegVals*);
int Execute (DecodedInstr*, RegVals*);
int Mem(DecodedInstr*, int, int *);
void RegWrite(DecodedInstr*, int, int *);
void UpdatePC(DecodedInstr*, int);
void PrintInstruction (DecodedInstr*);

/*Globally accessible Computer variable*/
Computer mips;
RegVals rVals;

/*
 *  Return an initialized computer with the stack pointer set to the
 *  address of the end of data memory, the remaining registers initialized
 *  to zero, and the instructions read from the given file.
 *  The other arguments govern how the program interacts with the user.
 */
void InitComputer (FILE* filein, int printingRegisters, int printingMemory,
                   int debugging, int interactive) {
    int k;
    unsigned int instr;
    
    /* Initialize registers and memory */
    
    for (k=0; k<32; k++) {
        mips.registers[k] = 0;
    }
    
    /* stack pointer - Initialize to highest address of data segment */
    mips.registers[29] = 0x00400000 + (MAXNUMINSTRS+MAXNUMDATA)*4;
    
    for (k=0; k<MAXNUMINSTRS+MAXNUMDATA; k++) {
        mips.memory[k] = 0;
    }
    
    k = 0;
    while (fread(&instr, 4, 1, filein)) {
        /*swap to big endian, convert to host byte order. Ignore this.*/
        mips.memory[k] = ntohl(endianSwap(instr));
        k++;
        if (k>MAXNUMINSTRS) {
            fprintf (stderr, "Program too big.\n");
            exit (1);
        }
    }
    
    mips.printingRegisters = printingRegisters;
    mips.printingMemory = printingMemory;
    mips.interactive = interactive;
    mips.debugging = debugging;
}

unsigned int endianSwap(unsigned int i) {
    return (i>>24)|(i>>8&0x0000ff00)|(i<<8&0x00ff0000)|(i<<24);
}

/*
 *  Run the simulation.
 */
void Simulate () {
    char s[40];  /* used for handling interactive input */
    unsigned int instr;
    int changedReg=-1, changedMem=-1, val;
    DecodedInstr d;
    
    /* Initialize the PC to the start of the code section */
    mips.pc = 0x00400000;
    while (1) {
        if (mips.interactive) {
            printf ("> ");
            fgets (s,sizeof(s),stdin);
            if (s[0] == 'q') {
                return;
            }
        }
        
        /* Fetch instr at mips.pc, returning it in instr */
        instr = Fetch (mips.pc);
        
        printf ("Executing instruction at %8.8x: %8.8x\n", mips.pc, instr);
        
        /*
         * Decode instr, putting decoded instr in d
         * Note that we reuse the d struct for each instruction.
         */
        Decode (instr, &d, &rVals);
        
        /*Print decoded instruction*/
        PrintInstruction(&d);
        
        /*
         * Perform computation needed to execute d, returning computed value
         * in val
         */
        val = Execute(&d, &rVals);
        
        UpdatePC(&d,val);
        
        /*
         * Perform memory load or store. Place the
         * address of any updated memory in *changedMem,
         * otherwise put -1 in *changedMem.
         * Return any memory value that is read, otherwise return -1.
         */
        val = Mem(&d, val, &changedMem);
        
        /*
         * Write back to register. If the instruction modified a register--
         * (including jal, which modifies $ra) --
         * put the index of the modified register in *changedReg,
         * otherwise put -1 in *changedReg.
         */
        RegWrite(&d, val, &changedReg);
        
        PrintInfo (changedReg, changedMem);
    }
}

/*
 *  Print relevant information about the state of the computer.
 *  changedReg is the index of the register changed by the instruction
 *  being simulated, otherwise -1.
 *  changedMem is the address of the memory location changed by the
 *  simulated instruction, otherwise -1.
 *  Previously initialized flags indicate whether to print all the
 *  registers or just the one that changed, and whether to print
 *  all the nonzero memory or just the memory location that changed.
 */
void PrintInfo ( int changedReg, int changedMem) {
    int k, addr;
    printf ("New pc = %8.8x\n", mips.pc);
    if (!mips.printingRegisters && changedReg == -1) {
        printf ("No register was updated.\n");
    } else if (!mips.printingRegisters) {
        printf ("Updated r%2.2d to %8.8x\n",
                changedReg, mips.registers[changedReg]);
    } else {
        for (k=0; k<32; k++) {
            printf ("r%2.2d: %8.8x  ", k, mips.registers[k]);
            if ((k+1)%4 == 0) {
                printf ("\n");
            }
        }
    }
    if (!mips.printingMemory && changedMem == -1) {
        printf ("No memory location was updated.\n");
    } else if (!mips.printingMemory) {
        printf ("Updated memory at address %8.8x to %8.8x\n",
                changedMem, Fetch (changedMem));
    } else {
        printf ("Nonzero memory\n");
        printf ("ADDR      CONTENTS\n");
        for (addr = 0x00400000+4*MAXNUMINSTRS;
             addr < 0x00400000+4*(MAXNUMINSTRS+MAXNUMDATA);
             addr = addr+4) {
            if (Fetch (addr) != 0) {
                printf ("%8.8x  %8.8x\n", addr, Fetch (addr));
            }
        }
    }
}

/*
 *  Return the contents of memory at the given address. Simulates
 *  instruction fetch.
 */
unsigned int Fetch ( int addr) {
    return mips.memory[(addr-0x00400000)/4];
}

/* Decode instr, returning decoded instruction. */
void Decode ( unsigned int instr, DecodedInstr* d, RegVals* rVals) {
    
    d->op = instr >> 0x1a;//0x1a = 26
    switch (d->op)
    {
            
        case 0x0:
        {//Case 0 for all R type instructions including addu,and,jr,or,slt
            
            d->type = R;
            d->regs.r.rs = (instr >> 21) & 31;//shift right instr by 21, the & setting bound for rs
            d->regs.r.rt = (instr >> 16) & 31;//shift right instr by 16 setting bound for rt
            d->regs.r.rd = (instr >> 11) & 31;//shift right instr by 11 setting bound for rd
            d->regs.r.shamt = (instr >> 6) & 31;//shift right by 6 setting bound for shamt
            d->regs.r.funct = instr & 63;//setting funct to be able to take in funct from 0 to 3f
            rVals->R_rs = mips.registers[d->regs.r.rs];//setting rs values into mips
            rVals->R_rt = mips.registers[d->regs.r.rt];//setting rt values into mips
            rVals->R_rd = mips.registers[d->regs.r.rd];//setting rd values into mips
            break;
        }
        case 0x2:
        {//Case 2 for all J type instructions including j and jal
            d->type = J;// J type instr
            d->regs.j.target = ((instr & 67108863)<<2)+(mips.pc & 4026531840);
            break;
        }
        case 0x3:
        {//Case 3 for all J type instructions
            d->type = J;
            d->regs.j.target = ((instr & 67108863)<<2)+(mips.pc & 4026531840);
            break;
        }
        case 0x4:// 4, 5, 9, 12, 13, 15, 35, 43
        {//Case 4 for I type 'beq'
            d->type = I;
            d->regs.i.rs = (instr >> 21) & 31;
            d->regs.i.rt = (instr >> 16) & 31;
            d->regs.i.addr_or_immed = (signed short)(instr & 65535);
            rVals->R_rs = mips.registers[d->regs.i.rs];
            rVals->R_rt = mips.registers[d->regs.i.rt];
            break;
        }
            
        case 0x5:
        {//Case 5 for I type 'bne'
            d->type = I;
            d->regs.i.rs = (instr >> 21) & 31;
            d->regs.i.rt = (instr >> 16) & 31;
            d->regs.i.addr_or_immed = (signed short)(instr & 65535);
            rVals->R_rs = mips.registers[d->regs.i.rs];
            rVals->R_rt = mips.registers[d->regs.i.rt];
            break;
        }
        case 0x9:
        {//Case 9 for I type 'addiu'
            d->type = I;
            d->regs.i.rs = (instr >> 21) & 31;
            d->regs.i.rt = (instr >> 16) & 31;
            d->regs.i.addr_or_immed = (signed short)(instr & 65535);
            rVals->R_rs = mips.registers[d->regs.i.rs];
            rVals->R_rt = mips.registers[d->regs.i.rt];
            break;
        }
            
        case 0xc:
        {//Case 0XC = 12 for I type 'andi'
            d->type = I;
            d->regs.i.rs = (instr >> 21) & 31;
            d->regs.i.rt = (instr >> 16) & 31;
            d->regs.i.addr_or_immed = (signed short)(instr & 65535);
            rVals->R_rs = mips.registers[d->regs.i.rs];
            rVals->R_rt = mips.registers[d->regs.i.rt];
            break;
        }
            
        case 0xd:
        {//Case 0XD = 13 for I type 'ori'
            d->type = I;
            d->regs.i.rs = (instr >> 21) & 31;
            d->regs.i.rt = (instr >> 16) & 31;
            d->regs.i.addr_or_immed = (signed short)(instr & 65535);
            rVals->R_rs = mips.registers[d->regs.i.rs];
            rVals->R_rt = mips.registers[d->regs.i.rt];
            break;
        }
            
        case 0xf:
        {//Case 0XF = 15 for I type 'lui'
            d->type = I;
            d->regs.i.rs = (instr >> 21) & 31;
            d->regs.i.rt = (instr >> 16) & 31;
            d->regs.i.addr_or_immed = (signed short)(instr & 65535);
            rVals->R_rs = mips.registers[d->regs.i.rs];
            rVals->R_rt = mips.registers[d->regs.i.rt];
            break;
        }
            
        case 0x23:
        {//Case 0X23 = 35 for I type 'lw'
            d->type = I;
            d->regs.i.rs = (instr >> 21) & 31;
            d->regs.i.rt = (instr >> 16) & 31;
            d->regs.i.addr_or_immed = (signed short)(instr & 65535);
            rVals->R_rs = mips.registers[d->regs.i.rs];
            rVals->R_rt = mips.registers[d->regs.i.rt];
            break;
        }
            
        case 0x2b:
        {//Case 0X2B = 43 for I type 'sw'
            d->type = I;
            d->regs.i.rs = (instr >> 21) & 31;
            d->regs.i.rt = (instr >> 16) & 31;
            d->regs.i.addr_or_immed = (signed short)(instr & 65535);
            rVals->R_rs = mips.registers[d->regs.i.rs];
            rVals->R_rt = mips.registers[d->regs.i.rt];
            break;
        }
            /* Your code goes here */
    }
}

/*
 *  Print the disassembled version of the given instruction
 *  followed by a newline.
 */
void PrintInstruction ( DecodedInstr* d) {
    
    
    switch (d->op)
    {
            
        case 0x0:
        {//R type only
            switch (d->regs.r.funct)
            {
                case 0x21: {// case is for addu instr
                    printf("addu\t$%d, $%d, $%d \n", d->regs.r.rd, d->regs.r.rs, d->regs.r.rt);
                    return;
                }
                case 0x23: {// case is for subu instr
                    printf("subu\t$%d, $%d, $%d \n", d->regs.r.rd, d->regs.r.rs, d->regs.r.rt);
                    return;
                }
                case 0x00: {// case is for sll instr
                    printf("sll\t$%d, $%d, $%d \n", d->regs.r.rd, d->regs.r.rt, d->regs.r.shamt);
                    return;
                }
                case 0x02: {// case is for srl instr
                    printf("srl\t$%d, $%d, $%d \n", d->regs.r.rd, d->regs.r.rt, d->regs.r.shamt);
                    return;
                }
                case 0x24: {// case is for and instr
                    printf("and\t$%d, $%d, $%d \n", d->regs.r.rd, d->regs.r.rs, d->regs.r.rt);
                    return;
                }
                case 0x25: {// case is for or instr
                    printf("or\t$%d, $%d, $%d \n", d->regs.r.rd, d->regs.r.rs, d->regs.r.rt);
                    return;
                }
                case 0x2a: {// case is for slt instr
                    printf("slt\t$%d, $%d, $%d \n", d->regs.r.rd, d->regs.r.rs, d->regs.r.rt);
                    return;
                }
                case 0x8: {// case is for jr instr
                    printf("jr\t$31\n");
                    return;
                }
                default: break;
            }
        }
            //I instruction next
        case 0X9: {// case is for addiu instr
            printf("addiu\t$%d, $%d, %d \n", d->regs.i.rt, d->regs.i.rs, d->regs.i.addr_or_immed);
            return;
        }
        case 0xc: {// case is for andi instr
            printf("andi\t$%d, $%d, 0x%d \n", d->regs.i.rt, d->regs.i.rs, d->regs.i.addr_or_immed);
            return;
        }
        case 0xd: {// case is for ori instr
            printf("ori\t$%d, $%d, 0x%d \n", d->regs.i.rt, d->regs.i.rs, d->regs.i.addr_or_immed);
            return;
        }
        case 0xf: {// case is for lui instr
            printf("lui\t$%d, 0x%d \n", d->regs.i.rt, d->regs.i.addr_or_immed << 16);
        }
        case 0x4: {// case is for beq instr
            printf("beq\t$%d, $%d, 0x%8.8x \n", d->regs.i.rs, d->regs.i.rt, (mips.pc + 4) + (d->regs.i.addr_or_immed << 2));
            return;
        }
        case 0x5: {// case is for bne instr
            printf("bne\t$%d, $%d, 0x%8.8x \n", d->regs.i.rs, d->regs.i.rt, (mips.pc + 4) + (d->regs.i.addr_or_immed << 2));
            return;
        }
       
        case 0x23: {// case is for lw instr
            printf("lw\t$%d, %d($%d)\n", d->regs.i.rt, d->regs.i.addr_or_immed, d->regs.i.rs);
            return;
        }
        case 0x2b: {// case is for sw instr
            printf("sw\t$%d, %d($%d)\n", d->regs.i.rt, d->regs.i.addr_or_immed, d->regs.i.rs);
            return;
        }
            //For J type
        case 0x2: {// case is for j instr
            printf("j\t0x%8.8x \n", d->regs.j.target);
            return;
        }
        case 0x3: {// case is for jal instr
            printf("jal\t0x%8.8x \n", d->regs.j.target);
            return;
        }
        default: exit(1);
    }
    /* Your code goes here */
}

/* Perform computation needed to execute d, returning computed value */
int Execute ( DecodedInstr* d, RegVals* rVals) {
    /* Your code goes here */
    
    switch (d->op)
    {
            
        case 0x0:
        {
            switch (d->regs.r.funct)
            {
                    
                case 0x21: {
                    return (unsigned int)rVals->R_rs + (unsigned int)rVals->R_rt;
                }
                    
                case 0x23: {
                    return (unsigned int)rVals->R_rs - (unsigned int)rVals->R_rt;
                }
                    
                case 0x00: {
                    return rVals->R_rs << d->regs.r.shamt;
                }
                    
                case 0x02: {
                    return rVals->R_rt >> d->regs.r.shamt;
                }
                    
                case 0x24: {
                    return rVals->R_rs&rVals->R_rt;
                }
                    
                case 0x25: {
                    return rVals->R_rs | rVals->R_rt;
                }
                   
                case 0x2a: {
                    if (rVals->R_rs < rVals->R_rt) {
                        return 1;
                    }
                    else {
                        return 0;
                    }
                }
                    
                case 0x8: {
                    return rVals->R_rs;
                }
                default: return 0;
            }
        }
            
           
        case 0X9: {
            return rVals->R_rs + d->regs.i.addr_or_immed;
        }
            
        case 0xc: {
            return rVals->R_rs & d->regs.i.addr_or_immed;
        }
            
        case 0xd: {
            return rVals->R_rs | d->regs.i.addr_or_immed;
        }
           
        case 0xf: {
            return d->regs.i.addr_or_immed << 16;
        }
            
        case 0x4: {
            if (rVals->R_rs == rVals->R_rt) {
                return (mips.pc + 4) + (d->regs.i.addr_or_immed << 2);
            }
            else {
                return 0;
            }
        }
            
        case 0x5: {
            if (rVals->R_rs != rVals->R_rt) {
                return (mips.pc + 4) + (d->regs.i.addr_or_immed << 2);
            }
            else {
                return 0;
            }
        }
            
        case 0x2: {
            return mips.pc+4;
        }
           
        case 0x3: {
            return mips.pc + 4;
        }
            
        case 0x23: {
            return rVals->R_rs + d->regs.i.addr_or_immed;
        }
            
        case 0x2b: {
            return rVals->R_rs + d->regs.i.addr_or_immed;
        }
        default: return(0);
    }
    
   
}

/*
 * Update the program counter based on the current instruction. For
 * instructions other than branches and jumps, for example, the PC
 * increments by 4 (which we have provided).
 */
void UpdatePC ( DecodedInstr* d, int val) {
    switch (d->op) {
            
        case 0x0: {
            if (d->op==0x0&&d->regs.r.funct == 0x8) {
                mips.pc = val;
            }
            else {
                mips.pc+=4;
            }
            break;
        }
            
        case 0x2: {
            mips.pc = d->regs.j.target;
            break;
            
        }
            
        case 0x3: {
            mips.pc = d->regs.j.target;
            break;
        }
            
        case 0x4: {
            if (val!=0) {
                mips.pc =val;
            }
            else {
                mips.pc += 4;
            }
            break;
        }
            //bne
        case 0x5: {
            if (val != 0) {
                mips.pc = val;
            }
            else {
                mips.pc += 4;
            }
            break;
        }
        default: mips.pc += 4;
            
    }
    /* Your code goes here */
}

/*
 * Perform memory load or store. Place the address of any updated memory
 * in *changedMem, otherwise put -1 in *changedMem. Return any memory value
 * that is read, otherwise return -1.
 *
 * Remember that we're mapping MIPS addresses to indices in the mips.memory
 * array. mips.memory[0] corresponds with address 0x00400000, mips.memory[1]
 * with address 0x00400004, and so forth.
 *
 */
int Mem( DecodedInstr* d, int val, int *changedMem) {
    switch (d->op) {
        case 0x23: {
            if (val<=0x00401000|| val >= 0x00403fff ) {
                printf("Memory Access Exception at: [0x%08x]: address [0x%08x]\n", mips.pc, val);
                exit(0);
            }
            else {
                val = mips.memory[(val - 0x00400000) / 4];
                return val;
            }
            break;
        }
        case 0x2b: {
            if ( val <= 0x00401000|| val >= 0x00403fff) {
                printf("Memory Access Exception at: [0x%08x]: address [0x%08x]\n", mips.pc, val);
                exit(0);
            }
            else {
                mips.memory[(val - 0x00400000) / 4] = mips.registers[d->regs.i.rt];
                *changedMem = val;
                return *changedMem;
            }
            break;
        }
        default: *changedMem = -1;
    }
    return val;
    
    exit(0);
}

/*
 * Write back to register. If the instruction modified a register--
 * (including jal, which modifies $ra) --
 * put the index of the modified register in *changedReg,
 * otherwise put -1 in *changedReg.
 */
void RegWrite( DecodedInstr* d, int val, int *changedReg) {
    /* Your code goes here */
    switch (d->op) {
            
        case 0x0: {
            if (d->regs.r.funct==0x8) {
                *changedReg = -1;
            }
            else {
                *changedReg = d->regs.r.rd;
                mips.registers[d->regs.r.rd] = val;
            }
            break;
        }
        case 0x9: {
            *changedReg = d->regs.i.rt;
            mips.registers[d->regs.i.rt] = val;
            break;
            
        }
        case 0xc: {
            
            *changedReg = d->regs.i.rt;
            mips.registers[d->regs.i.rt] = val;
            break;
            
        }
        case 0xd: {
            
            *changedReg = d->regs.i.rt;
            mips.registers[d->regs.i.rt] = val;
            break;
            
        }
        case 0xf: {
            *changedReg = d->regs.i.rt;
            mips.registers[d->regs.i.rt] = val;
            break;
            
        }
        case 0x23: {
            *changedReg = d->regs.i.rt;
            mips.registers[d->regs.i.rt] = val;
            break;
            
        }
        case 0x3: {
            *changedReg = 31;
            mips.registers[31] = val;
            break;
        }
        default: *changedReg = -1;
    }
}


