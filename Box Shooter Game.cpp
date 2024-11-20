#include "icb_gui.h"
#include <cstdlib>
#include <ctime>
#define MAX_BULLETS 15
#define MAX_BALLS 3
#define MAX_EXPLOSIONS 5

int keypressed;
ICBYTES m;
 


void InitializeGameObjects(bool bulletActive[], int bulletX[], int bulletY[], bool ballActive[], int ballX[], int ballY[], int ballWidth[], int ballHeight[], bool ballMovingUp[], int ballMoveSpeedX[], int ballMoveSpeedY[], bool explosionActive[], int explosionX[], int explosionY[], int explosionDuration[]) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        bulletActive[i] = false;
    }
    for (int i = 0; i < MAX_BALLS; i++) {
        ballActive[i] = false;
        ballMovingUp[i] = false;
    }
    for (int i = 0; i < MAX_EXPLOSIONS; i++) {
        explosionActive[i] = false;
    }
}

void FireBullet(bool bulletActive[], int bulletX[], int bulletY[], int boxlocation, int boxY) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bulletActive[i]) {
            bulletX[i] = boxlocation + 10;
            bulletY[i] = boxY - 10;
            bulletActive[i] = true;
            break;
        }
    }
}

void CreateBall(bool ballActive[], int ballX[], int ballY[], int ballWidth[], int ballHeight[], bool ballMovingUp[]) {
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

void CreateExplosion(bool explosionActive[], int explosionX[], int explosionY[], int explosionDuration[], int x, int y) {
    for (int i = 0; i < MAX_EXPLOSIONS; i++) {
        if (!explosionActive[i]) {
            explosionX[i] = x;
            explosionY[i] = y;
            explosionDuration[i] = 10;
            explosionActive[i] = true;
            break;
        }
    }
}

void HandleCollision(bool bulletActive[], int bulletX[], int bulletY[], bool ballActive[], int ballX[], int ballY[], int ballWidth[], int ballHeight[], bool ballMovingUp[], int ballMoveSpeedX[], int ballMoveSpeedY[], bool explosionActive[], int explosionX[], int explosionY[], int explosionDuration[], int bulletIdx, int ballIdx) {
    int leftCollisionZone = static_cast<int>(ballWidth[ballIdx] * 0.3);
    int rightCollisionZone = static_cast<int>(ballWidth[ballIdx] * 0.3);

    int hitPosition = bulletX[bulletIdx] - ballX[ballIdx];

    if (bulletX[bulletIdx] >= ballX[ballIdx] && bulletX[bulletIdx] <= ballX[ballIdx] + ballWidth[ballIdx] &&
        bulletY[bulletIdx] >= ballY[bulletIdx] && bulletY[bulletIdx] <= ballY[ballIdx] + ballHeight[ballIdx]) {

        if (hitPosition > leftCollisionZone && hitPosition < (ballWidth[ballIdx] - rightCollisionZone)) {
            ballActive[ballIdx] = false;
            CreateExplosion(explosionActive, explosionX, explosionY, explosionDuration, ballX[ballIdx] + ballWidth[ballIdx] / 2, ballY[ballIdx] + ballHeight[ballIdx] / 2);
        }
        else if (hitPosition <= leftCollisionZone) {
            ballMoveSpeedX[ballIdx] = 2;
            ballMoveSpeedY[ballIdx] = -5;
            ballMovingUp[ballIdx] = true;
        }
        else if (hitPosition >= ballWidth[ballIdx] - rightCollisionZone) {
            ballMoveSpeedX[ballIdx] = -2;
            ballMoveSpeedY[ballIdx] = -5;
            ballMovingUp[ballIdx] = true;
        }
        bulletActive[bulletIdx] = false;
    }
}

void MoveBall(bool& active, int& x, int& y, int width, bool& movingUp, int& moveSpeedX, int& moveSpeedY) {
    if (movingUp) {
        x += moveSpeedX;
        y += moveSpeedY;
        if (y <= 0 || x <= 0 || (x + width) >= 400) {
            active = false;
            movingUp = false;
        }
    }
    else {
        y += 2;
    }
}

void DrawExplosions(bool explosionActive[], int explosionX[], int explosionY[], int explosionDuration[]) {
    for (int i = 0; i < MAX_EXPLOSIONS; i++) {
        if (explosionActive[i]) {
            FillRect(m, explosionX[i] - 10, explosionY[i] - 10, 20, 20, 0xFF0000);
            explosionDuration[i]--;
            if (explosionDuration[i] <= 0) {
                explosionActive[i] = false;
            }
        }
    }
}
bool thread_continue = false;
VOID* SlidingBox(PVOID lpParam) {
    int frame, boxlocation = 10, boxY = 380;
    bool bulletActive[MAX_BULLETS], ballActive[MAX_BALLS], ballMovingUp[MAX_BALLS], explosionActive[MAX_EXPLOSIONS];
    int bulletX[MAX_BULLETS], bulletY[MAX_BULLETS], ballX[MAX_BALLS], ballY[MAX_BALLS], ballWidth[MAX_BALLS], ballHeight[MAX_BALLS];
    int ballMoveSpeedX[MAX_BALLS], ballMoveSpeedY[MAX_BALLS], explosionX[MAX_EXPLOSIONS], explosionY[MAX_EXPLOSIONS], explosionDuration[MAX_EXPLOSIONS];

    InitializeGameObjects(bulletActive, bulletX, bulletY, ballActive, ballX, ballY, ballWidth, ballHeight, ballMovingUp, ballMoveSpeedX, ballMoveSpeedY, explosionActive, explosionX, explosionY, explosionDuration);

    while (thread_continue) {
        FillRect(m, 0, 0, 400, 400, 0x000000);

        if (keypressed == 37) boxlocation -= 4;
        if (keypressed == 39) boxlocation += 4;

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
                MoveBall(ballActive[i], ballX[i], ballY[i], ballWidth[i], ballMovingUp[i], ballMoveSpeedX[i], ballMoveSpeedY[i]);

                for (int j = 0; j < MAX_BULLETS; j++) {
                    if (bulletActive[j]) {
                        HandleCollision(bulletActive, bulletX, bulletY, ballActive, ballX, ballY, ballWidth, ballHeight, ballMovingUp, ballMoveSpeedX, ballMoveSpeedY, explosionActive, explosionX, explosionY, explosionDuration, j, i);
                    }
                }
            }
        }

        DrawExplosions(explosionActive, explosionX, explosionY, explosionDuration);
        DisplayImage(frame, m);

        if (rand() % 50 == 0) {
            CreateBall(ballActive, ballX, ballY, ballWidth, ballHeight, ballMovingUp);
        }

        Sleep(30);
    }
    return NULL;
}

void butonfonk() {
    DWORD dw;
    m = 0;
    if (!thread_continue) {
        thread_continue = true;
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)SlidingBox, NULL, 0, &dw);
        SetFocus(ICG_GetMainWindow());
    }
    else {
        thread_continue = false;
    }
}

void WhenKeyPressed(int k) {
    keypressed = k;
    if (k == 32) {
        bool bulletActive[MAX_BULLETS];
        int bulletX[MAX_BULLETS], bulletY[MAX_BULLETS];
        FireBullet(bulletActive, bulletX, bulletY, 10, 380);
    }
}

void ICGUI_main() {
    ICG_Button(5, 5, 120, 25, "START / STOP", butonfonk);
    int frame = ICG_FrameMedium(5, 40, 400, 400);
    ICG_SetOnKeyPressed(WhenKeyPressed);
    CreateImage(m, 400, 400, ICB_UINT);
}
