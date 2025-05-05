#ifndef RENDERER_H
#define RENDERER_H
#define PRINTBUFFERSIZE 1000


#include "packet.h"

typedef struct Renderer {
	int INPUTBOXSIZE;
	int terminalSize;
	char* printBuffer[PRINTBUFFERSIZE];
	int nbBuffered;
	
	int editMode;
} Renderer;


Renderer* newRenderer();
void renderer_initialize_console(Renderer* renderer);
void renderer_print(Renderer* renderer, char* str);
void renderer_reset_input(Renderer* renderer);
void renderer_moveCursor(int x, int y) ;
void renderer_clearTerm() ;
void renderer_initialize_edit_mode(Renderer* renderer);
void renderer_exit_edit_mode(Renderer* renderer);
#endif