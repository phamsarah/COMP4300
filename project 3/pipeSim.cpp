#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <iostream>

uint64_t convertToInt(const char *string);
int getLine(FILE *file, char *buffer, size_t length);
void loadDataSection(const char* file, uint64_t* memory);
void loadTextSection(const char* file, uint64_t* memory);
int locateAddress(uint32_t address);
uint64_t getMemory(uint64_t* memory, uint32_t address);
uint32_t stringValue(char* str);
void setMemory(uint64_t* memory, uint32_t address, uint64_t data);
using namespace std;

#define ADD		0x0001
#define MULT	0x0002
#define LOAD	0x0003
#define STO		0x0004
#define ADDI	0x0005
#define B		0x0006
#define BEQZ	0x0007
#define BGE		0x0008
#define BNE		0x0009
#define LA		0x0010
#define LB		0x0011
#define LI		0x0012
#define SUBI	0x0013
#define SYSCALL	0x0014
#define END		0x0020
#define NOP	    0x0000
#define LABEL_SIZE 20

#define ARRAY_SIZE 100
#define START_ADDRESS 0x00400000
#define USER_TEXT 0x00400000
#define USER_DATA 0x01000000
#define STACK_DATA 0x7FFFFFFF
#define BUFFER_SIZE 512

char instructions[LABEL_SIZE][BUFFER_SIZE];
uint32_t pc;
uint32_t instr_memory[10];
uint32_t gpr[32];

// Each function in the method will have an instruction
uint64_t if_Instr;
uint64_t id_Instr;
uint64_t exe_Instr;
uint64_t mem_Instr;
uint64_t wb_Instr;

// Each instruction will have a pointer counter
uint32_t id_pc;
uint32_t exe_pc;
uint32_t mem_pc;
uint32_t wb_pc;

// Each buffer for register destination 
uint32_t id_regDest;
uint32_t exe_regDest;
uint32_t mem_regDest;
uint32_t wb_regDest;
uint32_t id_reg1;
uint32_t exe_reg1;
uint32_t id_reg2;
uint32_t exe_reg2;

// Buffer for offset/immediate
uint32_t id_offset;
uint32_t exe_offset;
uint32_t mem_offset;
uint32_t wb_offset;

// ALU
uint32_t exe_alu;
uint32_t mem_alu;
uint32_t wb_alu;

// Memory
uint32_t mem_mem;
uint32_t wb_Mem;

int program_end;
int instruction_count;
int user_mode;
int cycle_count;

uint32_t exe_result;

struct memory_blocks{
	uint64_t
		user_text[ARRAY_SIZE], 
		user_data[ARRAY_SIZE], 
		stack_data[ARRAY_SIZE];
};

// This method uses our custom defined ranges and locates the address within those boundaries
int locateAddress(uint32_t address) {
	if(address < USER_TEXT)
		return 0;
	else if((address >= USER_TEXT)&&(address < USER_DATA))
		return (address - USER_TEXT)/64;
	else if((address >= USER_DATA)&&(address < 0x38000000))
		return ((address - USER_DATA)/64)+ARRAY_SIZE;
	else if(address <= STACK_DATA)
		return ((STACK_DATA - address)/64) + (ARRAY_SIZE * 2);
	else
		return 0;
}

// This method will return the data at the given address inside of our memory
uint64_t getMemory(uint64_t* memory, uint32_t address) {
	return memory[locateAddress(address)];
}

// This method is the setter for the memory array given a location and the data
// this method uses locateAddress to identify the given address
void setMemory(uint64_t* memory, uint32_t address, uint64_t data) {
	memory[locateAddress(address)] = data;
}

// This is a custom method that will buffer through the file
int getLine(FILE *file, char *buffer, size_t length) {

	memset(buffer,0,length);

	for(int i = 0; i < length; i++) {
		int c = fgetc(file);
		if(!feof(file)) {
			if(c == '\r' || c == '\n' || c == '#') {
				buffer[i] = 0;
				if(c != '#') return i + 1;
			}
			else buffer[i] = c;
		}
		else return -1;
	}
	return 0;
}

