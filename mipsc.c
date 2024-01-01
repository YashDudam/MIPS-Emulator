// COMP1521 22T3 Assignment 2: mipsc -- a MIPS simulator
// starting point code v1.0 - 24/10/22


// PUT YOUR HEADER COMMENT HERE
// This program was written by Yash Dudam(z5419053)

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// ADD ANY ADDITIONAL #include HERE

#define MAX_LINE_LENGTH         256
#define INSTRUCTIONS_GROW       64

// ADD ANY ADDITIONAL #defines HERE
#define OPCODE_SIZE             6
#define FUNCTION_SIZE           6
#define REGISTERS_SIZE          32
#define IMMEDIATE_SIZE          16
#define DEFAULT_LUI_BIT_SHIFT   16
#define HI_INDEX                32
#define LO_INDEX                33
#define TRUE                    1
#define FALSE                   0
#define V0_INDEX                2
#define A0_INDEX                4
#define PRINT_INT_SYSCALL       1
#define EXIT_SYSCALL            10
#define PRINT_CHAR_SYSCALL      11

#define RTYPE_INSTRUCTION       0       // 000000
#define ADD_INSTRUCTION         32      // 100000
#define SUB_INSTRUCTION         34      // 100010
#define SLT_INSTRUCTION         42      // 101010
#define MFHI_INSTRUCTION        16      // 010000
#define MFLO_INSTRUCTION        18      // 010010
#define MULT_INSTRUCTION        24      // 011000
#define DIV_INSTRUCTION         26      // 011010
#define MUL_INSTRUCTION         28      // 011100
#define BEQ_INSTRUCTION         4       // 000100
#define BNE_INSTRUCTION         5       // 000101
#define ADDI_INSTRUCTION        8       // 001000
#define ORI_INSTRUCTION         13      // 001101
#define LUI_INSTRUCTION         15      // 001111
#define SYSCALL_INSTRUCTION     12      // 001100

void execute_instructions(uint32_t n_instructions, uint32_t instructions[], int trace_mode);
char *process_arguments(int argc, char *argv[], int *trace_mode);
uint32_t *read_instructions(char *filename, uint32_t *n_instructions_p);
uint32_t *instructions_realloc(uint32_t *instructions, uint32_t n_instructions);

// ADD ANY ADDITIONAL FUNCTION PROTOTYPES HERE
void init_registers(int32_t registers[]);
void handle_instructions(uint32_t instruction, int32_t registers[], int trace_mode);
void handle_branching_instructions(uint32_t instruction, int32_t registers[], int trace_mode, uint32_t *pc, uint32_t *n_instructions);
void handle_rtype_instruction(uint32_t instruction, int32_t registers[], int trace_mode);
void handle_itype_instruction(uint32_t instruction, int32_t registers[], int trace_mode, uint32_t opcode);
void do_add_instruction(uint32_t instruction, int32_t registers[], int trace_mode);
void do_sub_instruction(uint32_t instruction, int32_t registers[], int trace_mode);
void do_slt_instruction(uint32_t instruction, int32_t registers[], int trace_mode);
void do_mfhi_instruction(uint32_t instruction, int32_t registers[], int trace_mode);
void do_mflo_instruction(uint32_t instruction, int32_t registers[], int trace_mode);
void do_mult_instruction(uint32_t instruction, int32_t registers[], int trace_mode);
void do_div_instruction(uint32_t instruction, int32_t registers[], int trace_mode);
void do_mul_instruction(uint32_t instruction, int32_t registers[], int trace_mode);
void do_beq_instruction(uint32_t instruction, int32_t registers[], int trace_mode, uint32_t *pc, uint32_t *n_instructions);
void do_bne_instruction(uint32_t instruction, int32_t registers[], int trace_mode, uint32_t *pc, uint32_t *n_instructions);
void do_addi_instruction(uint32_t instruction, int32_t registers[], int trace_mode);
void do_ori_instruction(uint32_t instruction, int32_t registers[], int trace_mode);
void do_lui_instruction(uint32_t instruction, int32_t registers[], int trace_mode);
void do_syscall_instruction(uint32_t instruction, int32_t registers[], int trace_mode);
int is_branch_instruction(uint32_t instruction);
int get_opcode(uint32_t instruction);
int16_t extract_immediate(uint32_t instruction);
int get_register_index(uint32_t instruction, uint32_t offset);

