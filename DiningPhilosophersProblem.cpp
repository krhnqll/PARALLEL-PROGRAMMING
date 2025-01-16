#include <icb_gui.h>
#include <icbytes.h>
#include <windows.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>

//Ortak çalışma ile proje geliştirilmiştir.

#ifndef M_PI
#define M_PI 3.14159265358979323846 
#endif

#define NUM_PHILOSOPHERS 5
#define TIMER_ID 1


HWND hwndStartDeadlock, hwndStartNoDeadlock;
HANDLE chopsticks[NUM_PHILOSOPHERS];
HANDLE threads[NUM_PHILOSOPHERS];
bool stopThreads = false;
int states[NUM_PHILOSOPHERS];

POINT positions[NUM_PHILOSOPHERS];
enum { THINKING, HUNGRY, EATING };
HBRUSH brushes[3];
HBRUSH tableBrush;
HBRUSH plateBrush, foodBrush, forkBrush, spoonBrush;

const POINT tableCenter = { 250, 250 };
const int tableRadius = 100;
RECT windowRect;


void updatePhilosopherState(HWND hwnd, int id, int state) {
    states[id] = state;
    InvalidateRect(hwnd, NULL, TRUE);
}


DWORD WINAPI philosopherDeadlock(LPVOID param) {
    int id = (int)(INT_PTR)param;
    while (!stopThreads) {
        updatePhilosopherState(GetForegroundWindow(), id, THINKING);
        Sleep(2000);

        updatePhilosopherState(GetForegroundWindow(), id, HUNGRY);
        WaitForSingleObject(chopsticks[id], INFINITE); // Left chopstick
        WaitForSingleObject(chopsticks[(id + 1) % NUM_PHILOSOPHERS], INFINITE); // Right chopstick

        updatePhilosopherState(GetForegroundWindow(), id, EATING);
        Sleep(3000);

        ReleaseSemaphore(chopsticks[id], 1, NULL);
        ReleaseSemaphore(chopsticks[(id + 1) % NUM_PHILOSOPHERS], 1, NULL);
    }
    return 0;
}


DWORD WINAPI philosopherNoDeadlock(LPVOID param) {
    int id = (int)(INT_PTR)param;
    while (!stopThreads) {
        updatePhilosopherState(GetForegroundWindow(), id, THINKING);
        Sleep(2000);

        updatePhilosopherState(GetForegroundWindow(), id, HUNGRY);
        int left = min(id, (id + 1) % NUM_PHILOSOPHERS);
        int right = max(id, (id + 1) % NUM_PHILOSOPHERS);

        WaitForSingleObject(chopsticks[left], INFINITE);
        WaitForSingleObject(chopsticks[right], INFINITE);

        updatePhilosopherState(GetForegroundWindow(), id, EATING);
        Sleep(3000);

        ReleaseSemaphore(chopsticks[left], 1, NULL);
        ReleaseSemaphore(chopsticks[right], 1, NULL);
    }
    return 0;
}


void paintPhilosophers(HDC hdc) {
    SelectObject(hdc, tableBrush);
    Ellipse(hdc, tableCenter.x - tableRadius, tableCenter.y - tableRadius,
        tableCenter.x + tableRadius, tableCenter.y + tableRadius);

    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {

        HBRUSH brush = brushes[states[i]];
        SelectObject(hdc, brush);
        Ellipse(hdc, positions[i].x - 30, positions[i].y - 30,
            positions[i].x + 30, positions[i].y + 30);


        SelectObject(hdc, plateBrush);
        Ellipse(hdc, tableCenter.x - 15 + (int)(tableRadius / 1.5 * cos(i * 2 * M_PI / NUM_PHILOSOPHERS)),
            tableCenter.y - 15 + (int)(tableRadius / 1.5 * sin(i * 2 * M_PI / NUM_PHILOSOPHERS)),
            tableCenter.x + 15 + (int)(tableRadius / 1.5 * cos(i * 2 * M_PI / NUM_PHILOSOPHERS)),
            tableCenter.y + 15 + (int)(tableRadius / 1.5 * sin(i * 2 * M_PI / NUM_PHILOSOPHERS)));


        SelectObject(hdc, forkBrush);
        Ellipse(hdc, positions[i].x - 5, positions[i].y + 35, positions[i].x + 5, positions[i].y + 55);

        SelectObject(hdc, spoonBrush);
        Ellipse(hdc, positions[i].x - 15, positions[i].y + 35, positions[i].x - 5, positions[i].y + 55);
    }
}

