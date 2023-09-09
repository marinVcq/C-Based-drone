#include <stdbool.h>
#include <SDL_image.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <SDL_timer.h>
#include <SDL_ttf.h>
#include <string.h>
#include<conio.h>

#include "interface.h"
#include "vehicle.h"

#define MIN_ZOOM 0.2f
#define MAX_ZOOM 10.0f
#define NOISE_INTENSITY 0.2f

#define BUTTON_A 1
#define BUTTON_B 0
#define SELECT_BUTTON_INDEX 8
#define START_BUTTON_INDEX 9

#define DRONE_MAX_VELOCITY 400.0f
#define DRONE_ACCELERATION 40.0f
#define DRONE_DECELERATION 100.0f
#define KEY_UP SDLK_UP
#define KEY_DOWN SDLK_DOWN
#define KEY_LEFT SDLK_LEFT
#define KEY_RIGHT SDLK_RIGHT
#define KEY_ZOOM_IN SDLK_PLUS
#define KEY_ZOOM_OUT SDLK_MINUS
#define KEY_SWITCH_MAP SDLK_s

// Define constants for explosion frame dimensions and number of frames
#define EXPLOSION_FRAME_WIDTH 192  // Adjust according to your tileset
#define EXPLOSION_FRAME_HEIGHT 192 // Adjust according to your tileset
#define EXPLOSION_FRAMES_PER_ROW 5
#define EXPLOSION_TOTAL_FRAMES 16  // Adjust according to your tileset

#define MAX_VEHICLES 10 

typedef struct {
    bool up;
    bool down;
    bool left;
    bool right;
    bool zoomIn;
    bool zoomOut;
    bool switchMap;
} InputState;

InputState inputState = {0};

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;

SDL_Texture *droneTexture, *mapTexture1, *mapTexture2, *noiseTexture, *textTexture = NULL;
SDL_Texture *droneTextureIR = NULL;
SDL_Texture *smokeSpriteSheet = NULL;
SDL_Texture *greenRectangleTexture = NULL;
SDL_Texture *turret = NULL;
SDL_Texture *carriage = NULL;
SDL_Texture *radarBackgroundTexture = NULL;
SDL_Texture *radarBackgroundTextureIR = NULL;

SDL_Texture *vehicleBlipTexture = NULL;
SDL_Texture *mapTexture = NULL;
SDL_Texture *explosionTexture = NULL;
SDL_Texture *explosionTextureIR = NULL;
SDL_Texture *shellHoleTexture = NULL;
SDL_Texture *explosion = NULL;
SDL_Texture *destroyedVehicleTexture = NULL;
SDL_Texture *destroyedVehicleTextureIR = NULL;
SDL_Texture* explosionSmokeTexture = NULL;
SDL_Texture* explosionSmokeTextureIR = NULL;





TTF_Font *font = NULL;
SDL_Rect droneInterfaceRect = {0, 0,SCREEN_WIDTH, SCREEN_HEIGHT};

int currentMapTexture = 1; // Start with the first map texture
bool running = true;

int cursorX =400;
int cursorY=270;

// Starting drone parameters
float droneBatteryLevel = 100.0f;
float batteryDepletionRate = 0.001f;

// Starting camera's position and zoom
float cameraX = 2000.0f;
float cameraY = 2000.0f;
float zoomLevel = 0.3f;
int currentFrame = 0;
int totalFrames = 58;
int frameWidth =480;
int frameHeight=360;

// Variables for velocity and acceleration
float cameraVelocityX = 0.0f;
float cameraVelocityY = 0.0f;

bool buttonSelectPushed = false;
bool buttonStartPushed = false;


Uint32 selectButtonPressStartTime = 0;
Uint32 startButtonPressStartTime = 0;
Uint32 prevTime = 0;

int autoTargetAcquisitionActive = 1;
int radarActive = 0;
char *payloadStatus = "DISARMED";
char *payloadType = "AT";

