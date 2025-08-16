#include "raylib.h"
#include <limits.h>		/* for CHAR_BIT */

#define BITMASK(b) (1 << ((b) % CHAR_BIT))
#define BITSLOT(b) ((b) / CHAR_BIT)
#define BITSET(a, b) ((a)[BITSLOT(b)] |= BITMASK(b))
#define BITCLEAR(a, b) ((a)[BITSLOT(b)] &= ~BITMASK(b))
#define BITTEST(a, b) ((a)[BITSLOT(b)] & BITMASK(b))
#define BITNSLOTS(nb) ((nb + CHAR_BIT - 1) / CHAR_BIT)

#define MAX_ENTITIES 100
#define MAX_COMPONENTS 10
#define MAX_ARCHETYPES 100
#define MAX_SYSTEMS 100

Vector2 CreateVector2(float x, float y) {
    Vector2 vec;
    vec.x = x;
    vec.y = y;
    return vec;
}

unsigned int entities[MAX_ENTITIES] = {0};

typedef struct {
    void* _data;
    size_t _count;
    size_t _capacity;
    size_t _elem_size;
} ComponentArray;

typedef struct{
    int x;
} TestComponent;

typedef struct {
    unsigned int *entities;
    ComponentArray *components;
    size_t entityCount;
    size_t capacity;
    size_t componentArrayCount;
    unsigned char Signature[BITNSLOTS(MAX_COMPONENTS)];
} Archetype;

typedef void (*SystemFunc)(const char *signature);

SystemFunc systems[MAX_SYSTEMS];
size_t systemCount = 0;

void add_system(SystemFunc sys) {
    if (systemCount < MAX_SYSTEMS) {
        systems[systemCount++] = sys;
    }
}

ComponentArray init_component_array(size_t elem_size, size_t capacity) {
    ComponentArray array;
    array._elem_size = elem_size;
    array._capacity = capacity;
    array._data = malloc(elem_size * capacity);
    array._count = 0;
    return array;
}

void* component_array_add(ComponentArray *arr, void *element)
{
    if (arr->_count >= arr->_capacity){
        arr->_capacity *= 2;
        arr->_data = realloc(arr->_data,arr->_elem_size*arr->_capacity);
    }

    char* dest = (char*)arr->_data + (arr->_count * arr->_elem_size);
    memcpy(dest, element, arr->_elem_size);
    arr->_count++;
    return dest;
}

void component_array_remove(ComponentArray *arr, int index)
{
    if (index < 0 || index >= arr->_count) return;
    char* dest = (char*)arr->_data + (index * arr->_elem_size);
    char* end = (char*)arr->_data + ((arr->_count-1) * arr->_elem_size);
    memcpy(dest, end, arr->_elem_size);
    arr->_count--;
}

void* get_component(ComponentArray *arr, size_t index) {
    return (char*)arr->_data + (index * arr->_elem_size);
}

Archetype init_archetype()
{
    Archetype arch;
    arch.entityCount = 0;
    arch.componentArrayCount = 0;
    arch.capacity = 20;
    arch.components = malloc(arch.capacity * sizeof(ComponentArray));
    arch.entities = malloc(arch.capacity * sizeof(int));
    return arch;
}

void add_components_to_archetype(Archetype *arch, ComponentArray arr)
{
    if (arch->componentArrayCount >= arch->capacity)
    {
        arch->capacity *= 2;
        arch->components = realloc(arch->components, sizeof(ComponentArray) * arch->capacity);
    }
    arch->components[arch->componentArrayCount] = arr;
    arch->componentArrayCount++;
}


void add_entitiy_to_archetype(Archetype *arch, unsigned int entity)
{
    //TODO: make this append a new component to each component array
        if (arch->entityCount >= arch->capacity)
    {
        arch->capacity *= 2;
        arch->entities = realloc(arch->entities, sizeof(int) * arch->capacity);
    }
    arch->entities[arch->entityCount] = entity;
    arch->entityCount++;
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

    SetTargetFPS(60);
    
    ComponentArray test = init_component_array(sizeof(TestComponent), 10);
    ComponentArray test2 = init_component_array(sizeof(TestComponent), 10);
    TestComponent tc;
    tc.x = 5;

    component_array_add(&test, &tc);
    tc.x = 7;
    component_array_add(&test2, &tc);

    Archetype archTest = init_archetype();

    add_components_to_archetype(&archTest, test);
    add_components_to_archetype(&archTest, test2);
    TestComponent *test3 = (TestComponent*)get_component(&archTest.components[0],0);
    TestComponent *test4 = (TestComponent*)get_component(&archTest.components[1],0);

    printf("%d\n", test3->x);
    printf("%d\n", test4->x);

    tc.x = 100;
    component_array_add(&test, &tc);
    test3 = (TestComponent*)get_component(&archTest.components[0],1);
    printf("%d\n", test3->x);

    
    // Set our game to run at 60 frames-per-second
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