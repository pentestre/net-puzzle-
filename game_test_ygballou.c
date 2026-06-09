#include <assert.h>
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

#define M 25  // Taille de la grille
#define DEFAULT_SIZE 5

// Test de fonction d'exemple qui retourne toujours EXIT_SUCCESS
int test_dummy() { return EXIT_SUCCESS; }

// Test pour vérifier la récupération de la forme d'une pièce dans le jeu
bool test_game_get_piece_shape(void) {
  game g = game_default();

  // Vérifier que chaque case a un état initial non modifié
  bool initial_test = true;
  for (uint i = 0; i < DEFAULT_SIZE; i++) {
    for (uint j = 0; j < DEFAULT_SIZE; j++) {
      shape initial_shape = game_get_piece_shape(g, i, j);
      if (initial_shape != game_get_piece_shape(g, i, j)) {
        initial_test = false;  // Détecte toute incohérence
      }
    }
  }

  // Test de modification de la forme d'une pièce spécifique
  game_set_piece_shape(g, 0, 0, CORNER);
  bool test1 = (game_get_piece_shape(g, 0, 0) == CORNER);

  // Test pour une autre modification de pièce
  game_set_piece_shape(g, 4, 4, ENDPOINT);
  bool test2 = (game_get_piece_shape(g, 4, 4) == ENDPOINT);

  // Test d'une nouvelle modification pour la case (0,0)
  game_set_piece_shape(g, 0, 0, SEGMENT);
  bool test3 = (game_get_piece_shape(g, 0, 0) == SEGMENT);

  game_set_piece_shape(g, 2, 2, CROSS);
  bool test4 = (game_get_piece_shape(g, 2, 2) == CROSS);
  game_delete(g);

  return initial_test && test1 && test2 && test3 && test4;
}

// Test pour vérifier la création du jeu avec les pièces et orientations
// spécifiées
bool test_game_new() {
  shape shapes[M] = {EMPTY, ENDPOINT, SEGMENT, CORNER, TEE,
                     EMPTY, ENDPOINT, SEGMENT, CORNER, TEE,
                     EMPTY, ENDPOINT, SEGMENT, CORNER, TEE,
                     EMPTY, ENDPOINT, SEGMENT, CORNER, TEE,
                     EMPTY, ENDPOINT, SEGMENT, CORNER, TEE};
  direction orientation[M] = {NORTH, EAST,  SOUTH, WEST,  NORTH, EAST,  SOUTH,
                              WEST,  NORTH, EAST,  SOUTH, WEST,  NORTH, EAST,
                              SOUTH, WEST,  NORTH, EAST,  SOUTH, WEST,  NORTH,
                              EAST,  SOUTH, WEST,  WEST};

  if (M != DEFAULT_SIZE * DEFAULT_SIZE) {
    return false;
  }

  // Crée un jeu avec les pièces et orientations données
  game g = game_new(shapes, orientation);
  if (g == NULL) {
    return false;
  }

  uint i = 0;
  for (uint u = 0; u < DEFAULT_SIZE; u++) {
    for (uint o = 0; o < DEFAULT_SIZE; o++) {
      direction m = game_get_piece_orientation(g, u, o);
      shape h = game_get_piece_shape(g, u, o);
      if (h != shapes[i] || m != orientation[i]) {
        game_delete(g);
        return false;
      }
      i++;
    }
  }

  game_delete(g);
  return true;
}

