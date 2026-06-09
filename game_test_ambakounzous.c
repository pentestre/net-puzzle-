#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "game_aux.h"
#include "game_ext.h"
#include "game_struct.h"
#include "game_tools.h"
#include "queue.h"

/**
 * @brief Tests the dummy function.
 * @return EXIT_SUCCESS if the test passed, EXIT_FAILURE otherwise.
 **/
int test_dummy() { return EXIT_SUCCESS; }

/**
 * @brief Duplicates a game.
 * @param g the game to copy
 * @return the copy of the game
 * @pre @p g must be a valid pointer toward a game structure.
 **/
bool test_game_copy(void) {
  game g1 = game_new_empty_ext(DEFAULT_SIZE, DEFAULT_SIZE, false);
  if (g1 == NULL) {
    fprintf(stderr, "Error: game_new_empty failed\n");
    game_delete(g1);
    return false;
  }

  game_set_piece_shape(g1, 0, 0, ENDPOINT);
  game g2 = game_copy(g1);
  if (g2 == NULL) {
    fprintf(stderr, "Error: game_copy failed\n");
    game_delete(g1);
    return false;
  }

  bool result = game_equal(g1, g2, false);
  game_delete(g1);
  game_delete(g2);
  return result;
}

bool test_game_get_piece_orientation(void) {
  game game_instance = game_new_empty();
  if (game_instance == NULL) {
    fprintf(stderr, "Error: game_new_empty failed\n");
    return false;
  }

  for (uint row = 0; row < DEFAULT_SIZE; row++) {
    for (uint col = 0; col < DEFAULT_SIZE; col++) {
      if (game_get_piece_orientation(game_instance, row, col) != NORTH) {
        printf("Error: orientation of the piece at (%u, %u) should be NORTH.\n",
               row, col);
        game_delete(game_instance);
        return false;
      }
    }
  }

  game_set_piece_orientation(game_instance, 2, 2, EAST);
  if (game_get_piece_orientation(game_instance, 2, 2) != EAST) {
    printf("Error: orientation of the piece at (2, 2) should be EAST.\n");
    game_delete(game_instance);
    return false;
  }

  game_delete(game_instance);
  return true;
}

bool test_game_reset_orientation(void) {
  uint rows = 3, cols = 3;

  // Test with wrapping enabled
  bool wrapping = true;
  game g_wrap = game_new_empty_ext(rows, cols, wrapping);
  if (g_wrap == NULL) {
    fprintf(stderr, "Error: game_new_empty_ext failed for wrapping\n");
    return false;
  }

  // Set some orientations
  game_set_piece_orientation(g_wrap, 0, 0, EAST);
  game_set_piece_orientation(g_wrap, 1, 1, SOUTH);
  game_set_piece_orientation(g_wrap, 2, 2, WEST);

  // Reset orientations
  game_reset_orientation(g_wrap);

  bool success_wrap = true;
  for (uint i = 0; i < rows; i++) {
    for (uint j = 0; j < cols; j++) {
      if (game_get_piece_orientation(g_wrap, i, j) != NORTH) {
        success_wrap = false;
        printf(
            "Error: Orientation at (%u, %u) is not reset to NORTH with "
            "wrapping\n",
            i, j);
      }
    }
  }

  game_delete(g_wrap);

  // Test with wrapping disabled
  wrapping = false;
  game g_no_wrap = game_new_empty_ext(rows, cols, wrapping);
  if (g_no_wrap == NULL) {
    fprintf(stderr, "Error: game_new_empty_ext failed for no wrapping\n");
    return false;
  }

  // Set some orientations
  game_set_piece_orientation(g_no_wrap, 0, 0, EAST);
  game_set_piece_orientation(g_no_wrap, 1, 1, SOUTH);
  game_set_piece_orientation(g_no_wrap, 2, 2, WEST);

  // Reset orientations
  game_reset_orientation(g_no_wrap);

  bool success_no_wrap = true;
  for (uint i = 0; i < rows; i++) {
    for (uint j = 0; j < cols; j++) {
      if (game_get_piece_orientation(g_no_wrap, i, j) != NORTH) {
        success_no_wrap = false;
        printf(
            "Error: Orientation at (%u, %u) is not reset to NORTH without "
            "wrapping\n",
            i, j);
      }
    }
  }

  game_delete(g_no_wrap);

  bool success = success_wrap && success_no_wrap;
  printf("test_game_reset_orientation %s!\n", success ? "passed" : "failed");
  return success;
}

