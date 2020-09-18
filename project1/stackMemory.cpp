#include <iostream>
#include <string>
#include <stdio.h>
#include <fstream>
#include <cstdlib>

#define STACK_LEN 50
#define TEXT_LEN 100
#define DATA_LEN 50

using namespace std;

typedef unsigned int uint32;
typedef uint32 instr;
typedef uint32 mem_addr;

mem_addr stackTop = 0x00300000;
mem_addr textTop = 0x00100000;
mem_addr dataTop = 0x00200000;

mem_addr stack_seg[STACK_LEN];
instr text_seg[TEXT_LEN];
mem_addr data_seg[DATA_LEN];

class Mem{
	public: 
		Mem();
		bool load_text(mem_addr addr_in);
		bool load_data(mem_addr addr_in, mem_addr data);
		bool write(mem_addr addr_in, mem_addr data);
		mem_addr * read(mem_addr addr_in);

	private:
		int decode_bin(mem_addr addr_in);
		int decode_index(mem_addr addr_in);
		int available_memory;
};

Mem::Mem(){
	available_memory = -1;
	int hex_1;
	int hex_2;
        int hex_3;
	string line_1;
        string line_2 = "0000000000";
        string line_3 = "0";

	int i = 0;
	ifstream stackCode("stackCode.txt");

	if(stackCode.is_open()){
		while(getline(stackCode,line_1)){
			line_1 = line_1.substr(0, line_1.size()-1);
			
			if(line_1.compare("") == 0) continue;
			if(line_1.compare(".text") == 0) {
				i = 0;
				continue;
			}
			if(line_1.compare(".data") == 0){
				i = 1;
				continue;
			}
			else if (i == 0){
				sscanf(line_1.data(), "%x", &hex_1);
				load_text(hex_1);
			}
			if (i == 1){
				for (int j = 0; j < 10; j++){
					line_2[j] = line_1[j];
					line_3[0] = line_1[11];
				}

				sscanf(line_2.data(), "%x", &hex_2);
				hex_3 = atoi(line_3.c_str());
				load_data(hex_2, hex_3);
			}
		}
	}
	stackCode.close();
}

int Mem::decode_index(mem_addr addr_in){
	addr_in = addr_in << 15;
	addr_in = addr_in >> 15;
	return addr_in;
}

int Mem::decode_bin(mem_addr addr_in){
	addr_in = addr_in << 7;
	addr_in = addr_in >> 27;
	return addr_in;
}

bool Mem::load_data(mem_addr addr_in, mem_addr data){
	mem_addr tempIndex = addr_in;
	int index = (int) decode_index(tempIndex);
	
	if(available_memory < DATA_LEN){
		data_seg[index] = data;
		return true;
	}
	else return false;
}

bool Mem::load_text(mem_addr addr_in){
	available_memory++;
	if(available_memory < TEXT_LEN){
		text_seg[available_memory] = addr_in;
		return true;
	}
	else return false;
}


// This method will read each address and return each segment with the correct length
mem_addr * Mem::read(mem_addr addr_in){
	mem_addr tempBin = addr_in;
       	mem_addr tempIndex = addr_in;

	switch(decode_bin(tempBin)){
		// Case 1 reads the text and will check the length
		case 1: {
				int index = (int) decode_index(tempIndex);
				if(index < TEXT_LEN) return &text_seg[index];
				break;
			}
		// Case 2 reads the data and will check the length
		case 2: {
				int index = (int) decode_index(tempIndex);
				if(available_memory < DATA_LEN) return &data_seg[index];
				break;
			}
		// Case 3 reads the stack and will check the length
		case 3: {
				int index = (int) decode_index(tempIndex);
				if(available_memory < STACK_LEN) return &stack_seg[index];
				break;
			}
		default: {
				return &stackTop;
				break;
			 }

		
		return &stackTop;
	}
}

// This method will write to the stack with the given data being passed through
bool Mem::write(mem_addr addr_in, mem_addr data){
	mem_addr tempBin = addr_in;
	mem_addr tempIndex = addr_in;

	switch(decode_bin(tempBin)){
		// Cases 1 and 2 check whether or not the user has access to write, so we return false if they don't
		case 1:
		case 2:
			return false;
			break;
		// Case 3 will actuallly write to memory by adding data to the stack segment
		case 3: {
				int index = (int) decode_index(tempIndex);
				if (available_memory < STACK_LEN){
					stack_seg[index] = data;
					return true;
				}
				else return false; // No space to write memory
				break;
			}
		default: return false;
			 break;

		return false;
	}

}
