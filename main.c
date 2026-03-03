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
    // Resetta tutti i tasti
    memset(chip8->keys, false, 16);
    
    // La tastiera del CHIP-8 è:
    // 1 2 3 C  -> 1, 2, 3, C (4)
    // 4 5 6 D  -> 4, 5, 6, D (D)
    // 7 8 9 E  -> 7, 8, 9, E (E)
    // A 0 B F  -> A(10), 0, B(11), F(15)
    
    // Space Invaders usa:
    // 7 = sinistra
    // 8 = giù (non usato)
    // 9 = destra  
    // 0 = sparo
    
    // Freccia sinistra -> tasto 7
    if (GetAsyncKeyState(VK_LEFT) & 0x8000) chip8->keys[0x7] = true;
    
    // Freccia destra -> tasto 9
    if (GetAsyncKeyState(VK_RIGHT) & 0x8000) chip8->keys[0x9] = true;
    
    // Spazio -> tasto 0 (spara)
    if (GetAsyncKeyState(VK_SPACE) & 0x8000) chip8->keys[0x0] = true;
    
    // Alternative: tastierino numerico
    if (GetAsyncKeyState('4') & 0x8000) chip8->keys[0x7] = true;  // 4 = sinistra
    if (GetAsyncKeyState('6') & 0x8000) chip8->keys[0x9] = true;  // 6 = destra
    if (GetAsyncKeyState('5') & 0x8000) chip8->keys[0x0] = true;  // 5 = spara
    
    // Anche Z-X-C come alternative
    if (GetAsyncKeyState('Z') & 0x8000) chip8->keys[0x7] = true;  // Z = sinistra
    if (GetAsyncKeyState('X') & 0x8000) chip8->keys[0x9] = true; // X = destra
    if (GetAsyncKeyState('C') & 0x8000) chip8->keys[0x0] = true; // C = spara
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
    printf("Controls: Arrow keys or Z/X/C to move, SPACE or 5 to shoot\n");
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

