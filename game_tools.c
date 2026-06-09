#include "game_tools.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "game.h"
#include "game_aux.h"
#include "game_ext.h"
#include "game_struct.h"
#include "game_tools.h"
#include "queue.h"

#define MAX_NEIGHBORS 4  // Nombre maximum de voisins pour une case

// Structure pour stocker les coordonnées d'une case
typedef struct {
  uint i;
  uint j;
} position;

// Tableau contenant les directions possibles (NORD, EST, SUD, OUEST)
const direction all_dirs[MAX_NEIGHBORS] = {NORTH, EAST, SOUTH, WEST};

game game_load(char *filename) {
  FILE *file = fopen(filename, "r");
  if (!file) {
    fprintf(stderr, "Erreur d'ouverture du fichier\n");
    exit(EXIT_FAILURE);
  }

  uint nb_rows = 0, nb_cols = 0, wrapping = 0;
  if (fscanf(file, "%u %u %u\n", &nb_rows, &nb_cols, &wrapping) != 3) {
    fprintf(stderr, "Erreur de lecture des paramètres du jeu\n");
    fclose(file);
    exit(EXIT_FAILURE);
  }

  shape *shapes = malloc(nb_rows * nb_cols * sizeof(shape));
  direction *directions = malloc(nb_rows * nb_cols * sizeof(direction));
  if (!shapes || !directions) {
    fprintf(stderr, "Erreur d'allocation de mémoire\n");
    fclose(file);
    exit(EXIT_FAILURE);
  }

  for (uint i = 0; i < nb_rows; i++) {
    for (uint j = 0; j < nb_cols; j++) {
      char s_char = 'E', d_char = 'N';
      if (fscanf(file, " %c%c", &s_char, &d_char) != 2) {
        fprintf(stderr, "Erreur de lecture de la grille des cases\n");
        free(shapes);
        free(directions);
        fclose(file);
        exit(EXIT_FAILURE);
      }

      switch (s_char) {
        case 'E':
          shapes[i * nb_cols + j] = EMPTY;
          break;
        case 'N':
          shapes[i * nb_cols + j] = ENDPOINT;
          break;
        case 'S':
          shapes[i * nb_cols + j] = SEGMENT;
          break;
        case 'C':
          shapes[i * nb_cols + j] = CORNER;
          break;
        case 'T':
          shapes[i * nb_cols + j] = TEE;
          break;
        case 'X':
          shapes[i * nb_cols + j] = CROSS;
          break;
        default:
          shapes[i * nb_cols + j] = EMPTY;
          break;
      }
      switch (d_char) {
        case 'N':
          directions[i * nb_cols + j] = NORTH;
          break;
        case 'E':
          directions[i * nb_cols + j] = EAST;
          break;
        case 'S':
          directions[i * nb_cols + j] = SOUTH;
          break;
        case 'W':
          directions[i * nb_cols + j] = WEST;
          break;
        default:
          directions[i * nb_cols + j] = NORTH;
          break;
      }
    }
  }
  game g = game_new_ext(nb_rows, nb_cols, shapes, directions, wrapping);
  free(shapes);
  free(directions);
  fclose(file);
  return g;
}

void game_save(cgame g, char *filename) {
  FILE *file = fopen(filename, "w");
  if (!file) {
    fprintf(stderr, "Erreur d'ouverture du fichier\n");
    exit(EXIT_FAILURE);
  }

  fprintf(file, "%u %u %u\n", game_nb_rows(g), game_nb_cols(g),
          game_is_wrapping(g));

  for (uint i = 0; i < game_nb_rows(g); i++) {
    for (uint j = 0; j < game_nb_cols(g); j++) {
      shape s = game_get_piece_shape(g, i, j);
      direction d = game_get_piece_orientation(g, i, j);
      char s_char = 'E';

      char d_char = 'N';

      switch (s) {
        case EMPTY:
          s_char = 'E';
          break;
        case ENDPOINT:
          s_char = 'N';
          break;
        case SEGMENT:
          s_char = 'S';
          break;
        case CORNER:
          s_char = 'C';
          break;
        case TEE:
          s_char = 'T';
          break;
        case CROSS:
          s_char = 'X';
          break;
        default:
          s_char = 'E';
          break;
      }

      switch (d) {
        case NORTH:
          d_char = 'N';
          break;
        case EAST:
          d_char = 'E';
          break;
        case SOUTH:
          d_char = 'S';
          break;
        case WEST:
          d_char = 'W';
          break;
        default:
          d_char = 'N';
          break;
      }

      fprintf(file, "%c%c ", s_char, d_char);
    }
    fprintf(file, "\n");
  }

  fclose(file);
}

