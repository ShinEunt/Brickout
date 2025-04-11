#define _CRT_SECURE_NO_WARNINGS
#include "raylib.h"
#include <stdio.h>
#include <string.h>

#define MAX_BLOCKS 84

// 게임의 주요 화면 상태

/*타이틀 화면(TITLE_SCREEN) :
    난이도(EASY, NORMAL, HARD) 선택 및 게임 시작을 위한 안내 메시지를 표시.

    게임 플레이 화면(GAME_SCREEN) :
    플레이어는 패들을 좌우로 이동하고, 공은 벽과 패들 그리고 블록과 충돌하며 움직임.
    각 블록과 충돌하면 1000점이 추가되고, T 키를 사용해 테스트 목적으로 모든 블록을 제거 가능.
    또한, 공이 화면 아래로 떨어질 때마다 목숨이 차감되고, 모든 블록을 깨면 승리(VICTORY_SCREEN), 목숨이 0이면 게임 오버(GAMEOVER_SCREEN) 상태로 전환.

    승리 화면(VICTORY_SCREEN) :
    모든 블록이 깨지면 "Congratulations!" 메시지가 중앙에 표시되고, ENTER 키를 누르면 타이틀 화면으로 복귀.

    게임 오버 화면(GAMEOVER_SCREEN) :
    목숨이 0이 되면 "Game Over" 메시지를 중앙에 표시하고, ENTER 키를 누르면 타이틀 화면으로 복귀.
*/

typedef enum { TITLE_SCREEN, GAME_SCREEN, VICTORY_SCREEN, GAMEOVER_SCREEN } GameScreen; //게임의 현재 상태를 나타내며, 타이틀, 게임 플레이, 승리, 게임 오버 상태를 구분
typedef enum { EASY, NORMAL, HARD } Difficulty; //난이도를 나타내며, 게임 시작 시 선택

// 글로벌 변수들
static GameScreen currentScreen; //게임의 현재 화면 상태와 선택된 난이도를 저장
static Difficulty currentDifficulty; 
static int score;//현재 플레이어의 점수와 남은 목숨
static int lives;

static Rectangle paddle; //패들(플레이어가 조작하는 막대), 공의 위치, 공의 이동 속도, 그리고 공의 반지름을 정의
static Vector2 ball;
static Vector2 ballSpeed;
static float ballRadius;

static Rectangle blocks[MAX_BLOCKS]; //전체 블록 배열과 각 블록이 활성(아직 깨지지 않음)되어 있는지를 저장하는 배열.
static bool blockActive[MAX_BLOCKS];
static int blockCount; //현재 게임에서 생성된 블록의 개수를 저장.
static float collisionCooldown; //블록과의 충돌 쿨다운을 저장하는 변수

static Font customFont; //게임에서 사용하는 폰트

// 함수 선언
void InitGame(void);
void StartGame(void);
void UpdateTitleScreen(void);
void UpdateGameScreen(void);
void UpdateResultScreen(void);
void UpdateGame(void);
Color CalculateBgColor(void);
void DrawTitleScreen(void);
void DrawGameScreen(void);
void DrawResultScreen(const char* text);
void DrawGame(void);

// 초기 변수 설정
/*
주요 작업:

화면 상태를 타이틀로 설정

난이도(EASY), 점수(0), 목숨(3) 초기화

패들, 공, 공의 이동 속도, 공 반지름 초기화

블록 개수와 충돌 쿨다운 변수를 0으로 설정

블록 활성 배열을 모두 false로 초기화
*/
void InitGame(void)
{
    currentScreen = TITLE_SCREEN;
    currentDifficulty = EASY;
    score = 0;
    lives = 3;
    paddle = (Rectangle){ 350, 550, 100, 20 };
    ball = (Vector2){ 400, 300 };
    ballSpeed = (Vector2){ 5, -5 };
    ballRadius = 10.0f;
    blockCount = 0;
    collisionCooldown = 0.0f;
    for (int i = 0; i < MAX_BLOCKS; i++) {
        blockActive[i] = false;
    }
}

