#include "icb_gui.h"
#include <cstdlib>
#include <ctime>
#include <windows.h>
#include <thread>


ICBYTES m;
HANDLE hFireMutex;
int FRM;
int keypressed = 0;
bool thread_continue = false;

#define MAX_BULLETS 1
#define MAX_BALLS 6
#define MAX_EXPLOSIONS 5

int bulletX[MAX_BULLETS], bulletY[MAX_BULLETS];
bool bulletActive[MAX_BULLETS];

int boxlocation = 10;
int boxY = 380;

bool explosionActive[MAX_EXPLOSIONS] = { false };
int explosionX[MAX_EXPLOSIONS], explosionY[MAX_EXPLOSIONS], explosionDuration[MAX_EXPLOSIONS];
int explosionSize[MAX_EXPLOSIONS];

int ballX[MAX_BALLS], ballY[MAX_BALLS];
int ballWidth[MAX_BALLS], ballHeight[MAX_BALLS];
bool ballActive[MAX_BALLS] = { false };
bool ballMovingUp[MAX_BALLS] = { false };
int ballMoveSpeedX[MAX_BALLS] = { 0 };
int ballMoveSpeedY[MAX_BALLS] = { 0 };

void InitializeGameObjects();
void FireBullet();
void CreateBall();
void HandleCollision(int bulletIdx, int ballIdx);
void MoveBall(int i);
void DrawExplosions();
DWORD WINAPI SlidingBox(LPVOID lpParam);
void WhenKeyPressed(int k);
void butonfonk();

void InitializeGameObjects() {
    for (int i = 0; i < MAX_BULLETS; i++) {
        bulletActive[i] = false;
    }
    for (int i = 0; i < MAX_BALLS; i++) {
        ballActive[i] = false;
        ballMovingUp[i] = false;
        ballMoveSpeedX[i] = 0;
        ballMoveSpeedY[i] = 0;
    }
    for (int i = 0; i < MAX_EXPLOSIONS; i++) {
        explosionActive[i] = false;
    }
}

void FireBullet() {
    WaitForSingleObject(hFireMutex, INFINITE);
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bulletActive[i]) {
            bulletX[i] = boxlocation + 8;
            bulletY[i] = boxY - 10;
            bulletActive[i] = true;
            break;
        }
    }
    ReleaseMutex(hFireMutex);
}

void CreateBall() {
    for (int i = 0; i < MAX_BALLS; i++) {
        if (!ballActive[i]) {
            ballX[i] = rand() % 380 + 10;
            ballY[i] = 0;
            ballWidth[i] = ballHeight[i] = (rand() % 5 + 1) * 10;
            ballActive[i] = true;
            ballMovingUp[i] = false;
            break;
        }
    }
}

void HandleCollision(int bulletIdx, int ballIdx) {
    if (bulletX[bulletIdx] >= ballX[ballIdx] &&
        bulletX[bulletIdx] <= ballX[ballIdx] + ballWidth[ballIdx] &&
        bulletY[bulletIdx] >= ballY[ballIdx] &&
        bulletY[bulletIdx] <= ballY[ballIdx] + ballHeight[ballIdx]) {

        int centerZoneStart = static_cast<int>(ballWidth[ballIdx] * 0.3);
        int centerZoneEnd = static_cast<int>(ballWidth[ballIdx] * 0.7);
        int hitPosition = bulletX[bulletIdx] - ballX[ballIdx];

        if (hitPosition >= centerZoneStart && hitPosition <= centerZoneEnd) {
            ballActive[ballIdx] = false;

            explosionActive[ballIdx] = true;
            explosionX[ballIdx] = ballX[ballIdx];
            explosionY[ballIdx] = ballY[ballIdx];
            explosionDuration[ballIdx] = 10;
            explosionSize[ballIdx] = ballWidth[ballIdx]; 
        }
        else if (hitPosition < centerZoneStart) {
            ballMoveSpeedX[ballIdx] = 2;
            ballMoveSpeedY[ballIdx] = -5;
            ballMovingUp[ballIdx] = true;
        }
        else if (hitPosition > centerZoneEnd) {
            ballMoveSpeedX[ballIdx] = -2;
            ballMoveSpeedY[ballIdx] = -5;
            ballMovingUp[ballIdx] = true;
        }

        bulletActive[bulletIdx] = false;
    }
}

void MoveBall(int i) {
    if (!ballActive[i]) return;

    if (ballMovingUp[i]) {
        ballX[i] += ballMoveSpeedX[i];
        ballY[i] += ballMoveSpeedY[i];

        if (ballY[i] <= 0 || ballX[i] <= 0 || ballX[i] + ballWidth[i] >= 400) {
            ballActive[i] = false;
        }
    }
    else {
        ballY[i] += 2;
        if (ballY[i] > 400) {
            ballActive[i] = false;
        }
    }
}

void DrawExplosions() {
    for (int i = 0; i < MAX_EXPLOSIONS; i++) {
        if (explosionActive[i]) {
            int size = explosionSize[i];
            FillRect(m, explosionX[i] - size / 2, explosionY[i] - size / 2, size, size, 0xFF0000);
            explosionDuration[i]--;
            if (explosionDuration[i] <= 0) {
                explosionActive[i] = false;
            }
        }
    }
}

DWORD WINAPI SlidingBox(LPVOID lpParam) {
    InitializeGameObjects();
    srand((unsigned int)time(NULL));
    hFireMutex = CreateMutex(NULL, FALSE, NULL);

    while (thread_continue) {
        FillRect(m, 0, 0, 400, 400, 0x000000);

        FillRect(m, boxlocation, boxY, 20, 6, 0xff0000);

        for (int i = 0; i < MAX_BULLETS; i++) {
            if (bulletActive[i]) {
                bulletY[i] -= 5;
                FillRect(m, bulletX[i], bulletY[i], 4, 10, 0xffffff);
                if (bulletY[i] < 0) bulletActive[i] = false;
            }
        }

        for (int i = 0; i < MAX_BALLS; i++) {
            if (ballActive[i]) {
                FillRect(m, ballX[i], ballY[i], ballWidth[i], ballHeight[i], 0x00ff00);
                MoveBall(i);

                for (int j = 0; j < MAX_BULLETS; j++) {
                    if (bulletActive[j]) {
                        HandleCollision(j, i);
                    }
                }
            }
        }

        DrawExplosions();

        DisplayImage(FRM, m);

        if (rand() % 50 == 0) {
            CreateBall();
        }

        Sleep(30);
    }

    CloseHandle(hFireMutex);
    return 0;
}

void butonfonk() {
    if (!thread_continue) {
        thread_continue = true;
        std::thread gameThread(SlidingBox, nullptr);
        gameThread.detach();
        SetFocus(ICG_GetMainWindow());
    }
    else {
        thread_continue = false;
    }
}

void WhenKeyPressed(int k) {
    keypressed = k;

    if (k == 32) {
        FireBullet();
    }
    if (k == 37) {
        boxlocation -= 10;
        if (boxlocation < 0) boxlocation = 0;
    }
    if (k == 39) {
        boxlocation += 10;
        if (boxlocation > 380) boxlocation = 380;
    }
}

void ICGUI_Create() {
    ICG_MWSize(600, 400);
    ICG_MWTitle("SLIDING BOX GAME WITH MUTEX");
    FRM = ICG_FramePanel(5, 40, 400, 400);
}

void ICGUI_main() {
    ICGUI_Create();
    ICG_Button(5, 5, 120, 25, "START / STOP", butonfonk);
    ICG_SetOnKeyPressed(WhenKeyPressed);
    CreateImage(m, 400, 400, ICB_UINT);
}