float targetX = 0;
float targetY = 0;

// Function prototypes
bool loadAllTextures(SDL_Renderer *renderer);
void handleKeyEvents(SDL_Event event, InputState *inputState, bool *running);
void handleJoystickEvents(SDL_Joystick *joystick);
void update(float deltaTime);
void renderGame(SDL_Renderer *renderer, TTF_Font *font, float droneBatteryLevel, SDL_Texture *noiseTexture, int numVehicles, Vehicle *vehicles);

typedef struct {
    float x;
    float y;
    bool set; // Flag to indicate if the manual target is set
} ManualTarget;

ManualTarget manualTarget = {0.0f, 0.0f, false}; // Initialize manual target

// Function to clear the manual target
void clearManualTarget() {
    manualTarget.set = false; // Reset the manual target flag
}
// Function to set the manual target to the cursor position
void setManualTarget(int cursorX, int cursorY, float cameraX, float cameraY, float zoomLevel) {

    if(autoTargetAcquisitionActive == 0){
        // Calculate the world coordinates of the cursor
        clearManualTarget();
        manualTarget.x = (cursorX / zoomLevel) + cameraX;
        manualTarget.y = (cursorY / zoomLevel) + cameraY;
        manualTarget.set = true; // Set the manual target flag
        targetX = manualTarget.x;
        targetY = manualTarget.y;
        printf("Manual target position: x: %f y: %f\n", manualTarget.x, manualTarget.y);      
    }else{
        printf("can't set target manualy in auto AQ mode\n");
    }

}

// Define explosion variables
int explosionFrameIndex = 0; // Current explosion frame index
Uint32 lastExplosionTime = 0; // Time of the last explosion frame update
int explosionFrameDelay = 200; // Delay between explosion frames in milliseconds

// Define smoke variables
SDL_Texture* smokeTexture; // Load your smoke image texture here
Uint32 lastSmokeTime = 0;



// Function to animate the explosion and smoke
void animateExplosion(SDL_Renderer* renderer, Vehicle* vehicle, float cameraX, float cameraY, float zoomLevel) {
    // Calculate the explosion frame
    Uint32 currentTime = SDL_GetTicks();
    if (currentTime - lastExplosionTime >= explosionFrameDelay) {
        explosionFrameIndex++;

        // If we've reached the last frame of the explosion, reset the animation
        if (explosionFrameIndex >= 16) {
            explosionFrameIndex = 0;
            vehicle->isHit = false;
        }

        lastExplosionTime = currentTime;
    }

    // Calculate the explosion frame position and size
    int explosionWidth = 384;
    int explosionHeight = 384;
    int explosionX = (int)roundf((vehicle->x - cameraX - (explosionWidth / 2)) * zoomLevel);
    int explosionY = (int)roundf((vehicle->y - cameraY - (explosionHeight / 2)) * zoomLevel);
    int scaledExplosionWidth = (int)(explosionWidth * zoomLevel);
    int scaledExplosionHeight = (int)(explosionHeight * zoomLevel);

    SDL_Rect explosionRect = {explosionX, explosionY, scaledExplosionWidth, scaledExplosionHeight};

    SDL_Rect explosionFrames[16];
    for (int j = 0; j < 16; j++) {
        explosionFrames[j].x = (j % 4) * 192;   // Adjust based on the number of columns (4 columns)
        explosionFrames[j].y = (j / 4) * 192;   // Adjust based on the number of columns (4 columns)
        explosionFrames[j].w = 192;             // Width of each frame
        explosionFrames[j].h = 192;             // Height of each frame (same as the sprite sheet)
    }

    // Render the explosion frame
    if (currentMapTexture == 1 && explosionFrameIndex != 0) {
        SDL_RenderCopy(renderer, explosionTexture, &explosionFrames[explosionFrameIndex], &explosionRect);
    } else if(currentMapTexture != 1 && explosionFrameIndex != 0){
        SDL_RenderCopy(renderer, explosionTextureIR, &explosionFrames[explosionFrameIndex], &explosionRect);
    }
}

