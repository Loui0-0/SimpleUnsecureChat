#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "renderer.h"
#include "packet.h"



Renderer* newRenderer(){
	Renderer* renderer = malloc(sizeof(Renderer));
	if (renderer == NULL)
		return NULL;

	renderer->terminalSize = -1;
	renderer->INPUTBOXSIZE = 1;
	renderer->editMode = 0;
	renderer->nbBuffered = 0;

	return renderer;
}


void renderer_initialize_console(Renderer* renderer){

	renderer_clearTerm();

	//Get current terminal size 
	struct winsize ws;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
    renderer->terminalSize = ws.ws_row;
   	
   	//set scroll region
   	printf("\033[1;%dr", renderer->terminalSize - renderer->INPUTBOXSIZE); 

	renderer_reset_input(renderer);

}

void renderer_print(Renderer* renderer, char* str){
	if (renderer->editMode == 0){
	    //save cursor pos
	    printf("\0337");
		
		//move to the bottom of scroll region
		printf("\033[%d;1H", renderer->terminalSize - renderer->INPUTBOXSIZE);
			
		//scroll one line and print
		printf("\n%s", str);
	    
	    //Restore cursor to original position
	    printf("\0338");
	    fflush(stdout);

	} else if (renderer->nbBuffered < PRINTBUFFERSIZE){
		renderer->printBuffer[renderer->nbBuffered] = strdup(str);
		renderer->nbBuffered++;
	}
}

void renderer_reset_input(Renderer* renderer){
	if (renderer->editMode == 1){
		renderer_exit_edit_mode(renderer);
	}


	//clear input
	for (int i = 0 ; i < renderer->INPUTBOXSIZE; i++){
		printf("\033[%d;1H", renderer->terminalSize - renderer->INPUTBOXSIZE + 1 + i);
		printf("\033[2K");
	}


	//move to beggining of input 
	printf("\033[%d;1H", renderer->terminalSize - renderer->INPUTBOXSIZE + 1);

	printf(">");
    fflush(stdout);

}

void renderer_moveCursor(int x, int y) {
    printf("\033[%d;%dH", y + 1, x + 1);
}

void renderer_clearTerm() {
    printf("\e[2J");

}

void renderer_print_buffered(Renderer* renderer){
	for (int i = 0; i < renderer->nbBuffered; i++){
		if (renderer->printBuffer[i] != NULL){
			renderer_print(renderer, renderer->printBuffer[i]);
			free(renderer->printBuffer[i]);
		}
	}
	renderer->nbBuffered = 0;

}

void renderer_initialize_edit_mode(Renderer* renderer){
	renderer->editMode = 1;
	
	//save the screen
	printf("\033[?1049h");

	//reset scroll region
	printf("\033[r");

	renderer_clearTerm();
	renderer_moveCursor(0, 1);
	printf(">");
	fflush(stdout);
}

void renderer_exit_edit_mode(Renderer* renderer){
	if (renderer->editMode == 0){
		return;
	}

	renderer->editMode = 0;

	renderer_initialize_console(renderer);
	
	// restore the screen
	printf("\033[?1049l");

	renderer_print_buffered(renderer);
	
	fflush(stdout);
}