// This method goes through the data segment of the MIPS file and sets the data in memory
void loadDataSection(const char* file, uint64_t* memory) {
	FILE *fileOpen = fopen(file,"r");
	char buffer[BUFFER_SIZE], holder[BUFFER_SIZE];
	char* holder_2;
	char* data_memory;
	int data = 0, counter = 0, label_counter = 0, memory_counter = 0;
	bool lineIn = 0;
	uint32_t data_pointer = USER_DATA;

	while(getLine(fileOpen, buffer, BUFFER_SIZE) != -1) {
        //cout << "Buffer: " << buffer << endl;
		if(data == 1){	
			for(int i = 0; i < BUFFER_SIZE; i++){
				holder[i] = buffer[i];
				if(buffer[i] == ':') counter = i;
			}

			holder[counter] = 0;
			for(int i = counter; i < BUFFER_SIZE; i++){
				holder[i] = 0;
			}

			if((holder[0] != 0) && (holder[0] != '\t')){
                
				if((holder_2 = strstr(buffer,".asciiz")) != NULL){

					strcpy(instructions[label_counter], holder);
					instr_memory[label_counter] = data_pointer;

					for(int i = 0; i < BUFFER_SIZE; i++) {
						if(holder_2[i] == '"') lineIn = !lineIn;
						else if(lineIn) {
							data_memory = (char *)(&memory[locateAddress(data_pointer)]);
							data_memory[memory_counter] = holder_2[i];
							memory_counter++;
							if(holder_2[i] == 0) break;
						}

							if(holder_2[i] == 0) break;
					}
					data_memory[memory_counter] = 0;
					if((memory_counter % 8) == 0)
						data_pointer += (memory_counter * 8);
					else
					{
						memory_counter = (memory_counter / 8) * 64;
						memory_counter += 64;
						data_pointer += memory_counter;
					}

					label_counter++;
					memory_counter = 0;
			
				}
				else if((holder_2 = strstr(buffer,".space")) != NULL) {
					int space_value = convertToInt(holder_2);

					data_memory = (char *)(&memory[locateAddress(data_pointer)]);
					for(int i = 0; i < space_value; i++) data_memory[i] = 0;

					if((space_value % 8) == 0) data_pointer += (space_value * 8);
					else
					{
						space_value = (space_value / 8) * 64;
						space_value += 64;
						data_pointer += space_value;
					}
					memory_counter = 0;
				}
				else {
					uint64_t label_value = convertToInt(buffer);
					setMemory(memory,data_pointer,label_value);
					instr_memory[label_counter] = data_pointer;
					label_counter++;
					data_pointer += 64;
				}
			}
		}	
		if(strstr(buffer,".data") != NULL) {
			data = 1;
		}
	}
}

// This method will execute the instruction given the typeHolder, which is the type of instruction
void executeInstruction(uint64_t temp1, uint64_t temp2, char* holder, uint64_t* memory, uint64_t type, int typeHolder){
	temp1 = type;
	temp1 = temp1 << 32;

	if(typeHolder == 1){
		holder = strstr(holder,",");

		for(int i = 0; i < 10; i++) {
			if(strstr(holder,instructions[i]) != NULL){
				temp2 = instr_memory[i];
				break;
			}
		}
	}
	else if(typeHolder == 2){
		holder = strstr(holder, "$");
		temp2 = stringValue(holder);
	}
	else if(typeHolder == 3){ 
		temp2 = 0;

		if(temp1 == 'B'){
			if(holder[2] == '-') {
					temp2 = 1;
					temp2 = temp2 << 31;
				}
				temp2 += convertToInt(holder);
		}
	}

	setMemory(memory,pc, temp1 + temp2);
	pc += 64;

}

