#include "Chip8-emulator.h"
#include <conio.h>   /* _kbhit, _getch */
#include <windows.h> /* Sleep */

bool key_pressed = false; /* definizione del simbolo globale */

void initialize(Chip8* Chip8){
    Chip8->pc = 0x200; // il programma inizia a 0x200
    Chip8->I = 0; // registro index a 0
    Chip8->sp = 0; // stack pointer a 0
    Chip8->delay_timer = 0; //timer per il ritardo a 0 
    Chip8->sound_timer = 0; //timer per il suono a 0
    memset(Chip8->memory, 0, MEM_SIZE); // inizializza la memoria a 0
    memset(Chip8->V, 0, REG_COUNT); // inizializza i registri a 0
    memset(Chip8->stack, 0, STACK_SIZE * sizeof(unsigned short)); //inizializzo lo stack a 0
    memset(Chip8->DISPLAY, 0, DISPLAY_SIZE); // inizializza il display a 0 
    memset(Chip8->keys, false, 16 * sizeof(bool)); // inizializza lo stato dei tasti a false
} 

void loadRom(Chip8* Chip8, const char* filename){
    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        fprintf(stderr, "Errore nell'apertura del file: %s\n", filename);
        exit(EXIT_FAILURE);
    }
    fseek(file, 0, SEEK_END);
    long filesize = ftell(file); //ottiene la dimensione del file
    rewind(file); //riporta il puntatore ad inizio file
    if (filesize > MEM_SIZE - 0x200) { //verifica che il file non sia troppo grande per la memoria
        fprintf(stderr, "Il file è troppo grande per essere caricato nella memoria.\n");
        fclose(file);
        exit(EXIT_FAILURE);
    }
    fread(Chip8->memory + 0x200, 1, filesize, file); //carica il file nella memoria a partire da 0x200
    fclose(file);
}

