#include "vehicle.h"

int explosionSmokeWidth = 450;
int explosionSmokeHeight = 206;
int explosionSmokeSpeed = 1; // Adjust this value for the desired smoke speed

// Initialize the smoke
void initializeSmoke(Smoke* smoke, float x, float y, float speed) {
    smoke->x = x - explosionSmokeWidth/2;
    smoke->y = y - explosionSmokeHeight/2;
    smoke->speed = speed;
    smoke->active = true;
}

// Update the smoke's position
void updateSmoke(Smoke* smoke) {
    if (smoke->active) {
        // Update the smoke's position based on speed
        smoke->x -= smoke->speed;

        // Check if the smoke has left the screen
        if (smoke->x + explosionSmokeWidth < 0) {
            smoke->active = false;
        }
    }
}

void renderSmoke(SDL_Renderer* renderer, SDL_Texture* smokeTexture, Smoke* smoke, float cameraX, float cameraY, float zoomLevel) {
    if (smoke->active) {
        // Calculate the screen coordinates of the smoke
        int screenSmokeX = (int)roundf(smoke->x * zoomLevel - cameraX * zoomLevel);
        int screenSmokeY = (int)roundf(smoke->y * zoomLevel - cameraY * zoomLevel);

        // Calculate scaled dimensions based on zoom level
        int scaledSmokeWidth = (int)(explosionSmokeWidth * zoomLevel);
        int scaledSmokeHeight = (int)(explosionSmokeHeight * zoomLevel);

        // Render the smoke at the calculated position
        SDL_Rect smokeRect = {screenSmokeX, screenSmokeY, scaledSmokeWidth, scaledSmokeHeight};
        SDL_RenderCopyEx(renderer, smokeTexture, NULL, &smokeRect, 0, NULL, SDL_FLIP_NONE);
    }
}



void initializeVehicle(SDL_Renderer *renderer, Vehicle *vehicle, float x, float y, const char *carriagePath, const char *turretPath, const char *carriageIRPath, const char *turretIRPath) {
    vehicle->x = x;
    vehicle->y = y;
    vehicle->velocityX = 10.0f;
    vehicle->velocityY = 0.0f;
    vehicle->carriageWidth = 55;
    vehicle->carriageHeight = 31;
    vehicle->turretWidth = 55;
    vehicle->turretHeight = 31;
    vehicle->isHighlighted = false;
    vehicle->isDestroyed = false;
    vehicle->isHit = false;

    // Copy the texture paths into the Vehicle structure
    strcpy(vehicle->carriagePath, carriagePath);
    strcpy(vehicle->turretPath, turretPath);
    strcpy(vehicle->carriageIRPath, carriageIRPath);
    strcpy(vehicle->turretIRPath, turretIRPath);

    // Load the textures using the provided paths
    loadVehicleTexture(renderer, carriagePath, &(vehicle->carriageTexture), vehicle);
    loadVehicleTexture(renderer, turretPath, &(vehicle->turretTexture), vehicle);
    loadVehicleTexture(renderer, carriageIRPath, &(vehicle->carriageTextureIR), vehicle);
    loadVehicleTexture(renderer, turretIRPath, &(vehicle->turretTextureIR), vehicle);

}


void initializeWaypointPath(WaypointPath *path) {
    path->waypoints = NULL;
    path->numWaypoints = 0;
    path->currentWaypointIndex = 0;
}

void addWaypointToPath(WaypointPath *path, float x, float y) {
    Waypoint waypoint;
    waypoint.x = x;
    waypoint.y = y;
    
    path->waypoints = realloc(path->waypoints, (path->numWaypoints + 1) * sizeof(Waypoint));
    path->waypoints[path->numWaypoints] = waypoint;
    path->numWaypoints++;
}

SDL_Texture* createVehicleTexture(SDL_Renderer *renderer, const char *filePath) {
    SDL_Texture *texture = NULL;
    SDL_Surface *surface = IMG_Load(filePath);
    printf("the file path is : %s", filePath);
    if (surface) {
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
    }else{
        printf("failed to create texture\n");
    }
    return texture;
}

