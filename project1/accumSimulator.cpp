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
		int operand();
		mem_addr instr_mem_addr();
		mem_addr accumulator;
		mem_addr pointer;
		instruction *instruct;
		Mem *memory;
}

Simulator::Simulator(){
	accumulator = 0;
	pointer = textTop;
	memory = new Mem();
}

mem_addr Simulator::instruct(){
	instruction address = *instruct;
	address = address << 8;
	address = address >> 8;
	return address;
}

int Simulator::operand(){
	instruction operand_instruct = *instruct;
	operand_instruct = operand_instruct >> 24;
	return operand_instruct;
}

void Simulator::execute(){
	bool not_end_of_file = true;
	while(not_end_of_file){
		instruct = memory->read(pointer);
		pointer++;
		mem_addr *data = memory->read(instruct());

		switch(operand()){
			// Case 1 handles the "LOAD" instruction and adds data to the accumulator
			case 1:{
				accumulator = *data;
				break;
			       }
			// Case 2 handles the "STORE" instruction and writes to the memory
			case 2:{
				memory->write(instruct(), accumulator);
				break;
			       }
			// Case 3 handles the "ADD" instruction and adds onto the accumulator
			case 3:{
				accumulator = accumulator + *data;
				break;
			       }
			// Case 4 handles the "MULT" instruction and multiples in the accumulator
			case 4:{
				accumulator = accumulator * *data;
				break;
			       }     
			// Case 5 handles the "END" instruction and will cause the loop to end
			case 5{
				not_end_of_file = false;
				break;
		       	}
			default:
				break;

		}
	}
}

int main(){
	Simulator *simulator = new Simulator();
	simulator->execute();
	return 0;
}