// Test pour vérifier si le jeu a été gagné
bool test_game_won() {
  // Test avec NULL comme argument
  if (game_won(NULL)) {
    fprintf(stderr, "Erreur : game_won(NULL) devrait retourner false.\n");
    return false;
  }

  // Test 1 : le jeu vide n'est pas gagné
  game g = game_new_empty();
  if (g == NULL || game_won(g)) {
    fprintf(stderr, "Erreur : Le jeu vide ne devrait pas être gagné.\n");
    game_delete(g);
    return false;
  }

  // Test 2 : le jeu par défaut n'est pas gagné
  game p = game_default();
  if (p == NULL || game_won(p)) {
    fprintf(stderr, "Erreur : Le jeu par défaut ne devrait pas être gagné.\n");
    game_delete(p);
    return false;
  }

  // Test 3 : un jeu avec une seule case non-vide ne peut pas être gagné
  game_set_piece_shape(g, 2, 2, CORNER);
  if (game_won(g)) {
    fprintf(stderr,
            "Erreur : Un jeu avec une seule case non-vide ne devrait pas être "
            "gagné.\n");
    game_delete(g);
    return false;
  }
  game_delete(g);  // Réinitialisation du jeu

  // Test 4 : Un jeu vide avec l'option wrapping activée ne doit pas être gagné
  game g_empty_wrapping = game_new_empty_ext(1, 1, true);
  if (g_empty_wrapping == NULL || game_won(g_empty_wrapping)) {
    fprintf(stderr,
            "Erreur : Un jeu vide avec wrapping activé ne devrait pas être "
            "gagné.\n");
    game_delete(g_empty_wrapping);
    return false;
  }
  game_delete(g_empty_wrapping);

  // Test 5 : Un jeu non connecté ne doit pas être considéré comme gagné
  game g_not_connected = game_default_solution();
  if (g_not_connected == NULL) {
    fprintf(
        stderr,
        "Erreur : Échec de la création d'un jeu non connecté pour le test.\n");
    return false;
  }
  game_set_piece_shape(g_not_connected, 1, 3, TEE);
  if (game_won(g_not_connected)) {
    fprintf(stderr,
            "Erreur : Un jeu non connecté ne devrait pas être considéré comme "
            "gagné.\n");
    game_delete(g_not_connected);
    return false;
  }
  game_delete(g_not_connected);

  // Cas Complexe : Jeu 1x1 ne peut pas être gagné
  game g_1x1 = game_new_empty_ext(1, 1, false);
  game_set_piece_shape(g_1x1, 0, 0, SEGMENT);
  if (game_won(g_1x1)) {
    fprintf(stderr, "Erreur : Un jeu 1x1 ne devrait pas être gagné.\n");
    game_delete(g_1x1);
    return false;
  }
  game_delete(g_1x1);

  // Cas Limite : Jeu 1xN (ligne) ne peut pas être gagné
  game g_1xN = game_new_empty_ext(1, 5, false);
  for (uint j = 0; j < 5; j++) {
    game_set_piece_shape(g_1xN, 0, j, SEGMENT);
  }
  if (game_won(g_1xN)) {
    fprintf(stderr, "Erreur : Un jeu 1xN ne devrait pas être gagné.\n");
    game_delete(g_1xN);
    return false;
  }
  game_delete(g_1xN);

  // Cas Limite : Jeu Nx1 (colonne) ne peut pas être gagné
  game g_Nx1 = game_new_empty_ext(5, 1, false);
  for (uint i = 0; i < 5; i++) {
    game_set_piece_shape(g_Nx1, i, 0, SEGMENT);
  }
  if (game_won(g_Nx1)) {
    fprintf(stderr, "Erreur : Un jeu Nx1 ne devrait pas être gagné.\n");
    game_delete(g_Nx1);
    return false;
  }
  game_delete(g_Nx1);

  // Test 6 : Vérification des conditions internes de game_won
  // Jeu bien apparié mais non connecté
  game g_well_paired = game_default_solution();
  game_set_piece_shape(g_well_paired, 1, 3, SEGMENT);  // Déconnecter une pièce
  if (game_is_well_paired(g_well_paired) && !game_is_connected(g_well_paired) &&
      game_won(g_well_paired)) {
    fprintf(stderr,
            "Erreur : Un jeu bien apparié mais non connecté ne devrait pas "
            "être gagné.\n");
    game_delete(g_well_paired);
    return false;
  }
  game_delete(g_well_paired);

  // Jeu connecté mais non bien apparié
  game g_connected = game_default_solution();
  game_set_piece_orientation(g_connected, 1, 1,
                             EAST);  // Modifier l'orientation d'une pièce
  if (!game_is_well_paired(g_connected) && game_is_connected(g_connected) &&
      game_won(g_connected)) {
    fprintf(stderr,
            "Erreur : Un jeu connecté mais non bien apparié ne devrait pas "
            "être gagné.\n");
    game_delete(g_connected);
    return false;
  }
  game_delete(g_connected);

  // Test 7: Un jeu gagné doit être vécu comme tel
  game g_won = game_default_solution();
  if (g_won == NULL || !game_won(g_won)) {
    fprintf(stderr, "Erreur : Un jeu gagné devrait être gagnant.\n");
    game_delete(g_won);
    return false;
  }

  // Suppression des jeux
  game_delete(g_won);

  return true;
}

