#ifndef INTERFACE_H
#define INTERFACE_H

#include <stdbool.h>
#include <SDL_image.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <SDL_timer.h>
#include <SDL_ttf.h>
#include <string.h>
#include<conio.h>

#include "vehicle.h"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define MAP_WIDTH 6246
#define MAP_HEIGHT 6215



void renderText(SDL_Renderer* renderer, TTF_Font* font, const char* text, int x, int y, char *textColor);
bool initializeSDL(SDL_Window **window, SDL_Renderer **renderer);
bool loadTexture(SDL_Renderer *renderer, const char *imagePath, SDL_Texture **texture);
void generateDynamicNoiseTexture(SDL_Renderer *renderer, SDL_Texture **noiseTexture);

void renderMap(SDL_Renderer *renderer, float cameraX, float cameraY, float zoomLevel, SDL_Texture *mapTexture);
void renderBatteryLevel(SDL_Renderer *renderer, TTF_Font *font, float batteryLevel, int currentMapTexture);
void renderCameraPosition(SDL_Renderer *renderer,  TTF_Font *font, float cameraX,float cameraY, int currentMapTexture);
void renderCameraVelocity(SDL_Renderer *renderer,TTF_Font *font,float cameraVelocityX,float cameraVelocityY, int currentMapTexture);
void renderZoomLevel(SDL_Renderer *renderer,TTF_Font *font,float zoomLevel, int currentMapTexture);
void renderAQIndicator(SDL_Renderer *renderer, TTF_Font *font,int autoTargetAcquisitionActive, int currentMapTexture);
void renderRadarIndicator(SDL_Renderer *renderer,TTF_Font *font, int radarActive, int currentMapTexture);
void renderPayloadStatus(SDL_Renderer *renderer,TTF_Font *font,char *payloadStatus, int currentMapTexture);
void renderCurrentDate(SDL_Renderer *renderer, TTF_Font *font, int currentMapTexture);
void renderPayloadType(SDL_Renderer *renderer,TTF_Font *font,char *payloadType, int currentMapTexture);
void renderTargetVelocity(SDL_Renderer *renderer,TTF_Font *font,float targetVelocity, int currentMapTexture);
void renderRadarInterface(SDL_Renderer *renderer,Vehicle *vehicles, SDL_Texture *radarTexture,SDL_Texture *radarTextureIR, SDL_Texture *blipTexture, int numVehicles, float cameraX, float cameraY);

#endif