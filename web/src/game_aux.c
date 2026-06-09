#include "game_aux.h"

#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game_ext.h"
#include "game_struct.h"
#include "queue.h"

void game_print(cgame g) {
  assert(g != NULL);

  // Affichage des indices de colonnes
  printf("   ");
  for (uint j = 0; j < game_nb_cols(g); j++) {
    printf("%u ", j);
  }
  printf("\n");

  // Bordure supérieure
  printf("   ");
  for (uint j = 0; j < game_nb_cols(g); j++) {
    printf("--");
  }
  printf("-\n");

  // Contenu du plateau
  for (uint i = 0; i < game_nb_rows(g); i++) {
    printf("%2u | ", i);
    for (uint j = 0; j < game_nb_cols(g); j++) {
      shape s = game_get_piece_shape(g, i, j);
      direction d = game_get_piece_orientation(g, i, j);

      switch (s) {
        case ENDPOINT:
          switch (d) {
            case NORTH:
              printf("^ ");
              break;
            case SOUTH:
              printf("v ");
              break;
            case EAST:
              printf("> ");
              break;
            case WEST:
              printf("< ");
              break;
            default:
              printf("  ");
              break;
          }
          break;

        case SEGMENT:
          switch (d) {
            case NORTH:
            case SOUTH:
              printf("| ");
              break;
            case EAST:
            case WEST:
              printf("- ");
              break;
            default:
              printf("  ");
              break;
          }
          break;

        case CORNER:
          switch (d) {
            case NORTH:
              printf("└ ");
              break;
            case SOUTH:
              printf("┐ ");
              break;
            case EAST:
              printf("┌ ");
              break;
            case WEST:
              printf("┘ ");
              break;
            default:
              printf("  ");
              break;
          }
          break;

        case TEE:
          switch (d) {
            case NORTH:
              printf("┴ ");
              break;
            case SOUTH:
              printf("┬ ");
              break;
            case EAST:
              printf("├ ");
              break;
            case WEST:
              printf("┤ ");
              break;
            default:
              printf("  ");
              break;
          }
          break;

        case CROSS:
          printf("+ ");
          break;

        default:
          printf("  ");
          break;
      }
    }
    printf("|\n");
  }
  printf("   ");
  for (uint j = 0; j < game_nb_cols(g); j++) {
    printf("--");
  }
  printf("-\n");
}

game game_default(void) {
  shape shapes[DEFAULT_SIZE * DEFAULT_SIZE] = {
      CORNER,  ENDPOINT, ENDPOINT, CORNER,   ENDPOINT, TEE,     TEE,
      TEE,     TEE,      TEE,      ENDPOINT, ENDPOINT, TEE,     ENDPOINT,
      SEGMENT, ENDPOINT, TEE,      TEE,      CORNER,   SEGMENT, ENDPOINT,
      TEE,     ENDPOINT, ENDPOINT, ENDPOINT,
  };
  direction orientations[DEFAULT_SIZE * DEFAULT_SIZE] = {
      WEST, NORTH, WEST,  NORTH, SOUTH, SOUTH, WEST,  NORTH, EAST,
      EAST, EAST,  NORTH, WEST,  WEST,  EAST,  SOUTH, SOUTH, NORTH,
      WEST, NORTH, EAST,  WEST,  SOUTH, EAST,  SOUTH,
  };

  game g = game_new(shapes, orientations);
  return g;
}

game game_default_solution(void) {
  shape shapes[DEFAULT_SIZE * DEFAULT_SIZE] = {
      CORNER,  ENDPOINT, ENDPOINT, CORNER,   ENDPOINT, TEE,     TEE,
      TEE,     TEE,      TEE,      ENDPOINT, ENDPOINT, TEE,     ENDPOINT,
      SEGMENT, ENDPOINT, TEE,      TEE,      CORNER,   SEGMENT, ENDPOINT,
      TEE,     ENDPOINT, ENDPOINT, ENDPOINT,
  };
  direction orientations[DEFAULT_SIZE * DEFAULT_SIZE] = {
      EAST,  WEST,  EAST,  SOUTH, SOUTH, EAST,  SOUTH, SOUTH, NORTH,
      WEST,  NORTH, NORTH, EAST,  WEST,  SOUTH, EAST,  SOUTH, NORTH,
      SOUTH, NORTH, EAST,  NORTH, WEST,  NORTH, NORTH,
  };

  game g = game_new(shapes, orientations);
  return g;
}

