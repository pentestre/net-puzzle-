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

#include "queue.h"
#include "game_tools.h"

int main(int argc, char *argv[]) {
    // Vérification du nombre d'arguments
    if (argc < 3 || argc > 4) {
        fprintf(stderr, "Usage: %s <option> <input> [<output>]\n", argv[0]);
        fprintf(stderr, "Options:\n");
        fprintf(stderr, "  -s : Trouver une solution et la sauvegarder\n");
        fprintf(stderr, "  -c : Compter le nombre de solutions\n");
        return EXIT_FAILURE;
    }

    // Récupération des arguments
    char *option = argv[1];
    char *input_file = argv[2];
    char *output_file = (argc == 4) ? argv[3] : NULL;

    // Chargement du jeu depuis le fichier d'entrée
    game g = game_load(input_file);
    if (g == NULL) {
        fprintf(stderr, "Erreur : Impossible de charger le jeu depuis le fichier %s\n", input_file);
        return EXIT_FAILURE;
    }

    // Traitement en fonction de l'option
    if (strcmp(option, "-s") == 0) {
        // Option -s : Trouver une solution
        if (game_solve(g)) {
            // Si une solution est trouvée, sauvegarder ou afficher
            if (output_file) {
                // Sauvegarder dans le fichier de sortie
                game_save(g, output_file);
            } else {
                // Afficher sur la sortie standard
                game_print(g);
            }
        } else {
            // Aucune solution trouvée
            fprintf(stderr, "Aucune solution trouvée pour ce jeu.\n");
            game_delete(g);
            return EXIT_FAILURE;
        }
    } else if (strcmp(option, "-c") == 0) {
        // Option -c: Compter les solutions
        int solution_count = game_nb_solutions(g);
        if (output_file) {
            FILE *output_fp = fopen(output_file, "w");  // Correction ici
            if (!output_fp) {
                fprintf(stderr, "Erreur: Impossible d'ouvrir le fichier de sortie %s\n", output_file);
                game_delete(g);
                return EXIT_FAILURE;
            }
            fprintf(output_fp, "%d\n", solution_count);
            fclose(output_fp);
        } else {
            printf("%d\n", solution_count);
        }
    } else {
        fprintf(stderr, "Erreur: Option invalide %s\n", option);
        game_delete(g);
        return EXIT_FAILURE;
    }

    game_delete(g);
    return EXIT_SUCCESS;
}
