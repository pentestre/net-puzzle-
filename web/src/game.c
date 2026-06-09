#include "game.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game_aux.h"
#include "game_ext.h"
#include "game_struct.h"
#include "queue.h"

// Fonction de création d'un jeu vide en utilisant DEFAULT_SIZE
game game_new_empty(void) {
  // Utilisation de calloc pour initialiser tous les octets de la structure à
  // zéro
  game g = calloc(1, sizeof(struct game_s));
  if (!g) {
    fprintf(stderr, "Échec de l'allocation mémoire dans game_new_empty\n");
    exit(EXIT_FAILURE);
  }

  // Initialisation des dimensions
  g->width = DEFAULT_SIZE;
  g->height = DEFAULT_SIZE;

  // Allocation et initialisation (à zéro) des tableaux shapes et orientations
  g->shapes = calloc(DEFAULT_SIZE * DEFAULT_SIZE, sizeof(shape));
  g->orientations = calloc(DEFAULT_SIZE * DEFAULT_SIZE, sizeof(direction));
  if (!g->shapes || !g->orientations) {
    fprintf(stderr, "Échec de l'allocation mémoire dans game_new_empty\n");
    free(g->shapes);
    free(g->orientations);
    free(g);
    exit(EXIT_FAILURE);
  }

  // Indique que shapes et orientations ont été alloués dynamiquement
  g->alloc_shapes = true;
  g->alloc_orientations = true;

  // Initialisation des piles undo et redo via queue_new()
  g->undo_stack = queue_new();
  g->redo_stack = queue_new();

  return g;
}

// Fonction de création d'un jeu à partir de tableaux fournis (pour un jeu par
// défaut)
game game_new(shape *shapes, direction *orientations) {
  // Utilisation de calloc pour initialiser toute la structure à zéro
  game g = calloc(1, sizeof(struct game_s));
  if (!g) {
    fprintf(stderr, "Échec de l'allocation mémoire dans game_new\n");
    exit(EXIT_FAILURE);
  }

  // Initialisation des dimensions (ici DEFAULT_SIZE)
  g->width = DEFAULT_SIZE;
  g->height = DEFAULT_SIZE;

  // Allocation des tableaux shapes et orientations avec initialisation à zéro
  g->shapes = calloc(DEFAULT_SIZE * DEFAULT_SIZE, sizeof(shape));
  g->orientations = calloc(DEFAULT_SIZE * DEFAULT_SIZE, sizeof(direction));
  if (!g->shapes || !g->orientations) {
    fprintf(stderr, "Échec de l'allocation mémoire dans game_new\n");
    free(g->shapes);
    free(g->orientations);
    free(g);
    exit(EXIT_FAILURE);
  }

  // Copie des valeurs fournies ou initialisation par défaut si NULL
  for (uint i = 0; i < DEFAULT_SIZE * DEFAULT_SIZE; i++) {
    g->shapes[i] = (shapes == NULL) ? EMPTY : shapes[i];
    g->orientations[i] = (orientations == NULL) ? NORTH : orientations[i];
  }

  // Indique que shapes et orientations ont été alloués dynamiquement
  g->alloc_shapes = true;
  g->alloc_orientations = true;

  // Initialisation des piles undo et redo
  g->undo_stack = queue_new();
  g->redo_stack = queue_new();

  return g;
}

game game_copy(cgame g) {
  if (!g) {
    fprintf(stderr, "game_copy called with a null game\n");
    return NULL;
  }

  game new_game = malloc(sizeof(struct game_s));
  if (!new_game) {
    fprintf(stderr, "Memory allocation failed in game_copy\n");
    return NULL;
  }

  new_game->width = g->width;
  new_game->height = g->height;
  new_game->wrapping = g->wrapping;

  new_game->shapes = malloc(new_game->width * new_game->height * sizeof(shape));
  new_game->orientations =
      malloc(new_game->width * new_game->height * sizeof(direction));
  new_game->alloc_orientations = true;
  new_game->alloc_shapes = true;

  if (!new_game->shapes || !new_game->orientations) {
    fprintf(stderr, "Memory allocation failed in game_copy\n");
    free(new_game->shapes);
    free(new_game->orientations);
    free(new_game);
    return NULL;
  }

  for (uint i = 0; i < new_game->height * new_game->width; i++) {
    new_game->shapes[i] = g->shapes[i];
    new_game->orientations[i] = g->orientations[i];
  }

  new_game->undo_stack = queue_new();
  new_game->redo_stack = queue_new();

  if (!new_game->undo_stack || !new_game->redo_stack) {
    fprintf(stderr, "Memory allocation for stacks failed in game_copy\n");
    free(new_game->shapes);
    free(new_game->orientations);
    queue_free(new_game->undo_stack);
    queue_free(new_game->redo_stack);
    free(new_game);
    return NULL;
  }

  return new_game;
}

