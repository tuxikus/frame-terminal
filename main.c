#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <util.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

#include <SDL3/SDL.h>

#include "font.c"

#define APP_NAME "Frame Terminal"
#define WIDTH 1280
#define HEIGHT 800
#define DEBUG_WIDTH 640
#define DEBUG_HEIGHT 400
#define BG_COLOR 0x000000FF
#define FG_COLOR 0xFFFFFFFF

typedef struct frameTerminal {
        uint32_t     *fb;
        char         textBuffer[4096];
        uint32_t     cursorX;
        uint32_t     cursorY;
        bool         running;

        SDL_Window   *window;
        SDL_Window   *debugWindow;
        SDL_Renderer *renderer;
        SDL_Texture  *texture;
} frameTerminal;

SDL_Texture* textureInit(SDL_Renderer *renderer, uint32_t width, uint32_t height)
{
        SDL_Texture *texture = SDL_CreateTexture(
                renderer,
                SDL_PIXELFORMAT_ARGB8888,
                SDL_TEXTUREACCESS_STREAMING,
                width,
                height
                );

        if (!texture) {
                SDL_Log("Failed to create texture: %s", SDL_GetError());

                // TODO
                /* SDL_DestroyRenderer(renderer); */
                /* SDL_DestroyWindow(window); */
                /* SDL_Quit(); */

                return NULL;
        }
        
        return texture;
}

SDL_Renderer* rendererInit(SDL_Window *window)
{
        SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);

        if (!renderer) {
                SDL_Log("Failed to create renderer: %s", SDL_GetError());
                
                // TODO
                /* SDL_DestroyWindow(window); */
                /* SDL_Quit(); */
                
                return NULL;
        }

        return renderer;
}

SDL_Window* windowInit(char *name, uint32_t width, uint32_t height)
{
        SDL_Window *window = SDL_CreateWindow(
                name,
                width,
                height,
                0
                );

        if (!window) {
                SDL_Log("Failed to create window: %s", SDL_GetError());
                SDL_Quit();
                return NULL;
        }

        SDL_StartTextInput(window);

        return window;
}


/* TODO: */
/* if something fails call these: */
/* SDL_DestroyTexture(texture); */
/* SDL_DestroyRenderer(renderer); */
/* SDL_DestroyWindow(window); */
/* SDL_Quit(); */
/* 1. init framebuffer */
/* 2. init SDL_Window */
/* 3. init SDL_Renderer */
/* 4. init SDL_Texture */
int frameTerminalInit(frameTerminal *ft)
{
        ft->running = true;
        uint32_t fbSize = WIDTH * HEIGHT * sizeof(uint32_t);
        ft->fb = malloc(fbSize);
        SDL_Log("framebuffer created with %d pixel\n", fbSize);
        
        SDL_Window *window = windowInit(APP_NAME, WIDTH, HEIGHT);
        if (window == NULL) {
                return -1;
        }
        ft->window = window;

#ifdef DEBUG
        SDL_Window *debugWindow = windowInit("[DEBUG]", DEBUG_WIDTH, DEBUG_HEIGHT);
        if (window == NULL) {
                return -1;
        }
        ft->debugWindow = debugWindow;
#endif

        SDL_Renderer *renderer = rendererInit(ft->window);
        if (renderer == NULL) {
                return -1;
        }
        ft->renderer = renderer;

        SDL_Texture *texture = textureInit(ft->renderer, WIDTH, HEIGHT);
        if (texture == NULL) {
                return -1;
        }
        ft->texture = texture;
        
        SDL_Log("initinitialization done!\n");
        return 0;
}

void frameTerminalDestroy(frameTerminal *ft)
{

}


int cursorX = 0;
int cursorY = 0;

void drawBG(uint32_t *framebuffer)
{
        for (int i = 0; i < WIDTH * HEIGHT; i++)
                framebuffer[i] = BG_COLOR;
}