// YOU DO NOT NEED TO CHANGE MAIN
// but you can if you really want to
int main(int argc, char *argv[]) {
    int trace_mode;
    char *filename = process_arguments(argc, argv, &trace_mode);

    uint32_t n_instructions;
    uint32_t *instructions = read_instructions(filename, &n_instructions);

    execute_instructions(n_instructions, instructions, trace_mode);

    free(instructions);
    return 0;
}

// simulate execution of  instruction codes in  instructions array
// output from syscall instruction & any error messages are printed
//
// if trace_mode != 0:
//     information is printed about each instruction as it executed
//
// execution stops if it reaches the end of the array
void execute_instructions(uint32_t n_instructions, uint32_t instructions[], int trace_mode) {
    // REPLACE THIS FUNCTION WITH YOUR OWN IMPLEMENTATION

    int32_t registers[34];
    init_registers(registers);

    for (uint32_t pc = 0; pc < n_instructions; pc++) {
        if (trace_mode) {
            printf("%u: 0x%08X", pc, instructions[pc]);
        }
        if (is_branch_instruction(instructions[pc])) {
            handle_branching_instructions(instructions[pc], registers, trace_mode, &pc, &n_instructions);
        } else {
            handle_instructions(instructions[pc], registers, trace_mode);
        }
    }
}

// ADD YOUR FUNCTIONS HERE
// intialises all registers, including HI/LO, to 0
void init_registers(int32_t registers[]) {
    for (int i = 0; i < REGISTERS_SIZE + 2; i++) {
        registers[i] = 0;
    }
}

// handles rtype and itype instructions differently
void handle_instructions(uint32_t instruction, int32_t registers[], int trace_mode) {
    uint32_t opcode = get_opcode(instruction);

    if (opcode == RTYPE_INSTRUCTION) {
        handle_rtype_instruction(instruction, registers, trace_mode);
    } else {
        handle_itype_instruction(instruction, registers, trace_mode, opcode);
    }
}

// handles the two branching instructions
void handle_branching_instructions(uint32_t instruction, int32_t registers[], int trace_mode, uint32_t *pc, uint32_t *n_instructions) {
    uint32_t opcode = get_opcode(instruction);

    if (opcode == BEQ_INSTRUCTION) {
        do_beq_instruction(instruction, registers, trace_mode, pc, n_instructions);
    } else {
        do_bne_instruction(instruction, registers, trace_mode, pc, n_instructions);
    }
}

// implements the logic for all rtype instructions
void handle_rtype_instruction(uint32_t instruction, int32_t registers[], int trace_mode) {
    uint32_t mask = 1;
    mask = mask << FUNCTION_SIZE;
    mask = mask - 1;
    uint32_t function = mask & instruction;

    if (function == ADD_INSTRUCTION) {
        do_add_instruction(instruction, registers, trace_mode);
    } else if (function == SUB_INSTRUCTION) {
        do_sub_instruction(instruction, registers, trace_mode);
    } else if (function == SLT_INSTRUCTION) {
        do_slt_instruction(instruction, registers, trace_mode);
    } else if (function == MFHI_INSTRUCTION) {
        do_mfhi_instruction(instruction, registers, trace_mode);
    } else if (function == MFLO_INSTRUCTION) {
        do_mflo_instruction(instruction, registers, trace_mode);
    } else if (function == MULT_INSTRUCTION) {
        do_mult_instruction(instruction, registers, trace_mode);
    } else if (function == DIV_INSTRUCTION) {
        do_div_instruction(instruction, registers, trace_mode);
    } else if (function == SYSCALL_INSTRUCTION) {
        do_syscall_instruction(instruction, registers, trace_mode);
    } else {
        fprintf(stderr, "Invalid instruction\n");
    }
}