// This method goes through the text segment of the file and sets the text in the memory
void loadTextSection(const char* file, uint64_t* memory) {
	FILE *fileOpen = fopen(file,"r");
	char buffer[BUFFER_SIZE];
	char* holder;
	int text_section = 0, counter = 0;
	uint64_t temp1, temp2;

	while(getLine(fileOpen, buffer, 20) != -1) {
		cout << buffer << endl;
		if(strstr(buffer,".data") != NULL) {
			text_section = 0;
		}
		if(text_section == 1){
			if((holder = strstr(buffer,"MULT")) != NULL) executeInstruction(temp1, temp2, holder, memory, MULT, 1);
			else if((holder = strstr(buffer,"LOAD")) != NULL) executeInstruction(temp1, temp2, holder, memory, LOAD, 1);
			else if((holder = strstr(buffer,"STO")) != NULL) executeInstruction(temp1, temp2, holder, memory, STO, 1);
			else if((holder = strstr(buffer,"ADDI")) != NULL) executeInstruction(temp1, temp2, holder, memory, ADDI, 2);
			else if((holder = strstr(buffer,"BEQZ")) != NULL) executeInstruction(temp1, temp2, holder, memory, BEQZ, 2);
			else if((holder = strstr(buffer,"BGE")) != NULL) executeInstruction(temp1, temp2, holder, memory, BGE, 2);
			else if((holder = strstr(buffer,"BNE")) != NULL) executeInstruction(temp1, temp2, holder, memory, BNE, 2);
			else if((holder = strstr(buffer,"LA")) != NULL) executeInstruction(temp1, temp2, holder, memory, LA, 2);
			else if((holder = strstr(buffer,"LB")) != NULL) executeInstruction(temp1, temp2, holder, memory, LB, 2);
			else if((holder = strstr(buffer,"LI")) != NULL) executeInstruction(temp1, temp2, holder, memory, LI, 2);
			else if((holder = strstr(buffer,"SUBI")) != NULL) executeInstruction(temp1, temp2, holder, memory, SUBI, 2);
			else if((holder = strstr(buffer,"SYSCALL")) != NULL) executeInstruction(temp1, temp2, holder, memory, SYSCALL, 3);
			else if((holder = strstr(buffer,"END")) != NULL) executeInstruction(temp1, temp2, holder, memory, END, 3);
			else if((holder = strstr(buffer,"B")) != NULL) executeInstruction(temp1, temp2, holder, memory, B, 3);
			else if((holder = strstr(buffer,"ADD")) != NULL) executeInstruction(temp1, temp2, holder, memory, ADD, 1);

		}
		if(strstr(buffer,".text") != NULL) {
			text_section = 1;
		}
	}
}

// This method converts the string to an integer value
uint64_t convertToInt(const char *str) {
	const char *integer = "0123456789";
	int result = 0;
	int negative = 1;
	for(; *str; ++str) {
		int int_value;

		for(int_value = 0; integer[int_value]; ++int_value) {
			if(integer[int_value] == *str)
				break;
		}
		if(integer[int_value])
			result = 10 * result + int_value;		
	}

	return result;
}

// This method returns the total string value of a given string
uint32_t stringValue(char* str) {
	int number_shifts = 27;
	int in_number = 0;
	int result = 0;
	char* temp_index;
	for(int i = 0; i < BUFFER_SIZE; i++) {
		if(str[i] == '$'){
			temp_index = &str[i];
			in_number = 1;
		}
		else if(str[i] == ',') {
			if(in_number == 1){
				str[i] = 0;
				result += convertToInt(temp_index) << number_shifts;
				number_shifts -= 5;
				temp_index = &str[i + 1];
				in_number = 0;
			}
		}
		else if(str[i] == 0) {
			result += convertToInt(temp_index);
			break;
		}
	}
	return result;
}