void emulateCycle(Chip8* chip8){
    unsigned short opcode = (chip8->memory[chip8->pc] << 8) | chip8->memory[chip8->pc + 1]; //legge l'istruzione a 2 byte dalla memoria
    chip8->pc += 2; //incrementa il program counter di 2 byte per passare alla prossima istruzione
    switch( opcode & 0xF000){ // controlla i primi 4 bit dell'istruzione per determinare il tipo di operazione   
        case 0x0000 : 
        switch(opcode & 0x0FFF){ //controlla i restanti 12 bit dell'istruzione
            case 0x00E0 : // clear the display
                memset(chip8->DISPLAY, 0, DISPLAY_SIZE); 
                break;
            case 0x00EE : // ritorna da una subroutine, dovremo prelevare l'indirizzo di ritorno dallo stack e impostare il program counter a quell'indirizzo
                chip8->sp--;
                chip8->pc = chip8->stack[chip8->sp]; //imposta il pc al valore trovato nello stack
                break;
            case 0x00FE : // disable extended screen mode
                // Per screen mode standard (non implementato completamente)
                break;
            case 0x00FF : // enable extended screen mode  
                // Per screen mode 128x64 (non implementato completamente)
                break;
        }
        break;
        case 0x1000 : // jump to address NNN, imposta il program counter a NNN, ossia l'indirizzo specificato negli ultimi 12 bit dell'istruzione
            chip8->pc = opcode & 0x0FFF; //estrae l'indirizzo NNN dagli ultimi 12 bit dell'istruzione
            break;
        case 0x2000 : 
            chip8->stack[chip8->sp] = chip8->pc; //scrive nello stack l'RA
            chip8->sp++; //incrementa l'SP
            chip8->pc = opcode & 0x0FFF; //imposta il PC all'indirizzo NNN
        break;
        case 0x3000 : 
            if(chip8->V[(opcode >> 8) & 0x0F] == (opcode & 0x00FF)){ 
                //se il valore del registro Vx è uguale al byte NN, salta la prossima istruzioneincrementando il PC di 2 byte
                //la prima parte serve per estrarre il numero del registro scritto nei primi 4 bit dell'operazione, lo shift è importante se vogliamo tradurlo in intero 
                //la seconda parte serve per estrarre il byte NN dagli ultimi 8 bit dell'istruzione
                chip8->pc += 2; //salta la prossima istruzione incrementando il PC di 2 byte
            }
        break;
        case 0x4000 : 
            if(chip8->V[(opcode >>8) & 0x0F] != (opcode & 0x00FF)){ 
                //se il valore del registro Vx è diverso dal byte NN, salta la prossima istruzione incrementando il PC di 2 byte
                chip8->pc += 2; //salta la prossima istruzione incrementando il PC di 2 byte
            }
        break;
        case 0x5000 : 
            if(chip8->V[(opcode >> 8) & 0x0F] == chip8->V[opcode & 0x000F]){
                chip8->pc += 2; 
            }
        break;
        case 0x6000 : 
            chip8->V[(opcode >> 8) & 0x0F] = opcode & 0x00FF; // imposta il valore del registro Vx al byte NN
        break;
        case 0x7000 :
            chip8->V[(opcode >> 8) & 0x0F] += (opcode & 0x00FF); // aggiunge il byte NN al valore del registro Vx 
        break;
        case 0x8000 : 
            switch(opcode & 0x000F){ //controlla gli ultimi 4 bit per determinare l'operazione specifica da eseguiresui registri Vx e Vy
                case 0x0000 : 
                    chip8->V[(opcode >> 8) & 0x0F] = chip8->V[opcode & 0x000F]; // imposta il valore del registro Vx al valore del registro Vy
                break;
                case 0x0001 :   
                    chip8->V[(opcode >> 8) & 0x0F] |= chip8->V[(opcode & 0x000F)];
                break;
                case 0x0002 :
                    chip8->V[(opcode >> 8) & 0x0F] &= chip8->V[(opcode & 0x000F)];
                break;
                case 0x0003 :
                    chip8->V[(opcode >> 8) & 0x0F] ^= chip8->V[(opcode & 0x000F)];
                break;
                case 0x0004 :
                    chip8->V[(opcode >> 8) & 0x0F] += chip8->V[(opcode & 0x000F)];
                    chip8->V[0xF] = (chip8->V[(opcode >> 8) & 0x0F] < chip8->V[(opcode & 0x000F)]) ? 1 : 0; // se c'è un overflow, imposta VF a 1, altrimenti a 0
                break;
                case 0x0005 :
                    chip8->V[(opcode >> 8) & 0x0F] -= chip8->V[(opcode & 0x000F)];
                    chip8->V[0xF] = (chip8->V[(opcode >> 8) & 0x0F] > chip8->V[(opcode & 0x000F)]) ? 1 : 0; // se c'è un borrow, imposta VF a 1, altrimenti a 0
                break;
                case 0x0006 :
                    chip8->V[0xF] = chip8->V[(opcode >> 8) & 0x0F] & 0x1; // salva il bit meno significativo di Vx in VF
                    chip8->V[(opcode >> 8) & 0x0F] >>= 1; // sposta a destra il valore del registro Vx di 1 bit
                break;
                case 0x0007:
                    chip8->V[(opcode >> 8) & 0x0F] = chip8->V[(opcode & 0x000F)] - chip8->V[(opcode >> 8) & 0x0F];
                    chip8->V[0xF] = (chip8->V[(opcode & 0x000F)] > chip8->V[(opcode >> 8) & 0x0F]) ? 1 : 0; // se c'è un borrow, imposta VF a 1, altrimenti a 0chip 
                break;
                case 0x000E : 
                    chip8->V[0xF] = (chip8->V[(opcode >> 8) & 0X0F] >> 7) & 0x1; // salva il bit più significativo di Vx in VF
                    chip8->V[(opcode >> 8) & 0X0F] <<=1; //shift di una posizione a sinistra
                break; 
            }
            break; /* <--- aggiunto per evitare il fall‑through verso 0x9000 */
            case 0x9000 :   
            if(chip8->V[(opcode >> 8) & 0x0f] != chip8->V[opcode & 0X000F]){ //controlliamo se il valore dei registri è diverso
                chip8->pc += 2;
            }
            break;
            case 0xA000 : 
                chip8->I = opcode & 0X0FFF; //imposta il registro index all'indirizzo NNN
            break;
            case 0xB000 :   
                chip8->pc = (opcode & 0x0FFF) + chip8->V[0]; //imposta il program counter all'indirizzo NNN più il valore del registro V0
            break;
            case 0xC000 : 
                chip8->V[(opcode >> 8) & 0X0F] = (rand() % 256) & (opcode & 0x00FF); //rand() genera un numero casuale, poi l'operazione di modulo lo fa rientrare nell'intervallo 0-255, infine l'and lo limita ulteriormente al byte NN specificato negli ultimi 8 bit
            break;
            case 0xD000 : { //lo sprite è un'immagine di 8 pixel di larghezza e n pixel di altezza, memorizzata in memoria a partire dall'indirizzo specificato nel registro index I. L'istruzione disegna lo sprite sul display a coordinate (Vx, Vy) e imposta VF a 1 se c'è una collisione (cioè se un pixel disegnato sovrascrive un pixel già acceso)
                unsigned char x = chip8->V[(opcode >> 8) & 0x0F]; // Vx, serve perestrarre il numero del registro dall'opcode
                unsigned char y = chip8->V[(opcode >> 4) & 0x0F]; // Vy, serve per estrarre il numero del registro dall'opcode
                unsigned char height = opcode & 0x000F;           // n, serve per estrarre il numero di righe dello sprite dagli ultimi 4 bit dell'opcode
                chip8->V[0xF] = 0;                                // VF = 0 inizialmente

                for (int row = 0; row < height; row++) { //per ogni riga dello sprite, leggiamo un byte dalla memoriaa partire dall'indirizzo I
                    unsigned char spriteByte = chip8->memory[chip8->I + row]; // contiene la riga da copiare 
                    for (int col = 0; col < 8; col++) { // copieremo ogni elemento nella sua posizione all'interno della riga
                        // bit più significativo corrisponde al pixel di sinistra
                        unsigned char spritePixel = (spriteByte >> (7 - col)) & 0x1; //sprite pixel contiene il valore del pixel corrispondente alla posizione 
                        if (spritePixel) {
                            unsigned short px = (x + col) % 64; //calcola l'ascissa del pixel
                            unsigned short py = (y + row) % 32; //calcola l'ordinata del pixel
                            unsigned short index = py * 64 + px; //serve per identificare il pixel corrente preciso
                            if (chip8->DISPLAY[index]) {
                                chip8->V[0xF] = 1;   // collisione: cancellato un pixel
                            }
                            chip8->DISPLAY[index] ^= 1; // XOR con il pixel corrente, serve perchè la logica dietro l'aggiornamento degli sprite è di inversione
                        }
                    }
                }
            }
            break;
            case 0XE000 :
                switch(opcode & 0x00FF){
                    case 0x009E : 
                        if(chip8->keys[chip8->V[(opcode >> 8) & 0x0F]]){
                            chip8->pc += 2; //salta la prossima istruzione se il tasto corrispondente al valore del registro Vx è premuto
                        }
                    break;
                    case 0x00A1 : 
                        if(!chip8->keys[chip8->V[(opcode >> 8) & 0x0F]]){
                            chip8->pc += 2; //salta la prossima istruzione se il tasto corrispondente al valore del registro Vx non è premuto
                        }
                } 
            break;
            case 0xF000 : 
                switch(opcode & 0x00FF){
                    case 0x0007 : 
                        chip8->V[(opcode >> 8) & 0X0F] = chip8->delay_timer; 
                    break;
                    case 0x000A :
                        {
                            key_pressed = false; 
                            for(int i=0; i<16; i++){
                                if(chip8->keys[i]){
                                    chip8->V[(opcode >> 8) & 0X0F] = i; //salva il numero del tasto premuto nel registro Vx
                                    key_pressed = true; //indica che un tasto è stato premuto
                                }
                            } 
                        }
                    break;
                    case 0x0015 : 
                        chip8->delay_timer = chip8->V[(opcode >> 8) & 0x0F]; //imposta il timer di ritardo al valore del registro Vx
                    break;
                    case 0x0018: 
                        chip8->sound_timer = chip8->V[(opcode >> 8) & 0x0F]; //imposta il timer del suono al valore del registro Vx
                    break;
                    case 0X001E : 
                        chip8->I += chip8->V[(opcode >> 8) & 0x0F]; //Aggiunge il valore del registro Vx al registro index I
                    break;
                    case 0x0029 : 
                        chip8->V[0xF] = chip8->I / 5; //calcola il numero del carattere da disegnare in base al valore del registro index I e lo salva in VF
                    break;
                    case 0x0033 : 
                        chip8->memory[chip8->I] = chip8->V[(opcode >> 8) & 0x0F] / 100; //Salva le centinaia
                        chip8->memory[chip8->I + 1] = (chip8->V[(opcode >> 8) & 0x0F] / 10) % 10; //Salva le decine
                        chip8->memory[chip8->I + 2] = chip8->V[(opcode >> 8) & 0x0F] % 10; //Salva le unità
                    break;
                    case 0x0055 : 
                        for(int i = 0; i < ((opcode >> 8) & 0x0F); i++){
                            chip8->memory[chip8->I + i] = chip8->V[i]; //Salva i registri V0 fino a Vx nella memoria a partire dall'indirizzo I
                        }
                    break;
                    case 0x0065 : 
                        for(int i = 0; i < ((opcode >> 8) & 0x0F); i++){
                            chip8->V[i] = chip8->memory[chip8->I + i]; //Carica i registri V0 fino a Vx dalla memoria a partire dall'indirizzo I
                        }
                    break; 
                }
            break; 
        break;
    }
} 

void updateTimers(Chip8* chip8){ //il timer di ritardo e suono servono per creare effetti di temporizzazione nei giochi, come ad esempio il movimento dei personaggi o la durata di un suono
    if(chip8->delay_timer > 0) {
        chip8->delay_timer--;
    }
    if(chip8->sound_timer > 0) {
        chip8->sound_timer--;
    }
}

void drawGraphics(Chip8* chip8){ //aggiorna lo schermo
    for(int y = 0; y < 32; y++){
        for(int x = 0; x < 64; x++){
            putchar(chip8->DISPLAY[y * 64 + x] ? '#' : ' ');
        }
        putchar('\n');
    }
}

void setKeys(Chip8* chip8){
    int key; 
    printf("Premi un tasto (0-F): ");
    scanf("%x", &key); // legge un tasto esadecimale da 0 a F
    if (key >= 0 && key <= 0xF) {   
        chip8->keys[key] = true; // aggiorna lo stato del tasto premuto
    }
}