// Test de la fonction qui récupère la case adjacente en fonction de la
// direction
bool test_adjacent(game g, uint i, uint j, direction d, uint expected_i,
                   uint expected_j) {
  uint i_next, j_next;
  bool valid = game_get_ajacent_square(g, i, j, d, &i_next, &j_next);
  if (!valid || i_next != expected_i || j_next != expected_j) {
    printf(
        "Test failed: (%u, %u) direction %d -> expected (%u, %u), got (%u, "
        "%u)\n",
        i, j, d, expected_i, expected_j, i_next, j_next);
    return false;
  }
  return true;
}

// Test de la fonction game_get_ajacent_square
bool test_game_get_ajacent_square(void) {
  // Test avec wrapping activé
  game g_wrap = game_new_empty_ext(3, 3, true);
  if (!g_wrap) return false;

  bool success = true;

  // Tests avec wrapping activé
  success &= test_adjacent(g_wrap, 1, 1, NORTH, 0, 1);
  success &= test_adjacent(g_wrap, 1, 1, EAST, 1, 2);
  success &= test_adjacent(g_wrap, 1, 1, SOUTH, 2, 1);
  success &= test_adjacent(g_wrap, 1, 1, WEST, 1, 0);

  // Tests de wrapping
  success &= test_adjacent(g_wrap, 0, 1, NORTH, 2, 1);  // Wrapping nord
  success &= test_adjacent(g_wrap, 1, 2, EAST, 1, 0);   // Wrapping est
  success &= test_adjacent(g_wrap, 2, 1, SOUTH, 0, 1);  // Wrapping sud
  success &= test_adjacent(g_wrap, 1, 0, WEST, 1, 2);   // Wrapping ouest

  game_delete(g_wrap);

  // Test avec wrapping désactivé
  game g_no_wrap = game_new_empty_ext(3, 3, false);
  if (!g_no_wrap) return false;

  // Tests sans wrapping (doivent retourner false)
  uint i_next, j_next;
  success &= !game_get_ajacent_square(g_no_wrap, 0, 1, NORTH, &i_next, &j_next);
  success &= !game_get_ajacent_square(g_no_wrap, 1, 2, EAST, &i_next, &j_next);
  success &= !game_get_ajacent_square(g_no_wrap, 2, 1, SOUTH, &i_next, &j_next);
  success &= !game_get_ajacent_square(g_no_wrap, 1, 0, WEST, &i_next, &j_next);

  game_delete(g_no_wrap);

  if (success) {
    printf("All tests passed!\n");
  } else {
    printf("Some tests failed.\n");
  }
  return success;
}

// Test de la fonction game_delete pour vérifier la suppression du jeu
bool test_game_delete() {
  game g = game_default();
  game_delete(g);
  return true;
}

