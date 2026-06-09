#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "game.h"
#include "game_aux.h"
#include "game_ext.h"
#include "game_tools.h"

int main(int argc, char *argv[]) {
  // Vérification du nombre d'arguments
  if (argc < 7 || argc > 8) {
    fprintf(stderr,
            "Erreur: Utilisation correcte: ./game_random <nb_rows> <nb_cols> "
            "<wrapping> <nb_empty> <nb_extra> <shuffle> [filename]\n");
    return EXIT_FAILURE;
  }
  // Initialiser le générateur de nombres aléatoires
  srand(time(NULL));
  // Récupération et conversion des arguments
  uint nb_rows = strtoul(argv[1], NULL, 10);
  uint nb_cols = strtoul(argv[2], NULL, 10);
  bool wrapping = strtoul(argv[3], NULL, 10);
  uint nb_empty = strtoul(argv[4], NULL, 10);
  uint nb_extra = strtoul(argv[5], NULL, 10);
  bool shuffle = strtoul(argv[6], NULL, 10);
  // Vérifications des contraintes
  if (nb_rows * nb_cols < 2) {
    fprintf(stderr, "Erreur: La grille doit contenir au moins 2 cases.\n");
    return EXIT_FAILURE;
  }
  if (nb_empty > (nb_rows * nb_cols - 2)) {
    fprintf(stderr, "Erreur: Trop de cases vides demandées.\n");
    return EXIT_FAILURE;
  }

  // Générer un jeu aléatoire
  game random_game =
      game_random(nb_rows, nb_cols, wrapping, nb_empty, nb_extra);
  if (!random_game) {
    fprintf(stderr, "Erreur: Impossible de générer le jeu.\n");
    return EXIT_FAILURE;
  }

  // Mélanger le jeu si nécessaire
  if (shuffle) {
    game_shuffle_orientation(random_game);
  }

  // Affichage des paramètres du jeu
  printf("nb_rows=%u nb_cols=%u wrapping=%u\n", nb_rows, nb_cols, wrapping);
  printf("nb_empty=%u nb_extra=%u, shuffle=%u\n", nb_empty, nb_extra, shuffle);

  // Affichage ou sauvegarde
  if (argc == 7) {
    game_print(random_game);  // Afficher le jeu
  } else {
    game_print(random_game);
    char *filename = argv[7];  // Récupérer le nom du fichier
    game_save(random_game, filename);
    printf("Jeu sauvegardé dans %s\n", filename);
  }

  // Libération de la mémoire
  game_delete(random_game);
  return EXIT_SUCCESS;
}
