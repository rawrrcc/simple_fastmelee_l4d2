// fastmelee.cpp
// C++14, автоматизація fast melee для L4D2
// Емуляція натискань клавіш: слотів і Mouse1, поки затиснута Mouse5
// F3 перемикає слот (1/4/5)

#include <windows.h>
#include <thread>
#include <atomic>
#include <iostream>
#include <conio.h>

std::atomic<bool> running(false);
std::atomic<int> slot_mode(1); // 1, 4, 5
std::atomic<bool> menu_update(true);

// Helper: send key using SendInput with scan code
void send_key_scan(WORD vk, WORD sc) {
    INPUT input[2] = {};
    input[0].type = INPUT_KEYBOARD;
    input[0].ki.wVk = 0;
    input[0].ki.wScan = sc;
    input[0].ki.dwFlags = KEYEVENTF_SCANCODE;
    input[1] = input[0];
    input[1].ki.dwFlags |= KEYEVENTF_KEYUP;
    SendInput(2, input, sizeof(INPUT));
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
}

// Helper: send Mouse1 click
void send_mouse1() {
    INPUT input[2] = {};
    input[0].type = INPUT_MOUSE;
    input[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    input[1].type = INPUT_MOUSE;
    input[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;
    SendInput(2, input, sizeof(INPUT));
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

// Main fastmelee loop (slot 2 -> Mouse1 -> wait -> slot melee)
void fastmelee_loop() {
    while (running) {
        // 1. Switch to slot 2 (secondary)
        send_key_scan('2', 0x03); // 2 key (slot 2)
        std::this_thread::sleep_for(std::chrono::milliseconds(450)); // Short pause for slot switch
        // 2. Attack
        send_mouse1();
        std::this_thread::sleep_for(std::chrono::milliseconds(50)); // Wait for melee animation (adjust as needed)
        // 3. Switch to melee slot (1/4/5)
        switch (slot_mode.load()) {
        case 1: send_key_scan('1', 0x02); break; // 1 key
        case 4: send_key_scan('4', 0x05); break; // 4 key
        case 5: send_key_scan('5', 0x06); break; // 5 key
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(60)); // Short pause
    }
}

// Hotkey thread
DWORD WINAPI hotkey_thread(LPVOID) {
    bool prev_mouse5 = false;
    bool prev_f3 = false;
    while (true) {
        SHORT mouse5 = GetAsyncKeyState(VK_XBUTTON2);
        SHORT f3 = GetAsyncKeyState(VK_F3);
        if ((mouse5 & 0x8000) && !prev_mouse5) {
            running = true;
            std::thread(fastmelee_loop).detach();
        }
        if (!(mouse5 & 0x8000) && prev_mouse5) {
            running = false;
        }
        if ((f3 & 0x8000) && !prev_f3) {
            int mode = slot_mode.load();
            if (mode == 1) slot_mode = 4;
            else if (mode == 4) slot_mode = 5;
            else slot_mode = 1;
            menu_update = true;
        }
        prev_mouse5 = (mouse5 & 0x8000);
        prev_f3 = (f3 & 0x8000);
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    return 0;
}

void print_menu() {
    system("cls");
    std::cout << "=== FastMelee Script for L4D2 ===\n";
    std::cout << "Current mode: ";
    int mode = slot_mode.load();
    if (mode == 1) std::cout << "Slot 1 (key 1)\n";
    else if (mode == 4) std::cout << "Slot 4 (key 4)\n";
    else if (mode == 5) std::cout << "Slot 5 (key 5)\n";
    std::cout << "Activation: Mouse5 (side mouse button)\n";
    std::cout << "Switch mode: F3\n";
    std::cout << "Exit: Esc\n";
    std::cout << "Status: " << (running ? "ACTIVE" : "inactive") << "\n";
    std::cout << "\nIf it doesn't work in-game, run as administrator!\n";
}

int main() {
    SetConsoleOutputCP(65001); // UTF-8
    CreateThread(nullptr, 0, hotkey_thread, nullptr, 0, nullptr);
    print_menu();
    while (true) {
        if (menu_update) {
            print_menu();
            menu_update = false;
        }
        if (_kbhit()) {
            int ch = _getch();
            if (ch == 27) break; // Esc
        }
        static bool prev_running = false;
        if (prev_running != running) {
            print_menu();
            prev_running = running;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    running = false;
    std::cout << "\nExiting FastMelee.\n";
    return 0;
}
