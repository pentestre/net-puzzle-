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

int test_dummy() { return EXIT_SUCCESS; }

int test_game_set_piece_orientation() {
  game g = game_new_empty();
  if (g == NULL) {
    fprintf(stderr, "Error: Failed to create an empty game.\n");
    return EXIT_FAILURE;
  }

  game_set_piece_orientation(g, 1, 1, EAST);
  if (game_get_piece_orientation(g, 1, 1) != EAST) {
    fprintf(stderr, "Error: Expected EAST, but got %d.\n",
            game_get_piece_orientation(g, 1, 1));
    game_delete(g);
    return EXIT_FAILURE;
  }

  game_set_piece_orientation(g, 1, 2, SOUTH);
  if (game_get_piece_orientation(g, 1, 2) != SOUTH) {
    fprintf(stderr, "Error: Expected SOUTH, but got %d.\n",
            game_get_piece_orientation(g, 1, 2));
    game_delete(g);
    return EXIT_FAILURE;
  }

  game_set_piece_orientation(g, 2, 2, WEST);
  if (game_get_piece_orientation(g, 2, 2) != WEST) {
    fprintf(stderr, "Error: Expected WEST, but got %d.\n",
            game_get_piece_orientation(g, 2, 2));
    game_delete(g);
    return EXIT_FAILURE;
  }

  game_set_piece_orientation(g, 2, 3, NORTH);
  if (game_get_piece_orientation(g, 2, 3) != NORTH) {
    fprintf(stderr, "Error: Expected NORTH, but got %d.\n",
            game_get_piece_orientation(g, 2, 3));
    game_delete(g);
    return EXIT_FAILURE;
  }

  game_delete(g);
  return EXIT_SUCCESS;
}

bool test_game_new_empty() {
  // Création de deux jeux vides
  game g1 = game_new_empty();
  game g2 = game_new_empty();

  if (g1 == NULL || g2 == NULL) {
    fprintf(stderr, "Error: Failed to create empty games.\n");
    return false;
  }

  // Test 1 : Vérification des dimensions par défaut
  bool test_dimensions =
      (g1->width == DEFAULT_SIZE && g1->height == DEFAULT_SIZE);

  // Test 2 : Vérification que toutes les pièces sont EMPTY et NORTH
  bool test_pieces = true;
  for (uint i = 0; i < DEFAULT_SIZE; i++) {
    for (uint j = 0; j < DEFAULT_SIZE; j++) {
      if (game_get_piece_shape(g1, i, j) != EMPTY ||
          game_get_piece_orientation(g1, i, j) != NORTH) {
        test_pieces = false;
      }
    }
  }

  // Test 3 : Le jeu n'est pas gagné ni connecté immédiatement
  bool test_state =
      (!game_won(g1) && !game_is_connected(g1) && game_is_well_paired(g1));

  // Test 4 : Les deux jeux créés sont égaux
  bool test_equal = game_equal(g1, g2, true);

  // Test 5 : Modification d'un jeu et vérification de l'inégalité
  game_set_piece_orientation(g2, 0, 0, EAST);
  bool test_inequality = !game_equal(g1, g2, true);

  // Suppression des jeux pour éviter les fuites mémoire
  game_delete(g1);
  game_delete(g2);

  // Retourne la conjonction de tous les tests
  return test_dimensions && test_pieces && test_state && test_equal &&
         test_inequality;
}

