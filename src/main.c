#include "raylib.h"

Vector2 CreateVector2(float x, float y) {
    Vector2 vec;
    vec.x = x;
    vec.y = y;
    return vec;
}

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 640;
    const int screenHeight = 480;

    const int monitorWidth = GetMonitorWidth(GetCurrentMonitor());
    const int monitorHeight = GetMonitorHeight(GetCurrentMonitor());

    const char text[] = "This is a Test Window";
    const int fontSize = 20;

    InitWindow(640, 480, "raylib");

    Font font = LoadFont("build/external/raylib-master/examples/text/resources/dejavu.fnt");

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        if (IsKeyPressed(KEY_F)){
            ToggleBorderlessWindowed();
        }
        float textWidth = MeasureTextEx(font, text, fontSize, 2).x;
        float textHeight = MeasureTextEx(font, text, fontSize, 2).y;
        float x = (GetScreenWidth()- textWidth)/2.0f;
        float y = (GetScreenHeight() - textHeight)/2.0f;
    
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            DrawTextEx(font, text, CreateVector2(x,y), fontSize, 2, BLACK);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}