int main(int argc, char *argv[]) {

    srand(time(NULL)); // Initialize random number generator
    
    // Initialize vehicles
    Vehicle vehicles[MAX_VEHICLES];
    int numVehicles = 2; // to refact

    if (!initializeSDL(&window, &renderer)) {
        return 1;
    }else
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    if (!loadAllTextures(renderer)) {
        printf("error when loading texture\n");
        return 1;
    }else
        printf("All textures succesfully loaded!\n");

    // Load a font (you can adjust the font size and style)
    TTF_Font *font = TTF_OpenFont("digital.ttf", 22);
    if (!font) {
        printf("Failed to load font: %s\n", TTF_GetError());
        return 1;
    }

    initializeVehicle(renderer, &vehicles[0], 2000.0f, 2000.0f, "carriage.png", "turret.png", "carriageIR.png", "turretIR.png");
    initializeWaypointPath(&vehicles[0].waypointPath);
    addWaypointToPath(&vehicles[0].waypointPath, 4000.0f, 2000.0f);

    initializeVehicle(renderer, &vehicles[1], 1950.0f, 2200.0f, "carriage.png", "turret.png", "carriageIR.png", "turretIR.png");
    initializeWaypointPath(&vehicles[1].waypointPath);
    addWaypointToPath(&vehicles[1].waypointPath, 4000.0f, 2050.0f);




    while (running) {

        Uint32 currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - prevTime) / 1000.0f;
        prevTime = currentTime;

        // Handle key Events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            handleKeyEvents(event, &inputState, &running);
        }

        // Poll joystick events
        SDL_JoystickUpdate();
        int numJoysticks = SDL_NumJoysticks();

        if (numJoysticks > 0) {
            SDL_Joystick *joystick = SDL_JoystickOpen(0); // Open the first joystick
            if (joystick) {
                handleJoystickEvents(joystick);
                SDL_JoystickClose(joystick);
            }
        }

        // Update battery level
        droneBatteryLevel -= batteryDepletionRate *0.001f;
        droneBatteryLevel = fmaxf(droneBatteryLevel, 0.0f); // Ensure battery level doesn't go below 0

        // Update drone position and movement
        update(deltaTime);
        updateVehicles(deltaTime, vehicles, numVehicles);


        // Update battery level based on distance moved
        float distanceMoved = sqrt(cameraVelocityX * cameraVelocityX + cameraVelocityY * cameraVelocityY) * deltaTime;
        droneBatteryLevel -= distanceMoved * batteryDepletionRate;
        droneBatteryLevel = fmaxf(droneBatteryLevel, 0.0f);

        // Clamp camera position to stay within map bounds
        cameraX = fmaxf(0.0f, fminf(cameraX, MAP_WIDTH - SCREEN_WIDTH / zoomLevel));
        cameraY = fmaxf(0.0f, fminf(cameraY, MAP_HEIGHT - SCREEN_HEIGHT / zoomLevel));

        // Before generating the new noise texture and Generate dynamic noise texture with a certain intensity
        if (noiseTexture != NULL) {
            SDL_DestroyTexture(noiseTexture);
        }
        generateDynamicNoiseTexture(renderer, &noiseTexture);

        // Render the game
        renderGame(renderer, font, droneBatteryLevel, noiseTexture,numVehicles, vehicles);
    }
    // Clean up text resources
    SDL_DestroyTexture(textTexture);
    SDL_DestroyTexture(smokeSpriteSheet);
    SDL_DestroyTexture(noiseTexture);
    SDL_DestroyTexture(droneTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyTexture(explosionTexture);
    SDL_DestroyTexture(explosionTextureIR);
    SDL_DestroyTexture(shellHoleTexture);
    SDL_DestroyWindow(window);
    SDL_DestroyTexture(radarBackgroundTexture);
    SDL_DestroyTexture(vehicleBlipTexture);
    IMG_Quit(); // Clean up SDL_image resources
    SDL_Quit();
    TTF_Quit();

    return 0;
}

