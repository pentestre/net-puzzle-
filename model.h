// SDL2 Model by aurelien.esnard@u-bordeaux.fr

#ifndef MODEL_H
#define MODEL_H

#include <SDL.h>
#include <stdbool.h>
#include <stdio.h>

#include "game.h"
#include "game_aux.h"
#include "game_ext.h"
#include "game_struct.h"
#include "game_tools.h"
#include "queue.h"


#define BUTTON_COUNT 6
#define BUTTON_WIDTH 120
#define BUTTON_HEIGHT 40
#define BUTTON_SPACING 20
#define TOOLBAR_HEIGHT 50
#define TOOLBAR_PADDING 10

#define NB_SHAPES 6

/* Définition de la structure Env_t */
typedef struct Env_t {
  SDL_Texture* background;
  SDL_Texture* pieces[6];
  SDL_Texture* button_textures[BUTTON_COUNT];
  
  game current_game;
  game initial_game; // Ajouté pour stocker l'état initial
  int grid_width, grid_height;
  int cell_size;
  
  int hovered_button;
  int window_width;
  int window_height;
} Env;

/* **************************************************************** */

#ifdef __ANDROID__
#define PRINT(STR, ...)          \
  do {                           \
    SDL_Log(STR, ##__VA_ARGS__); \
  } while (0)
#define ERROR(STR, ...)          \
  do {                           \
    SDL_Log(STR, ##__VA_ARGS__); \
    exit(EXIT_FAILURE);          \
  } while (0)
#else
#define PRINT(STR, ...)         \
  do {                          \
    printf(STR, ##__VA_ARGS__); \
  } while (0)
#define ERROR(STR, ...)                  \
  do {                                   \
    fprintf(stderr, STR, ##__VA_ARGS__); \
    exit(EXIT_FAILURE);                  \
  } while (0)
#endif


/* **************************************************************** */

Env* init(SDL_Window* win, SDL_Renderer* ren, int argc, char* argv[]);
void render(SDL_Window* win, SDL_Renderer* ren, Env* env);
void clean(SDL_Window* win, SDL_Renderer* ren, Env* env);
bool process(SDL_Window* win, SDL_Renderer* ren, Env* env, SDL_Event* e);

/* **************************************************************** */

#endif