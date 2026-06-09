
#include "game_ext.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "game_aux.h"
#include "game_struct.h"
#include "queue.h"

game game_new_ext(uint nb_rows, uint nb_cols, shape* shapes,
                  direction* orientations, bool wrapping) {
  // Utilisation de calloc pour allouer et initialiser toute la structure à zéro
  game g = calloc(1, sizeof(struct game_s));
  if (!g) {
    fprintf(stderr, "Échec de l'allocation mémoire dans game_new_ext\n");
    exit(EXIT_FAILURE);
  }

  g->width = nb_cols;
  g->height = nb_rows;
  g->wrapping = wrapping;

  // Allocation et initialisation à zéro des tableaux de formes et orientations
  g->shapes = calloc(nb_rows * nb_cols, sizeof(shape));
  g->orientations = calloc(nb_rows * nb_cols, sizeof(direction));
  if (!g->shapes || !g->orientations) {
    fprintf(stderr, "Échec de l'allocation mémoire dans game_new_ext\n");
    free(g->shapes);
    free(g->orientations);
    free(g);
    exit(EXIT_FAILURE);
  }

  // Copie des valeurs fournies ou initialisation par défaut
  for (uint i = 0; i < nb_rows * nb_cols; i++) {
    g->shapes[i] = (shapes == NULL) ? EMPTY : shapes[i];
    g->orientations[i] = (orientations == NULL) ? NORTH : orientations[i];
  }

  g->alloc_shapes = true;
  g->alloc_orientations = true;

  // Initialisation des piles undo_stack et redo_stack
  g->undo_stack = queue_new();
  g->redo_stack = queue_new();

  return g;
}

game game_new_empty_ext(uint nb_rows, uint nb_cols, bool wrapping) {
  return game_new_ext(nb_rows, nb_cols, NULL, NULL, wrapping);
}

bool game_is_wrapping(cgame g) {
  assert(g != NULL);
  return g->wrapping;
}

uint game_nb_cols(cgame g) {
  if (!g) {
    fprintf(stderr, "Erreur: game_nb_cols appelé avec un pointeur NULL\n");
    exit(EXIT_FAILURE);
}
  assert(g != NULL);
  return g->width;
}

uint game_nb_rows(cgame g) {
  if (!g) {
    fprintf(stderr, "Erreur: game_nb_rows appelé avec un pointeur NULL\n");
    exit(EXIT_FAILURE);
}
  assert(g != NULL);
  return g->height;
}

void game_undo(game g) {
  if (g == NULL) {
    fprintf(stderr, "Erreur : tentative de undo sur un jeu NULL\n");
    return;
  }

  if (queue_is_empty(g->undo_stack)) {
    return;  // Rien à annuler
  }

  // Extraire le dernier mouvement de la pile undo
  move_t* last_move = queue_pop_tail(g->undo_stack);
  if (last_move == NULL) {
    fprintf(stderr, "Erreur : tentative d'extraction d'un mouvement NULL\n");
    return;
  }

  // Calculer l'inverse du mouvement
  uint inverse_turns = (4 - last_move->nb_quarter_turns) % 4;

  // Appliquer l'inverse au plateau
  uint index = last_move->i * g->width + last_move->j;
  g->orientations[index] = (g->orientations[index] + inverse_turns) % 4;

  // Créer un mouvement pour la pile redo
  move_t* redo_move = malloc(sizeof(move_t));
  if (redo_move == NULL) {
    fprintf(stderr, "Erreur : échec de l'allocation mémoire pour redo_move\n");
    free(last_move);
    return;
  }
  *redo_move = *last_move;

  // Ajouter à la pile redo
  queue_push_tail(g->redo_stack, redo_move);

  // Libérer le mouvement extrait
  free(last_move);
}

void game_redo(game g) {
  if (g == NULL) {
    fprintf(stderr, "Erreur : tentative de redo sur un jeu NULL\n");
    return;
  }

  if (queue_is_empty(g->redo_stack)) {
    return;  // Rien à rejouer
  }

  // Extraire le dernier mouvement de la pile redo
  move_t* last_move = queue_pop_tail(g->redo_stack);
  if (last_move == NULL) {
    fprintf(stderr, "Erreur : tentative d'extraction d'un mouvement NULL\n");
    return;
  }

  // Appliquer le mouvement au plateau
  uint index = last_move->i * g->width + last_move->j;
  g->orientations[index] =
      (g->orientations[index] + last_move->nb_quarter_turns) % 4;

  // Créer un mouvement pour la pile undo
  move_t* undo_move = malloc(sizeof(move_t));
  if (undo_move == NULL) {
    fprintf(stderr, "Erreur : échec de l'allocation mémoire pour undo_move\n");
    free(last_move);
    return;
  }
  *undo_move = *last_move;

  // Ajouter à la pile undo
  queue_push_tail(g->undo_stack, undo_move);

  // Libérer le mouvement extrait
  free(last_move);
}