// Test de l'affichage du jeu
bool test_game_print() {
  game g = game_default();
  if (g == NULL) {
    return false;
  }

  // Affiche le jeu
  game_print(g);
  game_delete(g);

  game i = game_default_solution();
  if (i == NULL) {
    return false;
  }
  game_print(i);
  game_delete(i);

  game o = game_new_empty();
  if (o == NULL) {
    return false;
  }
  game_print(o);
  game_delete(o);

  return true;
}

bool test_game_is_well_paired(void) {
  game g_default = game_default_solution();
  bool is_paired_default = game_is_well_paired(g_default);
  if (!is_paired_default) {
    printf("Erreur : La solution par défaut n'est pas bien appariée.\n");
  }
  game_delete(g_default);

  game g_modified_solution = game_default_solution();
  game_set_piece_orientation(g_modified_solution, 0, 0, SOUTH);
  bool is_not_paired = game_is_well_paired(g_modified_solution);
  if (is_not_paired) {
    printf("Erreur : La grille modifiée devrait être mal appariée.\n");
  }
  game_delete(g_modified_solution);

  game g_empty = game_new_empty();
  bool is_paired_empty = game_is_well_paired(g_empty);
  if (!is_paired_empty) {
    printf("Erreur : Le jeu vide devrait être bien apparié.\n");
  }
  game_delete(g_empty);

  g_default = game_default_solution();  // Recréez le jeu pour les vérifications
                                        // suivantes
  bool all_edges_paired = true;
  for (uint i = 0; i < DEFAULT_SIZE; i++) {
    for (uint j = 0; j < DEFAULT_SIZE; j++) {
      for (direction d = NORTH; d < NB_DIRS; d++) {
        edge_status status = game_check_edge(g_default, i, j, d);
        bool has_half_edge = game_has_half_edge(g_default, i, j, d);
        if (has_half_edge && status != MATCH) {
          printf("Erreur : Demi-arête mal appariée en (%u, %u) direction %d.\n",
                 i, j, d);
          all_edges_paired = false;
        }
      }
    }
  }
  if (!all_edges_paired) {
    printf(
        "Erreur : Certaines demi-arêtes ne sont pas correctement appariées.\n");
  }

  game_delete(g_default);

  return is_paired_default && !is_not_paired && is_paired_empty &&
         all_edges_paired;
}

bool test_game_is_wrapping() {
  bool wrapping = true;
  game g = game_new_empty_ext(3, 3, wrapping);
  assert(g != NULL);  // Vérification de l'allocation mémoire

  bool success = (game_is_wrapping(g) == wrapping);
  game_delete(g);

  wrapping = false;
  g = game_new_empty_ext(3, 3, wrapping);
  assert(g != NULL);  // Vérification de l'allocation mémoire

  success &= (game_is_wrapping(g) == wrapping);
  game_delete(g);

  printf("test_game_is_wrapping %s!\n", success ? "passed" : "failed");
  return success;
}

bool test_game_nb_cols(void) {
  bool success = true;

  // Cas 1: Test avec un jeu vide 3x3
  uint cols = 3;
  game g = game_new_empty_ext(3, cols, false);
  assert(g != NULL);  // Vérification de l'allocation mémoire
  if (game_nb_cols(g) != cols) {
    fprintf(stderr,
            "Erreur : game_nb_cols a retourné %u au lieu de %u pour une grille "
            "3x3.\n",
            game_nb_cols(g), cols);
    success = false;
  }
  game_delete(g);

  // Cas 2: Test avec un jeu vide 3x5
  cols = 5;
  g = game_new_empty_ext(3, cols, false);
  assert(g != NULL);
  if (game_nb_cols(g) != cols) {
    fprintf(stderr,
            "Erreur : game_nb_cols a retourné %u au lieu de %u pour une grille "
            "3x5.\n",
            game_nb_cols(g), cols);
    success = false;
  }
  game_delete(g);

  // Cas 3: Test avec un jeu vide 1x10 (grille étroite)
  cols = 10;
  g = game_new_empty_ext(1, cols, false);
  assert(g != NULL);
  if (game_nb_cols(g) != cols) {
    fprintf(stderr,
            "Erreur : game_nb_cols a retourné %u au lieu de %u pour une grille "
            "1x10.\n",
            game_nb_cols(g), cols);
    success = false;
  }
  game_delete(g);

  // Cas 4: Test avec un jeu vide 0x4 (grille vide en rangées)
  cols = 4;
  g = game_new_empty_ext(0, cols, false);
  assert(g != NULL);
  if (game_nb_cols(g) != cols) {
    fprintf(stderr,
            "Erreur : game_nb_cols a retourné %u au lieu de %u pour une grille "
            "0x4.\n",
            game_nb_cols(g), cols);
    success = false;
  }
  game_delete(g);

  printf("test_game_nb_cols %s!\n", success ? "passed" : "failed");
  return success;
}

