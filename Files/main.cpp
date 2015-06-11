//Main file for Chip-8 interpreter implementation, using SDL graphics

#include "chip8.h" //implemntation of cpu core
#include <iostream>
#include <string>
MyChip8 chip8;





int main(int argc, char *argv[])
{
	//Initialize the Chip8 system and load the game into memory

	chip8.initialize();

	chip8.loadGame(argv[1]);

	//Emulation loop
	while(1)
	{
		//Emulate one cycle
		chip8.emulateCycle();

		//If the draw flag is set, update the screen
		if(chip8.drawFlag){
			chip8.render();
			chip8.drawFlag = false;
        }
		//Store key press state(Press and Release)
		//chip8.setKeys();
	}

	//chip8.cleanup();
	return 0;
}