bool game_equal(cgame g1, cgame g2, bool ignore_orientation) {
  // Vérifier si l'un des jeux est NULL
  if (g1 == NULL || g2 == NULL) {
    fprintf(stderr, "game_equal appelé avec un jeu NULL\n");
    return false;
  }

  // Vérifier les dimensions et l'option wrapping
  if (g1->width != g2->width || g1->height != g2->height) {
    fprintf(stderr, "Les dimensions des jeux ne correspondent pas\n");
    return false;
  }
  if (g1->wrapping != g2->wrapping) {
    fprintf(stderr, "Les options de wrapping ne correspondent pas\n");
    return false;
  }

  // Comparaison des cellules (formes et, si nécessaire, orientations)
  uint total = g1->width * g1->height;
  for (uint idx = 0; idx < total; idx++) {
    if (g1->shapes[idx] != g2->shapes[idx]) {
      fprintf(stderr, "Les formes ne correspondent pas à l'indice %u\n", idx);
      return false;
    }
    if (!ignore_orientation && g1->orientations[idx] != g2->orientations[idx]) {
      fprintf(stderr, "Les orientations ne correspondent pas à l'indice %u\n",
              idx);
      return false;
    }
  }

  // Vérifier la longueur des piles undo_stack et redo_stack
  if (queue_length(g1->undo_stack) != queue_length(g2->undo_stack)) {
    fprintf(stderr,
            "Les longueurs des piles undo_stack ne correspondent pas\n");
    return false;
  }
  if (queue_length(g1->redo_stack) != queue_length(g2->redo_stack)) {
    fprintf(stderr,
            "Les longueurs des piles redo_stack ne correspondent pas\n");
    return false;
  }

  // Comparer les éléments des piles undo_stack
  queue *temp_undo1 = queue_new();
  queue *temp_undo2 = queue_new();
  while (!queue_is_empty(g1->undo_stack) && !queue_is_empty(g2->undo_stack)) {
    move_t *move1 = queue_pop_tail(g1->undo_stack);
    move_t *move2 = queue_pop_tail(g2->undo_stack);
    if (move1->i != move2->i || move1->j != move2->j ||
        move1->nb_quarter_turns != move2->nb_quarter_turns) {
      fprintf(stderr, "Les mouvements dans undo_stack ne correspondent pas\n");
      // Remettre les éléments extraits dans les piles avant de retourner false
      queue_push_tail(g1->undo_stack, move1);
      queue_push_tail(g2->undo_stack, move2);
      queue_free(temp_undo1);
      queue_free(temp_undo2);
      return false;
    }
    queue_push_tail(temp_undo1, move1);
    queue_push_tail(temp_undo2, move2);
  }
  // Restaurer les piles undo_stack
  while (!queue_is_empty(temp_undo1))
    queue_push_tail(g1->undo_stack, queue_pop_tail(temp_undo1));
  while (!queue_is_empty(temp_undo2))
    queue_push_tail(g2->undo_stack, queue_pop_tail(temp_undo2));
  queue_free(temp_undo1);
  queue_free(temp_undo2);

  // Comparer les éléments des piles redo_stack
  queue *temp_redo1 = queue_new();
  queue *temp_redo2 = queue_new();
  while (!queue_is_empty(g1->redo_stack) && !queue_is_empty(g2->redo_stack)) {
    move_t *move1 = queue_pop_tail(g1->redo_stack);
    move_t *move2 = queue_pop_tail(g2->redo_stack);
    if (move1->i != move2->i || move1->j != move2->j ||
        move1->nb_quarter_turns != move2->nb_quarter_turns) {
      fprintf(stderr, "Les mouvements dans redo_stack ne correspondent pas\n");
      // Restaurer les éléments avant de retourner false
      queue_push_tail(g1->redo_stack, move1);
      queue_push_tail(g2->redo_stack, move2);
      queue_free(temp_redo1);
      queue_free(temp_redo2);
      return false;
    }
    queue_push_tail(temp_redo1, move1);
    queue_push_tail(temp_redo2, move2);
  }
  // Restaurer les piles redo_stack
  while (!queue_is_empty(temp_redo1))
    queue_push_tail(g1->redo_stack, queue_pop_tail(temp_redo1));
  while (!queue_is_empty(temp_redo2))
    queue_push_tail(g2->redo_stack, queue_pop_tail(temp_redo2));
  queue_free(temp_redo1);
  queue_free(temp_redo2);

  return true;
}

