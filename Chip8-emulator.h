#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#define MEM_SIZE 4096
#define REG_COUNT 16
#define STACK_SIZE 16
#define DISPLAY_SIZE (64 * 32)

typedef struct Chip8 {
    unsigned char memory[MEM_SIZE]; // memoria
    unsigned char V[REG_COUNT]; // registri 
    unsigned short I; //registro index per indirizzamento della memoria
    unsigned short pc; //program counter
    unsigned short stack[STACK_SIZE]; //area di stack 
    unsigned char sp; // stack pointer
    unsigned char DISPLAY[DISPLAY_SIZE]; //display 64 X 32
    unsigned char delay_timer; // timer per il ritardo
    unsigned char sound_timer; //timer per il suono
    bool keys[16]; //array per tenere traccia dello stato dei tasti
} Chip8;

extern bool key_pressed; //variabile globale per tenere traccia se un tasto è stato premuto

void initialize(Chip8* Chip8); //per inizializzare il sistema
void loadRom(Chip8* Chip8, const char* filename); //per caricare il gioco nella memoria
void emulateCycle(Chip8* Chip8); //per eseguire un ciclo di emulazione, cioè una istruzione del gioco
void updateTimers(Chip8* Chip8); //per aggiornare i timer del ritardo e del suono
void drawGraphics(Chip8* Chip8); //per disegnare i pixel sul display
void setKeys(Chip8* Chip8); //per aggiornare lo stato dei tasti premuti