bool test_game_set_piece_shape(void) {
  game game_instance = game_new_empty();
  if (game_instance == NULL) {
    fprintf(stderr, "Error: game_new_empty failed\n");
    return false;
  }

  game_set_piece_shape(game_instance, 0, 0, ENDPOINT);
  if (game_get_piece_shape(game_instance, 0, 0) != ENDPOINT) {
    printf("Error: Expected ENDPOINT at (0, 0)\n");
    game_delete(game_instance);
    return false;
  }

  game_set_piece_shape(game_instance, 1, 1, SEGMENT);
  if (game_get_piece_shape(game_instance, 1, 1) != SEGMENT) {
    printf("Error: Expected SEGMENT at (1, 1)\n");
    game_delete(game_instance);
    return false;
  }

  game_set_piece_shape(game_instance, 2, 2, CORNER);
  if (game_get_piece_shape(game_instance, 2, 2) != CORNER) {
    printf("Error: Expected CORNER at (2, 2)\n");
    game_delete(game_instance);
    return false;
  }

  game_delete(game_instance);
  return true;
}

bool test_game_default(void) {
  game game1 = game_default();
  game game2 = game_default();

  bool all_cases_filled = true;
  bool all_orientations_valid = true;

  for (unsigned int row = 0; row < DEFAULT_SIZE; row++) {
    for (unsigned int col = 0; col < DEFAULT_SIZE; col++) {
      if (game_get_piece_shape(game1, row, col) == EMPTY) {
        all_cases_filled = false;
        printf("Empty case found at (%u, %u)\n", row, col);
      }
      direction piece_orientation = game_get_piece_orientation(game1, row, col);
      if (piece_orientation < 0 || piece_orientation >= NB_DIRS) {
        all_orientations_valid = false;
        printf("Invalid orientation at (%u, %u), found: %d\n", row, col,
               piece_orientation);
      }
    }
  }

  bool are_games_same = game_equal(game1, game2, false);

  game_delete(game1);
  game_delete(game2);

  return all_cases_filled && all_orientations_valid && are_games_same;
}

bool test_game_has_half_edge(void) {
  shape piece_shapes[5 * 5] = {
      ENDPOINT, SEGMENT, ENDPOINT, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
      EMPTY,    EMPTY,   EMPTY,    EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
      EMPTY,    EMPTY,   EMPTY,    EMPTY, CROSS, EMPTY, EMPTY};

  direction piece_orientations[5 * 5] = {
      EAST,  EAST,  WEST,  NORTH, NORTH, NORTH, NORTH, NORTH, NORTH,
      NORTH, NORTH, NORTH, NORTH, NORTH, NORTH, NORTH, NORTH, NORTH,
      NORTH, NORTH, NORTH, NORTH, NORTH, NORTH, NORTH};

  game game_instance = game_new(piece_shapes, piece_orientations);
  if (game_instance == NULL) {
    fprintf(stderr, "Error: game_new failed\n");
    return false;
  }

  bool test_result = true;

  for (uint row = 0; row < DEFAULT_SIZE; row++) {
    for (uint col = 0; col < DEFAULT_SIZE; col++) {
      for (direction direction_type = NORTH; direction_type < NB_DIRS;
           direction_type++) {
        bool has_half_edge =
            game_has_half_edge(game_instance, row, col, direction_type);

        // TESTS POUR TOUTE CONFIGURATION
        if (row == 0 && col == 0 &&
            game_get_piece_shape(game_instance, row, col) == ENDPOINT &&
            direction_type == EAST) {
          if (!has_half_edge) {
            printf(
                "Error: ENDPOINT at (%u, %u) should have a half-edge in "
                "direction %d.\n",
                row, col, direction_type);
            test_result = false;
          }
        }

        // TESTS POUR CROSS
        if (row == 4 && col == 2 &&
            game_get_piece_shape(game_instance, row, col) == CROSS) {
          if ((direction_type == NORTH && !has_half_edge) ||
              (direction_type == SOUTH && !has_half_edge) ||
              (direction_type == EAST && !has_half_edge) ||
              (direction_type == WEST && !has_half_edge)) {
            printf(
                "Error: CROSS at (%u, %u) should have half-edges in all "
                "directions %d.\n",
                row, col, direction_type);
            test_result = false;
          }
        }
      }
    }
  }

  game_delete(game_instance);
  return test_result;
}