// 타이틀 화면에서 엔터키를 누르면 게임 시작 (난이도에 따라 블록 개수 결정)
/*
주요 작업:

선택한 난이도에 따라 블록 개수 결정

for 루프를 통해 각 블록을 초기화

X 좌표: 20 + (i % 12) * 63 → 블록을 한 행당 12개씩 오른쪽으로 배치

Y 좌표: 30 + (i / 12) * 30 → 각 행마다 30픽셀 간격으로 배치

점수, 목숨, 패들 및 공의 초기 위치와 속도를 재설정

화면 상태를 GAME_SCREEN으로 전환
*/
void StartGame(void)
{
    if (currentDifficulty == EASY)
        blockCount = 36;
    else if (currentDifficulty == NORMAL)
        blockCount = 60;
    else
        blockCount = 84;

    // 블록 초기화 : 한 행당 12개, x = 20 + (i % 12) * 63, y = 30 + (i / 12) * 30  
    for (int i = 0; i < blockCount; i++) {
        blocks[i] = (Rectangle){ 20 + (i % 12) * 63, 30 + (i / 12) * 30, 60, 20 };
        blockActive[i] = true;
    }

    score = 0;
    lives = 3;
    paddle.x = 350;
    ball = (Vector2){ 400, 300 };
    ballSpeed = (Vector2){ 5, -5 };

    currentScreen = GAME_SCREEN;
}

// 타이틀 화면 업데이트: 난이도 선택 및 ENTER키 입력 처리
/*
주요 작업:

숫자키 1, 2, 3 입력을 통해 난이도를 EASY, NORMAL, HARD로 설정

ENTER키를 누르면 StartGame()을 호출하여 게임을 시작
*/
void UpdateTitleScreen(void)
{
    if (IsKeyPressed(KEY_ONE)) currentDifficulty = EASY;
    if (IsKeyPressed(KEY_TWO)) currentDifficulty = NORMAL;
    if (IsKeyPressed(KEY_THREE)) currentDifficulty = HARD;
    if (IsKeyPressed(KEY_ENTER))
        StartGame();
}

// 게임 화면 업데이트
/*
주요 작업:

패들 이동:

왼쪽, 오른쪽 키 입력에 따라 패들이 이동
패들이 화면 경계를 벗어나지 않도록 제한

공 이동 및 벽 충돌:

공의 위치 업데이트
공이 좌우 또는 위쪽 벽에 닿으면 속도를 반전

공이 바닥에 떨어지면:
목숨(lives) 차감
목숨이 0이면 GAMEOVER_SCREEN으로 전환
목숨이 남아 있으면 공과 공의 속도를 재설정

패들과 충돌:
공이 패들과 충돌하면 Y 속도를 반전하여 튕기게 처리

테스트 입력 (T키):
T키를 누르면 for 루프를 통해 모든 블록을 비활성화하여 승리 테스트를 할 수 있음

블록과의 충돌 검사:
충돌 쿨다운(collisionCooldown)을 업데이트
쿨다운이 0이면 각 블록에 대해 충돌 검사 후 충돌된 블록을 비활성화하고 점수를 1000점 추가하며 공의 Y속도를 반전

승리 조건 검사:
모든 블록이 비활성화되면 화면 상태를 VICTORY_SCREEN으로 전환
*/
void UpdateGameScreen(void)
{
    // 패들 이동 처리
    if (IsKeyDown(KEY_LEFT)) paddle.x -= 8;
    if (IsKeyDown(KEY_RIGHT)) paddle.x += 8;
    if (paddle.x < 0) paddle.x = 0;
    if (paddle.x > 800 - paddle.width) paddle.x = 800 - paddle.width;

    // 공 이동 및 벽 충돌 처리
    ball.x += ballSpeed.x;
    ball.y += ballSpeed.y;
    if (ball.x < ballRadius || ball.x > 800 - ballRadius) ballSpeed.x *= -1;
    if (ball.y < ballRadius) ballSpeed.y *= -1;

    // 공이 바닥에 떨어지면 목숨 차감 후 공/속도 재설정
    if (ball.y > 600)
    {
        lives--;
        if (lives <= 0)
            currentScreen = GAMEOVER_SCREEN;
        else {
            ball = (Vector2){ 400, 300 };
            ballSpeed = (Vector2){ 5, -5 };
        }
    }

    // 패들과 충돌 시 처리
    if (CheckCollisionCircleRec(ball, ballRadius, paddle))
        ballSpeed.y *= -1;

    // 테스트: T키를 누르면 모든 블록 파괴 (승리 화면 테스트용)
    if (IsKeyPressed(KEY_T)) {
        for (int i = 0; i < blockCount; i++)
            blockActive[i] = false;
    }

    // 충돌 쿨다운 업데이트
    collisionCooldown -= GetFrameTime();
    if (collisionCooldown < 0) collisionCooldown = 0;

    // 쿨다운이 끝나면 블록과의 충돌 검사 수행
    if (collisionCooldown == 0)
    {
        for (int i = 0; i < blockCount; i++)
        {
            if (blockActive[i] && CheckCollisionCircleRec(ball, ballRadius, blocks[i]))
            {
                blockActive[i] = false;
                ballSpeed.y *= -1;
                collisionCooldown = 0.1f;
                score += 1000;
                break;
            }
        }
    }

    // 모든 블록이 파괴되면 승리 처리
    bool allCleared = true;
    for (int i = 0; i < blockCount; i++)
    {
        if (blockActive[i])
        {
            allCleared = false;
            break;
        }
    }
    if (allCleared)
        currentScreen = VICTORY_SCREEN;
}