bool test_game_nb_rows(void) {
  bool success = true;

  // Cas 1: Test avec un jeu vide 3x3
  uint rows = 3;
  game g = game_new_empty_ext(rows, 3, false);
  assert(g != NULL);  // Vérification de l'allocation mémoire
  if (game_nb_rows(g) != rows) {
    fprintf(stderr,
            "Erreur : game_nb_rows a retourné %u au lieu de %u pour une grille "
            "3x3.\n",
            game_nb_rows(g), rows);
    success = false;
  }
  game_delete(g);

  // Cas 2: Test avec un jeu vide 5x3
  rows = 5;
  g = game_new_empty_ext(rows, 3, false);
  assert(g != NULL);
  if (game_nb_rows(g) != rows) {
    fprintf(stderr,
            "Erreur : game_nb_rows a retourné %u au lieu de %u pour une grille "
            "5x3.\n",
            game_nb_rows(g), rows);
    success = false;
  }
  game_delete(g);

  // Cas 3: Test avec un jeu vide 10x1 (grille étroite)
  rows = 10;
  g = game_new_empty_ext(rows, 1, false);
  assert(g != NULL);
  if (game_nb_rows(g) != rows) {
    fprintf(stderr,
            "Erreur : game_nb_rows a retourné %u au lieu de %u pour une grille "
            "10x1.\n",
            game_nb_rows(g), rows);
    success = false;
  }
  game_delete(g);

  // Cas 4: Test avec un jeu vide 4x0 (grille vide en colonnes)
  rows = 4;
  g = game_new_empty_ext(rows, 0, false);
  assert(g != NULL);
  if (game_nb_rows(g) != rows) {
    fprintf(stderr,
            "Erreur : game_nb_rows a retourné %u au lieu de %u pour une grille "
            "4x0.\n",
            game_nb_rows(g), rows);
    success = false;
  }
  game_delete(g);

  printf("test_game_nb_rows %s!\n", success ? "passed" : "failed");
  return success;
}