bool test_game_redo(void) {
  uint rows = 10, cols = 10;
  bool wrapping = false;

  game g = game_new_empty_ext(rows, cols, wrapping);
  if (g == NULL) {
    fprintf(stderr, "Error: game_new_empty_ext failed\n");
    return false;
  }

  bool success = (game_get_piece_orientation(g, 1, 1) == NORTH);

  game_play_move(g, 1, 1, 1);
  success &= (game_get_piece_orientation(g, 1, 1) == EAST);

  game_undo(g);
  success &= (game_get_piece_orientation(g, 1, 1) == NORTH);

  game_redo(g);
  success &= (game_get_piece_orientation(g, 1, 1) == EAST);

  game_delete(g);

  printf("test_game_redo %s!\n", success ? "passed" : "failed");
  return success;
}

bool test_game_new_ext(void) {
  uint rows = 3, cols = 3;
  shape shapes[] = {CORNER,   SEGMENT, CORNER,  ENDPOINT, TEE,
                    ENDPOINT, CORNER,  SEGMENT, CORNER};
  direction orientations[] = {NORTH, EAST,  SOUTH, WEST, NORTH,
                              EAST,  SOUTH, WEST,  NORTH};
  bool wrapping = true;

  game g = game_new_ext(rows, cols, shapes, orientations, wrapping);
  if (g == NULL) {
    fprintf(stderr, "Error: game_new_ext failed\n");
    return false;
  }

  bool success = true;
  success &= (game_nb_rows(g) == rows);
  success &= (game_nb_cols(g) == cols);
  success &= (game_is_wrapping(g) == wrapping);

  for (uint i = 0; i < rows; i++) {
    for (uint j = 0; j < cols; j++) {
      if (game_get_piece_shape(g, i, j) != shapes[i * cols + j]) {
        success = false;
        printf("Error: Shape mismatch at (%u, %u)\n", i, j);
      }
      if (game_get_piece_orientation(g, i, j) != orientations[i * cols + j]) {
        success = false;
        printf("Error: Orientation mismatch at (%u, %u)\n", i, j);
      }
    }
  }

  game_delete(g);
  return success;
}