void paintBackground(HDC hdc) {
    HBRUSH houseBrush = CreateSolidBrush(RGB(210, 180, 140));
    SelectObject(hdc, houseBrush);
    Rectangle(hdc, 50, 50, windowRect.right - 50, windowRect.bottom - 150);

    POINT roof[] = { {50, 50}, {windowRect.right / 2, 10}, {windowRect.right - 50, 50} };
    HBRUSH roofBrush = CreateSolidBrush(RGB(139, 0, 0));
    SelectObject(hdc, roofBrush);
    Polygon(hdc, roof, 3);

    DeleteObject(houseBrush);
    DeleteObject(roofBrush);
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        brushes[THINKING] = CreateSolidBrush(RGB(169, 169, 169));
        brushes[HUNGRY] = CreateSolidBrush(RGB(255, 0, 0));
        brushes[EATING] = CreateSolidBrush(RGB(0, 255, 0));
        tableBrush = CreateSolidBrush(RGB(139, 69, 19));
        plateBrush = CreateSolidBrush(RGB(255, 255, 255));
        foodBrush = CreateSolidBrush(RGB(255, 215, 0));
        forkBrush = CreateSolidBrush(RGB(169, 169, 169));

        for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
            chopsticks[i] = CreateSemaphore(NULL, 1, 1, NULL);
        }

        GetClientRect(hwnd, &windowRect);
        const double angleStep = 2 * M_PI / NUM_PHILOSOPHERS;
        for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
            positions[i].x = tableCenter.x + (int)(150 * cos(i * angleStep));
            positions[i].y = tableCenter.y + (int)(150 * sin(i * angleStep));
        }

        hwndStartDeadlock = CreateWindow("BUTTON", "Start No Deadlock", WS_CHILD | WS_VISIBLE,
            100, windowRect.bottom - 100, 150, 30, hwnd, (HMENU)1, NULL, NULL);
        hwndStartNoDeadlock = CreateWindow("BUTTON", "Start Deadlock", WS_CHILD | WS_VISIBLE,
            300, windowRect.bottom - 100, 150, 30, hwnd, (HMENU)2, NULL, NULL);

        SetTimer(hwnd, TIMER_ID, 100, NULL);
        break;
    }
    case WM_SIZE:
        GetClientRect(hwnd, &windowRect);
        InvalidateRect(hwnd, NULL, TRUE);
        break;
    case WM_COMMAND:
        if (LOWORD(wParam) == 1) {
            stopThreads = false;
            for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
                threads[i] = CreateThread(
                    NULL, 0, philosopherDeadlock, (LPVOID)(INT_PTR)i, 0, NULL);
            }
        }
        else if (LOWORD(wParam) == 2) {
            stopThreads = false;
            for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
                threads[i] = CreateThread(
                    NULL, 0, philosopherNoDeadlock, (LPVOID)(INT_PTR)i, 0, NULL);
            }
        }
        break;
    case WM_TIMER:
        if (wParam == TIMER_ID) {
            InvalidateRect(hwnd, NULL, TRUE);
        }
        break;
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        paintBackground(hdc);
        paintPhilosophers(hdc);
        EndPaint(hwnd, &ps);
        break;
    }
    case WM_DESTROY: {
        stopThreads = true;


        WaitForMultipleObjects(NUM_PHILOSOPHERS, threads, TRUE, INFINITE);
        for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
            CloseHandle(threads[i]);
            CloseHandle(chopsticks[i]);
        }

        KillTimer(hwnd, TIMER_ID);
        PostQuitMessage(0);
        break;
    }
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "DiningPhilosophers";

    RegisterClass(&wc);

    HWND hwnd = CreateWindow(wc.lpszClassName, "Dining Philosophers Problem",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT,
        800, 600, NULL, NULL, hInstance, NULL);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
