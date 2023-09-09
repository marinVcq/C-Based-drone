#ifndef VEHICLE_H
#define VEHICLE_H

#include <stdbool.h>
#include <SDL_image.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <SDL_timer.h>
#include <SDL_ttf.h>
#include <string.h>
#include<conio.h>



typedef struct {
    float x;
    float y;
} Waypoint;

typedef struct {
    Waypoint *waypoints;
    int numWaypoints;
    int currentWaypointIndex;
} WaypointPath;

// Structure to represent smoke
typedef struct {
    float x;
    float y;
    float speed;
    bool active;
} Smoke;

typedef struct {
    float x;
    float y;
    float velocityX;
    float velocityY;
    float rotation;
    int carriageWidth;
    int carriageHeight;
    int turretWidth;
    int turretHeight;
    char carriagePath[256]; 
    char turretPath[256]; 
    char carriageIRPath[256];
    char turretIRPath[256]; 
    SDL_Texture *carriageTexture;
    SDL_Texture *turretTexture;
    SDL_Texture *carriageTextureIR;
    SDL_Texture *turretTextureIR;
    bool isHighlighted;
    WaypointPath waypointPath;
    bool isDestroyed;
    bool isHit;
    Smoke explosionSmoke;
} Vehicle;




void initializeVehicle(SDL_Renderer *renderer, Vehicle *vehicle, float x, float y, const char *carriagePath, const char *turretPath, const char *carriageIRPath, const char *turretIRPath);
SDL_Texture* createVehicleTexture(SDL_Renderer *renderer, const char *filePath);
void loadVehicleTexture(SDL_Renderer *renderer, const char *path, SDL_Texture **texture, Vehicle *vehicle);
void updateVehicles(float deltaTime, Vehicle *vehicles, int numVehicles);
void initializeWaypointPath(WaypointPath *path);
void addWaypointToPath(WaypointPath *path, float x, float y);
bool isCursorOverVehicle(int cursorX, int cursorY, Vehicle vehicle, float cameraX, float cameraY, float zoomLevel);
void renderVehicle(SDL_Renderer *renderer, Vehicle *vehicle, float cameraX, float cameraY, float zoomLevel, SDL_Texture *carriageTexture, SDL_Texture *turretTexture, SDL_Texture *smokeSpriteSheet,SDL_Texture *greenRectangleTexture, Uint32 globalTimer, SDL_Texture *explosionTexture, int currentMapTexture);
void updateSmoke(Smoke* smoke);
void initializeSmoke(Smoke* smoke, float x, float y, float speed);
void renderSmoke(SDL_Renderer* renderer, SDL_Texture* smokeTexture, Smoke* smoke, float cameraX, float cameraY, float zoomLevel);
#endif