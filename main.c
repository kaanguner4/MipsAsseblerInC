/*
 *      PROJECT-3
 *      KAAN GÜNER
 *      220201068
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Maximum line length, number of lines, and number of labels definitions
#define MAX_LINE_LENGTH 256
#define MAX_LINES 1000
#define MAX_LABELS 100
#define BASE_ADDRESS 0x00400000 // Starting address of the program

// Structure to hold label information
struct Label {
    char label[MAX_LINE_LENGTH]; // Label name
    int address; // Label address
};

// Counter for labels and array to store label information
int labelCount = 0;
struct Label labels[MAX_LABELS];

// Function to read a file and store each line in an array
int readFile(const char *filename, char lines[MAX_LINES][MAX_LINE_LENGTH]) {
    FILE *file = fopen(filename, "r"); // Open the file in read mode

    int lineCount = 0;
    while (fgets(lines[lineCount], MAX_LINE_LENGTH, file)) { // Read lines from the file
        lines[lineCount][strcspn(lines[lineCount], "\r\n")] = '\0'; // Remove newline characters
        lineCount++;
    }

    fclose(file); // Close the file
    return lineCount; // Return the number of lines read
}

// Function to separate labels and determine their addresses
void labelSeparation(char lines[MAX_LINES][MAX_LINE_LENGTH], int lineCount) {
    int address = BASE_ADDRESS; // Starting address

    for (int i = 0; i < lineCount; i++) {
        if (lines[i][strlen(lines[i]) - 1] == ':') { // If the line ends with ':', treat it as a label
            strncpy(labels[labelCount].label, lines[i], strlen(lines[i]) - 1); // Copy the label name
            labels[labelCount].label[strlen(lines[i]) - 1] = '\0'; // Remove the ':' character
            labels[labelCount].address = address; // Assign address to the label
            labelCount++; // Increment the label count
        } else if (lines[i][0] != '\0') { // If the line is not empty
            address += 4; // Increment the address by 4 bytes (instruction length)
        }
    }
}

// Function to return the address of a given label
int getLabelAddress(const char *label) {
    for (int i = 0; i < labelCount; i++) {
        if (strcmp(labels[i].label, label) == 0) { // If label name matches
            return labels[i].address; // Return the address
        }
    }
}

// Function to convert register information to numeric value
int parseRegister(const char *reg) {
    return atoi(reg + 1); // Convert register name to number (e.g., $t0 -> 8)
}

// Function to convert immediate value to numeric value
int parseImmediate(const char *imm) {
    return atoi(imm); // Convert immediate value
}

// Function to translate an assembly instruction into machine code
char *translateInstruction(const char *instruction, int currentAddress) {
    static char formatted[30]; // Array for formatted machine code
    static char machineCode[30]; // Array to store machine code for J-type instructions
    char parts[4][MAX_LINE_LENGTH]; // Array to hold instruction parts
    int partCount = 0; // Count of instruction parts

    char *token = strtok((char *)instruction, " ,\t"); // Split the instruction (using commas and spaces as delimiters)
    while (token) {
        strncpy(parts[partCount++], token, MAX_LINE_LENGTH); // Store the part in the array
        token = strtok(NULL, " ,\t");
    }

    // Translation for R-type instructions (e.g., add, sub, and, or)
    if (strcmp(parts[0], "add") == 0) {
        // add instruction: rs (source register), rt (target register), rd (destination register)
        unsigned int result = (parseRegister(parts[2]) << 21) | (parseRegister(parts[3]) << 16) | (parseRegister(parts[1]) << 11) | 0x20;
        snprintf(formatted, sizeof(formatted), "0x%08X", result); // Format the machine code
        return formatted;
    }
    if (strcmp(parts[0], "sub") == 0) {
        unsigned int result = (parseRegister(parts[2]) << 21) | (parseRegister(parts[3]) << 16) | (parseRegister(parts[1]) << 11) | 0x22;
        snprintf(formatted, sizeof(formatted), "0x%08X", result);
        return formatted;
    }
    if (strcmp(parts[0], "and") == 0) {
        unsigned int result = (parseRegister(parts[2]) << 21) | (parseRegister(parts[3]) << 16) | (parseRegister(parts[1]) << 11) | 0x24;
        snprintf(formatted, sizeof(formatted), "0x%08X", result);
        return formatted;
    }
    if (strcmp(parts[0], "or") == 0) {
        unsigned int result = (parseRegister(parts[2]) << 21) | (parseRegister(parts[3]) << 16) | (parseRegister(parts[1]) << 11) | 0x25;
        snprintf(formatted, sizeof(formatted), "0x%08X", result);
        return formatted;
    }
    if (strcmp(parts[0], "sll") == 0) {
        // sll instruction: rd, rt, shamt (shift amount)
        unsigned int result = parseRegister(parts[1]) << 11 | parseRegister(parts[2]) << 16 | atoi(parts[3]) << 6 | 0x00;
        snprintf(formatted, sizeof(formatted), "0x%08X", result);
        return formatted;
    }
    if (strcmp(parts[0], "srl") == 0) {
        unsigned int result = parseRegister(parts[1]) << 11 | parseRegister(parts[2]) << 16 | atoi(parts[3]) << 6 | 0x02;
        snprintf(formatted, sizeof(formatted), "0x%08X", result);
        return formatted;
    }

    // Translation for I-type instructions (e.g., addi, andi, beq, bne)
    if (strcmp(parts[0], "addi") == 0) {
        // addi instruction: rt, rs, immediate
        unsigned int result = 0x20000000 | (parseRegister(parts[2]) << 21) | (parseRegister(parts[1]) << 16) | (parseImmediate(parts[3]) & 0xFFFF);
        snprintf(formatted, sizeof(formatted), "0x%08X", result);
        return formatted;
    }
    if (strcmp(parts[0], "andi") == 0) {
        unsigned int result = 0x30000000 | (parseRegister(parts[2]) << 21) | (parseRegister(parts[1]) << 16) | (parseImmediate(parts[3]) & 0xFFFF);
        snprintf(formatted, sizeof(formatted), "0x%08X", result);
        return formatted;
    }
    if (strcmp(parts[0], "beq") == 0) {
        // beq instruction: rs, rt, offset (branch target)
        int offset = (parseImmediate(parts[3]) - currentAddress - 4) / 4;
        unsigned int result = 0x10000000 | (parseRegister(parts[1]) << 21) | (parseRegister(parts[2]) << 16) | (offset & 0xFFFF);
        snprintf(formatted, sizeof(formatted), "0x%08X", result);
        return formatted;
    }
    if (strcmp(parts[0], "bne") == 0) {
        int offset = (parseImmediate(parts[3]) - currentAddress - 4) / 4;
        unsigned int result = 0x14000000 | (parseRegister(parts[1]) << 21) | (parseRegister(parts[2]) << 16) | (offset & 0xFFFF);
        snprintf(formatted, sizeof(formatted), "0x%08X", result);
        return formatted;
    }

    // Translation for J-type instructions (e.g., j)
    if (strcmp(parts[0], "j") == 0) {
        // j instruction: target address
        unsigned int address = getLabelAddress(parts[1]); // Get the label address
        unsigned int result = 0x08000000 | ((address >> 2) & 0x3FFFFFF); // Format the address
        snprintf(machineCode, sizeof(machineCode), "0x%08X", result);
        return machineCode;
    }
}

// Function to write machine codes to a file
void writeOutputFile(const char *filename, char machineCode[MAX_LINES][MAX_LINE_LENGTH], int codeCount) {
    FILE *file = fopen(filename, "w"); // Open the file in write mode

    fprintf(file, "Address\t\t\tCode\n"); // Header line
    int address = BASE_ADDRESS;
    for (int i = 0; i < codeCount; i++) {
        fprintf(file, "0x%08X\t\t%s\n", address, machineCode[i]); // Write address and codes
        address += 4;
    }
    fclose(file); // Close the file
}

// Main function
int main() {
    char lines[MAX_LINES][MAX_LINE_LENGTH]; // Array for assembly codes
    int lineCount = readFile("marsmips.asm", lines); // Read the file and get the line count

    labelSeparation(lines, lineCount); // Separate labels

    char machineCode[MAX_LINES][MAX_LINE_LENGTH]; // Array for machine codes
    int codeCount = 0;

    int address = BASE_ADDRESS;
    for (int i = 0; i < lineCount; i++) {
        if (!strchr(lines[i], ':') && lines[i][0] != '\0') { // Process lines that are not labels
            strcpy(machineCode[codeCount++], translateInstruction(lines[i], address)); // Translate instructions
            address += 4;
        }
    }

    writeOutputFile("marsmips.obj", machineCode, codeCount); // Write the translated codes to a file

    printf("\nDone\n"); // Completion message

    return 0;
}
