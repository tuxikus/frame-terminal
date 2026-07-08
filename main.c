#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <SDL3/SDL.h>

#include "font.c"

#define APP_NAME "terminal"
#define WIDTH 800
#define HEIGHT 400
#define CHAR_WIDTH 8
#define CHAR_HEIGHT 8
#define BG_COLOR 0x000000FF
#define FG_COLOR 0xFFFFFFFF

uint8_t* doubleBitmapScaling(uint8_t *input, uint8_t *output)
{
  int i = 0;
  int col = 0;
  int row = 0;
  for (int bit = 0; bit < 64; bit++)
    {
      uint8_t pixel = input[bit];
      output[row + col] = pixel;
      output[row + col + 1] = pixel;
      output[row + 16 + col] = pixel;
      output[row + 16 + col + 1] = pixel;
      col += 2;
      i += 1;
      if (i == 8)
	{
	  i = 0;
	  col = 0;
	  row += 32;
	}
    }

  return output;
}

void printBitmap(uint8_t* bitmap, uint32_t size)
{
  printf("===================================\n");
  for (int i = 0; i < size*size; i++)
    {
      printf("%d", bitmap[i]);
      if ((i + 1) % size == 0)
	printf("\n");
    }
  printf("===================================\n");
}

void drawBG(uint32_t *framebuffer)
{
  for (int i = 0; i < WIDTH * HEIGHT; i++)
    framebuffer[i] = BG_COLOR;
}

int cursorX = 0;
int cursorY = 0;
void framebufferAppend(uint32_t *framebuffer, char* text, uint32_t textLength, bool doubleScaling)
{
  for (int i = 0; i < textLength; i++)
    {
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
  
      for (int row = 0; row < characterBitmapDimension; row++)
	{
	  for (int col = 0; col < characterBitmapDimension; col++)
	    {
	      if (characterBitmap[row * characterBitmapDimension + col])
		framebuffer[(cursorY + row) * WIDTH + (cursorX + col)] = FG_COLOR;
	    }
	}
  
      cursorX += characterBitmapDimension;
      if (cursorX + characterBitmapDimension > WIDTH)
	{
	  cursorX = 0;
	  cursorY += characterBitmapDimension;
	}      
    }

}

int main(void)
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow(
        APP_NAME,
        WIDTH,
        HEIGHT,
        0
    );

    if (!window)
    {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);

    if (!renderer)
    {
        SDL_Log("Failed to create renderer: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_Texture *texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        WIDTH,
        HEIGHT
    );

    if (!texture)
    {
        SDL_Log("Failed to create texture: %s", SDL_GetError());

        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();

        return 1;
    }

    uint32_t framebufferSize = WIDTH * HEIGHT * sizeof(uint32_t);
    uint32_t *framebuffer = malloc(framebufferSize);
    printf("framebuffer size: %d\n", framebufferSize);

    if (!framebuffer)
    {
        SDL_Log("Out of memory");

        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();

        return 1;
    }

    bool running = true;
    drawBG(framebuffer);
    while (running)
    {
	
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
                running = false;

	    if (event.type == SDL_EVENT_KEY_DOWN)
	      {
		if (event.key.key == SDLK_ESCAPE)
		  running = !running;
		
		if (event.key.key == SDLK_A)
		  framebufferAppend(framebuffer, "AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz0123456789!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~", 94, true);
	      }
        }

        SDL_UpdateTexture(
            texture,
            NULL,
            framebuffer,
            WIDTH * sizeof(uint32_t)
        );

        SDL_RenderClear(renderer);
        SDL_RenderTexture(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }

    free(framebuffer);

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}