// This method will print the instruction, registers used, offset, and address associated given a type
void printInstruction(uint32_t opcode, uint32_t regDest, uint32_t registerSource, uint32_t immediate, uint32_t pc, uint32_t offset, uint32_t gpr[], int type){

	cout << opcode << endl;

	if(type == 1){
		cout << "Destination register: " << regDest << endl;
		cout << "Computation register: " << registerSource << endl;
		cout << "Immediate: " << immediate << endl;
		if(opcode == ADDI) gpr[regDest] = gpr[registerSource] + immediate;
		else gpr[regDest] = gpr[registerSource] - immediate;
		cout << "Reg" << regDest << " = " << gpr[regDest] << endl;
	}
	else if(type == 2){
		cout << "Register: " << regDest << endl;
		cout << "Immediate: " << offset << endl;
	}
	else if(type == 3){
		cout << "Register: " << regDest << endl;
		cout << "Offset:  " << offset << endl;
		cout << "Address: " << pc + offset << endl;
	}
	else if(type == 4){
		cout << "Destination register: " << regDest << endl;
		cout << "Address register: " << gpr[registerSource] << endl;
		cout << "Offset: " << immediate << endl;
	}
	else if(type == 5){
		cout << "First register: " << regDest << endl;
		cout << "Second register: " << registerSource << endl;
		cout << "Offset: " << immediate << endl;
	}
}

// This method will count the number of instructions and fetch the instructions
void instr_fetch(uint64_t* memory){
    if(program_end != 1){
        if_Instr = getMemory(memory, pc);
        pc += 64;
    }
    if((if_Instr >> 32) == END) program_end = 1;

    switch((if_Instr >> 32)){
        case B:
        case BEQZ:
        case BGE:
        case BNE:
        case LI:
        case LA:
        case ADD:
        case ADDI:
        case SUBI:
        case NOP:
        case SYSCALL:
        case LB:
            instruction_count++;
            break;
    }
}

void instr_decode(){
    uint32_t regDest, regSrc1;
		uint32_t immediate, offset, boffset, temp4;
		uint64_t temp1, temp2;
		uint64_t accum;
		uint32_t opcode;	
		uint32_t address;
		uint64_t instruction;
		char* pointerchar;

        instruction = id_Instr;
		address = id_Instr;
		opcode = id_Instr >> 32;
		regDest = (address >> 27);
		regSrc1 = ((address << 5) >> 27);
		immediate =  ((address << 10) >> 10);
		offset = ((address << 5) >> 5);
		boffset = address;
		int32_t reg1 = signed(id_reg1);

        switch(opcode){
		case B:
			if((address >> 31) == 1)
				pc = id_pc - boffset;
			else
				pc = id_pc + boffset;
				break;
		case LI:
		case LA:
			id_regDest = regDest;
			id_offset = offset;
			break;
		case BEQZ:
			id_reg1 = gpr[regDest];
			id_offset = offset;
			if(id_reg1 == 0)
				pc = id_pc + id_offset;
			break;
		case BGE:
			id_reg1 = gpr[regDest]+1;
			id_reg2 = gpr[regSrc1]+1;
			id_offset = immediate;
			if(id_reg1 >= id_reg2)
				pc = id_pc - id_offset;
			break;
		case BNE:
			id_reg1 = gpr[regDest];
			id_reg2 = gpr[regSrc1];
			id_offset = immediate;
			if(id_reg2 != id_reg1)
				pc = id_pc + id_offset;
			break;
		case ADD:
			id_reg1 = gpr[regSrc1];
			id_reg2 = gpr[immediate];
			id_regDest = regDest;
			break;
		case ADDI:
		case SUBI:
		case LB:
			id_reg1 = gpr[regSrc1];
			id_offset = immediate;
			id_regDest = regDest;
			break;
		case NOP:
		case SYSCALL:
		case END:
			break;
		}
}

