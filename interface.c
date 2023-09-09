#include "interface.h"

bool initializeSDL(SDL_Window **window, SDL_Renderer **renderer) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) != 0) {
        printf("Failed to initialize SDL: %s\n", SDL_GetError());
        return false;
    }

    // Initialize SDL_image with PNG support
    if (IMG_Init(IMG_INIT_PNG) == 0) {
        printf("Failed to initialize SDL_image: %s\n", IMG_GetError());
        return false;
    }

    // Initialize SDL_ttf
    if (TTF_Init() < 0) {
        printf("Failed to initialize SDL_ttf: %s\n", TTF_GetError());
        return false;
    }

    *window = SDL_CreateWindow("Drone Simulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                               SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!*window) {
        printf("Failed to create window: %s\n", SDL_GetError());
        return false;
    }

    *renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED);
    if (!*renderer) {
        printf("Failed to create renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(*window);
        return false;
    }

    return true;
}

bool loadTexture(SDL_Renderer *renderer, const char *imagePath, SDL_Texture **texture) {
    SDL_Surface *surface = IMG_Load(imagePath);
    if (!surface) {
        return false;
    }
    *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return (*texture != NULL);
}

void renderText(SDL_Renderer* renderer, TTF_Font* font, const char* text, int x, int y, char *color) {
    SDL_Color greenColor = {0, 255, 0} ;
    SDL_Color whiteColor = {255, 255, 255} ;
    SDL_Surface* textSurface ;


    if(strcmp(color, "GREEN") == 0){
        textSurface = TTF_RenderText_Solid(font, text, greenColor);


    }else if(strcmp(color, "WHITE") == 0){
        textSurface = TTF_RenderText_Solid(font, text, whiteColor);

    }
    if (textSurface == NULL) {
        // Handle error
        fprintf(stderr, "Failed to render text: %s\n", TTF_GetError());
        return;
    }

    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (textTexture == NULL) {
        // Handle error
        fprintf(stderr, "Failed to create texture from surface: %s\n", SDL_GetError());
        SDL_FreeSurface(textSurface);
        return;
    }

    SDL_Rect renderRect = {x, y, textSurface->w, textSurface->h};
    SDL_RenderCopy(renderer, textTexture, NULL, &renderRect);

    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}

void renderMap(SDL_Renderer *renderer, float cameraX, float cameraY, float zoomLevel, SDL_Texture *mapTexture) {
    
    // Calculate the aspect ratio of the map texture
    float mapAspect = (float)MAP_WIDTH / (float)MAP_HEIGHT;

    // Calculate the new dimensions of the source rectangle
    int sourceWidth = (int)(SCREEN_WIDTH / zoomLevel);
    int sourceHeight = (int)(SCREEN_HEIGHT / zoomLevel);

    // Ensure the source rectangle stays within map bounds
    if (cameraX + sourceWidth > MAP_WIDTH) {
        cameraX = MAP_WIDTH - sourceWidth;
    }
    if (cameraY + sourceHeight > MAP_HEIGHT) {
        cameraY = MAP_HEIGHT - sourceHeight;
    }

    // Render the map texture
    SDL_Rect sourceRect = {(int)cameraX, (int)cameraY, sourceWidth, sourceHeight};
    SDL_Rect destinationRect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    SDL_RenderCopy(renderer, mapTexture, &sourceRect, &destinationRect);
}

void renderBatteryLevel(SDL_Renderer *renderer, TTF_Font *font, float batteryLevel, int currentMapTexture) {
    char batteryText[50];
    snprintf(batteryText, sizeof(batteryText), "BAT: %.f%%", batteryLevel);
    if(currentMapTexture == 1){
        renderText(renderer, font, batteryText, 590, 30,"WHITE");

    }else{
        renderText(renderer, font, batteryText, 590, 30,"GREEN");

    }
}

void renderCameraPosition(SDL_Renderer *renderer,  TTF_Font *font, float cameraX,float cameraY, int currentMapTexture){
    char xPosText[50];
    char yPosText[50];
    snprintf(xPosText, sizeof(xPosText), "X POS: %.2f", cameraX + SCREEN_WIDTH/2);
    snprintf(yPosText, sizeof(yPosText), "Y POS: %.2f", cameraY + SCREEN_HEIGHT/2);

    if(currentMapTexture == 1){
        renderText(renderer, font, xPosText, 590, 500, "WHITE");
        renderText(renderer, font, yPosText, 590, 525, "WHITE");
    }else{
        renderText(renderer, font, xPosText, 590, 500, "GREEN");
        renderText(renderer, font, yPosText, 590, 525, "GREEN");
    }

}

void renderCameraVelocity(SDL_Renderer *renderer,TTF_Font *font,float cameraVelocityX,float cameraVelocityY, int currentMapTexture){
    char velocityText[50];
    snprintf(velocityText, sizeof(velocityText), "CAM VEL: %.2f", sqrt(cameraVelocityX * cameraVelocityX + cameraVelocityY * cameraVelocityY));
    if(currentMapTexture == 1){
        renderText(renderer, font, velocityText, 590, 550, "WHITE");
    }else{
        renderText(renderer, font, velocityText, 590, 550, "GREEN");
    }
}

void renderZoomLevel(SDL_Renderer *renderer,TTF_Font *font,float zoomLevel, int currentMapTexture){
    char zoomText[50];
    snprintf(zoomText, sizeof(zoomText), "ZOOM LVL: %.2f", zoomLevel);
    if(currentMapTexture == 1){
        renderText(renderer, font, zoomText, 590, 475, "WHITE");
    }else{
        renderText(renderer, font, zoomText, 590, 475, "GREEN");
    }
}