// @copyright University of Bordeaux. All rights reserved, 2024.

/* ************************************************************************** */

/** @brief Hard-coding of pieces (shape & orientation) in an integer array.
 * @details The 4 least significant bits encode the presence of an half-edge in
 * the N-E-S-W directions (in that order). Thus, binary coding 1100 represents
 * the piece "└" (a corner in north orientation).
 */
static uint _code[NB_SHAPES][NB_DIRS] = {
    {0b0000, 0b0000, 0b0000, 0b0000},  // EMPTY {" ", " ", " ", " "}
    {0b1000, 0b0100, 0b0010, 0b0001},  // ENDPOINT {"^", ">", "v", "<"},
    {0b1010, 0b0101, 0b1010, 0b0101},  // SEGMENT {"|", "-", "|", "-"},
    {0b1100, 0b0110, 0b0011, 0b1001},  // CORNER {"└", "┌", "┐", "┘"}
    {0b1101, 0b1110, 0b0111, 0b1011},  // TEE {"┴", "├", "┬", "┤"}
    {0b1111, 0b1111, 0b1111, 0b1111}   // CROSS {"+", "+", "+", "+"}
};

/* ************************************************************************** */

/** encode a shape and an orientation into an integer code */
static uint _encode_shape(shape s, direction o) { return _code[s][o]; }

/* ************************************************************************** */

/** decode an integer code into a shape and an orientation */
static bool _decode_shape(uint code, shape *s, direction *o) {
  assert(code >= 0 && code < 16);
  assert(s);
  assert(o);
  for (int i = 0; i < NB_SHAPES; i++)
    for (int j = 0; j < NB_DIRS; j++)
      if (code == _code[i][j]) {
        *s = i;
        *o = j;
        return true;
      }
  return false;
}

/* ************************************************************************** */

/** add an half-edge in the direction d */
static void _add_half_edge(game g, uint i, uint j, direction d) {
  assert(g);
  assert(i < game_nb_rows(g));
  assert(j < game_nb_cols(g));
  assert(d < NB_DIRS);

  shape s = game_get_piece_shape(g, i, j);
  direction o = game_get_piece_orientation(g, i, j);
  uint code = _encode_shape(s, o);
  uint mask = 0b1000 >> d;     // mask with half-edge in the direction d
  assert((code & mask) == 0);  // check there is no half-edge in the direction d
  uint newcode = code | mask;  // add the half-edge in the direction d
  shape news = EMPTY;          // Initialisation par défaut de 'news'
  direction newo = NORTH;      // Initialisation par défaut de 'newo'
  _decode_shape(newcode, &news, &newo);
  game_set_piece_shape(g, i, j, news);
  game_set_piece_orientation(g, i, j, newo);
}

/* ************************************************************************** */

#define OPPOSITE_DIR(d) ((d + 2) % NB_DIRS)

/* ************************************************************************** */

/**
 * @brief Add an edge between two adjacent squares.
 * @details This is done by modifying the pieces of the two adjacent squares.
 * More precisely, we add an half-edge to each adjacent square, so as to build
 * an edge between these two squares.
 * @param g the game
 * @param i row index
 * @param j column index
 * @param d the direction of the adjacent square
 * @pre @p g must be a valid pointer toward a game structure.
 * @pre @p i < game height
 * @pre @p j < game width
 * @return true if an edge can be added, false otherwise
 */
