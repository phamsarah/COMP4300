#include <iostream>
#include <string>
#include <fstream>
#include <stdio.h>
#include <cstdlib>

#define STACK_LEN 100
#define TEXT_LEN 200
#define DATA_LEN 100

using namespace std;

typedef unsigned int uint32;
typedef uint32 mem_addr;
typedef uin32 instruction;

mem_addr stackTop = 0x00300000;
mem_addr textTop = 0x00100000;
mem_addr dataTop = 0x00200000;

mem_addr stack_seg[STACK_LEN];
instruction text_seg[TEXT_LEN];
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
		string shorten(string& str);
		int available_memory;
}

Mem::Mem(){
	available_memory = -1;
	int hex_1, hex_2, hex_3;
	string line_1, line_2 = "0000000000", line_3 = "0";
	int i = 0;
	
	ifstream accumFile("accumCode.txt");

	if(accumFile.is_open()){
		while(getline(accumFile,line_1)){
			line_1 = line_1.substr(0, line_1.size()-1);

			if(line_1.compare("") == 0) continue;
			if(line_1.compare(".text") == 0){
				i = 0;
				continue;
			}
			if(line_1.compare(".data") == 0){
				i = 1;
				continue;
			}
			if(i == 0){ // If we are in the .text section then store the line into a hex
				scanff(line_1.data(), "%x", &hex_1);
				load_text(hex_1);
			}
			else if(i == 1){ // If we are in the .data section, then store line 1 and 2 as hex
				for(int j = 0; j < 10; j++){
					line_2[j] = line_1[j];
					line_3[0] = line_1[11];
				}

				sscanf(line_2.data(), "%x", &hex_2);
				hex_3 = atoi(line_3.c_str());
				load_data(hex_2,hex_3);
			}
		}
	}
	accumFile.close();
 }

// Removes whitespace from the front and back of the string
string Mem::removeWhitespace(string& str){
	size_t front = str.find_first_not_of(' ');
	size_t back = str.find_last_not_of(' ');
	if(front == std::string::npos) return "";
	return str.substr(front, (back-front+1));
}

// Decodes a given address and returns the address shifted to the index
int Mem::decode_index(mem_addr addr_in){
	addr_in = addr_in << 15;
	addr_in = addr_in >> 15;
	return addr_in;
}

// Decodes a given address and returns the address shifted to the bin
int Mem:decode_bin(mem_addr addr_in){
	addr_in = addr_in << 7;
	addr_in = addr_in >> 27;
	return addr_in;
}

// Will load in the memory coming from .text and checks the length of the text segment
bool Mem::load_text(mem_addr addr_in){
	available_memory++;
	if(available_memory < TEXT_LEN){
		text_seg[available_memory] = addr_in;
		return true;
	}
	else return false;
}

// Will load in the memory coming from .data and checks the length of the data segment
bool Mem::load_data(mem_addr addr_in, mem_addr data){
	mem_addr tempIndex = addr_in;
	int index = (int) decode_index(tempIndex);

	if(available_memory < DATA_LEN){
		data_seg[index] = data;
		return true;
	}
	else return false;
}

// Takes in a memory address and will read it, check the length and return it to it's corresponding segment
mem_addr * Mem::read(mem_addr addr_in){

	mem_addr tempBin = addr_in;
	mem_addr tempindex = addr_in;
	int index = (int) decode_index(tempIndex);

	switch(decode_bin(tempBin)){
		// Case 1 will handle if the memory address is .text
		case 1:{
			if(index < TEXT_LEN) return &text_seg[index];
			break;
		       }
		// Case 2 will handle if the memory address is .data
		case 2:{
			if(available_memory < DATA_LEN) return &data_seg[index];
		      	break;
		       }
		// Case 3 will handle if the memory address is stack
		case 3:{
			if(available_memory < STACK_LEN) return &stack_seg[index];
			break;
		       }
		default:{
			return &stackTop;
			break;
			 }
	return &stackTop;

	}
}

// Takes in an address and data and will write the data into that address
bool Mem::write(mem_addr addr_in, mem_addr data){
	mem_addr tempBin = addr_in;
	mem_addr tempIndex = addr_in;

	switch(decode_bin(tempBin)){
		// Cases 1 and 2 handle if the user has no writing permissions
		case 1:
		case 2:
			return false;
			break;
		// Case 3 Will actually write to the stack segment
		case 3:{
			int index = (int) decode_index(tempIndex);
			if(available_memory < STACK_LEN){
				stack_seg[index] = data;
				return true;
			}
			else return false;
			break;
		       }
		default: return false;
			 break;

		return false;
	}
}