bool test_game_equal() {
  game g1 = game_default();
  if (g1 == NULL) {
    fprintf(stderr, "Error: game_default failed to create a game\n");
    return false;
  }

  game g2 = game_copy(g1);
  if (g2 == NULL) {
    fprintf(stderr, "Error: game_copy failed to create a game\n");
    game_delete(g1);
    return false;
  }

  // Vérification d'égalité pour des jeux identiques
  if (!game_equal(g1, g2, false) || !game_equal(g1, g2, true)) {
    fprintf(stderr, "Error: games are not equal when they should be\n");
    game_delete(g1);
    game_delete(g2);
    return false;
  }

  // Test de dimensions différentes
  game g3 = game_new_empty_ext(5, 5, false);
  if (game_equal(g1, g3, true)) {
    fprintf(stderr,
            "Error: games with different dimensions should not be equal\n");
    game_delete(g1);
    game_delete(g2);
    game_delete(g3);
    return false;
  }
  game_delete(g3);

  // Test des options wrapping différentes
  game g_wrap1 = game_new_empty_ext(3, 3, true);
  game g_wrap2 = game_new_empty_ext(3, 3, false);
  if (game_equal(g_wrap1, g_wrap2, true)) {
    fprintf(
        stderr,
        "Error: games with different wrapping options are marked as equal\n");
    game_delete(g_wrap1);
    game_delete(g_wrap2);
    return false;
  }
  game_delete(g_wrap1);
  game_delete(g_wrap2);

  // Test avec NULL
  if (game_equal(g1, NULL, true) || game_equal(NULL, g2, true)) {
    fprintf(stderr, "Error: game_equal did not handle NULL games correctly\n");
    game_delete(g1);
    game_delete(g2);
    return false;
  }

  // Modification de l'orientation
  game_set_piece_orientation(g1, 2, 2, WEST);
  if (game_equal(g1, g2, false) || !game_equal(g1, g2, true)) {
    fprintf(
        stderr,
        "Error: games are equal when they should not be (orientation test)\n");
    game_delete(g1);
    game_delete(g2);
    return false;
  }

  // Test des piles undo/redo
  game_play_move(g1, 1, 1, 1);
  game_undo(g1);
  if (!game_equal(g1, g2, true)) {
    fprintf(stderr, "Error: undo did not restore equality\n");
    game_delete(g1);
    game_delete(g2);
    return false;
  }

  game_redo(g1);
  if (!game_equal(g1, g2, true) || game_equal(g1, g2, false)) {
    fprintf(stderr, "Error: redo failed to preserve game state\n");
    game_delete(g1);
    game_delete(g2);
    return false;
  }

  // Test de formes de pièces différentes
  game g_shape1 = game_new_empty_ext(4, 4, false);
  game g_shape2 = game_new_empty_ext(4, 4, false);
  game_set_piece_shape(g_shape1, 0, 0, SEGMENT);
  game_set_piece_shape(g_shape2, 0, 0, CORNER);
  if (game_equal(g_shape1, g_shape2, true)) {
    fprintf(stderr,
            "Error: games with different piece shapes are marked as equal\n");
    game_delete(g_shape1);
    game_delete(g_shape2);
    return false;
  }
  game_delete(g_shape1);
  game_delete(g_shape2);

  game_delete(g1);
  game_delete(g2);
  return true;
}