uint8_t* doubleBitmapScaling(uint8_t *input, uint8_t *output)
{
        int i = 0;
        int col = 0;
        int row = 0;

        for (int bit = 0; bit < 64; bit++) {
                uint8_t pixel = input[bit];
                output[row + col] = pixel;
                output[row + col + 1] = pixel;
                output[row + 16 + col] = pixel;
                output[row + 16 + col + 1] = pixel;
                col += 2;
                i += 1;
                if (i == 8) {
                        i = 0;
                        col = 0;
                        row += 32;
                }
        }

        return output;
}

void framebufferAppend(uint32_t *framebuffer, char* text, uint32_t textLength, bool doubleScaling)
{
        for (int i = 0; i < textLength; i++) {
                uint8_t* characterBitmap = getCharacterBitmap(text[i]);
                int characterBitmapDimension;
  
                if (doubleScaling) {
                        uint8_t scaledCharacterBitmap[16*16];
                        doubleBitmapScaling(characterBitmap, scaledCharacterBitmap);
                        characterBitmap = scaledCharacterBitmap;
                        characterBitmapDimension = 16;
                } else {
                        characterBitmapDimension = 8;
                }

                if (text[i] == '\n') {
                        cursorY += characterBitmapDimension;
                        cursorX = 0;
                }
      
                if (text[i] < 32 || text[i] > 126)
                        continue;
  
                for (int row = 0; row < characterBitmapDimension; row++) {
                        for (int col = 0; col < characterBitmapDimension; col++) {
                                if (characterBitmap[row * characterBitmapDimension + col])
                                        framebuffer[(cursorY + row) * WIDTH + (cursorX + col)] = FG_COLOR;
                        }
                }
  
                cursorX += characterBitmapDimension;
                if (cursorX + characterBitmapDimension > WIDTH) {
                        cursorX = 0;
                        cursorY += characterBitmapDimension;
                }      
        }
}

int main(void)
{
        if (!SDL_Init(SDL_INIT_VIDEO)) {
                SDL_Log("SDL_Init failed: %s", SDL_GetError());
                return 1;
        }

        frameTerminal ft = {0};
        frameTerminalInit(&ft);
        
        int master_fd;
        struct termios term;
        tcgetattr(STDIN_FILENO, &term);

        term.c_lflag |= ECHO | ICANON;
        term.c_cc[VERASE] = 0x7f;   // DEL (Backspace)

        pid_t pid = forkpty(&master_fd, NULL, &term, NULL);
        fcntl(master_fd, F_SETFL, O_NONBLOCK);

        if (pid == -1) {
                perror("forkpty");
                exit(1);
        }

        if (pid == 0) {
                execlp("sh", "sh", NULL);
                perror("execlp");
                exit(1);
        }
    
        while (ft.running) {
                ssize_t n = read(master_fd, ft.textBuffer, sizeof(ft.textBuffer));

                if (n > 0) {
                        framebufferAppend(ft.fb, ft.textBuffer, n, false);
                }

                SDL_Event event;
                while (SDL_PollEvent(&event)) {
                        if (event.type == SDL_EVENT_QUIT)
                                ft.running = false;

                        if (event.type == SDL_EVENT_KEY_DOWN) {
                                if (event.key.key == SDLK_RETURN)
                                        write(master_fd, "\r", 1);
                                else if (event.key.key == SDLK_TAB)
                                        write(master_fd, "\t", 1);
                                else if (event.key.key == SDLK_BACKSPACE) 
                                        write(master_fd, "\177", 1);
                        }
                        else if (event.type == SDL_EVENT_TEXT_INPUT) {
                                write(master_fd, event.text.text, strlen(event.text.text));
                        }
                }

                SDL_UpdateTexture(
                        ft.texture,
                        NULL,
                        ft.fb,
                        WIDTH * sizeof(uint32_t)
                        );

                SDL_RenderClear(ft.renderer);
                SDL_RenderTexture(ft.renderer, ft.texture, NULL, NULL);
                SDL_RenderPresent(ft.renderer);  
        }

        free(ft.fb);

        SDL_DestroyTexture(ft.texture);
        SDL_DestroyRenderer(ft.renderer);
        SDL_DestroyWindow(ft.window);

        SDL_Quit();

        frameTerminalDestroy(&ft);

        return 0;
}