// 승리/게임오버 화면 업데이트: 엔터키를 누르면 타이틀로 복귀
/*
주요 작업:

ENTER키 입력 감지 후 화면 상태를 TITLE_SCREEN으로 변경
*/
void UpdateResultScreen(void)
{
    if (IsKeyPressed(KEY_ENTER))
        currentScreen = TITLE_SCREEN;
}

// 전체 업데이트 함수
/*
주요 작업:

switch 문을 사용하여 현재 화면 상태에 맞는 함수 (UpdateTitleScreen, UpdateGameScreen, UpdateResultScreen)를 호출
*/
void UpdateGame(void)
{
    switch (currentScreen)
    {
    case TITLE_SCREEN:   UpdateTitleScreen();   break;
    case GAME_SCREEN:    UpdateGameScreen();    break;
    case VICTORY_SCREEN:
    case GAMEOVER_SCREEN: UpdateResultScreen(); break;
    }
}

// 배경색 계산 함수
// 5000점 단위로 화면이 점점 밝아짐, 80000점 이상부터 고정
Color CalculateBgColor(void)
{
    if (currentScreen == GAME_SCREEN)
    {
        int brightness = 0;
        if (score <= 30000)
        {
            int steps = score / 5000;  // 5000점 단위
            brightness = steps * 14;
        }
        else if (score < 80000)
        {
            int steps = (score - 30000) / 5000;  // 30000점 초과부터 5000점 단위
            brightness = 84 + (int)(steps * 11.6f); // 약 11.6씩 증가
        }
        else
        {
            brightness = 200;
        }
        // brightness가 200을 넘지 않도록
        if (brightness > 200) brightness = 200;
        return (Color) { (unsigned char)brightness, (unsigned char)brightness, (unsigned char)brightness, 255 };
    }
    else
    {
        return BLACK;
    }
}

// 타이틀 화면 그리기
/*
주요 작업:

게임 제목("Brick Out"), 난이도("Difficulty: Easy/Normal/Hard"),
시작 안내("Press Enter to Start") 및 옵션 메시지("Press 1 for Easy, 2 for Normal, 3 for Hard")를 출력

각 텍스트의 크기를 MeasureTextEx()를 이용해 측정하고, 화면 중앙에 배치하도록 좌표를 계산
*/
void DrawTitleScreen(void)
{
    const char* title = "Brick Out";
    const char* startText = "Press Enter to Start";
    const char* diffText = "Difficulty: ";
    char diffOpt[10];
    if (currentDifficulty == EASY) strcpy(diffOpt, "Easy");
    else if (currentDifficulty == NORMAL) strcpy(diffOpt, "Normal");
    else strcpy(diffOpt, "Hard");

    char fullDiffText[50];
    sprintf(fullDiffText, "%s%s", diffText, diffOpt);

    const char* optionText = "Press 1 for Easy, 2 for Normal, 3 for Hard";

    float titleFontSize = 40;
    float otherFontSize = 20;
    float spacing = 2.0f;

    Vector2 titleDim = MeasureTextEx(customFont, title, titleFontSize, spacing);
    Vector2 diffDim = MeasureTextEx(customFont, fullDiffText, otherFontSize, spacing);
    Vector2 startDim = MeasureTextEx(customFont, startText, otherFontSize, spacing);
    Vector2 optionDim = MeasureTextEx(customFont, optionText, otherFontSize, spacing);

    DrawTextEx(customFont, title, (Vector2) { (800 - titleDim.x) / 2, 150 }, titleFontSize, spacing, WHITE);
    DrawTextEx(customFont, fullDiffText, (Vector2) { (800 - diffDim.x) / 2, 250 }, otherFontSize, spacing, WHITE);
    DrawTextEx(customFont, startText, (Vector2) { (800 - startDim.x) / 2, 350 }, otherFontSize, spacing, GRAY);
    DrawTextEx(customFont, optionText, (Vector2) { (800 - optionDim.x) / 2, 400 }, otherFontSize, spacing, GRAY);
}

