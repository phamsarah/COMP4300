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

// This method is the user interface which mostly a print function that prints the instructions, retrieves data from memory, and prints registers associated
void userInterface(struct memory_blocks* memory, uint32_t pc){
	uint32_t gpr[32];
	int user_mode = 1, instruction_counter = 0, cycles = 0;
	uint32_t regDest, registerSource, opcode, address, immediate, offset, boffset, temp4;
	uint64_t temp1, temp2, accum = 0;
	char* pointerchar = (char *)memory->user_data;
	pc = START_ADDRESS;
	while(user_mode == 1) {

		address = getMemory(memory->user_text,pc);
		opcode = getMemory(memory->user_text,pc) >> 32;
		regDest = (address >> 27);
		registerSource = ((address << 5) >> 27);
		immediate =  ((address << 10) >> 10);
		offset = ((address << 5) >> 5);
		boffset = ((address << 1) >> 1);

		pc += 64;
		instruction_counter++;

		switch(opcode) {
		
		case MULT:
		case ADD:
		case STO:
		case LOAD:
			printInstruction(opcode, regDest, registerSource, immediate, pc, offset, gpr, 0);
			break;
		case SYSCALL:
			printInstruction(opcode, regDest, registerSource, immediate, pc, offset, gpr, 0);
			cycles += 8;
			break;
		case ADDI:
			printInstruction(opcode, regDest, registerSource, immediate, pc, offset, gpr, 1);
			cycles += 6;
			break;
		case SUBI:
			printInstruction(opcode, regDest, registerSource, immediate, pc, offset, gpr, 1);
			cycles += 6;
			break;
		case LI:
			printInstruction(opcode, regDest, registerSource, immediate, pc, offset, gpr, 2);
			gpr[regDest] = offset;
			cycles += 3;
			break;
		case LA:
			printInstruction(opcode, regDest, registerSource, immediate, pc, offset, gpr, 3);
			gpr[regDest] = pc + offset;
			cycles += 5;
			break;
		case LB:
			printInstruction(opcode, regDest, registerSource, immediate, pc, offset, gpr, 4);
			pointerchar = (char *) memory->user_data;
			temp4 = (gpr[registerSource] - USER_DATA)/8;
			pointerchar += temp4 + 1;
			cout << pointerchar << endl;
			gpr[regDest] = uint32_t(pointerchar[0]);
			cycles += 6;
			break;
		case BGE:
			printInstruction(opcode, regDest, registerSource, immediate, pc, offset, gpr, 5);
			if(gpr[regDest] == 0)
				pc += immediate;
			cycles += 5;
			break;
		case BNE:
			printInstruction(opcode, regDest, registerSource, immediate, pc, offset, gpr, 5);
			if(gpr[regDest] != gpr[registerSource])
				pc += immediate;
			cycles += 5;
			break;
		case BEQZ:
			printInstruction(opcode, regDest, registerSource, immediate, pc, offset, gpr, 5);
			if(gpr[regDest] == 0)
				pc += offset;
			cycles += 5;
			break;
		case B:
			printInstruction(opcode, regDest, registerSource, immediate, pc, offset, gpr, 0);
			cout << "Previous program counter: " << pc << endl;
			if((address >> 31) == 1)
				pc = pc - boffset;
			else
				pc = pc + boffset;

			cout << "The offset: " << boffset << endl;
			cout << "New program counter: " << pc << endl;
			cycles += 4;
			break;
		case END:
			user_mode = 0;
			break;
		}
	}

	cout << "Number of Cycles: " << cycles << endl;	
	cout << "Number of Instructions: " << instruction_counter << endl;
}

// This is the main method, you can change the file name to lab2.s to test that file, the file must be in the same directory
int main(int argc, char* argv[]) {

	//const char *file = "lab2.s";
	const char *file = "palindrome.s";
	if(argc != 1) file = argv[1];
	struct memory_blocks* memory;
	memory = (struct memory_blocks *) malloc(sizeof(struct memory_blocks));
	pc = START_ADDRESS;

	// We load the file into our custom memory structure
	loadDataSection(file, memory->user_text);
	loadTextSection(file, memory->user_text);

	userInterface(memory,pc);

	return 0;
}