static bool _add_edge(game g, uint i, uint j, direction d) {
  assert(g);
  assert(i < game_nb_rows(g));
  assert(j < game_nb_cols(g));
  assert(d < NB_DIRS);

  uint nexti, nextj;
  bool next = game_get_ajacent_square(g, i, j, d, &nexti, &nextj);
  if (!next) return false;

  // check if the two half-edges are free
  bool he = game_has_half_edge(g, i, j, d);
  if (he) return false;
  bool next_he = game_has_half_edge(g, nexti, nextj, OPPOSITE_DIR(d));
  if (next_he) return false;

  _add_half_edge(g, i, j, d);
  _add_half_edge(g, nexti, nextj, OPPOSITE_DIR(d));

  return true;
}

/* ************************************************************************** */
game game_random(uint nb_rows, uint nb_cols, bool wrapping, uint nb_empty,
                 uint nb_extra) {
  if (nb_rows < 2 || nb_cols < 2 || nb_empty > (nb_cols * nb_rows - 2)) {
    fprintf(stderr, "Erreur : dimensions invalides.\n");
    return NULL;
  }
  game g = game_new_empty_ext(nb_rows, nb_cols, wrapping);
  if (!g) {
    fprintf(stderr, "Erreur : impossible de créer le jeu.\n");
    return NULL;
  }

  // Placement initial d'un jeu à 2 pièces
  uint i = rand() % (nb_rows - 1);

  uint j = rand() % (nb_cols - 1);

  bool horizontal = rand() % 2;

  if (horizontal) {
    game_set_piece_shape(g, i, j, ENDPOINT);
    game_set_piece_shape(g, i, j + 1, ENDPOINT);
    game_set_piece_orientation(g, i, j, EAST);
    game_set_piece_orientation(g, i, j + 1, WEST);
  } else {
    game_set_piece_shape(g, i, j, ENDPOINT);
    game_set_piece_shape(g, i + 1, j, ENDPOINT);
    game_set_piece_orientation(g, i, j, SOUTH);
    game_set_piece_orientation(g, i + 1, j, NORTH);
  }

  // Liste des pièces placées
  uint piece_count = 2;
  position *pieces = malloc((nb_rows * nb_cols) * sizeof(position));
  if (!pieces) {
    fprintf(stderr, "Erreur d'allocation de mémoire pour les pièces.\n");
    game_delete(g);
    return NULL;
  }
  pieces[0] = (position){i, j};
  pieces[1] = horizontal ? (position){i, j + 1} : (position){i + 1, j};

  // Expansion du graphe
  while (piece_count < (nb_rows * nb_cols - nb_empty)) {
    uint index = rand() % piece_count;
    uint i = pieces[index].i;
    uint j = pieces[index].j;

    for (int attempt = 0; attempt < MAX_NEIGHBORS; attempt++) {
      direction d = all_dirs[rand() % MAX_NEIGHBORS];
      uint i_next, j_next;

      if (!game_get_ajacent_square(g, i, j, d, &i_next, &j_next)) continue;
      if (game_get_piece_shape(g, i_next, j_next) != EMPTY) continue;

      _add_edge(g, i, j, d);
      pieces[piece_count++] = (position){i_next, j_next};
      break;
    }
  }

  // Ajout de cycles supplémentaires
  for (uint extra = 0; extra < nb_extra; extra++) {
    uint i, j, i_next, j_next;
    direction d;
    bool found = false;

    while (!found) {
      uint index = rand() % piece_count;
      i = pieces[index].i;
      j = pieces[index].j;
      d = all_dirs[rand() % MAX_NEIGHBORS];

      if (game_get_ajacent_square(g, i, j, d, &i_next, &j_next)) {
        if (game_get_piece_shape(g, i_next, j_next) != EMPTY &&
            game_check_edge(g, i, j, d) == NOEDGE) {
          found = true;
        }
      }
    }
    _add_edge(g, i, j, d);
  }

  free(pieces);
  return g;
}