//0969368338
// Load all textures
bool loadAllTextures(SDL_Renderer *renderer) {
    return
        loadTexture(renderer, "viewMap.png", &mapTexture1) &&
        loadTexture(renderer, "viewMapIR2.png", &mapTexture2) &&
        loadTexture(renderer, "drone_interface.png", &droneTexture) &&
        loadTexture(renderer, "drone_interface_green.png", &droneTextureIR) &&
        loadTexture(renderer, "engineSmoke.png", &smokeSpriteSheet) &&
        loadTexture(renderer, "radar_background.png", &radarBackgroundTexture) &&
        loadTexture(renderer, "vehicle_blip.png", &vehicleBlipTexture) &&
        loadTexture(renderer, "ATexplosion.png", &explosionTexture) &&
        loadTexture(renderer, "ATexplosionIR.png", &explosionTextureIR) &&
        loadTexture(renderer, "destroyedVehicle.png", &destroyedVehicleTexture) &&
        loadTexture(renderer, "destroyedVehicleIR.png", &destroyedVehicleTextureIR) &&
        loadTexture(renderer, "explosionSmoke.png", &explosionSmokeTexture) &&
        loadTexture(renderer, "explosionSmokeIR.png", &explosionSmokeTextureIR) &&
        loadTexture(renderer, "greenRectangle.png", &greenRectangleTexture);
        return false;
}

void handleKeyEvents(SDL_Event event, InputState *inputState, bool *running) {
    switch (event.type) {
        case SDL_QUIT:
            *running = false;
            break;
        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
                case KEY_UP: inputState->up = true; break;
                case KEY_DOWN: inputState->down = true; break;
                case KEY_LEFT: inputState->left = true; break;
                case KEY_RIGHT: inputState->right = true; break;
                case KEY_ZOOM_IN: inputState->zoomIn = true; break;
                case KEY_ZOOM_OUT: inputState->zoomOut = true; break;
                case KEY_SWITCH_MAP: inputState->switchMap = true; break;
            }
            break;
        case SDL_KEYUP:
            switch (event.key.keysym.sym) {
                case KEY_UP: inputState->up = false; break;
                case KEY_DOWN: inputState->down = false; break;
                case KEY_LEFT: inputState->left = false; break;
                case KEY_RIGHT: inputState->right = false; break;
                case KEY_ZOOM_IN: inputState->zoomIn = false; break;
                case KEY_ZOOM_OUT: inputState->zoomOut = false; break;
                case KEY_SWITCH_MAP: inputState->switchMap = false; break;
            }
            break;
        // Handle other event types if needed
    }
}
bool payloadTypeChanged = false;
Uint32 lastPayloadTypeChangeTime = 0;
Uint32 payloadTypeChangeDelay = 1000;
bool payloadStatusChanged = false;
Uint32 lastPayloadStatusChangeTime = 0;
Uint32 payloadStatusChangeDelay = 1000;
bool mapTextureSwitched = false;
Uint32 lastMapTextureSwitchTime = 0;
Uint32 mapTextureSwitchDelay = 1000;
bool autoTargetAcquisitionChanged = false;
Uint32 lastAutoTargetAcquisitionChangeTime = 0;
Uint32 autoTargetAcquisitionChangeDelay = 1000;
bool radarActiveChanged = false;
Uint32 lastRadarActiveChangeTime = 0;
Uint32 radarActiveChangeDelay = 1000; 
bool payloadFired = false;
Uint32 lastShotTime = 0;
Uint32 reloadTime = 5000;  // 5 seconds reload time

