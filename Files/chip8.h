#include <iostream>
#include <fstream>
#include <string>
#include <SDL2/SDL.h>

unsigned char chip8_fontset[80] =
{
  0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
  0x20, 0x60, 0x20, 0x20, 0x70, // 1
  0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
  0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
  0x90, 0x90, 0xF0, 0x10, 0x10, // 4
  0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
  0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
  0xF0, 0x10, 0x20, 0x40, 0x40, // 7
  0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
  0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
  0xF0, 0x90, 0xF0, 0x90, 0x90, // A
  0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
  0xF0, 0x80, 0x80, 0x80, 0xF0, // C
  0xE0, 0x90, 0x90, 0x90, 0xE0, // D
  0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
  0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

int keys[16] = {27, 30, 31, 32, 20, 26, 8, 4, 22, 7, 29, 6, 33, 21, 9, 25};
const Uint8 *state = SDL_GetKeyboardState(NULL);

class MyChip8
{
	unsigned short opcode;
	unsigned char memory[4096];
	unsigned char V[16];
	unsigned short I;
	unsigned short pc;
	unsigned char gfx[64][32];
	unsigned char delay_timer;
	unsigned char sound_timer;
	unsigned short stack[16];
	unsigned short sp;



	unsigned short x, y, kk, addr, n;
	unsigned char r1;
	int sum, pressed, px, i, j;

	SDL_Window*   g_window = 0;
	SDL_Renderer* g_pRenderer = 0;


public:
	bool drawFlag;
	MyChip8();
	void initialize();
	void emulateCycle();
	void render();
	bool loadGame(const char*);
	void setKeys();
	void test_gfx();
};

MyChip8::MyChip8()
{


	drawFlag = false;

	SDL_Init(SDL_INIT_EVERYTHING);

	g_window = SDL_CreateWindow("Chip8", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
				640, 480, SDL_WINDOW_SHOWN);

	//gScreenSurface = SDL_GetWindowSurface( g_window );

	g_pRenderer = SDL_CreateRenderer(g_window, -1, 0);
}

void MyChip8::test_gfx()
{
	for(i = 0; i<64; i++){
		for(j = 0; j<32; j++)
		{
			if((i%2 == 0)&(j%2 != 0))
				gfx[i][j] = 1;
			else
				gfx[i][j] = 0;
		}
	}



}

void MyChip8::initialize()
{
	pc = 0x200; //Program counter starts at 0x200
	opcode = 0; //Reset current opcode
	I = 0;		//Reset index register
	sp = 0;		//Reset stack pointer

	//Clear display
	for(i = 0; i < 64; i++)
	{
		for(j = 0; j<32; j++)
			gfx[i][j] = 0;
	}
	//Clear Stack
	for(i = 0; i<16; i++)
		stack[i] = 0;
	//Clear registers V0 - VF
	for(i = 0; i<16; i++)
		V[i] = 0;
	//Clear memory
	for(i = 0; i<4096; i++)
		memory[i] = 0;

	//load fontset
	for(i = 0; i<80; i++);
		memory[i] = chip8_fontset[i];

	//Reset timers
	delay_timer = 0;
	sound_timer = 0;

	SDL_Surface* pixels[64][32];

}

void MyChip8::emulateCycle()
{
	//Fetch Opcode

	opcode = (memory[pc] << 8) | (memory[pc + 1]);

	//Decode Opcode
	switch(opcode & 0xF000)
	{
		case 0x0000:
			switch(opcode & 0x000F)
			{
				case 0x0000: //0x00E0 clears the screen
					for(int i = 0; i < 64; i++)
					{
						for(int j = 0; j<32; j++)
							gfx[i][j] = 0;
					}
					drawFlag = true;
					pc = pc + 2;
					break;

				case 0x000E: //0x00EE: Returns from subroutine
					pc = stack[sp];
					stack[sp] = 0;
					sp--;
					break;

				default:
					printf("Unknown opcode [0x0000]: 0x%X\n", opcode);

			}
			break;

		case 0x1000: //0x1nnn: jump to location nnn
			pc = opcode & 0x0FFF;
			break;

		case 0x2000: //0x2nnn: call subroutine at nnn
			sp++;					//increment stack pointer
			stack[sp] = pc;			//place return adress at top of stack
			pc = opcode & 0x0FFF;	//plane address nnn in the pc
			break;

		case 0x3000: //0x3xkk: Skip the next instruction if Vx = kk
			x = (opcode & 0x0F00) >> 8;
			if(V[x] == (opcode & 0x00FF))
				pc += 4;
			else
				pc += 2;
			break;

		case 0x4000: //0x4xkk: Skip the next instruction if Vx != kk
			x = (opcode & 0x0F00) >> 8;
			if(V[x] != (opcode & 0x00FF))
				pc += 4;
			else
				pc += 2;
			break;

		case 0x5000: //0x5xy0: Skip next instruction if Vx = Vy
			x = (opcode & 0x0F00) >> 8;
			y = (opcode & 0x00F0) >> 4;
			if(V[x] == V[y])
				pc += 4;
			else
				pc += 2;
			break;

		case 0x6000: //0x6xkk: set Vx = kk
			kk = opcode & 0x00FF;
			x = (opcode & 0x0F00) >> 8;
			V[x] = kk;
			pc += 2;
			break;

		case 0x7000: // 0x7xkk: Set Vx = Vx + kk
			x = (opcode & 0x0F00) >> 8;
			kk = opcode & 0x00FF;
			V[x] = V[x] + kk;
			pc += 2;

		case 0x8000:
			x = (opcode & 0x0F00) >> 8;
			y = (opcode & 0x00F0) >> 4;

			switch(opcode & 0x000F)
			{
				case 0x0000:
					V[x] = V[y];
					pc += 2;
					break;

				case 0x0001:
					V[x] = V[x] | V[y];
					pc += 2;
					break;

				case 0x0002:
					V[x] = V[x] & V[y];
					pc += 2;
					break;

				case 0x0003:
					V[x] = V[x] ^ V[y];
					pc += 2;
					break;

				case 0x0004:
					sum = V[x] + V[y];
					if(sum > 255)
						V[15] = 1;
					else
						V[15] = 0;
					V[x] = sum & 0xFF;
					pc += 2;
					break;

				case 0x0005:
					if(V[x] > V[y])
						V[15] = 1;
					else
						V[15] = 1;
					V[x] = V[x] - V[y];
					pc += 2;
					break;

				case 0x0006:
					if((V[x] & 0x1) == 1)
						V[15] = 1;
					else
						V[15] = 0;
					V[x] = V[x] >> 1;
					px += 2;
					break;

				case 0x0007:
					if(V[x] > V[y])
						V[15] = 1;
					else
						V[15] = 0;

					V[x] = V[y] - V[x];
					pc += 2;
					break;

				case 0x000E:
					if((V[x] & 0x80) == 1)
						V[15] = 1;
					else
						V[15] = 0;
					V[x] = V[x] << 1;
					pc += 2;
					break;
			}

			break;



		case 0x9000: //0x9xy0: Skip next instruction if Vx != Vy
			x = (opcode & 0x0F00) >> 8;
			y = (opcode & 0x00F0) >> 4;
			if(V[x] != V[y])
				pc += 4;
			else
				pc += 2;
			break;

		case 0xA000: // ANNN: Sets I to the adress NNN
			//Execute Opcode
			I = opcode & 0x0FFF;
			pc += 2;
			break;

		case 0xB000: //0xBnnn: Jump to location nnn + V0
			addr = opcode & 0x0FFF;
			pc = pc + V[0];
			pc += 2;
			break;

		case 0xC000: //0xCxkk: Set Vx = random byte AND kk
			srand(memory[I]);
			r1 = rand() % 255;
			x = (opcode & 0x0F00) >> 8;
			kk = opcode & 0x00FF;
			V[x] = r1 & kk;
			pc += 2;
			break;

		case 0xD000: // Graphics instruction, figure this out last

			x = V[(opcode & 0x0F00) >> 8];
			y = V[(opcode & 0x00F0) >> 4];
			n = opcode & 0x000F;


			V[15] = 0;

			for(int i = 0; i<n; i++)
			{
				unsigned char temp = gfx[x][y+n];
				gfx[x][y+n] = gfx[x][y+n] ^ memory[I + n];
				for(int j = 0; j< 8; j++)
				{
					if((temp & (0x80 >> j)) != 0)
					{
						if((gfx[x][y+n] & (0x80>>j)) == 1)
							V[15] = 1;
					}
				}
			}

			pc += 2;
			drawFlag = true;

			break;

		case 0xE000: //0xEx9E: Skip next instruction if key with value Vx is pressed

			x = (opcode & 0x0F00) >> 8;


			pressed = state[keys[V[x]]];

			if(pressed)
				pc += 4;
			else
				pc += 2;
			break;

		case 0xF000:

			x = (opcode & 0x0F00) >> 8;

			switch(opcode & 0x00FF)
			{

				case 0x0007:
					V[x] = delay_timer;
					px += 2;
					break;

				/*case 0x000A: //0xFx0A: wait for key press, store the value in Vx

					while(SDL_PollEvent(&event)){
						switch(event.type){
							case SDL_KEYDOWN:
								switch(event.key.keysym.sym){
									case SDLK_1:
										V[x] = 0x1;
										pc += 2;
										break;
									case SDLK_2:
										V[x] = 0x1;
										pc += 2;
										break;
									case SDLK_3:
										V[x] = 0x1;
										pc += 2;
										break;
									case SDLK_4:
										V[x] = 0xC;
										pc += 2;
										break;
									case SDLK_q:
										V[x] = 0x4;
										pc += 2;
										break;
									case SDLK_w:
										V[x] = 0x5;
										pc += 2;
										break;
									case SDLK_e:
										V[x] = 0x6;
										pc += 2;
										break;
									case SDLK_r:
										V[x] = 0xD;
										pc += 2;
										break;
									case SDLK_a:
										V[x] = 0x7;
										pc += 2;
										break;
									case SDLK_s:
										V[x] = 0x8;
										pc += 2;
										break;
									case SDLK_d:
										V[x] = 0x9;
										pc += 2;
										break;
									case SDLK_f:
										V[x] = 0xE;
										pc += 2;
										break;
									case SDLK_z:
										V[x] = 0xA;
										pc += 2;
										break;
									case SDLK_x:
										V[x] = 0x0;
										pc += 2;
										break;
									case SDLK_c:
										V[x] = 0xB;
										pc += 2;
										break;
									case SDLK_v:
										V[x] = 0xF;
										pc += 2;
										break;
								}
						}
					}
*/
				case 0x0015:
					delay_timer = V[x];
					pc += 2;
					break;

				case 0x0018:
					sound_timer = V[x];
					pc += 2;
					break;

				case 0x001E:
					I = I + V[x];
					pc += 2;
					break;

				case 0x0029:
					I = V[(opcode & 0x0F00) >> 8] * 0x5;
					pc += 2;
					break;

				case 0x0033:
					memory[I] = V[x]/100;
					memory[I + 1] = (V[x]%100)/10;
					memory[I + 2] = V[x]%10;
					pc += 2;
					break;

				case 0x0055: //serial Load
					for(int i = 0; i<x; i++)
						memory[I + i] = V[i];
					pc += 2;
					break;

				case 0x0065: //serial Store
					for(int i = 0; i < x; i++)
						V[i] = memory[I + i];
					pc +=2;
					break;

			}

			break;

		//default:
			//printf("Unknown opcode: 0x%X\n", opcode);

	}


	//Update timers
	if(delay_timer > 0)
		--delay_timer;

	if(sound_timer > 0)
	{
		printf("BEEP!\n"); //Do some real sound with this
		--sound_timer;
	}
}



void MyChip8::render(){

	SDL_SetRenderDrawColor(g_pRenderer, 0, 0, 0, 255);
	SDL_RenderClear(g_pRenderer);
	SDL_RenderPresent(g_pRenderer);



	for(i = 0; i<64; i++){
		for(j = 0; j<32; j++)
			{

				SDL_SetRenderDrawColor(g_pRenderer, gfx[i][j]*255, gfx[i][j]*255, gfx[i][j]*255, 255);
				SDL_Rect box = {i*8, j*8, 8, 8};
				SDL_RenderFillRect(g_pRenderer, &box);
				SDL_RenderPresent(g_pRenderer);



			}

	}




}


bool MyChip8::loadGame(const char* file){


    long lSize;
    char *buffer;

	FILE *fp = fopen(file, "rb");

	if(fp == NULL){
		fputs("File Error", stderr);
		return false;
	}


    fseek(fp, 0, SEEK_END);
    lSize = ftell(fp);
    rewind(fp);

    buffer = (char*)malloc(sizeof(char)*lSize);
    if (buffer == NULL) {fputs("Memory error", stderr); return false;}

    fread(buffer, 1, lSize, fp);

    for(i = 0; i<lSize; i++){
        memory[i+512] = buffer[i];
    }
    return true;


}

void MyChip8::setKeys(){
	int a =1;
}


