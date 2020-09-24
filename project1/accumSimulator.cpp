#include <iostream>
#include <stdio.h>
#include <cstdlib>
#include "accumMemory.cpp"

using namespace std;

class Simulator{
public:
    Simulator();
	void execute();
private:
    Mem *memory;
	int operand();
	mem_addr mem_address();
    instr *instruction; // This instruction is the current isntruction that is read in
	mem_addr accumulator; // The accumulator
    mem_addr pointer_counter; // The pointer counter to point to the next available location in the memory
};

// The Simulator function intiializes a memory object and sets the accumulator to zero and the pointer counter from the accumMemory.cpp
Simulator::Simulator(){
    accumulator = 0;
    pointer_counter = textTop;
	memory = new Mem();
}

// This function will return the current instruction memory address that is shifted (all of the bits) by 8 left and right
mem_addr Simulator::mem_address()
{
    instr address = *instruction;
    address = address << 8;
    address = address >> 8;
    return address;
}

// This function will return the current instruction's operand value by shifting the value (all of the bits) right by 24
int Simulator::operand(){
    instr operand_code = *instruction;
    operand_code = operand_code >> 24;
    return operand_code;
}

// This function executes any instructions including LOAD, STORE, ADD, MULT, and END, all instructions that an accumulator uses
void Simulator::execute(){
    
	bool nextInstruct = true;
	while(nextInstruct){
        
        instruction = memory->read(pointer_counter); // We read in the next instruction from the memory by using our pointer counter
        ++pointer_counter; // Pointer counter increments meaning we already read in the current line and are onto the next
		
		switch(operand()){
             
                // Case 1 implements the LOAD function by reading in what is at the memory address and storing the given data into the accumulator
			case 1:{
				mem_addr *data = memory->read(mem_address());
                accumulator = *data;
				break;
			}
                
                // Case 2 implements the STORE function by writing the data that is stored in the accumulator into the current free memory address
			case 2:{
				memory->write(mem_address(),accumulator);
				break;
			}
                // Case 3 implements the ADD function by reading in what is in the memory at the given memory address, adding that given data with the data in the accumulator, and storing the results in the accumulator
			case 3:{
				mem_addr *data = memory->read(mem_address());
                accumulator = accumulator + *data;
				break;
			}
                // Case 4 implements the MULT function by reading in what is in the memory at the given memory address, multiplying that data with the data in the accumulator, and storing the results in the accumulator
			case 4:{
				mem_addr *data = memory->read(mem_address());
                accumulator = accumulator * *data;
				break;
			}
                // Case 5 implements the END function by eiting the while loop by setting the nextInstruct to false and PRINTS what is in the accumulator
			case 5:{
				nextInstruct = false;
				cout << std::dec << accumulator << " is in the accumulator." <<endl;
				break;
			}
			default:
				break;
		}
	}
}

// Main function that initializes the Accumulator simulator and executes the simulation
int main()
{
    Simulator *sim = new Simulator();
    sim->execute();
    return 0;
}
