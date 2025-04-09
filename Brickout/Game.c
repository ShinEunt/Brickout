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

    // 타이틀 화면 텍스트에 사용할 커스텀 폰트 로드 (프로젝트 폴더의 fonts/NotoSansKR-Black.otf)
    Font customFont = LoadFontEx("fonts/NotoSansKR-Black.ttf", 64, NULL, 0);

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
        if (currentScreen == SCREEN_TITLE) {
            // 난이도 선택: 숫자키 1, 2, 3
            if (IsKeyPressed(KEY_ONE)) currentDifficulty = DIFF_EASY;
            if (IsKeyPressed(KEY_TWO)) currentDifficulty = DIFF_NORMAL;
            if (IsKeyPressed(KEY_THREE)) currentDifficulty = DIFF_HARD;

            // [ENTER] 키를 누르면 난이도에 맞게 블록 초기화 후 게임플레이로 전환
            if (IsKeyPressed(KEY_ENTER)) {
                switch (currentDifficulty) {
                case DIFF_EASY:   blockCount = 36; break;
                case DIFF_NORMAL: blockCount = 60; break;
                case DIFF_HARD:   blockCount = 84; break;
                }
                // 블록 초기화 (한 줄에 12개 기준, y 좌표를 30으로 하여 위쪽으로 배치)
                for (int i = 0; i < blockCount; i++) {
                    blocks[i] = (Rectangle){ 20 + (i % 12) * 63, 30 + (i / 12) * 30, 60, 20 };
                    active[i] = true;
                }
                currentScreen = SCREEN_GAMEPLAY;
            }
        }
        else if (currentScreen == SCREEN_GAMEPLAY) {
            // 게임플레이 상태: 패들 이동 및 충돌 처리
            if (IsKeyDown(KEY_LEFT)) paddle.x -= 8;
            if (IsKeyDown(KEY_RIGHT)) paddle.x += 8;

            if (paddle.x < 0) paddle.x = 0;
            if (paddle.x > 800 - paddle.width) paddle.x = 800 - paddle.width;

            ball.x += speed.x;
            ball.y += speed.y;

            if (ball.x < radius || ball.x > 800 - radius) speed.x *= -1;
            if (ball.y < radius) speed.y *= -1;

            if (ball.y > 600) ball = (Vector2){ 400, 300 };

            if (CheckCollisionCircleRec(ball, radius, paddle))
                speed.y *= -1;

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
            // 타이틀 화면: 텍스트들을 커스텀 폰트로 중앙 정렬하여 출력
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

            // 폰트 사이즈 및 간격 설정
            float titleFontSize = 40;
            float instrFontSize = 20;
            float diffFontSize = 20;
            float optionFontSize = 20;
            float spacing = 2.0f;

            Vector2 titleSize = MeasureTextEx(customFont, titleText, titleFontSize, spacing);
            Vector2 diffSize = MeasureTextEx(customFont, fullDiffText, diffFontSize, spacing);
            Vector2 instrSize = MeasureTextEx(customFont, instrText, instrFontSize, spacing);
            Vector2 optionSize = MeasureTextEx(customFont, optionText, optionFontSize, spacing);

            DrawTextEx(customFont, titleText, (Vector2) { (800 - titleSize.x) / 2, 150 }, titleFontSize, spacing, WHITE);
            DrawTextEx(customFont, fullDiffText, (Vector2) { (800 - diffSize.x) / 2, 250 }, diffFontSize, spacing, WHITE);
            DrawTextEx(customFont, instrText, (Vector2) { (800 - instrSize.x) / 2, 350 }, instrFontSize, spacing, GRAY);
            DrawTextEx(customFont, optionText, (Vector2) { (800 - optionSize.x) / 2, 400 }, optionFontSize, spacing, GRAY);
        }
        else if (currentScreen == SCREEN_GAMEPLAY) {
            DrawRectangleRec(paddle, WHITE);
            DrawCircleV(ball, radius, WHITE);
            Color rowColors[7] = { RED, ORANGE, YELLOW, GREEN, BLUE, DARKBLUE, PURPLE };
            for (int i = 0; i < blockCount; i++) {
                if (active[i])
                    DrawRectangleRec(blocks[i], rowColors[i / 12]);
            }
        }

        EndDrawing();
    }

    // 사용한 폰트 메모리 해제
    UnloadFont(customFont);
    CloseWindow();
    return 0;
}
