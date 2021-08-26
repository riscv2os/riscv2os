#include "my_sdl.h"

static SDL_Window *window = NULL;
static SDL_Renderer *renderer;
static SDL_Texture *texture;

#define width 640
#define height 480
uint32_t screen[width*height];

bool update_screen(void *pixels) {
	uint8_t r = 0xFF, g = 0x00, b = 0xFF;  // 紅+藍=粉紅
	uint32_t color = r << 16 | g << 8 | b; // color=粉紅
	// 顯示單一顏色
	uint32_t *d = pixels;
	for (size_t y = 0; y < height; ++y) {
		for (size_t x = 0; x < width; ++x) {
			d[x] = color;
		}
		d += width;
	}
}

int main(int argc, char* args[])
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		fprintf(stderr, "Failed to call SDL_Init()\n");
		exit(1);
	}

	window = SDL_CreateWindow("memory renderer", SDL_WINDOWPOS_UNDEFINED,
								SDL_WINDOWPOS_UNDEFINED, width, height,
								0 /* flags */);
	if (!window) {
		fprintf(stderr, "Window could not be created! SDL_Error: %s\n",
				SDL_GetError());
		exit(1);
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
								SDL_TEXTUREACCESS_STREAMING, width, height);

	bool quit = false;
    SDL_Event event;
	while (!quit) {
		while (SDL_PollEvent(&event)) { /* Run event handler */
			switch (event.type) {
			case SDL_QUIT:
            	quit = true;
			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_ESCAPE) {
            		quit = true;
					break;
				}
			}
		}

    	void *pixels_ptr = screen;
		int pitch = 0;
		if (SDL_LockTexture(texture, NULL, &pixels_ptr, &pitch))
			exit(-1);

		update_screen(pixels_ptr);

		SDL_UnlockTexture(texture);

		SDL_RenderCopy(renderer, texture, NULL, &(SDL_Rect){0, 0, width, height});
		SDL_RenderPresent(renderer);
		// SDL_Delay(200);
		// SDL_UpdateWindowSurface(window);
	}

	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