bool test_game_play_move(void) {
  // Tester game_play_move avec un jeu NULL
  game_play_move(
      NULL, 0, 0,
      1);  // Cela ne devrait pas causer de crash ou de comportement incorrect.

  // Créer un jeu par défaut
  game g = game_default();
  if (g == NULL) {
    fprintf(stderr, "Erreur : game_default n'a pas réussi à créer un jeu\n");
    return false;
  }

  // Copier le jeu par défaut pour comparer les états après les mouvements
  game g_copy = game_copy(g);
  if (g_copy == NULL) {
    fprintf(stderr,
            "Erreur : game_copy n'a pas réussi à créer une copie du jeu\n");
    game_delete(g);  // Libérer g
    return false;
  }

  // Tester avec différents mouvements sur toutes les pièces du plateau
  for (uint i = 0; i < game_nb_rows(g); i++) {
    for (uint j = 0; j < game_nb_cols(g); j++) {
      direction original_orientation = game_get_piece_orientation(g_copy, i, j);

      // Appliquer des rotations valides (90 degrés)
      game_play_move(g, i, j, 1);
      direction new_orientation = game_get_piece_orientation(g, i, j);
      if (new_orientation != (original_orientation + 1) % NB_DIRS) {
        fprintf(stderr,
                "Erreur: L'orientation de la pièce (%u, %u) est incorrecte "
                "après une rotation de 90 degrés (attendu: %d, obtenu: %d)\n",
                i, j, (original_orientation + 1) % NB_DIRS, new_orientation);
        game_delete(g);
        game_delete(g_copy);
        return false;
      }

      // Appliquer des rotations invalides (grandes valeurs ou négatives)
      game_play_move(g, i, j, -5);  // Rotation -5 (équivalent à -1 quart)
      new_orientation = game_get_piece_orientation(g, i, j);
      if (new_orientation != (original_orientation + 4 - 1) % NB_DIRS) {
        fprintf(stderr,
                "Erreur: L'orientation de la pièce (%u, %u) est incorrecte "
                "après une rotation de -5 (attendu: %d, obtenu: %d)\n",
                i, j, (original_orientation + 4 - 1) % NB_DIRS,
                new_orientation);
        game_delete(g);
        game_delete(g_copy);
        return false;
      }

      // Remettre à l'orientation initiale
      game_play_move(g, i, j, 4);  // Équivalent à 360 degrés
      new_orientation = game_get_piece_orientation(g, i, j);
      if (new_orientation != original_orientation) {
        fprintf(stderr,
                "Erreur: La pièce (%u, %u) n'est pas revenue à son orientation "
                "initiale (attendu: %d, obtenu: %d)\n",
                i, j, original_orientation, new_orientation);
        game_delete(g);
        game_delete(g_copy);
        return false;
      }
    }
  }

  // Tester undo et redo
  uint test_i = 0, test_j = 0;
  direction original_orientation =
      game_get_piece_orientation(g, test_i, test_j);

  // Appliquer une rotation de 90 degrés
  game_play_move(g, test_i, test_j, 1);

  // Appliquer une rotation de -90 degrés pour revenir à l'orientation initiale
  game_play_move(g, test_i, test_j, -1);
  direction undo_orientation = game_get_piece_orientation(g, test_i, test_j);
  if (undo_orientation != original_orientation) {
    fprintf(stderr,
            "Erreur : Après une rotation de -90 degrés, l'orientation devrait "
            "être rétablie à son état initial\n");
    game_delete(g);
    game_delete(g_copy);
    return false;
  }

  // Appliquer une rotation de 90 degrés à nouveau
  game_play_move(g, test_i, test_j, 1);
  direction redo_orientation = game_get_piece_orientation(g, test_i, test_j);
  if (redo_orientation != (original_orientation + 1) % NB_DIRS) {
    fprintf(stderr,
            "Erreur : Après une rotation de 90 degrés, l'orientation devrait "
            "correspondre à l'état après le mouvement\n");
    game_delete(g);
    game_delete(g_copy);
    return false;
  }

  // Libérer la mémoire
  game_delete(g);
  game_delete(g_copy);

  return true;
}

bool test_game_shuffle_orientation(void) {
  game g = game_default();
  assert(g != NULL);

  game_set_piece_orientation(g, 0, 0, NORTH);
  game_set_piece_orientation(g, 1, 1, EAST);
  game_set_piece_orientation(g, 2, 2, SOUTH);

  srand(time(NULL));

  direction initial_orientations[3] = {game_get_piece_orientation(g, 0, 0),
                                       game_get_piece_orientation(g, 1, 1),
                                       game_get_piece_orientation(g, 2, 2)};

  bool shuffled_00 = false, shuffled_11 = false, shuffled_22 = false;
  int attempts = 5;

  for (int i = 0; i < attempts; i++) {
    game_shuffle_orientation(g);

    if (game_get_piece_orientation(g, 0, 0) != initial_orientations[0]) {
      shuffled_00 = true;
    }
    if (game_get_piece_orientation(g, 1, 1) != initial_orientations[1]) {
      shuffled_11 = true;
    }
    if (game_get_piece_orientation(g, 2, 2) != initial_orientations[2]) {
      shuffled_22 = true;
    }
  }

  game_delete(g);
  return shuffled_00 && shuffled_11 && shuffled_22;
}

