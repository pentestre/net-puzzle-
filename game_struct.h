#ifndef GAME_STRUCT_H
#define GAME_STRUCT_H

#include "game.h"
#include "queue.h"

// Structure pour représenter un mouvement dans le jeu
typedef struct move_s {
  uint i, j;
  int nb_quarter_turns;
} move_t;

// Structure principale du jeu
struct game_s {
  uint width;               // Largeur de la grille de jeu
  uint height;              // Hauteur de la grille de jeu
  shape* shapes;            // Tableau des formes des pièces
  direction* orientations;  // Tableau des orientations des pièces
  bool wrapping;            // Indicateur du mode de bordure (wrapping)
  queue* undo_stack;        // Pile des mouvements à annuler
  queue* redo_stack;        // Pile des mouvements à rejouer
  bool alloc_shapes;  // Indicateur de l'allocation de mémoire pour shapes
  bool alloc_orientations;  // Indicateur de l'allocation de mémoire pour
                            // orientations
};

#endif  // GAME_STRUCT_H