void game_delete(game g) {
  if (g == NULL) {
    fprintf(stderr, "Erreur : tentative de suppression d'un jeu NULL\n");
    return;
  }

  // Libération des tableaux de formes et orientations si alloués dynamiquement
  if (g->alloc_shapes) {
    if (g->shapes != NULL) {
      free(g->shapes);
    } else {
      fprintf(stderr, "Erreur : 'shapes' est NULL mais marqué comme alloué\n");
    }
  }
  if (g->alloc_orientations) {
    if (g->orientations != NULL) {
      free(g->orientations);
    } else {
      fprintf(stderr,
              "Erreur : 'orientations' est NULL mais marqué comme alloué\n");
    }
  }

  // Libération des piles undo_stack et redo_stack
  if (g->undo_stack != NULL) {
    queue_free(g->undo_stack);  // Assure-toi que queue_free libère aussi les
                                // éléments si nécessaire
  } else {
    fprintf(stderr, "Aucune pile undo_stack à libérer\n");
  }
  if (g->redo_stack != NULL) {
    queue_free(g->redo_stack);
  } else {
    fprintf(stderr, "Aucune pile redo_stack à libérer\n");
  }

  // Libération de la structure du jeu
  free(g);
}

// Affecte la forme d'une pièce à la position (i, j) du jeu
void game_set_piece_shape(game g, uint i, uint j, shape s) {
  if (g == NULL || i >= g->height || j >= g->width) {
    fprintf(stderr, "game_set_piece_shape out of bounds (i=%u, j=%u)\n", i, j);
    exit(EXIT_FAILURE);
  }
  g->shapes[i * g->width + j] = s;
}

// Affecte l'orientation d'une pièce à la position (i, j) du jeu
void game_set_piece_orientation(game g, uint i, uint j, direction o) {
  if (g == NULL) {
    fprintf(stderr, "game_set_piece_orientation appelé avec un jeu NULL\n");
    exit(EXIT_FAILURE);
  }
  if (i >= g->height || j >= g->width) {
    fprintf(stderr, "game_set_piece_orientation out of bounds (i=%u, j=%u)\n",
            i, j);
    exit(EXIT_FAILURE);
  }
  g->orientations[i * g->width + j] = o;
}

// Récupère la forme de la pièce située en (i, j) dans le jeu
shape game_get_piece_shape(cgame g, uint i, uint j) {
  if (g == NULL || i >= g->height || j >= g->width) {
    fprintf(stderr, "game_get_piece_shape out of bounds (i=%u, j=%u)\n", i, j);
    exit(EXIT_FAILURE);
  }
  return g->shapes[i * g->width + j];
}

// Récupère l'orientation de la pièce située en (i, j) dans le jeu
direction game_get_piece_orientation(cgame g, uint i, uint j) {
  if (g == NULL || i >= g->height || j >= g->width) {
    fprintf(stderr, "game_get_piece_orientation out of bounds (i=%u, j=%u)\n",
            i, j);
    exit(EXIT_FAILURE);
  }
  return g->orientations[i * g->width + j];
}

// Applique un mouvement (rotation) à la pièce située en (i, j)
void game_play_move(game g, uint i, uint j, int nb_quarter_turns) {
  if (g == NULL) {
    fprintf(stderr, "game_play_move appelé avec un jeu NULL\n");
    return;
  }
  if (i >= g->height || j >= g->width) {
    fprintf(stderr,
            "Indices invalides : (%u, %u) pour un jeu de taille (%u, %u)\n", i,
            j, g->height, g->width);
    return;
  }

  // Vider la redo_stack car un nouveau mouvement invalide les mouvements
  // annulés précédemment
  while (!queue_is_empty(g->redo_stack)) {
    move_t *redo_move = queue_pop_tail(g->redo_stack);
    free(redo_move);
  }

  // Allouer et enregistrer le mouvement dans l'undo_stack
  move_t *move = malloc(sizeof(move_t));
  if (move == NULL) {
    fprintf(stderr, "Échec de l'allocation mémoire dans game_play_move\n");
    return;
  }
  move->i = i;
  move->j = j;
  move->nb_quarter_turns = nb_quarter_turns;
  queue_push_tail(g->undo_stack, move);

  // Mettre à jour l'orientation de la pièce (on suppose ici que les
  // orientations sont codées de 0 à 3)
  uint index = i * g->width + j;
  g->orientations[index] = (g->orientations[index] + nb_quarter_turns) % 4;
}

// Vérifie si le jeu est gagné : il faut que le jeu soit bien apparié et
// connecté
bool game_won(cgame g) {
  if (g == NULL) {
      return false;
  }

  // Le jeu est gagné si toutes les pièces sont bien appariées et connectées
  return game_is_well_paired(g) && game_is_connected(g);
}

// Réinitialise l'orientation de toutes les pièces à NORTH
void game_reset_orientation(game g) {
  if (!g) return;
  for (uint i = 0; i < g->height; i++) {
    for (uint j = 0; j < g->width; j++) {
      game_set_piece_orientation(g, i, j, NORTH);
    }
  }
}

// Mélange aléatoirement l'orientation de toutes les pièces
void game_shuffle_orientation(game g) {
  if (!g) return;
  for (uint i = 0; i < g->height; i++) {
    for (uint j = 0; j < g->width; j++) {
      direction random_orientation = (direction)(rand() % 4);
      game_set_piece_orientation(g, i, j, random_orientation);
    }
  }
}