// implements the logic for all itype instructions
void handle_itype_instruction(uint32_t instruction, int32_t registers[], int trace_mode, uint32_t opcode) {
    if (opcode == MUL_INSTRUCTION) {
        do_mul_instruction(instruction, registers, trace_mode);
    } else if (opcode == ADDI_INSTRUCTION) {
        do_addi_instruction(instruction, registers, trace_mode);
    } else if (opcode == ORI_INSTRUCTION) {
        do_ori_instruction(instruction, registers, trace_mode);
    } else if (opcode == LUI_INSTRUCTION) {
        do_lui_instruction(instruction, registers, trace_mode);
    }
}

// implements add instruction
void do_add_instruction(uint32_t instruction, int32_t registers[], int trace_mode) {
    int reg_one = get_register_index(instruction, 21);
    int reg_two = get_register_index(instruction, 16);
    int reg_three = get_register_index(instruction, 11);

    registers[reg_three] = registers[reg_one] + registers[reg_two];

    if (trace_mode) {
        printf(" add  $%d, $%d, $%d\n", reg_three, reg_one, reg_two);
        printf(">>> $%d = %d\n", reg_three, registers[reg_three]);
    }
    if (reg_three == 0) {
        registers[reg_three] = 0;
    }
}

// implements sub instruction
void do_sub_instruction(uint32_t instruction, int32_t registers[], int trace_mode) {
    int reg_one = get_register_index(instruction, 21);
    int reg_two = get_register_index(instruction, 16);
    int reg_three = get_register_index(instruction, 11);

    registers[reg_three] = registers[reg_one] - registers[reg_two];

    if (trace_mode) {
        printf(" sub  $%d, $%d, $%d\n", reg_three, reg_one, reg_two);
        printf(">>> $%d = %d\n", reg_three, registers[reg_three]);
    }
    if (reg_three == 0) {
        registers[reg_three] = 0;
    }
}

// implements slt instruction
void do_slt_instruction(uint32_t instruction, int32_t registers[], int trace_mode) {
    int reg_one = get_register_index(instruction, 21);
    int reg_two = get_register_index(instruction, 16);
    int reg_three = get_register_index(instruction, 11);

    registers[reg_three] = (registers[reg_one] < registers[reg_two]) ? TRUE : FALSE;

    if (trace_mode) {
        printf(" slt  $%d, $%d, $%d\n", reg_three, reg_one, reg_two);
        printf(">>> $%d = %d\n", reg_three, registers[reg_three]);
    }
    if (reg_three == 0) {
        registers[reg_three] = 0;
    }
}

// implements mfhi instruction
void do_mfhi_instruction(uint32_t instruction, int32_t registers[], int trace_mode) {
    int reg = get_register_index(instruction, 11);

    registers[reg] = registers[HI_INDEX];

    if (trace_mode) {
        printf(" mfhi $%d\n", reg);
        printf(">>> $%d = %d\n", reg, registers[reg]);
    }
    if (reg == 0) {
        registers[reg] = 0;
    }
}

// implements mflo instruction
void do_mflo_instruction(uint32_t instruction, int32_t registers[], int trace_mode) {
    int reg = get_register_index(instruction, 11);

    registers[reg] = registers[LO_INDEX];

    if (trace_mode) {
        printf(" mflo $%d\n", reg);
        printf(">>> $%d = %d\n", reg, registers[reg]);
    }
    if (reg == 0) {
        registers[reg] = 0;
    }
}

