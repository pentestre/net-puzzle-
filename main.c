// SDL2 Demo by aurelien.esnard@u-bordeaux.fr

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <stdbool.h>
#include <stdio.h>

#include "game.h"
#include "game_aux.h"
#include "game_ext.h"
#include "game_struct.h"
#include "game_tools.h"
#include "model.h"  
#include "queue.h"

#ifndef APP_NAME
#define APP_NAME "Game SDL"
#endif

#ifndef SCREEN_WIDTH
#define SCREEN_WIDTH 800
#endif
#ifndef SCREEN_HEIGHT
#define SCREEN_HEIGHT 600
#endif
#ifndef DELAY
#define DELAY 16
#endif

int main(int argc, char* argv[]) {
  // Initialisation de SDL2 et des extensions
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    fprintf(stderr, "Erreur : SDL_Init VIDEO (%s)\n", SDL_GetError());
    return EXIT_FAILURE;
  }

  if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) != IMG_INIT_PNG) {
    fprintf(stderr, "Erreur : IMG_Init PNG (%s)\n", SDL_GetError());
    SDL_Quit();
    return EXIT_FAILURE;
  }

  if (TTF_Init() != 0) {
    fprintf(stderr, "Erreur : TTF_Init (%s)\n", SDL_GetError());
    IMG_Quit();
    SDL_Quit();
    return EXIT_FAILURE;
  }

  // Création de la fenêtre
  SDL_Window* win = SDL_CreateWindow(
      APP_NAME, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
      SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
  if (!win) {
    fprintf(stderr, "Erreur : SDL_CreateWindow (%s)\n", SDL_GetError());
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    return EXIT_FAILURE;
  }

  // Création du renderer
  SDL_Renderer* ren = SDL_CreateRenderer(
      win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (!ren) {
    fprintf(stderr, "Erreur : SDL_CreateRenderer (%s)\n", SDL_GetError());
    SDL_DestroyWindow(win);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    return EXIT_FAILURE;
  }

  // Initialisation de l'environnement
  Env* env = init(win, ren, argc, argv);
  if (!env) {
    fprintf(stderr, "Erreur lors de l'initialisation de l'environnement\n");
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    return EXIT_FAILURE;
  }

  // Boucle principale
  SDL_Event e;
  bool quit = false;
  while (!quit) {
    // Gestion des événements
    while (SDL_PollEvent(&e)) {
      quit = process(win, ren, env, &e);  // Appel de la fonction process
    }

    // Fond gris clair
    SDL_SetRenderDrawColor(ren, 0xA0, 0xA0, 0xA0, 0xFF);  // Gris clair
    SDL_RenderClear(ren);

    // Rendu des éléments du jeu
    render(win, ren, env);

    SDL_RenderPresent(ren);  // Présentation du rendu
    SDL_Delay(DELAY);        // Régulation de la vitesse (~60 FPS)
  }

  // Nettoyage et libération des ressources
  clean(win, ren, env);

  SDL_DestroyRenderer(ren);
  SDL_DestroyWindow(win);
  TTF_Quit();
  IMG_Quit();
  SDL_Quit();

  return EXIT_SUCCESS;
}
