#include <iostream>
#include <stdio.h>
#include <cstdlib>
#include "stackMemory.cpp"

using namespace std;

class Simulator{
    
public:
    Simulator();
	void execute();
private:
    Mem *memory;
	int operand();
	mem_addr mem_address();
    instr *instruction; // This instruction is the current instruction that is read in
	mem_addr topStack; // Points to the top of the Stack where the next data will be written
	mem_addr pointer_counter; // Stack's use a pointer counter to point to the next available location in the memory

};

// The Simulator function initializes a memory object and sets the top of the stack and the pointer counter from the stackMemory.cpp
Simulator::Simulator(){
    
	topStack = stackTop;
    pointer_counter = textTop;
	memory = new Mem();
    
}

// This function will return the current instruction memory address that is shifted (all bits) by 8 left and right
mem_addr Simulator::mem_address(){
    instr address = *instruction;
    address = address << 8;
    address = address >> 8;
    return address;
}

// This function will return the current instruction's operand value by shifting the value (all bits) right by 24
int Simulator::operand(){
    instr operand_code = *instruction;
    operand_code = operand_code >> 24;
    return operand_code;
}

// This function executes any instructions including PUSH, POP, ADD, MULT, and END, all instructions that a stack uses
void Simulator::execute(){
    
	bool nextInstruct = true;
	while(nextInstruct){
        
        instruction = memory->read(pointer_counter); // We read in the next instruction from the memory by using our pointer counter
        ++pointer_counter; // Pointer counter increments meaning we already read in the current line and we are onto the next
		
		switch(operand()){
                
                // Case 1 implements the PUSH function by reading in what is at the memory address and writing the given data to the top of the stack. We also increment the topStack counter because we now have added data and our top is pushed up.
			case 1:{
				mem_addr *data = memory->read(mem_address());
				memory->write(topStack, *data);
                ++topStack;
				break;
			}
                
                // Case 2 implements the POP function by reading in what is at the top of the stack and decrementing the topStack counter because topStack only points to an OPEN spot on the stack, so the data on the top would be - 1 from the topStack.
			case 2:{
                --topStack;
				mem_addr *data = memory->read(topStack);
				cout << std::dec << *data << " is at the top of the stack." <<endl;
				break;
			}
                // Case 3 implements the ADD function by reading in what is currently at the top of the stack, reading in what is on the SECOND most top of the stack, adding those two values and then WRITING to the top of the stack the results
			case 3:{
                --topStack;
				mem_addr *data_1 = memory->read(topStack);
                --topStack;
				mem_addr *data_2 = memory->read(topStack);
                
				mem_addr result = *data_1 + *data_2;
                
				memory->write(topStack, result);
                ++topStack;
				break;
			}
              // Case 4 implements the MULT function by reading in what is currently at the top of the stack, reading in what is on the SECOND most top of the stack, multiplying those two values and then WRITING to the top of the stack the results
			case 4:{
                --topStack;
				mem_addr *data_1 = memory->read(topStack);
                --topStack;
				mem_addr *data_2 = memory->read(topStack);
                
				mem_addr result = *data_1 * *data_2;
                
				memory->write(topStack, result);
                topStack++;
				break;
			}
                // Case 5 implements the END function by exiting the while loop by setting the nextInstruct value to false
			case 5:{
                nextInstruct = false;
				break;
			}
			default:
				break;
		}
	}
}

// Main function that initializes the Stack simulator and executes the simulation
int main()
{
    Simulator *simulator = new Simulator();
    simulator->execute();
    return 0;
}
