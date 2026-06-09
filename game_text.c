#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "game_aux.h"
#include "game_ext.h"
#include "game_tools.h"

int main(int argc, char *argv[]) {
  char car;
  int i, j;
  game g;

  // Charge un jeu à partir d'un fichier s'il est fourni en argument
  if (argc > 1) {
    g = game_load(argv[1]);
    if (g == NULL) {
      fprintf(stderr, "Erreur lors du chargement du fichier %s\n", argv[1]);
      return EXIT_FAILURE;
    }
  } else {
    g = game_default();
  }

  while (!game_won(g)) {
    game_print(g);  // Affiche l'état du jeu
    printf("> ? [h for help]\n");
    if (scanf(" %c", &car) != 1) {  // Récupère l'action de l'utilisateur
      fprintf(stderr, "Erreur lors de la lecture de l'entrée.\n");
      continue;
    }

    if (car == 'h') {
      printf("> action: help\n");
      printf("- press 'c <i> <j>' to rotate piece clockwise in square (i,j)\n");
      printf(
          "- press 'a <i> <j>' to rotate piece anti-clockwise in square "
          "(i,j)\n");
      printf("- press 'r' to shuffle game\n");
      printf("- press 'q' to quit\n");
      printf("- press 'z' to undo the last move\n");
      printf("- press 'y' to redo the last undone move\n");
      printf("- press 's <filename>' to save the game\n");
    } else if (car == 'r') {
      printf("> action: restart\n");
      game_shuffle_orientation(g);
    } else if (car == 'q') {
      printf("> action: quit\n");
      printf("What a shame, you gave up :-( \n");
      game_delete(g);
      return EXIT_SUCCESS;
    } else if (car == 'c' || car == 'a') {
      // Vérifie la validité des indices de la case avant de jouer un mouvement
      if (scanf("%d %d", &i, &j) != 2 || i < 0 || j < 0 ||
          i >= game_nb_rows(g) || j >= game_nb_cols(g)) {
        printf("Erreur: indices invalides (%d, %d)\n", i, j);
        continue;
      }
      if (car == 'c') {
        printf("> action: play move 'c' into square (%d,%d)\n", i, j);
        game_play_move(g, i, j, 1);
      } else if (car == 'a') {
        printf("> action: play move 'a' into square (%d,%d)\n", i, j);
        game_play_move(g, i, j, -1);
      }
    } else if (car == 'z') {
      // Annule le dernier mouvement
      printf("> action: undo last move\n");
      game_undo(g);
    } else if (car == 'y') {
      // Refait le dernier mouvement annulé
      printf("> action: redo last undone move\n");
      game_redo(g);
    } else if (car == 's') {
      // Sauvegarde l'état courant du jeu dans un fichier
      char filename[256];
      if (scanf("%s", filename) != 1) {
        fprintf(stderr, "Erreur lors de la lecture du nom de fichier.\n");
        continue;
      }
      printf("> action: save game to file %s\n", filename);
      game_save(g, filename);
    } else {
      printf("Commande inconnue: %c\n", car);
    }
  }
  printf("Félicitations, vous avez gagné !\n");
  game_print(g);   // Affiche l'état final du jeu
  game_delete(g);  // Libère les ressources utilisées par le jeu
  return EXIT_SUCCESS;
}
