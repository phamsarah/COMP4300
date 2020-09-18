#include <iostream>
#include <cstdlib>
#include <stdio.h>
#include "stackMemory.cpp"

using namespace std;

class Simulator{
	public: 
		Simulator();
		void execute();
	private:
		int instruction_operand();

		mem_addr memory_address();
		mem_addr topStack;
		mem_addr pointer; 

		instr *instruct;
		Mem *memory;
};

Simulator::Simulator(){
	topStack = stackTop;
	pointer = textTop;
	memory = new Mem();
}

void Simulator::execute(){
	bool not_end_of_file = true;
	while(not_end_of_file){
		instruct = memory->read(pointer);
		pointer++;

		switch(instruction_operand()){
			// PUSH
			case 1:{
				mem_addr *data = memory->read(memory_address());
				memory->write(topStack, *data);
				topStack++;
				break;
			       }
			// POP
			case 2: {
				topStack--;
				mem_addr *data = memory->read(topStack);
				break;		
				}
			// ADD
			case 3: {
				topStack--;
				mem_addr *data1 = memory->read(topStack);
				topStack--;
				mem_addr *data2 = memory->read(topStack);
				mem_addr addition_result = *data1 + *data2;
				memory->write(topStack,addition_result);
				break;
				}
			// MULT
			case 4: {
				topStack--;
				mem_addr *data1 = memory->read(topStack);
				topStack--;
				mem_addr *data2 = memory->read(topStack);
				mem_addr mult_result = *data1 * *data2;
				memory->write(topStack,mult_result);
				topStack++;
				break;
				}
			// END
			case 5: {
				not_end_of_file = false;
				break;	
				}
			default:
				break;
		}
	}
}

mem_addr Simulator::memory_address(){
	instr address;
        address = *instruct;
	address = address << 8;
	address = address >> 8;
	return address;
}

int Simulator::instruction_operand(){
	instr operand;
       	operand = *instruct;
	operand = operand >> 24;
	return operand;
}

int main(){
	Simulator *simulator = new Simulator();
	simulator->execute();
	return 0;
}
