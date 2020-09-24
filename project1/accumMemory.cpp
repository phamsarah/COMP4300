#include <iostream>
#include <cstdlib>
#include <string>
#include <stdio.h>
#include <fstream>

#define STACK_LEN  50
#define TEXT_LEN  100
#define	DATA_LEN  50

using namespace std;

typedef unsigned int uint32;
typedef uint32 instr;
typedef uint32 mem_addr;

mem_addr stackTop = 0x00300000;
mem_addr textTop = 0x00100000;
mem_addr dataTop = 0x00200000;

mem_addr stackSeg[STACK_LEN];
instr textSeg[TEXT_LEN];
mem_addr dataSeg[DATA_LEN];

class Mem{
public:
	Mem();
	bool textLoad(mem_addr addr_in);
	bool dataLoad(mem_addr addr_in, mem_addr data);
    bool write(mem_addr addr_in, mem_addr data);
    mem_addr * read(mem_addr addr_in);
private:
	int binaryDecode(mem_addr addr_in);
	int indexDecode(mem_addr addr_in);
	string remove_whitespace(string& str);
	int available_memory; // This is the pointer counter for the text segment
};

// Inside our memory, we store our data from the accumCode.txt file that is in assembly code
Mem::Mem(){
	int hex_1, hex_2, hex_3;
	string line_1, line_2 = "0000000000", line_3 = "0";
	int i = 0;
    available_memory = -1;
    
	ifstream accumCode ("accumCode.txt");
	if (accumCode.is_open()){
        
		while (getline(accumCode,line_1)){
            
            line_1 = line_1.substr(0, line_1.size() - 1);
            
			if (line_1.compare("") == 0) continue;
			if (line_1.compare(".text") == 0){
				i = 0; 
				continue;
			}
			if (line_1.compare(".data") == 0){
				i = 1; 
				continue;
			}
			// If i is 0, then that implies that we are in the .text section and we store the line as a hex and store the data by sneding in an address into our textLoad function
			if (i == 0){
				sscanf(line_1.data(),"%x", &hex_1);
				textLoad(hex_1);
			}
            // If i is 1, then that implies that we are in the .data section and we store the data as a hex and sending the hex into our dataLoad function
			else if (i == 1){
				for (int j = 0; j < 10; j++){
                    line_2[j] = line_1[j];
                    line_3[0] = line_1[11];
				}
				sscanf(line_2.data(), "%x", &hex_2);
                hex_3 = atoi(line_3.c_str());
				dataLoad(hex_2,hex_3);
			}
		}
	}
    accumCode.close();
}

// This function removes whitespace from the front and back of a given string
string Mem::remove_whitespace(string& stringWithSpace){
    size_t front = stringWithSpace.find_first_not_of(' ');
    if (front == std::string::npos) return "";
    size_t back = stringWithSpace.find_last_not_of(' ');
    return stringWithSpace.substr(front, (back - front + 1));
}

// This function will load in the text from a given address by storing that address into the text segment at the pointer counter index
bool Mem::textLoad(mem_addr addr_in){
    available_memory++;
    
	if (available_memory < TEXT_LEN){
		textSeg[available_memory] = addr_in;
		return true;
	}
	else return false;
}

// This function will load the .data data in and stores the data into the data segment after checking if there is available memory in the data length
bool Mem::dataLoad(mem_addr addr_in, mem_addr data){
	int index = (int) indexDecode(addr_in);
	if (available_memory < DATA_LEN){
		dataSeg[index] = data;
		return true;
	}
	else return false;
}

// This function decodes the given address into the memory index by shifting all the bits left 16 and right 15
int Mem::indexDecode(mem_addr addr_in){
    addr_in = addr_in << 15;
    addr_in = addr_in >> 15;
    return addr_in;
}

// This function decodes the given address into binary by shifting the bits left 7 and right 27
int Mem::binaryDecode(mem_addr addr_in){
    addr_in = addr_in << 7;
    addr_in = addr_in >> 27;
    return addr_in;
}

// This read method will first decode the given address and determine the length of given segment and returns the data at that address. Used for text, data, and stack segments.
mem_addr * Mem::read(mem_addr addr_in){

    int index = (int) indexDecode(addr_in);
    
switch(binaryDecode(addr_in)){
    case 1:
        if (index < TEXT_LEN) return &textSeg[index];
        break;
    case 2:
        if (available_memory < DATA_LEN) return &dataSeg[index];
        break;
    case 3:
        if (available_memory < STACK_LEN) return &stackSeg[index];
        break;
    default:
        return &stackTop;
        break;
    }
    return &stackTop;
}

// This Write method uses case 3 to identify if we are writing to the stack, it will first decode the given address in binary and returns true when the data is stored in the stack after making sure there is enough space in the stack
bool Mem::write(mem_addr addr_in, mem_addr data){
    
switch(binaryDecode(addr_in)){
	case 1:
	case 2:
        return false;
		break;
	case 3:
		{
			int index = (int) indexDecode(addr_in);

			if (available_memory < STACK_LEN){
				stackSeg[index] = data;
				return true;
			}
			else return false;
            break;
		}
		
	default:
        return false;
		break;
	}
    
	return false;
}