// Function to handle joystick events
void handleJoystickEvents(SDL_Joystick *joystick) {

    // Handle joystick input
    float joyX = SDL_JoystickGetAxis(joystick, 0) / 32767.0f;
    float joyY = SDL_JoystickGetAxis(joystick, 1) / 32767.0f;

    // Apply acceleration threshold to joystick input
    if (fabs(joyX) > 0.1f && !buttonStartPushed) {
        cameraVelocityX += joyX * DRONE_ACCELERATION;
    }
    if (fabs(joyY) > 0.1f && !buttonStartPushed) {
        cameraVelocityY += joyY * DRONE_ACCELERATION;
    }

    // Check button presses for zooming and unzooming
    if (SDL_JoystickGetButton(joystick, BUTTON_B)) {
        inputState.zoomIn = true;
    } else {
        inputState.zoomIn = false;
    }
    if (SDL_JoystickGetButton(joystick, BUTTON_A)) {
        inputState.zoomOut = true;
    } else {
        inputState.zoomOut = false;
    }

    // Check for "Start" button press
    if (SDL_JoystickGetButton(joystick, START_BUTTON_INDEX)) {
        buttonStartPushed = true;
    } else {
        buttonStartPushed = false;
    }

    // Check for "Select" button press to switch map texture
    if (SDL_JoystickGetButton(joystick, SELECT_BUTTON_INDEX)) {
        Uint32 currentTime = SDL_GetTicks();
        if (!mapTextureSwitched || (currentTime - lastMapTextureSwitchTime >= mapTextureSwitchDelay)) {
            lastMapTextureSwitchTime = currentTime;
            mapTextureSwitched = true;
            // Switch the map texture
            if (currentMapTexture == 1) {
                currentMapTexture = 2;
            } else {
                currentMapTexture = 1;
            }
        }
    } else {
        mapTextureSwitched = false;
    }

    // Check for "Start" button press and up joyX button press simultaneously
    if (SDL_JoystickGetButton(joystick, START_BUTTON_INDEX) && joyY < -0.1f) {
        Uint32 currentTime = SDL_GetTicks();
        if (!payloadTypeChanged || (currentTime - lastPayloadTypeChangeTime >= payloadTypeChangeDelay)) {
            payloadTypeChanged = true;
            lastPayloadTypeChangeTime = currentTime;

            // Change the payloadType to "AP" if it's currently "AT," and vice versa
            if (strcmp(payloadType, "AT") == 0) {
                payloadType = "AP";
            } else {
                payloadType = "AT";
            }
        }
    } else {
        payloadTypeChanged = false;
    }

    // Check for "Start" button press and joyX down button press simultaneously
    if (SDL_JoystickGetButton(joystick, START_BUTTON_INDEX) && joyY > 0.1f) {
        Uint32 currentTime = SDL_GetTicks();
        if (!payloadStatusChanged || (currentTime - lastPayloadStatusChangeTime >= payloadStatusChangeDelay)) {
            payloadStatusChanged = true;
            lastPayloadStatusChangeTime = currentTime;

            // Change the payloadStatus to "READY" if it's currently "DISARMED," and vice versa
            if (strcmp(payloadStatus, "DISARMED") == 0) {
                payloadStatus = "READY";
            } else {
                payloadStatus = "DISARMED";
            }
        }
    } else {
        payloadStatusChanged = false;
    }

    // Check for "Start" button press and joyY right button press simultaneously
    if (SDL_JoystickGetButton(joystick, START_BUTTON_INDEX) && joyX > 0.1f) {
        Uint32 currentTime = SDL_GetTicks();
        if (!autoTargetAcquisitionChanged || (currentTime - lastAutoTargetAcquisitionChangeTime >= autoTargetAcquisitionChangeDelay)) {
            autoTargetAcquisitionChanged = true;
            clearManualTarget();

            lastAutoTargetAcquisitionChangeTime = currentTime;

            if(autoTargetAcquisitionActive == 0){
                autoTargetAcquisitionActive = 1;  // Activate auto target acquisition
            }else{
                autoTargetAcquisitionActive = 0;  // Desactivate auto target acquisition
            }
        }
    } else {
        autoTargetAcquisitionChanged = false;
    }
    
    // Check for "Start" button press and left JoyX button press simultaneously
    if (SDL_JoystickGetButton(joystick, START_BUTTON_INDEX) && joyX < -0.1f) {
        Uint32 currentTime = SDL_GetTicks();
        if (!radarActiveChanged || (currentTime - lastRadarActiveChangeTime >= radarActiveChangeDelay)) {
            radarActiveChanged = true;
            lastRadarActiveChangeTime = currentTime;
            if(radarActive == 0){
                radarActive = 1;  // Activate radar
            }else{
                radarActive = 0;  // Desactivate radar
            }
        }
    } else {
        radarActiveChanged = false;
    }
    if (SDL_JoystickGetButton(joystick, START_BUTTON_INDEX) && SDL_JoystickGetButton(joystick, BUTTON_A)) {
        // Check if the payload is ready to fire
        if (strcmp(payloadStatus, "READY") == 0) {
            payloadStatus = "DISARMED";
            payloadFired = true;
            lastShotTime = SDL_GetTicks();
            // Set payload status to "FIRING" (you can define this status)
        }else{
            printf("PAYLOA stat %s", payloadStatus);
        }
    }
    if (SDL_JoystickGetButton(joystick, START_BUTTON_INDEX) && SDL_JoystickGetButton(joystick, BUTTON_B)) {
        // Call the function to set the manual target to the cursor position
        setManualTarget(cursorX, cursorY, cameraX, cameraY, zoomLevel);
    }
}