void instr_execute(){
    uint32_t opcode;
    uint32_t register_1 = exe_Instr, register_2 = exe_Instr, mem_test = mem_Instr, wb_test = wb_Instr;
    uint64_t instr;

    mem_test = (mem_test >> 27);
    wb_test = (wb_test >> 27);

    register_1 = (register_1 << 17) >> 17;
    register_2 = (register_2 << 15) >> 15;

    if(register_1 == mem_regDest) exe_reg1 = mem_alu;
    else if (register_1 == wb_regDest) exe_reg1 = wb_alu;

    if(register_2 == mem_regDest) exe_reg2 = mem_alu;
    else if (register_2 == wb_regDest) exe_reg2 = wb_alu;

    opcode = exe_Instr >> 32;

    switch(opcode){
        case B:
        case BEQZ:
        case BGE:
        case BNE:
            break;
        case LI:
            exe_offset = exe_offset;
            exe_alu = exe_offset;
            break;
        case LA:
            exe_offset = exe_offset + exe_pc - 128;
            exe_alu = exe_offset;
            break;
        case ADD:
            exe_alu = exe_reg1 + exe_reg2;
        case ADDI:
        case LB:
            exe_alu = exe_reg1 + exe_offset;
            break;
        case SUBI:
            exe_alu = exe_reg1 - exe_offset;
            break;
        case NOP:
        case SYSCALL:
            break; 

    }
}

void memory_access(uint64_t* memory){
    uint32_t opcode;
    uint64_t instr;

    opcode = mem_Instr >> 32;

    switch(opcode){
        case B:
        case BEQZ:
        case BGE:
        case BNE:
        case LI:
        case LA:
        case ADD:
        case ADDI:
        case SUBI:
        case NOP:
        case SYSCALL:
            break;
        case LB:
            char* pointerchar;
            pointerchar = (char *) memory;
            uint32_t temp_4 = (mem_alu - USER_DATA) / 8;
            pointerchar += temp_4 + 1;
            cout << pointerchar << endl;
            mem_mem = uint32_t(pointerchar[0]);
            break;
    }
}

void write_back(){
    uint32_t opcode;
    uint64_t instr;

    opcode = wb_Instr >> 32;

    switch(opcode){
        case B:
        case BEQZ:
        case BGE:
        case BNE:
        case NOP:
        case SYSCALL:
            break;
        case ADD:
        case ADDI:
        case SUBI:
            gpr[wb_regDest] = wb_alu;
            break;
        case LI:
        case LA:
            gpr[wb_regDest] = wb_offset;
        case LB:
            gpr[wb_regDest] = wb_Mem;
            break;
        case END:
            user_mode = 0;
    }
}
// This is the main method, you can change the file name to lab2.s to test that file, the file must be in the same directory
int main(int argc, char* argv[]) {

	const char *file = "lab3c.s";
	if(argc != 1) file = argv[1];
	struct memory_blocks* memory;
	memory = (struct memory_blocks *) malloc(sizeof(struct memory_blocks));
	pc = START_ADDRESS;

	// We load the file into our custom memory structure
	loadDataSection(file, memory->user_text);
	loadTextSection(file, memory->user_text);

	uint32_t gpr[32];
	int user_mode = 1, instruction_counter = 0, cycles = 0;
	uint32_t regDest, registerSource, opcode, address, immediate, offset, boffset, temp4;
	uint64_t temp1, temp2, accum = 0;
	char* pointerchar = (char *)memory->user_data;
	pc = START_ADDRESS;
    wb_Instr = 0;
    mem_Instr = 0;
    exe_Instr = 0;
    id_Instr = 0;
    if_Instr = 0;

	while(user_mode == 1) {

        // We want to copy the instructions for each buffer 
        id_Instr = if_Instr;
        exe_Instr = id_Instr;
        mem_Instr = exe_Instr;
        wb_Instr = mem_Instr;

        // Copying the pointer counter  
        exe_pc = id_pc;
        wb_pc = mem_pc;
        mem_pc = exe_pc;
        id_pc = pc;

        wb_regDest = mem_regDest;
        mem_regDest = exe_regDest;
        exe_regDest = id_regDest;

        exe_reg1 = id_reg1;
        exe_reg2 = id_reg2;

        wb_offset = mem_offset;
        mem_offset = exe_offset;
        exe_offset = id_offset;

        wb_alu = mem_alu;
        mem_alu = exe_alu;

        wb_Mem = mem_mem;

        instr_fetch(memory -> user_text);

        instr_decode();

        instr_execute();

        memory_access(memory -> user_text);

        write_back();
        cycle_count++;
    }

	return 0;
}