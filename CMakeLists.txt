cmake_minimum_required(VERSION 3.5)
project(game_text C)

enable_testing()
include(CTest)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "-g -Wall")
set(CMAKE_C_FLAGS_DEBUG "-g -Og")
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG")

# Copier les fichiers nécessaires dans le répertoire de build (à l'exception de big_game.txt)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/test_files/default.txt DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/test_files)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/test_files/default.sol DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/test_files)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/test_files/game11.sol1 DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/test_files)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/test_files/game11.sol2 DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/test_files)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/test_files/game11.txt DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/test_files)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/test_files/big_game.txt DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/test_files)

# Ajout de l'exécutable game_sdl
add_executable(game_sdl main.c model.c)

# Lier les bibliothèques SDL nécessaires
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

target_link_libraries(game_sdl game ${SDL2_LIBRARIES} SDL2_image SDL2_ttf m)


# Copier les ressources dans le dossier build
file(COPY res DESTINATION ${CMAKE_CURRENT_BINARY_DIR})


# Ajout des sources du projet
set(GAME_SOURCES game.c game_aux.c game_ext.c queue.c game_tools.c game_random.c game_solve.c)

# Création de la bibliothèque statique game
add_library(game STATIC ${GAME_SOURCES})

# Ajout des exécutables pour les tests
add_executable(game_text game_text.c)
add_executable(game_test_ambakounzous game_test_ambakounzous.c)
add_executable(game_test_pkouakou game_test_pkouakou.c)
add_executable(game_test_ygballou game_test_ygballou.c)
add_executable(game_random game_random.c)
add_executable(game_solve game_solve.c)

# Lier les exécutables aux bibliothèques nécessaires
target_link_libraries(game_text game)
target_link_libraries(game_test_ambakounzous game)
target_link_libraries(game_test_pkouakou game)
target_link_libraries(game_test_ygballou game)
target_link_libraries(game_random game)
target_link_libraries(game_solve game)

# Ajouter les chemins d'inclusion pour les fichiers d'en-tête
include_directories(${CMAKE_SOURCE_DIR})

# Définir les tests
add_test(test_pkouakou_dummy ./game_test_pkouakou dummy)
add_test(test_pkouakou_game_set_piece_orientation ./game_test_pkouakou game_set_piece_orientation)
add_test(test_pkouakou_game_new_empty ./game_test_pkouakou game_new_empty)
add_test(test_pkouakou_game_equal ./game_test_pkouakou game_equal)
add_test(test_pkouakou_game_play_move ./game_test_pkouakou game_play_move)
add_test(test_pkouakou_game_default_solution ./game_test_pkouakou game_default_solution)
add_test(test_pkouakou_game_check_edge ./game_test_pkouakou game_check_edge)
add_test(test_pkouakou_game_shuffle_orientation ./game_test_pkouakou game_shuffle_orientation)
add_test(test_pkouakou_game_new_empty_ext ./game_test_pkouakou game_new_empty_ext)
add_test(test_pkouakou_game_undo ./game_test_pkouakou game_undo)
add_test(test_pkouakou_game_load ./game_test_pkouakou game_load)
add_test(test_pkouakou_game_solve ./game_test_pkouakou game_solve)
add_test(test_pkouakou_game_nb_solutions ./game_test_pkouakou game_nb_solutions)

add_test(test_ygballou_game_print ./game_test_ygballou game_print)
add_test(test_ygballou_dummy ./game_test_ygballou dummy)
add_test(test_ygballou_game_new ./game_test_ygballou game_new)
add_test(test_ygballou_game_won ./game_test_ygballou game_won)
add_test(test_ygballou_game_get_ajacent_square ./game_test_ygballou game_get_ajacent_square)
add_test(test_ygballou_game_is_well_paired ./game_test_ygballou game_is_well_paired)
add_test(test_ygballou_game_delete ./game_test_ygballou game_delete)
add_test(test_ygballou_game_get_piece_shape ./game_test_ygballou game_get_piece_shape)
add_test(test_ygballou_game_nb_rows ./game_test_ygballou game_nb_rows)
add_test(test_ygballou_game_nb_cols ./game_test_ygballou game_nb_cols)
add_test(test_ygballou_game_is_wrapping ./game_test_ygballou game_is_wrapping)
add_test(test_ygballou_game_save ./game_test_ygballou game_save)
add_test(test_ygballou_game_random ./game_test_ygballou game_random)

add_test(test_ambakounzous_dummy ./game_test_ambakounzous dummy)
add_test(test_ambakounzous_game_copy ./game_test_ambakounzous game_copy)
add_test(test_ambakounzous_game_get_piece_orientation ./game_test_ambakounzous game_get_piece_orientation)
add_test(test_ambakounzous_game_reset_orientation ./game_test_ambakounzous game_reset_orientation)
add_test(test_ambakounzous_game_set_piece_shape ./game_test_ambakounzous game_set_piece_shape)
add_test(test_ambakounzous_game_default ./game_test_ambakounzous game_default)
add_test(test_ambakounzous_game_has_half_edge ./game_test_ambakounzous game_has_half_edge)
add_test(test_ambakounzous_game_is_connected ./game_test_ambakounzous game_is_connected)
add_test(test_ambakounzous_game_new_ext ./game_test_ambakounzous game_new_ext)
add_test(test_ambakounzous_game_redo ./game_test_ambakounzous game_redo)