void renderAQIndicator(SDL_Renderer *renderer, TTF_Font *font,int autoTargetAcquisitionActive, int currentMapTexture){
    char temp[50];

    if (autoTargetAcquisitionActive) {
        snprintf(temp, sizeof(temp), "AUTO AQ");
    }else{
        snprintf(temp, sizeof(temp), "MANUAL AQ");

    }
    if(currentMapTexture == 1){
        renderText(renderer, font, temp, 30, 550, "WHITE");

    }else{
        renderText(renderer, font, temp, 30, 550, "GREEN");
    }
}

void renderRadarIndicator(SDL_Renderer *renderer,TTF_Font *font, int radarActive, int currentMapTexture){
    char temp[50];
    if (radarActive) {
        snprintf(temp, sizeof(temp), "RADAR ON");
    }else{
        snprintf(temp, sizeof(temp), "RADAR OFF");
    }    

    if(currentMapTexture == 1){
        renderText(renderer, font, temp, 30, 50, "WHITE");

    }else{
        renderText(renderer, font, temp, 30, 50, "GREEN");
    }
}

void renderPayloadStatus(SDL_Renderer *renderer,TTF_Font *font,char *payloadStatus, int currentMapTexture){
    char temp[50];
    if (strcmp(payloadStatus, "READY")==0) {
        snprintf(temp, sizeof(temp), "READY");
        
    }else{
        snprintf(temp, sizeof(temp), "DISARMED");
    }
    if(currentMapTexture == 1){
        renderText(renderer, font, temp, 30, 525, "WHITE");

    }else{
        renderText(renderer, font, temp, 30, 525, "GREEN");
    }

}

void renderCurrentDate(SDL_Renderer *renderer, TTF_Font *font, int currentMapTexture){
    char date[50];
    time_t tm;
    time(&tm);
    snprintf(date, sizeof(date), ctime(&tm));
    if(currentMapTexture == 1){
        renderText(renderer, font, date, 30, 25, "WHITE");

    }else{
        renderText(renderer, font, date, 30, 25, "GREEN");
    }
}

void renderPayloadType(SDL_Renderer *renderer,TTF_Font *font,char *payloadType, int currentMapTexture){
    char missileTypeText[50];
    if(strcmp(payloadType, "AT") == 0){
        snprintf(missileTypeText, sizeof(missileTypeText), "PAYLOAD: AT");
    }else{
        snprintf(missileTypeText, sizeof(missileTypeText), "PAYLOAD: AP");
    }
    if(currentMapTexture == 1){
        renderText(renderer, font, missileTypeText, 30, 500, "WHITE");

    }else{
        renderText(renderer, font, missileTypeText, 30, 500, "GREEN");

    }
}

void renderTargetVelocity(SDL_Renderer *renderer,TTF_Font *font,float targetVelocity, int currentMapTexture){
    char targetVelocityText[50];
    snprintf(targetVelocityText, sizeof(targetVelocityText), "TARGET VEL: %.2f", targetVelocity);
    if(currentMapTexture == 1){
        renderText(renderer, font, targetVelocityText, 30, 475, "WHITE");
    }else{
        renderText(renderer, font, targetVelocityText, 30, 475, "GREEN");
    }
}

void generateDynamicNoiseTexture(SDL_Renderer *renderer, SDL_Texture **noiseTexture) {

    // Generate a random noise intensity between 0.1 and 0.2
    float intensity = 0.04f + (float)rand() / RAND_MAX * 0.04f;

    SDL_Surface *noiseSurface = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0xFF, 0xFF00, 0xFF0000, 0xFF000000);

    // Generate dynamic noise pixels
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            Uint8 randomValue = rand() % 256;
            Uint32 pixelColor = SDL_MapRGBA(noiseSurface->format, randomValue, randomValue, randomValue, (Uint8)(intensity * 255));
            *((Uint32 *)noiseSurface->pixels + y * SCREEN_WIDTH + x) = pixelColor;
        }
    }

    // Create a texture from the noise surface
    *noiseTexture = SDL_CreateTextureFromSurface(renderer, noiseSurface);

    // Clean up the surface
    SDL_FreeSurface(noiseSurface);
}

void renderRadarInterface(SDL_Renderer *renderer,Vehicle *vehicles, SDL_Texture *radarTexture, SDL_Texture *radarTextureIR, SDL_Texture *blipTexture, int numVehicles, float cameraX, float cameraY){

    int radarWidth = 200;
    int radarHeight = 200;
    int radarCenterX = 10 + radarWidth / 2;
    int radarCenterY = 10 + radarHeight / 2;
    int blipWidth = 10;
    int blipHeight = 10;
    float radarScaleX= 150/6246;
    float radarScaleY = 150/6215;

    SDL_Rect radarBackgroundRect = {30, 85, radarWidth, radarHeight};

    // Render Radar Background
    SDL_RenderCopy(renderer, radarTexture, NULL, &radarBackgroundRect);

    // Render Blip points
    for (int i = 0; i < numVehicles; i++) {
        float relativeX = vehicles[i].x - (cameraX + SCREEN_WIDTH/2);
        float relativeY = vehicles[i].y - (cameraY + SCREEN_WIDTH/2);

        // Scale the relative positions to fit within the radar
        int blipX = radarCenterX + (int)(relativeX * radarScaleX);
        int blipY = radarCenterY + (int)(relativeY * radarScaleY);

        SDL_Rect blipRect = {blipX, blipY, blipWidth, blipHeight};
        SDL_RenderCopy(renderer, blipTexture, NULL, &blipRect);
    }
}






    


















