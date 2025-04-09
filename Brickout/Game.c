#define _CRT_SECURE_NO_WARNINGS
#include "raylib.h"
#include <stdio.h>
#include <string.h>

#define MAX_BLOCKS 84   // Hard 난이도일 경우 최대 블록 수 (7행 x 12열)

typedef enum GameScreen {
    SCREEN_TITLE,
    SCREEN_GAMEPLAY
} GameScreen;

typedef enum Difficulty {
    DIFF_EASY,
    DIFF_NORMAL,
    DIFF_HARD
} Difficulty;

int main(void) {
    InitWindow(800, 600, "Brick Out");
    SetTargetFPS(60);

    GameScreen currentScreen = SCREEN_TITLE;      // 초기 상태: 타이틀 화면
    Difficulty currentDifficulty = DIFF_EASY;       // 기본 난이도: Easy

    // 게임플레이 관련 변수들
    Rectangle paddle = { 350, 550, 100, 20 };
    Vector2 ball = { 400, 300 };
    Vector2 speed = { 5, -5 };
    float radius = 10;

    // 최대 난이도에 맞춘 블록 배열과 활성 상태 배열
    Rectangle blocks[MAX_BLOCKS];
    bool active[MAX_BLOCKS] = { 0 };
    int blockCount = 0; // 난이도 선택에 따라 설정됨

    while (!WindowShouldClose()) {

        // 타이틀 화면 입력 처리
        if (currentScreen == SCREEN_TITLE) {
            if (IsKeyPressed(KEY_ONE)) currentDifficulty = DIFF_EASY;
            if (IsKeyPressed(KEY_TWO)) currentDifficulty = DIFF_NORMAL;
            if (IsKeyPressed(KEY_THREE)) currentDifficulty = DIFF_HARD;

            if (IsKeyPressed(KEY_ENTER)) {
                // 난이도에 따라 블록 수 결정: Easy = 36, Normal = 60, Hard = 84
                switch (currentDifficulty) {
                case DIFF_EASY:   blockCount = 36; break;
                case DIFF_NORMAL: blockCount = 60; break;
                case DIFF_HARD:   blockCount = 84; break;
                }
                // 블록 초기화 (한 줄에 12개 기준)
                for (int i = 0; i < blockCount; i++) {
                    blocks[i] = (Rectangle){ 20 + (i % 12) * 63, 20 + (i / 12) * 30, 60, 20 };
                    active[i] = true;
                }
                currentScreen = SCREEN_GAMEPLAY;
            }
        }
        // 게임플레이 상태 입력 처리
        else if (currentScreen == SCREEN_GAMEPLAY) {
            if (IsKeyDown(KEY_LEFT)) paddle.x -= 8;
            if (IsKeyDown(KEY_RIGHT)) paddle.x += 8;

            // 패들 경계 제한
            if (paddle.x < 0) paddle.x = 0;
            if (paddle.x > 800 - paddle.width) paddle.x = 800 - paddle.width;

            // 공 이동
            ball.x += speed.x;
            ball.y += speed.y;

            // 벽 충돌
            if (ball.x < radius || ball.x > 800 - radius) speed.x *= -1;
            if (ball.y < radius) speed.y *= -1;

            // 바닥에 닿으면 공 리셋
            if (ball.y > 600) ball = (Vector2){ 400, 300 };

            // 패들 충돌
            if (CheckCollisionCircleRec(ball, radius, paddle))
                speed.y *= -1;

            // 블록 충돌 처리
            for (int i = 0; i < blockCount; i++) {
                if (active[i] && CheckCollisionCircleRec(ball, radius, blocks[i])) {
                    active[i] = false;
                    speed.y *= -1;
                    break;
                }
            }
        }

        BeginDrawing();
        ClearBackground(BLACK);

        if (currentScreen == SCREEN_TITLE) {
            // 타이틀 화면 텍스트 (중앙 정렬)
            const char* titleText = "Brick Out";
            const char* instrText = "Press Enter to Start";
            const char* diffText = "Difficulty: ";
            char diffOption[10];
            switch (currentDifficulty) {
            case DIFF_EASY:   strcpy(diffOption, "Easy"); break;
            case DIFF_NORMAL: strcpy(diffOption, "Normal"); break;
            case DIFF_HARD:   strcpy(diffOption, "Hard"); break;
            }
            char fullDiffText[50];
            sprintf_s(fullDiffText, sizeof(fullDiffText), "%s%s", diffText, diffOption);
            const char* optionText = "Press 1 for Easy, 2 for Normal, 3 for Hard";

            int titleFontSize = 40, instrFontSize = 20, diffFontSize = 20, optionFontSize = 20;
            int titleWidth = MeasureText(titleText, titleFontSize);
            int diffWidth = MeasureText(fullDiffText, diffFontSize);
            int instrWidth = MeasureText(instrText, instrFontSize);
            int optionWidth = MeasureText(optionText, optionFontSize);

            DrawText(titleText, (800 - titleWidth) / 2, 150, titleFontSize, WHITE);
            DrawText(fullDiffText, (800 - diffWidth) / 2, 250, diffFontSize, WHITE);
            DrawText(instrText, (800 - instrWidth) / 2, 350, instrFontSize, GRAY);
            DrawText(optionText, (800 - optionWidth) / 2, 400, optionFontSize, GRAY);
        }
        else if (currentScreen == SCREEN_GAMEPLAY) {
            DrawRectangleRec(paddle, WHITE);
            DrawCircleV(ball, radius, WHITE);
            // 각 행마다 지정된 색상 배열
            Color rowColors[7] = { RED, ORANGE, YELLOW, GREEN, BLUE, DARKBLUE, PURPLE };
            for (int i = 0; i < blockCount; i++) {
                if (active[i])
                    DrawRectangleRec(blocks[i], rowColors[i / 12]);
            }
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