void loadVehicleTexture(SDL_Renderer *renderer, const char *path, SDL_Texture **texture, Vehicle *vehicle) {
    SDL_Surface *surface = IMG_Load(path);
    if (surface == NULL) {
        printf("Failed to load surface from: %s\n", path);
        // Handle the error as needed
    } else {
        // Create an SDL_Texture from the loaded surface
        *texture = SDL_CreateTextureFromSurface(renderer, surface);
        if (*texture == NULL) {
            printf("Failed to create texture from surface for: %s\n", path);
            // Handle the error as needed
        }
        SDL_FreeSurface(surface);
    }
}

void updateVehicles(float deltaTime, Vehicle *vehicles, int numVehicles) {
    for (int i = 0; i < numVehicles; i++) {
        if (vehicles[i].waypointPath.numWaypoints > 0 && !vehicles[i].isDestroyed) {
            Waypoint *currentWaypoint = &vehicles[i].waypointPath.waypoints[vehicles[i].waypointPath.currentWaypointIndex];
            
            // Calculate direction to the current waypoint
            float dx = currentWaypoint->x - vehicles[i].x;
            float dy = currentWaypoint->y - vehicles[i].y;
            
            // Calculate the distance to the current waypoint
            float distanceToWaypoint = sqrt(dx * dx + dy * dy);
            
            // Set vehicle velocity towards the waypoint
            if (distanceToWaypoint > 1.0f) { // Some small threshold to avoid oscillation
                float speed = 10.0f; // Adjust as needed
                vehicles[i].velocityX = (dx / distanceToWaypoint) * speed;
                vehicles[i].velocityY = (dy / distanceToWaypoint) * speed;
                
                // Calculate the new rotation angle based on movement direction
                float targetRotation = atan2f(dy, dx) * 180.0f / M_PI;
                vehicles[i].rotation += (targetRotation - vehicles[i].rotation) * 0.1f;

            } else {
                // Reached the current waypoint, move to the next one
                vehicles[i].waypointPath.currentWaypointIndex++;
                if (vehicles[i].waypointPath.currentWaypointIndex >= vehicles[i].waypointPath.numWaypoints) {
                    // Reached the end of the path, reset to the first waypoint
                    vehicles[i].waypointPath.currentWaypointIndex = 0;
                }
            }
        }
        
        // Update vehicle position and other logic here if needed
        vehicles[i].x += vehicles[i].velocityX * deltaTime;
        vehicles[i].y += vehicles[i].velocityY * deltaTime;
        
        // Optionally, you can add code here to smoothly interpolate the rotation angle
        // towards the targetRotation over time for smoother pivoting.
        // For example:
    }
}

bool isCursorOverVehicle(int cursorX, int cursorY, Vehicle vehicle, float cameraX, float cameraY, float zoomLevel) {

    // Calculate the screen coordinates of the tank's bounding box
    int vehicleX = (int)roundf((vehicle.x - cameraX) * zoomLevel);
    int vehicleY = (int)roundf((vehicle.y - cameraY) * zoomLevel);
    int vehicleWidth = (int)(vehicle.carriageWidth * zoomLevel);
    int vehicleHeight = (int)(vehicle.carriageHeight * zoomLevel);

    // Check if the cursor is inside the tank's bounding box
    if (cursorX >= vehicleX && cursorX <= vehicleX + vehicleWidth &&
        cursorY >= vehicleY && cursorY <= vehicleY + vehicleHeight) {
        return true;
    }
    return false;
}