// implements mult instruction
void do_mult_instruction(uint32_t instruction, int32_t registers[], int trace_mode) {
    int reg_one = get_register_index(instruction, 21);
    int reg_two = get_register_index(instruction, 16);

    uint64_t num_one = registers[reg_one];
    uint64_t num_two = registers[reg_two];

    uint64_t product = num_one * num_two;

    uint64_t mask = 1;
    mask = mask << 32;
    mask = mask - 1;
    uint32_t lo = mask & product;

    mask = mask << 32;
    uint64_t temp = mask & product;
    uint32_t hi = temp >> 32;

    registers[HI_INDEX] = hi;
    registers[LO_INDEX] = lo;

    if (trace_mode) {
        printf(" mult $%d, $%d\n", reg_one, reg_two);
        printf(">>> HI = %d\n", registers[HI_INDEX]);
        printf(">>> LO = %d\n", registers[LO_INDEX]);
    }

}

// implements div instruction
void do_div_instruction(uint32_t instruction, int32_t registers[], int trace_mode) {
    int reg_one = get_register_index(instruction, 21);
    int reg_two = get_register_index(instruction, 16);

    if (registers[reg_two] != 0) {
        registers[HI_INDEX] = registers[reg_one] % registers[reg_two];
        registers[LO_INDEX] = registers[reg_one] / registers[reg_two];
    }

    if (trace_mode) {
        printf(" div  $%d, $%d\n", reg_one, reg_two);
        printf(">>> HI = %d\n", registers[HI_INDEX]);
        printf(">>> LO = %d\n", registers[LO_INDEX]);
    }
}

// implements mul instruction
void do_mul_instruction(uint32_t instruction, int32_t registers[], int trace_mode) {
    int reg_one = get_register_index(instruction, 21);
    int reg_two = get_register_index(instruction, 16);
    int reg_three = get_register_index(instruction, 11);

    registers[reg_three] = registers[reg_one] * registers[reg_two];

    if (trace_mode) {
        printf(" mul  $%d, $%d, $%d\n", reg_three, reg_two, reg_one);
        printf(">>> $%d = %d\n", reg_three, registers[reg_three]);
    }
    if (reg_three == 0) {
        registers[reg_three] = 0;
    }
}

// implements beq instruction
void do_beq_instruction(uint32_t instruction, int32_t registers[], int trace_mode, uint32_t *pc, uint32_t *n_instructions) {
    int reg_one = get_register_index(instruction, 21);
    int reg_two = get_register_index(instruction, 16);

    int16_t offset = extract_immediate(instruction);

    int branch_taken = FALSE;
    if (registers[reg_one] == registers[reg_two]) {
        branch_taken = TRUE;

        if (*pc + offset < 0 || *pc + offset >= *n_instructions) {
            if (trace_mode) {
                printf(" beq  $%d, $%d, %d\n", reg_one, reg_two, offset);
                printf(">>> branch taken to PC = %u\n", *pc + 1);
            }
            fprintf(stderr, "Illegal branch to non-instruction: PC = %u\n", *pc + offset);
            exit(0);
        } else {
            *pc = *pc + offset - 1;
        }
    }

    if (trace_mode) {
        if (branch_taken) {
            printf(" beq  $%d, $%d, %d\n", reg_one, reg_two, offset);
            printf(">>> branch taken to PC = %u\n", *pc + 1);
        } else {
            printf(" beq  $%d, $%d, %d\n", reg_one, reg_two, offset);
            printf(">>> branch not taken\n");
        }
    }
}

