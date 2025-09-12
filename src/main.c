#include "raylib.h"
#include <limits.h> /* for CHAR_BIT */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>	

#define BITMASK(b) (1 << ((b) % CHAR_BIT))
#define BITSLOT(b) ((b) / CHAR_BIT)
#define BITSET(a, b) ((a)[BITSLOT(b)] |= BITMASK(b))
#define BITCLEAR(a, b) ((a)[BITSLOT(b)] &= ~BITMASK(b))
#define BITTEST(a, b) ((a)[BITSLOT(b)] & BITMASK(b))
#define BITNSLOTS(nb) ((nb + CHAR_BIT - 1) / CHAR_BIT)

#define MAX_ENTITIES 1000
#define MAX_COMPONENTS 10
#define MAX_ARCHETYPES 100
#define MAX_SYSTEMS 100

typedef enum {
    POSITION_COMPONENT,
    VELOCITY_COMPOENT,
    GUN_COMPONENT,
    SHIP_COMPONENT
} COMPONENTS_ENUM;

Vector2 CreateVector2(float x, float y) {
    Vector2 vec;
    vec.x = x;
    vec.y = y;
    return vec;
}

unsigned int entities[MAX_ENTITIES] = {0};
size_t active_entities = 0;

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
    size_t componetArrayCapacity;
  
    unsigned int signature;
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
        size_t newCapacity = (arr->_capacity == 0) ? 1 : arr->_capacity * 2;
        arr->_capacity = newCapacity;
        arr->_data = realloc(arr->_data,arr->_elem_size*arr->_capacity);
    }

    char* dest = (char*)arr->_data + (arr->_count * arr->_elem_size);
    memcpy(dest, element, arr->_elem_size);
    arr->_count++;
    return dest;
}

void remove_from_component_array(ComponentArray *arr, int index)
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
    arch.capacity = 8;
    arch.componetArrayCapacity = 8;
    arch.components = malloc(arch.capacity * sizeof(ComponentArray));
    arch.entities = malloc(arch.capacity * sizeof(int));
    return arch;
}

void add_component_array_to_archetype(Archetype *arch, ComponentArray arr)
{
    if (arch->componentArrayCount >= arch->componetArrayCapacity)
    {
        size_t newCapacity = (arch->componetArrayCapacity == 0) ? 1 : arch->componetArrayCapacity * 2;
        arch->componetArrayCapacity = newCapacity;
        arch->components = realloc(arch->components, sizeof(ComponentArray) * arch->componetArrayCapacity);
    }
    arch->components[arch->componentArrayCount] = arr;
    arch->componentArrayCount++;
}


void add_entity_to_archetype(Archetype *arch, unsigned int entity)
{
        if (arch->entityCount >= arch->capacity)
    {
        size_t newCapacity = (arch->capacity == 0) ? 1 : arch->capacity * 2;
        arch->capacity = newCapacity;
        arch->entities = realloc(arch->entities, sizeof(int) * arch->capacity);
    }
    arch->entities[arch->entityCount++] = entity;

    for (int i = 0; i < arch->componentArrayCount; i++)
    {
        ComponentArray *arr = &arch->components[i];
        size_t elemSize = arr->_elem_size;
        if (elemSize == 0) continue;

        void *zeroed = calloc(1, elemSize);
        component_array_add(arr, zeroed);
        free(zeroed);
    }
}

bool remove_entitiy_from_archetype(Archetype *arr, unsigned int entity)
{
    unsigned int target = -1;
    for (int i = 0; i < arr->entityCount; i++)
    {
        if (arr->entities[i] == entity)
        {
            target = arr->entities[i];
            arr->entities[i] = arr->entities[arr->entityCount - 1];
            for (int j = 0; j < arr->componetArrayCapacity; j++)
            {
                /* 
                  has the effect of removing components at index i, 
                  while also moving components from the end of each 
                  component array and placing them at index i
                */
                remove_from_component_array(&arr->components[j], i); 
            }
            arr->entityCount--;
            return true;
        }
    }
    return false;
}

bool archetype_update_signature(Archetype *arr, unsigned int signature)
{
    arr->signature = signature;
}
typedef struct {
    float x;
    float y;
} PositionComponent;

typedef struct{
    float x;
    float y;
} VelocityComponent;

typedef struct{
    float damage;
    float rof;
} GunComponent;

typedef struct{
    float hp;
    float top_speed;
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