bool game_get_ajacent_square(cgame g, uint i, uint j, direction d,
                             uint* pi_next, uint* pj_next) {
  if (!g || !pi_next || !pj_next) {
    fprintf(stderr, "game_get_adjacent_square: invalid parameters\n");
    return false;
  }

  uint rows = game_nb_rows(g);
  uint cols = game_nb_cols(g);
  bool wrapping = game_is_wrapping(g);

  // Initialisation des valeurs de sortie par défaut
  *pi_next = i;
  *pj_next = j;

  if (i >= rows || j >= cols) {
    fprintf(stderr,
            "game_get_adjacent_square: indices out of bounds (i=%u, j=%u)\n", i,
            j);
    return false;
  }

  switch (d) {
    case NORTH:
      *pi_next = (wrapping && i == 0) ? rows - 1 : i - 1;
      *pj_next = j;
      return (wrapping || i > 0);
    case SOUTH:
      *pi_next = (wrapping && i == rows - 1) ? 0 : i + 1;
      *pj_next = j;
      return (wrapping || i < rows - 1);
    case EAST:
      *pi_next = i;
      *pj_next = (wrapping && j == cols - 1) ? 0 : j + 1;
      return (wrapping || j < cols - 1);
    case WEST:
      *pi_next = i;
      *pj_next = (wrapping && j == 0) ? cols - 1 : j - 1;
      return (wrapping || j > 0);
    default:
      fprintf(stderr, "game_get_adjacent_square: invalid direction\n");
      return false;
  }
}

bool game_is_well_paired(cgame g) {
  assert(g != NULL);

  for (uint i = 0; i < g->height; i++) {
      for (uint j = 0; j < g->width; j++) {
          for (direction d = 0; d < NB_DIRS; d++) {
              uint pi_next = 0, pj_next = 0;

              // Vérifie si la case adjacente est valide
              bool valid_adjacent = game_get_ajacent_square(g, i, j, d, &pi_next, &pj_next);
              edge_status status = game_check_edge(g, i, j, d);

              // Si une arête est en mismatch, le jeu n'est pas bien apparié
              if (status == MISMATCH) {
                  return false;
              }

              // Si pas de voisin valide et une arête devrait exister
              if (!valid_adjacent && status != NOEDGE) {
                  // Vérifie le cas de wrapping
                  if (game_is_wrapping(g)) {
                      // Calcul des coordonnées du voisin via wrapping
                      switch (d) {
                          case NORTH:
                              pi_next = (i == 0) ? g->height - 1 : i - 1;
                              pj_next = j;
                              break;
                          case SOUTH:
                              pi_next = (i == g->height - 1) ? 0 : i + 1;
                              pj_next = j;
                              break;
                          case EAST:
                              pi_next = i;
                              pj_next = (j == g->width - 1) ? 0 : j + 1;
                              break;
                          case WEST:
                              pi_next = i;
                              pj_next = (j == 0) ? g->width - 1 : j - 1;
                              break;
                          default:
                              assert(false && "game_is_well_paired: invalid direction");
                              return false;
                      }
                      // Vérifie la validité de l'arête après wrapping
                      edge_status wrap_status = game_check_edge(g, pi_next, pj_next, (d + 2) % NB_DIRS);
                      if (wrap_status == MISMATCH) {
                          return false;
                      }
                  } else {
                      return false;  // Si wrapping n'est pas activé, l'arête ne devrait pas exister
                  }
              }
          }
      }
  }

  return true;
}