bool test_game_check_edge() {
  game g = game_new_empty();
  if (g == NULL) {
    fprintf(stderr, "Error: Failed to create an empty game.\n");
    return false;
  }

  game_set_piece_orientation(g, 1, 0, EAST);
  game_set_piece_orientation(g, 1, 1, SOUTH);
  game_set_piece_orientation(g, 0, 0, WEST);
  game_set_piece_orientation(g, 0, 1, NORTH);

  edge_status test1 = game_check_edge(g, 0, 0, NORTH);
  if (test1 != NOEDGE) {
    fprintf(stderr, "Error: Edge status is incorrect.\n");
    game_delete(g);
    return false;
  }

  edge_status test2 = game_check_edge(g, 1, 0, EAST);
  if (test2 != NOEDGE) {
    fprintf(stderr, "Error: Edge status is incorrect.\n");
    game_delete(g);
    return false;
  }

  edge_status test3 = game_check_edge(g, 1, 1, SOUTH);
  if (test3 != NOEDGE) {
    fprintf(stderr, "Error: Edge status is incorrect.\n");
    game_delete(g);
    return false;
  }

  edge_status test4 = game_check_edge(g, 0, 1, NORTH);
  if (test4 != NOEDGE) {
    fprintf(stderr, "Error: Edge status is incorrect.\n");
    game_delete(g);
    return false;
  }

  game_delete(g);
  return true;
}

bool test_game_new_empty_ext() {
  uint rows = 3, cols = 3;
  bool wrapping = true;

  game g = game_new_empty_ext(rows, cols, wrapping);
  assert(g != NULL);

  bool success = true;
  success &= (game_nb_rows(g) == rows);
  success &= (game_nb_cols(g) == cols);
  success &= (game_is_wrapping(g) == wrapping);

  for (uint i = 0; i < rows * cols; ++i) {
    success &= (game_get_piece_shape(g, i / cols, i % cols) == EMPTY);
    success &= (game_get_piece_orientation(g, i / cols, i % cols) == NORTH);
  }

  game_delete(g);
  return success;
}

bool test_game_undo(void) {
  uint rows = 10, cols = 10;
  bool wrapping = false;

  game g = game_new_empty_ext(rows, cols, wrapping);
  if (g == NULL) {
    fprintf(stderr, "Error: game_new_empty_ext failed\n");
    return false;
  }

  bool success = (game_get_piece_orientation(g, 1, 1) == NORTH);

  // Première rotation (à l'est)
  game_play_move(g, 1, 1, 1);
  success &= (game_get_piece_orientation(g, 1, 1) == EAST);

  // Deuxième rotation (au sud)
  game_play_move(g, 1, 1, 1);
  success &= (game_get_piece_orientation(g, 1, 1) == SOUTH);

  // Troisième rotation (à l'ouest)
  game_play_move(g, 1, 1, 1);
  success &= (game_get_piece_orientation(g, 1, 1) == WEST);

  // Test de undo : Annule tous les mouvements, un à un
  game_undo(g);  // Annule le dernier mouvement (ouest -> sud)
  success &= (game_get_piece_orientation(g, 1, 1) == SOUTH);

  game_undo(g);  // Annule le deuxième mouvement (sud -> est)
  success &= (game_get_piece_orientation(g, 1, 1) == EAST);

  game_undo(g);  // Annule le premier mouvement (est -> nord)
  success &= (game_get_piece_orientation(g, 1, 1) == NORTH);

  // Test de redo : Rejoue les mouvements annulés, un à un
  game_redo(g);  // Rejoue le premier mouvement (nord -> est)
  success &= (game_get_piece_orientation(g, 1, 1) == EAST);

  game_redo(g);  // Rejoue le deuxième mouvement (est -> sud)
  success &= (game_get_piece_orientation(g, 1, 1) == SOUTH);

  game_redo(g);  // Rejoue le troisième mouvement (sud -> ouest)
  success &= (game_get_piece_orientation(g, 1, 1) == WEST);

  game_delete(g);

  printf("test_game_undo %s!\n", success ? "passed" : "failed");
  return success;
}