// Function to update drone movement and parameters
void update(float deltaTime){

    if(!buttonStartPushed){
        // Drone movement
        if (inputState.up && cameraY > 0) {
            cameraVelocityY -= DRONE_ACCELERATION;
        }
        if (inputState.down && cameraY < MAP_HEIGHT - SCREEN_HEIGHT / zoomLevel) {
            cameraVelocityY += DRONE_ACCELERATION;
        }
        if (inputState.left && cameraX > 0) {
            cameraVelocityX -= DRONE_ACCELERATION;
        }
        if (inputState.right && cameraX < MAP_WIDTH - SCREEN_WIDTH / zoomLevel) {
            cameraVelocityX += DRONE_ACCELERATION;
        }
        if (inputState.zoomIn) {
            zoomLevel = fminf(zoomLevel + 0.05f, MAX_ZOOM);
        }
        if (inputState.zoomOut) {
            zoomLevel = fmaxf(zoomLevel - 0.05f, MIN_ZOOM);
        }        
    }


    // Limit velocities to DRONE_MAX_VELOCITY
    cameraVelocityX = fminf(fmaxf(cameraVelocityX, -DRONE_MAX_VELOCITY), DRONE_MAX_VELOCITY);
    cameraVelocityY = fminf(fmaxf(cameraVelocityY, -DRONE_MAX_VELOCITY), DRONE_MAX_VELOCITY);
    cameraX += cameraVelocityX * deltaTime;
    cameraY += cameraVelocityY * deltaTime;
    

    // Apply deceleration when no movement keys are pressed
    if (cameraVelocityX > 0) {
        cameraVelocityX -= DRONE_DECELERATION * deltaTime;
        if (cameraVelocityX < 0) {
            cameraVelocityX = 0;
        }
    } else if (cameraVelocityX < 0) {
        cameraVelocityX += DRONE_DECELERATION * deltaTime;
        if (cameraVelocityX > 0) {
            cameraVelocityX = 0;
        }
    }
    if (cameraVelocityY > 0) {
        cameraVelocityY -= DRONE_DECELERATION * deltaTime;
        if (cameraVelocityY < 0) {
            cameraVelocityY = 0;
        }
    } else if (cameraVelocityY < 0) {
        cameraVelocityY += DRONE_DECELERATION * deltaTime;
        if (cameraVelocityY > 0) {
            cameraVelocityY = 0;
        }
    }
}
Uint32 globalTimer = 0;