bool test_game_save() {
  // Test avec default.txt
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
  game g_txt = game_new_ext(nb_rows_txt, nb_cols_txt, shapes_txt,
                            orientations_txt, false);

  game_save(g_txt, "test_save.txt");

  game loaded_game_txt = game_load("test_save.txt");
  if (loaded_game_txt == NULL) {
    fprintf(
        stderr,
        "Erreur : Impossible de charger le jeu à partir de test_save.txt\n");
    game_delete(g_txt);
    return false;
  }

  bool result_txt = game_equal(g_txt, loaded_game_txt, false);
  game_delete(g_txt);
  game_delete(loaded_game_txt);

  if (!result_txt) {
    fprintf(stderr,
            "Erreur : Le jeu sauvegardé dans test_save.txt ne correspond pas "
            "au jeu attendu\n");
    return false;
  }

  // Test avec default.sol
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
  game g_sol = game_new_ext(nb_rows_sol, nb_cols_sol, shapes_sol,
                            orientations_sol, false);

  game_save(g_sol, "test_save_sol.txt");

  game loaded_game_sol = game_load("test_save_sol.txt");
  if (loaded_game_sol == NULL) {
    fprintf(stderr,
            "Erreur : Impossible de charger le jeu à partir de "
            "test_save_sol.txt\n");
    game_delete(g_sol);
    return false;
  }

  bool result_sol = game_equal(g_sol, loaded_game_sol, false);
  game_delete(g_sol);
  game_delete(loaded_game_sol);

  if (!result_sol) {
    fprintf(stderr,
            "Erreur : Le jeu sauvegardé dans test_save_sol.txt ne correspond "
            "pas au jeu attendu\n");
    return false;
  }

  return result_txt && result_sol;
}
// Fonction pour vérifier les erreurs liées aux pièces du jeu
void check_pieces(game random, uint nb_empty) {
  int empty_count = 0;
  for (int i = 0; i < game_nb_cols(random); i++) {
    for (int j = 0; j < game_nb_rows(random); j++) {
      shape normal = game_get_piece_shape(random, i, j);
      if (normal == EMPTY) {
        empty_count++;
      }
    }
  }
  if (empty_count != nb_empty) {
    fprintf(stderr, "Erreur: le nombre de cases vides est incorrect.\n");
    game_delete(random);
    exit(EXIT_FAILURE);
  }
}