bool test_game_default_solution() {
  game g_solution = game_default_solution();
  if (g_solution == NULL) {
    fprintf(stderr, "Error: Failed to create a default solution game.\n");
    return false;
  }

  // Test expected shapes and orientations
  shape expected_shapes[DEFAULT_SIZE * DEFAULT_SIZE] = {
      CORNER,  ENDPOINT, ENDPOINT, CORNER,   ENDPOINT, TEE,     TEE,
      TEE,     TEE,      TEE,      ENDPOINT, ENDPOINT, TEE,     ENDPOINT,
      SEGMENT, ENDPOINT, TEE,      TEE,      CORNER,   SEGMENT, ENDPOINT,
      TEE,     ENDPOINT, ENDPOINT, ENDPOINT,
  };

  direction expected_orientations[DEFAULT_SIZE * DEFAULT_SIZE] = {
      EAST,  WEST,  EAST,  SOUTH, SOUTH, EAST,  SOUTH, SOUTH, NORTH,
      WEST,  NORTH, NORTH, EAST,  WEST,  SOUTH, EAST,  SOUTH, NORTH,
      SOUTH, NORTH, EAST,  NORTH, WEST,  NORTH, NORTH,
  };

  for (uint i = 0; i < DEFAULT_SIZE; i++) {
    for (uint j = 0; j < DEFAULT_SIZE; j++) {
      if (game_get_piece_shape(g_solution, i, j) !=
              expected_shapes[i * DEFAULT_SIZE + j] ||
          game_get_piece_orientation(g_solution, i, j) !=
              expected_orientations[i * DEFAULT_SIZE + j]) {
        fprintf(stderr,
                "Error: Expected shape or orientation mismatch at (%d, %d)\n",
                i, j);
        game_delete(g_solution);
        return false;
      }
    }
  }

  game_delete(g_solution);
  return true;
}

bool test_game_load() {
  // Test avec default.txt
  game g_txt = game_load("test_files/default.txt");
  if (g_txt == NULL) {
    fprintf(stderr,
            "Erreur : Impossible de charger le jeu à partir de default.txt\n");
    return false;
  }

  shape shapes_txt[] = {
      CORNER,  ENDPOINT, ENDPOINT, CORNER,   ENDPOINT, TEE,     TEE,
      TEE,     TEE,      TEE,      ENDPOINT, ENDPOINT, TEE,     ENDPOINT,
      SEGMENT, ENDPOINT, TEE,      TEE,      CORNER,   SEGMENT, ENDPOINT,
      TEE,     ENDPOINT, ENDPOINT, ENDPOINT,
  };
  direction orientations_txt[] = {
      WEST, NORTH, WEST,  NORTH, SOUTH, SOUTH, WEST,  NORTH, EAST,
      EAST, EAST,  NORTH, WEST,  WEST,  EAST,  SOUTH, SOUTH, NORTH,
      WEST, NORTH, EAST,  WEST,  SOUTH, EAST,  SOUTH,
  };
  uint nb_rows_txt = 5;
  uint nb_cols_txt = 5;
  game expected_game_txt = game_new_ext(nb_rows_txt, nb_cols_txt, shapes_txt,
                                        orientations_txt, false);

  bool result_txt = game_equal(g_txt, expected_game_txt, false);
  game_delete(g_txt);
  game_delete(expected_game_txt);

  if (!result_txt) {
    fprintf(stderr,
            "Erreur : Le jeu chargé à partir de default.txt ne correspond pas "
            "au jeu attendu\n");
    return false;
  }

  // Test avec default.sol
  game g_sol = game_load("test_files/default.sol");
  if (g_sol == NULL) {
    fprintf(stderr,
            "Erreur : Impossible de charger le jeu à partir de default.sol\n");
    return false;
  }

  shape shapes_sol[] = {
      CORNER,  ENDPOINT, ENDPOINT, CORNER,   ENDPOINT, TEE,     TEE,
      TEE,     TEE,      TEE,      ENDPOINT, ENDPOINT, TEE,     ENDPOINT,
      SEGMENT, ENDPOINT, TEE,      TEE,      CORNER,   SEGMENT, ENDPOINT,
      TEE,     ENDPOINT, ENDPOINT, ENDPOINT,
  };
  direction orientations_sol[] = {
      EAST,  WEST,  EAST,  SOUTH, SOUTH, EAST,  SOUTH, SOUTH, NORTH,
      WEST,  NORTH, NORTH, EAST,  WEST,  SOUTH, EAST,  SOUTH, NORTH,
      SOUTH, NORTH, EAST,  NORTH, WEST,  NORTH, NORTH,
  };
  uint nb_rows_sol = 5;
  uint nb_cols_sol = 5;
  game expected_game_sol = game_new_ext(nb_rows_sol, nb_cols_sol, shapes_sol,
                                        orientations_sol, false);

  bool result_sol = game_equal(g_sol, expected_game_sol, false);
  game_delete(g_sol);
  game_delete(expected_game_sol);

  if (!result_sol) {
    fprintf(stderr,
            "Erreur : Le jeu chargé à partir de default.sol ne correspond pas "
            "au jeu attendu\n");
    return false;
  }

  return result_txt && result_sol;
}