bool test_game_is_connected(void) {
  shape shapes[5 * 5] = {EMPTY,    EMPTY,   EMPTY,    EMPTY,    EMPTY,
                         EMPTY,    EMPTY,   EMPTY,    ENDPOINT, EMPTY,
                         EMPTY,    EMPTY,   EMPTY,    SEGMENT,  EMPTY,
                         EMPTY,    EMPTY,   EMPTY,    ENDPOINT, EMPTY,
                         ENDPOINT, SEGMENT, ENDPOINT, EMPTY,    EMPTY};

  direction orientations[5 * 5] = {
      NORTH, NORTH, NORTH, NORTH, NORTH, NORTH, NORTH, NORTH, SOUTH,
      NORTH, NORTH, NORTH, NORTH, NORTH, NORTH, NORTH, NORTH, NORTH,
      NORTH, NORTH, EAST,  EAST,  WEST,  NORTH, NORTH};

  shape shapes_connected[5 * 5] = {
      EMPTY,    EMPTY, EMPTY, EMPTY, EMPTY,   EMPTY, EMPTY, EMPTY, ENDPOINT,
      EMPTY,    EMPTY, EMPTY, EMPTY, SEGMENT, EMPTY, EMPTY, EMPTY, EMPTY,
      ENDPOINT, EMPTY, EMPTY, EMPTY, EMPTY,   EMPTY, EMPTY};

  direction orientations_connected[5 * 5] = {
      NORTH, NORTH, NORTH, NORTH, NORTH, NORTH, NORTH, NORTH, SOUTH,
      NORTH, NORTH, NORTH, NORTH, NORTH, NORTH, NORTH, NORTH, NORTH,
      NORTH, NORTH, NORTH, NORTH, NORTH, NORTH, NORTH};

  game g_empty = game_new_empty();
  game g_default = game_default();
  game g_disconnected = game_new(shapes, orientations);
  game g_connected = game_new(shapes_connected, orientations_connected);
  game g_solution = game_default_solution();

  // Vérification pour le jeu vide
  bool test_empty_game = game_is_connected(g_empty);
  if (!test_empty_game) {
    printf("Erreur : le jeu vide devrait être considéré comme connecté.\n");
  }

  // Vérification pour le jeu par défaut (supposé non connecté)
  bool test_default_game = !game_is_connected(g_default);
  if (!test_default_game) {
    printf("Erreur : le jeu par défaut ne devrait pas être connecté.\n");
  }

  // Vérification pour un jeu configuré comme non connecté
  bool test_disconnected_game = !game_is_connected(g_disconnected);
  if (!test_disconnected_game) {
    printf(
        "Erreur : le jeu configuré comme non connecté ne devrait pas être "
        "connecté.\n");
  }

  // Vérification pour un jeu configuré comme connecté
  bool test_connected_game = game_is_connected(g_connected);
  if (!test_connected_game) {
    printf("Erreur : le jeu configuré comme connecté devrait être connecté.\n");
  }

  // Vérification pour le jeu solution (supposé connecté)
  bool test_solution_game = game_is_connected(g_solution);
  if (!test_solution_game) {
    printf("Erreur : le jeu solution devrait être connecté.\n");
  }

  game_delete(g_empty);
  game_delete(g_default);
  game_delete(g_disconnected);
  game_delete(g_connected);
  game_delete(g_solution);

  return test_empty_game && test_default_game && test_disconnected_game &&
         test_connected_game && test_solution_game;
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Usage: %s <test_name>\n", argv[0]);
    return EXIT_FAILURE;
  }

  if (strcmp(argv[1], "dummy") == 0) {
    return test_dummy();
  } else if (strcmp(argv[1], "game_copy") == 0) {
    return test_game_copy() ? EXIT_SUCCESS : EXIT_FAILURE;
  } else if (strcmp(argv[1], "game_get_piece_orientation") == 0) {
    return test_game_get_piece_orientation() ? EXIT_SUCCESS : EXIT_FAILURE;
  } else if (strcmp(argv[1], "game_reset_orientation") == 0) {
    return test_game_reset_orientation() ? EXIT_SUCCESS : EXIT_FAILURE;
  } else if (strcmp(argv[1], "game_set_piece_shape") == 0) {
    return test_game_set_piece_shape() ? EXIT_SUCCESS : EXIT_FAILURE;
  } else if (strcmp(argv[1], "game_default") == 0) {
    return test_game_default() ? EXIT_SUCCESS : EXIT_FAILURE;
  } else if (strcmp(argv[1], "game_has_half_edge") == 0) {
    return test_game_has_half_edge() ? EXIT_SUCCESS : EXIT_FAILURE;
  } else if (strcmp(argv[1], "game_is_connected") == 0) {
    return test_game_is_connected() ? EXIT_SUCCESS : EXIT_FAILURE;
  } else if (strcmp(argv[1], "game_new_ext") == 0) {
    return test_game_new_ext() ? EXIT_SUCCESS : EXIT_FAILURE;
  } else if (strcmp(argv[1], "game_redo") == 0) {
    return test_game_redo() ? EXIT_SUCCESS : EXIT_FAILURE;
  } else {
    printf("Test inconnu: %s\n", argv[1]);
    return EXIT_FAILURE;
  }
}