// Test de base sans options (pas de wrapping, pas de cases vides, pas de pièces
// supplémentaires)
bool test_game_random(void) {
  // Test sans option wrapping, avec 0 case vide et 0 case supplémentaire
  game game_no_wrapping = game_random(DEFAULT_SIZE, DEFAULT_SIZE, false, 0, 0);
  if (!game_no_wrapping) {
    fprintf(stderr,
            "Erreur: game_random a échoué pour le test sans wrapping.\n");
    return false;
  }
  check_pieces(game_no_wrapping, 0);  // Vérifie qu'il n'y a pas de cases vides

  if (!game_won(game_no_wrapping) || !game_is_connected(game_no_wrapping)) {
    fprintf(stderr,
            "Erreur: le jeu sans wrapping n'est pas gagné ou connecté.\n");
    game_delete(game_no_wrapping);
    return false;
  }

  game_delete(game_no_wrapping);

  // Test avec l'option wrapping activée
  game game_with_wrapping = game_random(DEFAULT_SIZE, DEFAULT_SIZE, true, 0, 0);
  if (!game_with_wrapping) {
    fprintf(stderr,
            "Erreur: game_random a échoué pour le test avec wrapping.\n");
    return false;
  }

  check_pieces(game_with_wrapping,
               0);  // Vérifie qu'il n'y a pas de cases vides

  if (!game_won(game_with_wrapping) || !game_is_connected(game_with_wrapping)) {
    fprintf(stderr,
            "Erreur: le jeu avec wrapping n'est pas gagné ou connecté.\n");
    game_delete(game_with_wrapping);
    return false;
  }

  game_delete(game_with_wrapping);

  // Test avec nb_empty
  game game_with_empty = game_random(DEFAULT_SIZE, DEFAULT_SIZE, false, 4, 0);
  if (!game_with_empty) {
    fprintf(stderr,
            "Erreur: game_random a échoué pour le test avec nb_empty.\n");
    return false;
  }

  check_pieces(game_with_empty, 4);  // Vérifie qu'il y a 4 cases vides

  if (!game_won(game_with_empty) || !game_is_connected(game_with_empty)) {
    fprintf(stderr,
            "Erreur: le jeu avec nb_empty n'est pas gagné ou connecté.\n");
    game_delete(game_with_empty);
    return false;
  }

  game_delete(game_with_empty);

  // Test avec nb_extra
  game game_with_extra = game_random(DEFAULT_SIZE, DEFAULT_SIZE, false, 0, 2);
  if (!game_with_extra) {
    fprintf(stderr,
            "Erreur: game_random a échoué pour le test avec nb_extra.\n");
    return false;
  }

  int edge_count = 0;
  for (uint i = 0; i < game_nb_rows(game_with_extra); i++) {
    for (uint j = 0; j < game_nb_cols(game_with_extra); j++) {
      shape s = game_get_piece_shape(game_with_extra, i, j);

      if (s != EMPTY) {
        // Vérifier la connexion avec la pièce à droite
        if (j + 1 < game_nb_cols(game_with_extra)) {
          shape right = game_get_piece_shape(game_with_extra, i, j + 1);
          if (right != EMPTY &&
              game_check_edge(
                  game_with_extra, i, j,
                  game_get_piece_orientation(game_with_extra, i, j)) == MATCH) {
            edge_count++;
          }
        }

        // Vérifier la connexion avec la pièce en bas
        if (i + 1 < game_nb_rows(game_with_extra)) {
          shape down = game_get_piece_shape(game_with_extra, i + 1, j);
          if (down != EMPTY &&
              game_check_edge(
                  game_with_extra, i, j,
                  game_get_piece_orientation(game_with_extra, i, j)) == MATCH) {
            edge_count++;
          }
        }
      }
    }
  }

  printf("Nombre d'arêtes: %d\n", edge_count);

  if (!game_won(game_with_extra) || !game_is_connected(game_with_extra)) {
    fprintf(stderr,
            "Erreur: le jeu avec nb_extra n'est pas gagné ou connecté.\n");
    game_delete(game_with_extra);
    return false;
  }

  game_delete(game_with_extra);

  return true;
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Usage: %s <test_name>\n", argv[0]);
    return EXIT_FAILURE;
  }

  if (strcmp(argv[1], "dummy") == 0) {
    return test_dummy();
  } else if (strcmp(argv[1], "game_get_piece_shape") == 0) {
    return test_game_get_piece_shape() ? EXIT_SUCCESS : EXIT_FAILURE;
  } else if (strcmp(argv[1], "game_new") == 0) {
    return test_game_new() ? EXIT_SUCCESS : EXIT_FAILURE;
  } else if (strcmp(argv[1], "game_won") == 0) {
    return test_game_won();
  } else if (strcmp(argv[1], "game_delete") == 0) {
    return test_game_delete() ? EXIT_SUCCESS : EXIT_FAILURE;
  } else if (strcmp(argv[1], "game_get_ajacent_square") == 0) {
    return test_game_get_ajacent_square() ? EXIT_SUCCESS : EXIT_FAILURE;
  } else if (strcmp(argv[1], "game_print") == 0) {
    return test_game_print() ? EXIT_SUCCESS : EXIT_FAILURE;
  } else if (strcmp(argv[1], "game_is_well_paired") == 0) {
    return test_game_is_well_paired() ? EXIT_SUCCESS : EXIT_FAILURE;
  } else if (strcmp(argv[1], "game_nb_rows") == 0) {
    return test_game_nb_rows() ? EXIT_SUCCESS : EXIT_FAILURE;
  } else if (strcmp(argv[1], "game_nb_cols") == 0) {
    return test_game_nb_cols() ? EXIT_SUCCESS : EXIT_FAILURE;
  } else if (strcmp(argv[1], "game_is_wrapping") == 0) {
    return test_game_is_wrapping() ? EXIT_SUCCESS : EXIT_FAILURE;
  } else if (strcmp(argv[1], "game_save") == 0) {
    return test_game_save() ? EXIT_SUCCESS : EXIT_FAILURE;
  } else if (strcmp(argv[1], "game_random") == 0) {
    return test_game_random() ? EXIT_SUCCESS : EXIT_FAILURE;
  } else {
    printf("Test inconnu: %s\n", argv[1]);
    return EXIT_FAILURE;
  }
}