bool test_game_solve() {
  // Charger un jeu à partir de game11.txt
  game g = game_load("test_files/game11.txt");
  if (g == NULL) {
    fprintf(stderr,
            "Erreur : Impossible de charger le jeu à partir de game11.txt\n");
    return false;
  }

  // Vérifier que game_solve() trouve une solution
  bool solved = game_solve(g);
  if (!solved) {
    fprintf(
        stderr,
        "Erreur : game_solve() n'a pas trouvé de solution pour game11.txt\n");
    game_delete(g);
    return false;
  }

  // Vérifier que le jeu est bien résolu
  if (!game_won(g)) {
    fprintf(
        stderr,
        "Erreur : Le jeu n'est pas résolu correctement après game_solve()\n");
    game_delete(g);
    return false;
  }

  // Libérer la mémoire
  game_delete(g);

  printf("test_game_solve() : OK\n");
  return true;
}

bool test_game_nb_solutions() {
  // Charger un jeu à partir de game11.txt
  game g = game_load("test_files/game11.txt");
  if (g == NULL) {
    fprintf(stderr,
            "Erreur : Impossible de charger le jeu à partir de game11.txt\n");
    return false;
  }

  // Compter le nombre de solutions
  uint nb_solutions = game_nb_solutions(g);

  // Vérifier que le nombre de solutions est correct
  // Pour game11.txt, le nombre de solutions attendu est 2 (par exemple)
  uint expected_nb_solutions = 2;
  if (nb_solutions != expected_nb_solutions) {
    fprintf(stderr,
            "Erreur : Le nombre de solutions trouvées (%u) ne correspond pas "
            "au nombre attendu (%u)\n",
            nb_solutions, expected_nb_solutions);
    game_delete(g);
    return false;
  }

  // Libérer la mémoire
  game_delete(g);

  printf("test_game_nb_solutions() : OK\n");
  return true;
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <test_name>\n", argv[0]);
    return EXIT_FAILURE;
  }

  if (strcmp(argv[1], "dummy") == 0) {
    return test_dummy();
  } else if (strcmp(argv[1], "game_set_piece_orientation") == 0) {
    return test_game_set_piece_orientation();
  } else if (strcmp(argv[1], "game_new_empty") == 0) {
    return test_game_new_empty();
  } else if (strcmp(argv[1], "game_equal") == 0) {
    return test_game_equal();
  } else if (strcmp(argv[1], "game_check_edge") == 0) {
    return test_game_check_edge() ? EXIT_SUCCESS : EXIT_FAILURE;
  } else if (strcmp(argv[1], "game_play_move") == 0) {
    return test_game_play_move();
  } else if (strcmp(argv[1], "game_shuffle_orientation") == 0) {
    return test_game_shuffle_orientation() ? EXIT_SUCCESS : EXIT_FAILURE;
  } else if (strcmp(argv[1], "game_undo") == 0) {
    return test_game_undo() ? EXIT_SUCCESS : EXIT_FAILURE;
  } else if (strcmp(argv[1], "game_new_empty_ext") == 0) {
    return test_game_new_empty_ext() ? EXIT_SUCCESS : EXIT_FAILURE;
  } else if (strcmp(argv[1], "game_default_solution") == 0) {
    return test_game_default_solution() ? EXIT_SUCCESS : EXIT_FAILURE;
  } else if (strcmp(argv[1], "game_load") == 0) {
    return test_game_load() ? EXIT_SUCCESS : EXIT_FAILURE;
  } else if (strcmp(argv[1], "game_solve") == 0) {
    return test_game_solve() ? EXIT_SUCCESS : EXIT_FAILURE;
  } else if (strcmp(argv[1], "game_nb_solutions") == 0) {
    return test_game_nb_solutions() ? EXIT_SUCCESS : EXIT_FAILURE;
  } else {
    fprintf(stderr, "Test non reconnu : %s\n", argv[1]);
    return EXIT_FAILURE;
  }
}