void renderVehicle(SDL_Renderer *renderer, Vehicle *vehicle, float cameraX, float cameraY, float zoomLevel, SDL_Texture *carriageTexture, SDL_Texture *turretTexture, SDL_Texture *smokeSpriteSheet, SDL_Texture *greenRectangleTexture, Uint32 globalTimer, SDL_Texture *explosionTexture, int currentMapTexture) {
    
    // Calculate tank's rotation angle based on movement direction
    float vehicleRotation = vehicle->rotation;

    // Calculate the screen coordinates of the tank
    int screenVehicleX = (int)roundf((vehicle->x - cameraX) * zoomLevel);
    int screenVehicleY = (int)roundf((vehicle->y - cameraY) * zoomLevel);

    // Calculate scaled dimensions based on zoom level
    int scaledCarriageWidth = (int)(vehicle->carriageWidth * zoomLevel);
    int scaledCarriageHeight = (int)(vehicle->carriageHeight * zoomLevel);

    int scaledTurretWidth = (int)(vehicle->turretWidth * zoomLevel);
    int scaledTurretHeight = (int)(vehicle->turretHeight * zoomLevel);

    SDL_Point carriagePivot = {
        scaledCarriageWidth / 2,
        scaledCarriageHeight / 2
    };

    SDL_Point turretPivot = {
        scaledTurretWidth / 2,
        scaledTurretHeight / 2
    };

    int turretX = screenVehicleX;
    int turretY = screenVehicleY;

    // Render tank carriage with rotation
    SDL_Rect carriageRect = {screenVehicleX, screenVehicleY, scaledCarriageWidth, scaledCarriageHeight};

    // Calculate the position for the tank's turret, centered on the carriage
    SDL_Rect turretRect = {
        turretX,
        turretY,
        scaledTurretWidth,
        scaledTurretHeight
    };

    SDL_RenderCopyEx(renderer, carriageTexture, NULL, &carriageRect, vehicleRotation, &carriagePivot, SDL_FLIP_NONE);
    SDL_RenderCopyEx(renderer, turretTexture, NULL, &turretRect, vehicleRotation, &turretPivot, SDL_FLIP_NONE);

    int smokeWidth = 44;
    int smokeHeight = 88;

    // Calculate the position and size of the engine smoke relative to the tank
    int smokeX = (int)roundf((vehicle->x - cameraX - (smokeWidth) - vehicle->carriageWidth / 2) * zoomLevel);

    // Calculate the vertical position of the smoke, which is fixed to the tank's vertical position
    int smokeY = (int)roundf((vehicle->y - cameraY - (smokeHeight / 2) + vehicle->carriageHeight / 2) * zoomLevel);

    // Scale the smoke dimensions based on the zoom level
    int scaledSmokeWidth = (int)(smokeWidth * zoomLevel);
    int scaledSmokeHeight = (int)(smokeHeight * zoomLevel);

    // Create a destination rectangle for rendering the scaled smoke
    SDL_Rect smokeRect = {smokeX, smokeY, scaledSmokeWidth, scaledSmokeHeight};

    SDL_Rect smokeFrames[4];
    for (int i = 0; i < 4; i++) {
        smokeFrames[i].x = i * 175;  // Adjust based on the frame width
        smokeFrames[i].y = 0;        // Assuming all frames are on the same row
        smokeFrames[i].w = 175;      // Width of each frame
        smokeFrames[i].h = 357;      // Height of each frame (same as the sprite sheet)
    }

    // Calculate the frame to display based on the animation time
    int smokeFrameIndex = (globalTimer / 250) % 4;  // Change frame every 100 milliseconds

    // Calculate the angle of rotation (counterclockwise, -90 degrees)
    double angle = -90.0;
    // Render the smoke frame with rotation
    SDL_RenderCopyEx(renderer, smokeSpriteSheet, &smokeFrames[smokeFrameIndex], &smokeRect, angle, NULL, SDL_FLIP_NONE);
    // Calculate the frame to display based on the global timer

    // Check if the vehicle is highlighted
    if (vehicle->isHighlighted) {
        // Calculate the screen coordinates and dimensions of the tank
        int vehicleX = (int)roundf((vehicle->x - cameraX) * zoomLevel);
        int vehicleY = (int)roundf((vehicle->y - cameraY) * zoomLevel);
        int vehicleWidth = (int)(vehicle->carriageWidth * zoomLevel);
        int vehicleHeight = (int)(vehicle->carriageHeight * zoomLevel);

        // Calculate the position and dimensions of the green rectangle
        int rectX = vehicleX - (int)(10 * zoomLevel) - 10 * zoomLevel; // Adjust as needed for the desired offset
        int rectY = vehicleY - (int)(10 * zoomLevel) - 10 * zoomLevel; // Adjust as needed for the desired offset
        int rectWidth = vehicleWidth + (int)(20 * zoomLevel) * 2; // Adjust as needed for the desired size
        int rectHeight = vehicleHeight + (int)(20 * zoomLevel) * 2; // Adjust as needed for the desired size

        // Render the green rectangle at the calculated position and dimensions
        SDL_Rect greenRectDest = {rectX, rectY, rectWidth, rectHeight};
        SDL_RenderCopy(renderer, greenRectangleTexture, NULL, &greenRectDest);
    }
}