bool game_has_half_edge(cgame g, uint i, uint j, direction d) {
  assert(g != NULL);
  assert(i < g->height);
  assert(j < g->width);

  shape s = game_get_piece_shape(g, i, j);

  switch (s) {
    case ENDPOINT:
      return (d == g->orientations[i * g->width + j]);
    case SEGMENT:
      return (d == g->orientations[i * g->width + j] ||
              d == (g->orientations[i * g->width + j] + 2) % NB_DIRS);
    case CORNER:
      return (d == g->orientations[i * g->width + j] ||
              d == (g->orientations[i * g->width + j] + 1) % NB_DIRS);
    case TEE:
      return (d != (g->orientations[i * g->width + j] + 2) % NB_DIRS);
    case CROSS:
      return true;  // CROSS a des bords dans toutes les directions
    default:
      return false;
  }
}

edge_status game_check_edge(cgame g, uint i, uint j, direction d) {
  assert(g != NULL);
  assert(i < g->height);
  assert(j < g->width);

  uint i_next, j_next;
  bool has_adjacent = game_get_ajacent_square(g, i, j, d, &i_next, &j_next);
  bool current_has_half_edge = game_has_half_edge(g, i, j, d);

  if (!has_adjacent) {
      return current_has_half_edge ? MISMATCH : NOEDGE;
  }

  direction opposite = (d + 2) % NB_DIRS;
  bool adjacent_has_half_edge = game_has_half_edge(g, i_next, j_next, opposite);

  if (current_has_half_edge && adjacent_has_half_edge) {
      return MATCH;
  } else if (current_has_half_edge || adjacent_has_half_edge) {
      return MISMATCH;
  } else {
      return NOEDGE;
  }
}

void explore(cgame g, bool* visited, uint i, uint j) {
  assert(g != NULL);
  assert(visited != NULL);

  // Vérifie si la case a déjà été visitée
  if (visited[i * g->width + j]) {
      return;
  }

  // Marque la case actuelle comme visitée
  visited[i * g->width + j] = true;

  // Parcourt toutes les directions possibles (NORTH, EAST, SOUTH, WEST)
  for (direction d = 0; d < NB_DIRS; d++) {
      uint pi_next = 0, pj_next = 0;

      // Récupère la case adjacente dans la direction donnée
      if (game_get_ajacent_square(g, i, j, d, &pi_next, &pj_next)) {
          // Vérifie que la case adjacente n'a pas été visitée et que l'arête est valide (MATCH)
          if (!visited[pi_next * g->width + pj_next] &&
              game_check_edge(g, i, j, d) == MATCH) {
              // Explore récursivement la case adjacente
              explore(g, visited, pi_next, pj_next);
          }
      }
  }
}

bool game_is_connected(cgame g) {
  assert(g != NULL);

  uint total_squares = g->width * g->height;
  bool *visited = calloc(total_squares, sizeof(bool));  // Tableau dynamique pour suivre les cases visitées
  if (!visited) {
      fprintf(stderr, "Échec de l'allocation mémoire dans game_is_connected\n");
      return false;
  }

  // Trouver la première pièce non vide pour commencer l'exploration
  uint start_i = 0, start_j = 0;
  bool found_start = false;
  for (uint i = 0; i < g->height && !found_start; i++) {
      for (uint j = 0; j < g->width && !found_start; j++) {
          if (game_get_piece_shape(g, i, j) != EMPTY) {
              start_i = i;
              start_j = j;
              found_start = true;
          }
      }
  }

  // Si aucune pièce non vide n'a été trouvée, le jeu est considéré comme connecté
  if (!found_start) {
      free(visited);
      return true;
  }

  // Explorer le réseau de pièces connectées
  explore(g, visited, start_i, start_j);

  // Vérifier que toutes les pièces non vides ont été visitées
  for (uint i = 0; i < g->height; i++) {
      for (uint j = 0; j < g->width; j++) {
          uint index = i * g->width + j;
          if (game_get_piece_shape(g, i, j) != EMPTY && !visited[index]) {
              free(visited);
              return false;
          }
      }
  }

  free(visited);
  return true;
}

