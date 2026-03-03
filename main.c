#include "Chip8-emulator.h"
#include <windows.h>
#include <conio.h>
#include <string.h>

// Nasconde il cursore del terminale
void hideCursor() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
}

// Mostra il cursore del terminale
void showCursor() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = TRUE;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
}

// Mappatura dei tasti Windows ai tasti CHIP-8 per Space Invaders
void updateKeys(Chip8* chip8) {
    // Resettiamo tutto a false
    memset(chip8->keys, false, 16);
    
    // Space Invaders puo usare diversi tasti:
    // Per muoversi: 4, 6, 7, 9 o frecce
    // Per sparare: 0, 5, Q, W, Space
    
    // --- SINISTRA (tasti 4, 7) ---
    if (GetAsyncKeyState('A') & 0x8000) chip8->keys[0x7] = true;
    if (GetAsyncKeyState('4') & 0x8000) chip8->keys[0x7] = true;
    if (GetAsyncKeyState('7') & 0x8000) chip8->keys[0x7] = true;
    if (GetAsyncKeyState(VK_LEFT) & 0x8000) chip8->keys[0x7] = true;
    
    // --- DESTRA (tasti 6, 9) ---
    if (GetAsyncKeyState('D') & 0x8000) chip8->keys[0x9] = true;
    if (GetAsyncKeyState('6') & 0x8000) chip8->keys[0x9] = true;
    if (GetAsyncKeyState('9') & 0x8000) chip8->keys[0x9] = true;
    if (GetAsyncKeyState(VK_RIGHT) & 0x8000) chip8->keys[0x9] = true;
    
    // --- SPARA (tasti 0, 5, Q, W, Space) ---
    if (GetAsyncKeyState('Q') & 0x8000) chip8->keys[0x5] = true;
    if (GetAsyncKeyState('W') & 0x8000) chip8->keys[0x5] = true;
    if (GetAsyncKeyState('5') & 0x8000) chip8->keys[0x5] = true;
    if (GetAsyncKeyState('0') & 0x8000) chip8->keys[0x0] = true;
    if (GetAsyncKeyState(VK_SPACE) & 0x8000) chip8->keys[0x0] = true;
    if (GetAsyncKeyState(VK_RETURN) & 0x8000) chip8->keys[0x5] = true; // Enter
    
    // --- TASTO PER AVVIARE IL GIOCO ---
    // Alcune versioni usano il tasto 5 per iniziare
    // Prova anche Z, X, C come alternative
    if (GetAsyncKeyState('Z') & 0x8000) chip8->keys[0x5] = true;
    if (GetAsyncKeyState('X') & 0x8000) chip8->keys[0x5] = true;
    if (GetAsyncKeyState('C') & 0x8000) chip8->keys[0x5] = true;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <rom_file>\n", argv[0]);
        return 1;
    }

    // Nasconde il cursore
    hideCursor();
    
    Chip8 chip8;
    
    // Inizializza l'emulatore
    initialize(&chip8);
    
    // Carica la ROM
    printf("Loading ROM: %s\n", argv[1]);
    loadRom(&chip8, argv[1]);
    
    printf("ROM loaded!\n");
    printf("Controls: A/D (move), Q/W/SPACE/ENTER (shoot/start)\n");
    printf("Press ESC to exit.\n\n");
    
    Sleep(1000);
    
    // Loop principale di emulazione
    while (1) {
        // Aggiorna i tasti (input in tempo reale)
        updateKeys(&chip8);
        
        // Esegue un ciclo di emulazione (una istruzione)
        emulateCycle(&chip8);
        
        // Aggiorna i timer (60 Hz)
        updateTimers(&chip8);
        
        // Disegna la grafica
        drawGraphics(&chip8);
        
        // Piccola pausa per controllare la velocità (circa 60 Hz)
        Sleep(16); // ~16ms = 60fps
        
        // Controlla se è premuto ESC per uscire
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
            break;
        }
        
        // Pulisci schermo per il prossimo frame
        system("cls");
    }
    
    // Mostra il cursore prima di uscire
    showCursor();
    printf("Exiting emulator...\n");
    return 0;
}