// implements bne instruction
void do_bne_instruction(uint32_t instruction, int32_t registers[], int trace_mode, uint32_t *pc, uint32_t *n_instructions) {
    int reg_one = get_register_index(instruction, 21);
    int reg_two = get_register_index(instruction, 16);

    int16_t offset = extract_immediate(instruction);

    int branch_taken = FALSE;
    if (registers[reg_one] != registers[reg_two]) {
        branch_taken = TRUE;

        if (*pc + offset < 0 || *pc + offset >= *n_instructions) {
            if (trace_mode) {
                printf(" bne  $%d, $%d, %d\n", reg_one, reg_two, offset);
                printf(">>> branch taken to PC = %u\n", *pc + 1);
            }
            fprintf(stderr, "Illegal branch to non-instruction: PC = %u\n", *pc + offset);
            exit(0);
        } else {
            *pc = *pc + offset - 1;
        }
    }

    if (trace_mode) {
        if (branch_taken) {
            printf(" bne  $%d, $%d, %d\n", reg_one, reg_two, offset);
            printf(">>> branch taken to PC = %d\n", *pc + 1);
        } else {
            printf(" bne  $%d, $%d, %d\n", reg_one, reg_two, offset);
            printf(">>> branch not taken\n");
        }
    }
}

// implements addi instruction
void do_addi_instruction(uint32_t instruction, int32_t registers[], int trace_mode) {
    int16_t immediate = extract_immediate(instruction);

    int reg_one = get_register_index(instruction, 21);
    int reg_two = get_register_index(instruction, 16);

    registers[reg_two] = registers[reg_one] + immediate;

    if (trace_mode) {
        printf(" addi $%d, $%d, %d\n", reg_two, reg_one, immediate);
        printf(">>> $%d = %d\n", reg_two, registers[reg_two]);
    }
    if (reg_two == 0) {
        registers[reg_two] = 0;
    }
}

// implements ori instruction
void do_ori_instruction(uint32_t instruction, int32_t registers[], int trace_mode) {
    int16_t immediate = extract_immediate(instruction);

    int reg_one = get_register_index(instruction, 21);
    int reg_two = get_register_index(instruction, 16);

    uint16_t immediate_unsigned = immediate;

    registers[reg_two] = registers[reg_one] | immediate_unsigned;

    if (trace_mode) {
        printf(" ori  $%d, $%d, %d\n", reg_two, reg_one, immediate);
        printf(">>> $%d = %u\n", reg_two, registers[reg_two]);
    }
    if (reg_two == 0) {
        registers[reg_two] = 0;
    }
}

// implements lui instruction
void do_lui_instruction(uint32_t instruction, int32_t registers[], int trace_mode) {
    int16_t immediate = extract_immediate(instruction);

    int reg = get_register_index(instruction, 16);

    registers[reg] = immediate << DEFAULT_LUI_BIT_SHIFT;

    if (trace_mode) {
        printf(" lui  $%d, %d\n", reg, immediate);
        printf(">>> $%d = %d\n", reg, registers[reg]);
    }
    if (reg == 0) {
        registers[reg] = 0;
    }
}

// implements syscalls
void do_syscall_instruction(uint32_t instruction, int32_t registers[], int trace_mode) {
    if (trace_mode) {
        printf(" syscall\n");
        printf(">>> syscall %d\n", registers[V0_INDEX]);
    }
    if (registers[V0_INDEX] == PRINT_INT_SYSCALL) {
        if (trace_mode) {
            printf("<<< ");
        }
        printf("%d", registers[A0_INDEX]);
    } else if (registers[V0_INDEX] == PRINT_CHAR_SYSCALL) {
        if (trace_mode) {
            printf("<<< ");
        }
        printf("%c", registers[A0_INDEX]);
    } else if (registers[V0_INDEX] == EXIT_SYSCALL) {
        exit(0);
    } else {
        fprintf(stderr, "Unknown system call: %d\n", registers[V0_INDEX]);
        exit(0);
    }
    if (trace_mode) {
        printf("\n");
    }
}

// extracts the last 16 bits of 32 bit instruction
// and returns as a signed 16 bit integer
int16_t extract_immediate(uint32_t instruction) {
    uint32_t mask = 1;
    mask = mask << IMMEDIATE_SIZE;
    mask = mask - 1;
    int16_t offset = mask & instruction;
    return offset;
}