/*------------------------------------------------------------------------------------------------*/

#include <limits.h>

/**
 * @brief Fonction récursive pour parcourir la grille et essayer toutes les
 * orientations possibles.
 * @param g Le jeu en cours
 * @param pos Position actuelle dans la grille (index linéaire)
 * @param count Pointeur vers le compteur de solutions
 * @param count_only Si `true`, compte uniquement les solutions, sinon affiche
 * ou s'arrête à la première solution
 * @param print_solution Si `true`, affiche les solutions trouvées
 */

static uint solve_and_count_recursive(game g, uint i, uint j, bool count_only,
                                      bool print_solution, uint max_solutions) {
  // Réinitialisation du compteur de solutions si nécessaire
  static uint solution_count = 0;
  if (i == 0 && j == 0) {  // Réinitialiser au début
    solution_count = 0;
  }

  // Si on a dépassé le nombre maximal de solutions, on s'arrête
  if (solution_count >= max_solutions) {
    return solution_count;
  }

  // Si toutes les pièces ont été parcourues, vérifier si le jeu est gagné
  if (i >= game_nb_rows(g)) {
    if (game_won(g)) {  // Vérifie que la grille est valide
      if (print_solution) {
        game_print(g);  // Affiche la solution
        printf("\n");
      }
      solution_count++;
    }
    return solution_count;
  }

  // Passer à la ligne suivante si la colonne actuelle dépasse la grille
  if (j >= game_nb_cols(g)) {
    return solve_and_count_recursive(g, i + 1, 0, count_only, print_solution,
                                     max_solutions);
  }

  // Ignorer les cases vides
  if (game_get_piece_shape(g, i, j) == EMPTY) {
    return solve_and_count_recursive(g, i, j + 1, count_only, print_solution,
                                     max_solutions);
  }

  // Déterminer le nombre d'orientations possibles
  uint num_orientations = (game_get_piece_shape(g, i, j) == CROSS)     ? 1
                          : (game_get_piece_shape(g, i, j) == SEGMENT) ? 2
                                                                       : 4;

  // Sauvegarder l'orientation originale
  direction original_orientation = game_get_piece_orientation(g, i, j);

  // Explorer toutes les orientations possibles
  for (direction d = 0; d < num_orientations; d++) {
    game_set_piece_orientation(g, i, j, d);

    // Vérification des connexions locales
    bool valid = true;

    // Vérification avec la pièce à gauche
    if (j > 0 && game_has_half_edge(g, i, j, WEST)) {
      valid &= game_has_half_edge(g, i, j - 1, EAST);
    }

    // Vérification avec la pièce au-dessus
    if (i > 0 && game_has_half_edge(g, i, j, NORTH)) {
      valid &= game_has_half_edge(g, i - 1, j, SOUTH);
    }

    // Si une configuration invalide est détectée, passez à la suivante
    if (!valid) {
      continue;
    }

    // Poursuivre l'exploration avec la configuration actuelle
    uint count = solve_and_count_recursive(g, i, j + 1, count_only,
                                           print_solution, max_solutions);

    // Si on ne compte pas les solutions et qu'une solution est trouvée, arrêter
    if (!count_only && count > 0) {
      return solution_count;  // Arrêt immédiat
    }
  }

  // Restaurer l'orientation originale
  game_set_piece_orientation(g, i, j, original_orientation);

  return solution_count;
}

bool game_solve(game g) {
  // Cherche une seule solution
  return solve_and_count_recursive(g, 0, 0, false, true, 1) > 0;
}

uint game_nb_solutions(cgame g) {
  // Compter toutes les solutions possibles
  game temp_game = game_copy(g);
  uint count = solve_and_count_recursive(temp_game, 0, 0, true, false,
                                         UINT_MAX);  // Pas de limite
  game_delete(temp_game);
  return count;
}