// Function to render the game
void renderGame(SDL_Renderer *renderer, TTF_Font *font, float droneBatteryLevel, SDL_Texture *noiseTexture, int numVehicles,Vehicle *vehicles ) {

    float targetVelocity = 0;
    cameraX = round(cameraX);
    cameraY = round(cameraY);

    Uint32 currentTime = SDL_GetTicks();

    // Update the global timer (outside your renderGame function)
    Uint32 newTime = SDL_GetTicks();
    globalTimer += newTime - currentTime;

    Uint32 lastTime = SDL_GetTicks(); 

    // Clear the renderer
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Explosion texture and frame index
    explosion = (currentMapTexture == 1) ? explosionTexture : explosionTextureIR;

    // Calculate the time elapsed since the last frame
    int deltaTime = currentTime - lastTime;

    // Add a delay to control the frame rate (e.g., 60 frames per second)
    int frameDelay = 1000 / 60;  // 60 FPS
    if (deltaTime < frameDelay) {
        SDL_Delay(frameDelay - deltaTime);
        currentTime = SDL_GetTicks();  // Update current time after the delay
        deltaTime = currentTime - lastTime;
    }

    lastTime = currentTime;

    // Render the map
    mapTexture = (currentMapTexture == 1) ? mapTexture1 : mapTexture2;

    renderMap(renderer, cameraX, cameraY, zoomLevel, mapTexture);

    // Inside the renderGame function
    for (int i = 0; i < numVehicles; i++) {

        // Check if the cursor is over the tank and auto target acquisition is active
        if (isCursorOverVehicle(cursorX, cursorY, vehicles[i], cameraX, cameraY, zoomLevel) && autoTargetAcquisitionActive) {
            // If the cursor is over the tank, unmark all other vehicles
            for (int j = 0; j < numVehicles; j++) {
                if (j != i) {
                    vehicles[j].isHighlighted = false;
                }
            }
            // Mark the current vehicle as highlighted
            vehicles[i].isHighlighted = true;
        }

        if (payloadFired && vehicles[i].isHighlighted) {
            payloadFired = false;
            vehicles[i].isHit = true;
        }

        if (vehicles[i].isDestroyed) {

            // Calculate the screen coordinates of the tank
            int screenVehicleX = (int)roundf((vehicles[i].x - cameraX) * zoomLevel) - 185/2 * zoomLevel;
            int screenVehicleY = (int)roundf((vehicles[i].y - cameraY) * zoomLevel) - 50 * zoomLevel;

            // Calculate scaled dimensions based on zoom level
            int scaledVehicleWidth = (int)(185 * zoomLevel);
            int scaledVehicleHeight = (int)(100 * zoomLevel);


            SDL_Rect vehicleRect = {screenVehicleX, screenVehicleY, scaledVehicleWidth, scaledVehicleHeight};

            if(currentMapTexture == 1){
                SDL_RenderCopyEx(renderer, destroyedVehicleTexture, NULL, &vehicleRect, 0, NULL, SDL_FLIP_NONE);

            }else{
                SDL_RenderCopyEx(renderer, destroyedVehicleTextureIR, NULL, &vehicleRect, 0, NULL, SDL_FLIP_NONE);
            }

            // Initialize smoke when the vehicle is destroyed
            if (!vehicles[i].explosionSmoke.active) {
                initializeSmoke(&vehicles[i].explosionSmoke, vehicles[i].x, vehicles[i].y, 0.3);
                printf("smoke is initialized:\n");
            }

            // Update and render smoke in each frame
            updateSmoke(&vehicles[i].explosionSmoke);
            if(currentMapTexture == 1){
                renderSmoke(renderer, explosionSmokeTexture, &vehicles[i].explosionSmoke, cameraX, cameraY, zoomLevel);

            }else{
                renderSmoke(renderer, explosionSmokeTextureIR, &vehicles[i].explosionSmoke, cameraX, cameraY, zoomLevel);

            }

        } else {
            // Render the vehicle
            if(currentMapTexture == 1){
                renderVehicle(renderer, &vehicles[i], cameraX, cameraY, zoomLevel, vehicles[i].carriageTexture, vehicles[i].turretTexture, smokeSpriteSheet,greenRectangleTexture, globalTimer, explosionTexture, currentMapTexture);
            }else{
                renderVehicle(renderer, &vehicles[i], cameraX, cameraY, zoomLevel, vehicles[i].carriageTextureIR, vehicles[i].turretTextureIR, smokeSpriteSheet,greenRectangleTexture, globalTimer, explosionTexture, currentMapTexture);

            }
        }

        // Inside your renderGame function
        if (vehicles[i].isHit) {

            vehicles[i].isDestroyed = true;
            vehicles[i].velocityX = 0.0f;
            vehicles[i].velocityY = 0.0f;

            animateExplosion(renderer, &vehicles[i], cameraX, cameraY, zoomLevel);
        }
    }

    // Render the noise texture
    SDL_RenderCopy(renderer, noiseTexture, NULL, NULL);

    // Render the drone interface on top
    if(currentMapTexture == 1){
        SDL_RenderCopy(renderer, droneTexture, NULL, &droneInterfaceRect);

    }else{
        SDL_RenderCopy(renderer, droneTextureIR, NULL, &droneInterfaceRect);

    }

    // Render Interface
    renderBatteryLevel(renderer, font, droneBatteryLevel, currentMapTexture);
    renderZoomLevel(renderer, font, zoomLevel, currentMapTexture);
    renderCameraPosition(renderer, font, cameraX, cameraY, currentMapTexture);
    renderCameraVelocity(renderer, font, cameraVelocityX, cameraVelocityY, currentMapTexture);
    renderAQIndicator(renderer, font, autoTargetAcquisitionActive, currentMapTexture);
    renderRadarIndicator(renderer, font, radarActive, currentMapTexture);
    renderPayloadStatus(renderer, font, payloadStatus, currentMapTexture);
    renderCurrentDate(renderer, font, currentMapTexture);
    renderPayloadType(renderer, font, payloadType, currentMapTexture);
    renderTargetVelocity(renderer, font, targetVelocity, currentMapTexture);
    renderRadarInterface(renderer,vehicles, radarBackgroundTexture,radarBackgroundTextureIR, vehicleBlipTexture,numVehicles, cameraX, cameraY);
    // Check if the manual target is set
    if (manualTarget.set) {
        // Calculate the screen coordinates and dimensions of the manual target
        int manualTargetX = (int)roundf((manualTarget.x - cameraX) * zoomLevel);
        int manualTargetY = (int)roundf((manualTarget.y - cameraY) * zoomLevel);
        int manualTargetWidth = 100+ (int)(20 * zoomLevel); // Adjust as needed for the desired size
        int manualTargetHeight = 100 + (int)(20 * zoomLevel); // Adjust as needed for the desired size

        // Calculate the position and dimensions of the green rectangle
        int rectX = manualTargetX - manualTargetWidth / 2;
        int rectY = manualTargetY - manualTargetHeight / 2;

        // Render the green rectangle at the manual target position
        SDL_Rect greenRectDest = {rectX, rectY, manualTargetWidth, manualTargetHeight};
        SDL_RenderCopy(renderer, greenRectangleTexture, NULL, &greenRectDest);
    }

    // Present the renderer
    SDL_RenderPresent(renderer);
}