// given an instruction and a offset, returns the
// index of a register
//
// for example, an add instruction has bit pattern
//       000000ssssstttttddddd00000100000
// get_register_index(instruction, 21) returns sssss
// get_register_index(instruction, 16) returns ttttt
// get_register_index(instruction, 11) returns ddddd
int get_register_index(uint32_t instruction, uint32_t offset) {
    uint32_t mask = 1;
    mask = mask << 5;
    mask = mask - 1;
    mask = mask << offset;
    uint32_t reg = mask & instruction;
    reg = reg >> offset;
    return reg;
}

// returns TRUE if a given instruction is a branch instruction
// returns FALSE otherwise
int is_branch_instruction(uint32_t instruction) {
    uint32_t opcode = get_opcode(instruction);

    return (opcode == BEQ_INSTRUCTION || opcode == BNE_INSTRUCTION);
}

// returns the opcode (first 6 bits) of a given instruction
int get_opcode(uint32_t instruction) {
    uint32_t mask = 1;
    mask = mask << OPCODE_SIZE;
    mask = mask - 1;
    mask = mask << 26;
    uint32_t opcode = mask & instruction;
    opcode = opcode >> 26;

    return opcode;
}

// DO NOT CHANGE ANY CODE BELOW HERE

// check_arguments is given command-line arguments
// it sets *trace_mode to 0 if -r is specified
//         *trace_mode is set to 1 otherwise
// the filename specified in command-line arguments is returned
char *process_arguments(int argc, char *argv[], int *trace_mode) {
    if (
        argc < 2 ||
        argc > 3 ||
        (argc == 2 && strcmp(argv[1], "-r") == 0) ||
        (argc == 3 && strcmp(argv[1], "-r") != 0)
    ) {
        fprintf(stderr, "Usage: %s [-r] <file>\n", argv[0]);
        exit(1);
    }
    *trace_mode = (argc == 2);
    return argv[argc - 1];
}


// read hexadecimal numbers from filename one per line
// numbers are return in a malloc'ed array
// *n_instructions is set to size of the array
uint32_t *read_instructions(char *filename, uint32_t *n_instructions_p) {
    FILE *f = fopen(filename, "r");
    if (f == NULL) {
        perror(filename);
        exit(1);
    }

    uint32_t *instructions = NULL;
    uint32_t n_instructions = 0;
    char line[MAX_LINE_LENGTH + 1];
    while (fgets(line, sizeof line, f) != NULL) {

        // grow instructions array in steps of INSTRUCTIONS_GROW elements
        if (n_instructions % INSTRUCTIONS_GROW == 0) {
            instructions = instructions_realloc(instructions, n_instructions + INSTRUCTIONS_GROW);
        }

        char *endptr;
        instructions[n_instructions] = (uint32_t)strtoul(line, &endptr, 16);
        if (*endptr != '\n' && *endptr != '\r' && *endptr != '\0') {
            fprintf(stderr, "line %d: invalid hexadecimal number: %s",
                    n_instructions + 1, line);
            exit(1);
        }
        if (instructions[n_instructions] != strtoul(line, &endptr, 16)) {
            fprintf(stderr, "line %d: number too large: %s",
                    n_instructions + 1, line);
            exit(1);
        }
        n_instructions++;
    }
    fclose(f);
    *n_instructions_p = n_instructions;
    // shrink instructions array to correct size
    instructions = instructions_realloc(instructions, n_instructions);
    return instructions;
}


// instructions_realloc is wrapper for realloc
// it calls realloc to grow/shrink the instructions array
// to the specified size
// it exits if realloc fails
// otherwise it returns the new instructions array
uint32_t *instructions_realloc(uint32_t *instructions, uint32_t n_instructions) {
    instructions = realloc(instructions, n_instructions * sizeof *instructions);
    if (instructions == NULL) {
        fprintf(stderr, "out of memory");
        exit(1);
    }
    return instructions;
}
