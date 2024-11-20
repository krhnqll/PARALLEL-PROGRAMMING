#include "icb_gui.h"

int FRM1, BTN;
ICBYTES screen, A;
bool animationRunning = false;
HANDLE animationThreadHandle;

void startAnimation(void* v);
DWORD WINAPI animationThread(LPVOID lpParam);

void ICGUI_Create() {
    ICG_MWSize(800, 600);
    ICG_MWColor(100, 50, 0);
    ICG_MWTitle("TEZGAH LAN BU");
}

void ICGUI_main() {
    FRM1 = ICG_FrameMedium(0, 0, 800, 600);
    BTN = ICG_TButton(350, 550, 100, 30, "Start Animation", startAnimation, startAnimation);
}

void drawCar(ICBYTES& screen, int x, int y, unsigned int color, bool brakeOn = false, bool doorsOpen = false) {
    FillRect(screen, x, y, 60, 30, color);
    FillRect(screen, x + 5, y - 10, 50, 10, color);

    if (doorsOpen) {
        FillRect(screen, x - 5, y + 5, 5, 20, color); 
        FillRect(screen, x + 60, y + 5, 5, 20, color); 
    }

    unsigned int lightColor = brakeOn ? 0xFF0000 : 0xFFFF00;
    FillRect(screen, x, y + 5, 5, 5, lightColor);
    FillRect(screen, x + 55, y + 5, 5, 5, lightColor);

    FillRect(screen, x + 5, y + 30, 15, 5, 0x000000);
    FillRect(screen, x + 40, y + 30, 15, 5, 0x000000);
}

void drawRoad(ICBYTES& screen) {
    FillRect(screen, 250, 0, 300, 600, 0x707070);
    for (int i = 0; i < 600; i += 40) {
        FillRect(screen, 395, i, 10, 20, 0xFFFFFF);
    }
}

void drawTrees(ICBYTES& screen) {
    for (int i = 50; i < 600; i += 100) {
        FillRect(screen, 100, i, 20, 40, 0x228B22);
        FillRect(screen, 90, i - 30, 40, 30, 0x006400);
    }
    for (int i = 80; i < 600; i += 100) {
        FillRect(screen, 650, i, 20, 40, 0x228B22);
        FillRect(screen, 640, i - 30, 40, 30, 0x006400);
    }
}

void drawMan(ICBYTES& screen, int x, int y, unsigned int color, bool fallen = false) {
    if (!fallen) {
        FillRect(screen, x, y, 10, 20, color); 
        FillRect(screen, x + 2, y - 10, 6, 6, color); 
    }
    else {
        FillRect(screen, x, y + 10, 20, 10, color); 
        FillRect(screen, x + 10, y, 6, 6, color); 
    }
}

void drawBullet(ICBYTES& screen, int x, int y, unsigned int color) {
    FillRect(screen, x, y, 5, 2, color);
}

void drawBlood(ICBYTES& screen, int x, int y) {
    FillRect(screen, x, y + 20, 10, 5, 0x8B0000);   
    FillRect(screen, x + 5, y + 25, 15, 3, 0x8B0000);
    FillRect(screen, x + 10, y + 27, 7, 3, 0x8B0000);
}

DWORD WINAPI animationThread(LPVOID lpParam) {
    animationRunning = true;


    CreateImage(screen, 800, 600, ICB_UINT);

    int car1_y = 600, car2_y = 0;
    int man1_x = 340, man1_y = 300;
    int man2_x = 420, man2_y = 300;

    int bullet1_x = man1_x + 10, bullet1_y = 300;
    int bullet2_x = man2_x, bullet2_y = 300;

    bool man1_out = false, man2_out = false;
    bool man2_fallen = false, man1_fallen = false;
    bool blood_drawn1 = false, blood_drawn2 = false;
    bool cars_stopped = false;
    bool bullet1_visible = true;
    bool bullet2_visible = true;

    for (int frame = 0; frame <= 300 && animationRunning; ++frame) {
        FillRect(screen, 0, 0, 800, 600, 0x643A0D);
        drawRoad(screen);
        drawTrees(screen);

        if (frame <= 100) {
            car1_y = 600 - frame * 2;
            car2_y = frame * 2;
        }
        else if (frame > 100 && frame <= 150) {
            car1_y += 1;
            car2_y -= 1;
        }
        else {
            cars_stopped = true;
        }

        if (cars_stopped && frame >= 150 && frame < 170) {
            man1_out = true;
            man2_out = true;
        }

        drawCar(screen, 320, car1_y, 0xFFFFFF, cars_stopped && frame > 100, man1_out);
        drawCar(screen, 420, car2_y, 0x888888, cars_stopped && frame > 100, man2_out);

        if (man1_out) {
            if (frame < 200) {
                drawMan(screen, man1_x--, man1_y, 0xFFFFFF); 
            }
            else {
                drawMan(screen, man1_x++, man1_y, 0xFFFFFF);
                if (bullet1_visible) {
                    drawBullet(screen, bullet1_x + (frame - 200), bullet1_y, 0xFF0000); 
                }
            }
        }

        if (man2_out) {
            if (frame < 200) {
                drawMan(screen, man2_x++, man2_y, 0x888888);
            }
            else if (!man2_fallen) {
                drawMan(screen, man2_x--, man2_y, 0x888888);
                if (bullet2_visible) {
                    drawBullet(screen, bullet2_x - (frame - 200), bullet2_y, 0xFF0000); 
                }
            }
            else {
                drawMan(screen, man2_x, man2_y, 0x888888, true);
                if (!blood_drawn2) {
                    drawBlood(screen, man2_x, man2_y); 
                    blood_drawn2 = true;
                }
            }
        }

        if (!man2_fallen && bullet1_x + (frame - 200) >= man2_x && bullet1_y == man2_y) {
            man2_fallen = true;   
            bullet1_visible = false; 
        }

        if (!man1_fallen && bullet2_x - (frame - 200) <= man1_x && bullet2_y == man1_y) {
            man1_fallen = true;   
            bullet2_visible = false; 
        }

        if (man1_fallen && !blood_drawn1) {
            drawBlood(screen, man1_x, man1_y); 
            blood_drawn1 = true;
        }

        if (blood_drawn2) {
            drawBlood(screen, man2_x, 310);
        }

        DisplayImage(FRM1, screen);
        Sleep(30);
    }

    animationRunning = false;
    DestroyWindow(GetConsoleWindow()); 
    return 0;
}

void startAnimation(void* v) {
    if (animationRunning) return;

    animationThreadHandle = CreateThread(NULL, 0, animationThread, NULL, 0, NULL);
    if (animationThreadHandle == NULL) {
        MessageBox(NULL, "Thread başlatılamadı!", "Hata", MB_OK);
    }
}