// 게임 화면 그리기
/*
주요 작업:

패들과 공:

DrawRectangleRec()와 DrawCircleV() 함수를 사용하여 흰색(WHITE)으로 적용.

블록:

블록은 for 루프를 통해 한 행당 12개씩 배치되어 있으며, 각 행에 대해 다른 색상(RED, ORANGE, YELLOW, GREEN, BLUE, DARKBLUE, PURPLE)을 적용.

점수 표시:

오른쪽 상단에 점수를 커스텀 폰트로 출력 (좌표는 화면 오른쪽 여백 10픽셀)

목숨 표시:

오른쪽 하단에 남은 목숨 수를 커스텀 폰트로 출력
*/
void DrawGameScreen(void)
{
    DrawRectangleRec(paddle, WHITE);
    DrawCircleV(ball, ballRadius, WHITE);

    // 각 행별 블록 색상 (한 행당 12개, 최대 7행)
    Color blockColors[7] = { RED, ORANGE, YELLOW, GREEN, BLUE, DARKBLUE, PURPLE };
    for (int i = 0; i < blockCount; i++)
    {
        if (blockActive[i])
            DrawRectangleRec(blocks[i], blockColors[i / 12]);
    }

    float spacing = 2.0f;
    // 오른쪽 상단에 점수 표시
    char scoreText[50];
    sprintf(scoreText, "Score: %d", score);
    float scoreFontSize = 20;
    Vector2 scoreDim = MeasureTextEx(customFont, scoreText, scoreFontSize, spacing);
    DrawTextEx(customFont, scoreText, (Vector2) { 800 - scoreDim.x - 10, 10 }, scoreFontSize, spacing, WHITE);

    // 오른쪽 하단에 목숨 표시
    char livesText[50];
    sprintf(livesText, "Lives: %d", lives);
    float livesFontSize = 20;
    Vector2 livesDim = MeasureTextEx(customFont, livesText, livesFontSize, spacing);
    DrawTextEx(customFont, livesText, (Vector2) { 800 - livesDim.x - 10, 600 - livesDim.y - 10 }, livesFontSize, spacing, WHITE);
}

// 승리/게임오버 화면 그리기 (텍스트 중앙 출력)
void DrawResultScreen(const char* text)
{
    float fontSize = 40;
    float spacing = 2.0f;
    Vector2 textDim = MeasureTextEx(customFont, text, fontSize, spacing);
    DrawTextEx(customFont, text, (Vector2) { (800 - textDim.x) / 2, (600 - textDim.y) / 2 }, fontSize, spacing, WHITE);
}

// 전체 드로잉 함수
/*구조:
먼저 CalculateBgColor()를 호출하여 배경색을 결정하고,
현재 화면 상태에 따라 적절한 그리기 함수를 호출
BeginDrawing()과 EndDrawing() 사이에 switch 문을 사용하여 각 화면(타이틀, 게임, 승리, 게임오버)을 그림.
*/
void DrawGame(void)
{
    Color bgColor = CalculateBgColor();
    BeginDrawing();
    ClearBackground(bgColor);

    switch (currentScreen)
    {
    case TITLE_SCREEN:
        DrawTitleScreen();
        break;
    case GAME_SCREEN:
        DrawGameScreen();
        break;
    case VICTORY_SCREEN:
        DrawResultScreen("Congratulations!");
        break;
    case GAMEOVER_SCREEN:
        DrawResultScreen("Game Over");
        break;
    }

    EndDrawing();
}

int main(void)
{
    InitWindow(800, 600, "Brick Out");
    SetTargetFPS(60); //initWindow 함수와 SetTargetFPS 함수로 게임창의 크기와 프레임 속도를 결정

    // 폰트 파일 사용 (NotoSansKR-Black.ttf)
    customFont = LoadFontEx("fonts/NotoSansKR-Black.ttf", 64, NULL, 0);

    InitGame(); //InitGame 함수 호출로 초기 상태 설정

    while (!WindowShouldClose())
    {
        UpdateGame(); //UpdateGame 를 호출해 화면 상태에 따른 입력 및 업데이트 처리 DrawGame()를 호출해 화면을 그림
        DrawGame();
    }

    UnloadFont(customFont);
    CloseWindow();
    return 0;
}
