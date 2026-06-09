#include "model.h"
#include <SDL.h>
#include <SDL_image.h>  
#include <SDL_ttf.h>    
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "game.h"
#include "game_aux.h"
#include "game_ext.h"
#include "game_tools.h"
#include "queue.h"
#include "game_struct.h"


Env* init(SDL_Window* win, SDL_Renderer* ren, int argc, char* argv[]) {
    Env* env = malloc(sizeof(Env));
    if (!env) {
        fprintf(stderr, "Erreur d'allocation mémoire\n");
        exit(EXIT_FAILURE);
    }

    /* Initialisation des textures */
    env->background = IMG_LoadTexture(ren, "res/background.png");
    if (!env->background) {
        fprintf(stderr, "Erreur chargement background\n");
        SDL_SetRenderDrawColor(ren, 240, 240, 245, 255);
    }

    /* Chargement des pièces du jeu */
    const char* piece_files[] = {
        [ENDPOINT] = "res/endpoint.png",
        [SEGMENT] = "res/segment.png",
        [CORNER] = "res/corner.png",
        [TEE] = "res/tee.png",
        [CROSS] = "res/cross.png"
    };

    for (int i = 0; i < NB_SHAPES; i++) {
        if (piece_files[i]) {
            env->pieces[i] = IMG_LoadTexture(ren, piece_files[i]);
            if (!env->pieces[i]) {
                fprintf(stderr, "Erreur chargement pièce %d (%s)\n", i, piece_files[i]);
            }
        } else {
            env->pieces[i] = NULL;
        }
    }

    /* Initialisation des boutons */
    env->hovered_button = -1;
    SDL_GetWindowSize(win, &env->window_width, &env->window_height);
    
    TTF_Font* font = TTF_OpenFont("res/arial.ttf", 14);
    if (font) {
        SDL_Color black = {0, 0, 0, 255};
        
        const char* button_texts[BUTTON_COUNT] = {
            "New", "Restart", "Solve", "Undo", "Redo", "Quit"
        };
        
        for (int i = 0; i < BUTTON_COUNT; i++) {
            SDL_Surface* surf = TTF_RenderText_Blended(font, button_texts[i], black);
            if (surf) {
                env->button_textures[i] = SDL_CreateTextureFromSurface(ren, surf);
                SDL_FreeSurface(surf);
            } else {
                fprintf(stderr, "Erreur création surface bouton %d\n", i);
            }
        }
        TTF_CloseFont(font);
    }

    /* Initialisation du jeu */
    if (argc > 1) {
        env->current_game = game_load(argv[1]);
        env->initial_game = game_load(argv[1]);
    } else {  
        game game_default = game_random(4, 4, 1, 0, 0);  
        game_shuffle_orientation(game_default);
        env->current_game = game_default;
        env->initial_game = game_copy(env->current_game);
    }
    
    env->grid_width = game_nb_cols(env->current_game);
    env->grid_height = game_nb_rows(env->current_game);
    
    return env;
}

void render(SDL_Window* win, SDL_Renderer* ren, Env* env) {
    SDL_GetWindowSize(win, &env->window_width, &env->window_height);

    /* Fond d'écran */
    if (env->background) {
        SDL_RenderCopy(ren, env->background, NULL, NULL);
    } else {
        SDL_SetRenderDrawColor(ren, 240, 240, 245, 255);
        SDL_RenderClear(ren);
    }

    /* Barre d'outils */
    SDL_Rect toolbar_rect = {0, 0, env->window_width, TOOLBAR_HEIGHT};
    SDL_SetRenderDrawColor(ren, 220, 220, 230, 255);
    SDL_RenderFillRect(ren, &toolbar_rect);
    SDL_SetRenderDrawColor(ren, 180, 180, 190, 255);
    SDL_RenderDrawRect(ren, &toolbar_rect);

    /* Boutons sur une ligne */
    int total_width = BUTTON_COUNT * BUTTON_WIDTH + (BUTTON_COUNT - 1) * BUTTON_SPACING;
    int start_x = (env->window_width - total_width) / 2;

    for (int i = 0; i < BUTTON_COUNT; i++) {
        if (env->button_textures[i]) {
            int text_w, text_h;
            SDL_QueryTexture(env->button_textures[i], NULL, NULL, &text_w, &text_h);
            
            SDL_Rect btn_rect = {
                start_x + i * (BUTTON_WIDTH + BUTTON_SPACING),
                TOOLBAR_PADDING,
                BUTTON_WIDTH,
                BUTTON_HEIGHT
            };

            /* Couleur du bouton selon l'état */
            if (env->hovered_button == i) {
                SDL_SetRenderDrawColor(ren, 200, 220, 255, 255);
            } else {
                SDL_SetRenderDrawColor(ren, 240, 240, 250, 255);
            }
            SDL_RenderFillRect(ren, &btn_rect);
            
            /* Bordure du bouton */
            SDL_SetRenderDrawColor(ren, 180, 180, 190, 255);
            SDL_RenderDrawRect(ren, &btn_rect);
            
            /* Texte centré */
            SDL_Rect text_rect = {
                btn_rect.x + (btn_rect.w - text_w)/2,
                btn_rect.y + (btn_rect.h - text_h)/2,
                text_w,
                text_h
            };
            SDL_RenderCopy(ren, env->button_textures[i], NULL, &text_rect);
        }
    }

    /* Grille de jeu */
    int available_width = env->window_width - 40;
    int available_height = env->window_height - TOOLBAR_HEIGHT - 40;
    env->cell_size = fmin(
        available_width / env->grid_width,
        available_height / env->grid_height
    );
    
    int grid_width = env->grid_width * env->cell_size;
    int grid_height = env->grid_height * env->cell_size;
    int grid_x = (env->window_width - grid_width) / 2;
    int grid_y = TOOLBAR_HEIGHT + (env->window_height - TOOLBAR_HEIGHT - grid_height) / 2;
    
    SDL_Rect cell_rect;
    for (int i = 0; i < env->grid_height; i++) {
        for (int j = 0; j < env->grid_width; j++) {
            shape s = game_get_piece_shape(env->current_game, i, j);
            direction d = game_get_piece_orientation(env->current_game, i, j);
            
            cell_rect.x = grid_x + j * env->cell_size;
            cell_rect.y = grid_y + i * env->cell_size;
            cell_rect.w = env->cell_size;
            cell_rect.h = env->cell_size;

            if (s != EMPTY && s < NB_SHAPES && env->pieces[s]) {
                SDL_RenderCopyEx(ren, env->pieces[s], NULL, &cell_rect, d * 90, NULL, SDL_FLIP_NONE);
            }
        }
    }

    SDL_RenderPresent(ren);
}

bool process(SDL_Window* win, SDL_Renderer* ren, Env* env, SDL_Event* e) {
    switch (e->type) {
        case SDL_QUIT:
            return true;
            
        case SDL_WINDOWEVENT:
            if (e->window.event == SDL_WINDOWEVENT_RESIZED) {
                render(win, ren, env);
            }
            break;
            
        case SDL_MOUSEMOTION:
            /* Gestion du survol des boutons */
            env->hovered_button = -1;
            if (e->motion.y <= TOOLBAR_HEIGHT) {
                int total_width = BUTTON_COUNT * BUTTON_WIDTH + (BUTTON_COUNT - 1) * BUTTON_SPACING;
                int start_x = (env->window_width - total_width) / 2;
                
                for (int i = 0; i < BUTTON_COUNT; i++) {
                    SDL_Rect btn_rect = {
                        start_x + i * (BUTTON_WIDTH + BUTTON_SPACING),
                        TOOLBAR_PADDING,
                        BUTTON_WIDTH,
                        BUTTON_HEIGHT
                    };
                    if (SDL_PointInRect(&(SDL_Point){e->motion.x, e->motion.y}, &btn_rect)) {
                        env->hovered_button = i;
                        break;
                    }
                }
            }
            break;
            
        case SDL_MOUSEBUTTONDOWN:
            if (e->button.y <= TOOLBAR_HEIGHT) {
                /* Clic dans la barre d'outils */
                int total_width = BUTTON_COUNT * BUTTON_WIDTH + (BUTTON_COUNT - 1) * BUTTON_SPACING;
                int start_x = (env->window_width - total_width) / 2;
                
                for (int i = 0; i < BUTTON_COUNT; i++) {
                    SDL_Rect btn_rect = {
                        start_x + i * (BUTTON_WIDTH + BUTTON_SPACING),
                        TOOLBAR_PADDING,
                        BUTTON_WIDTH,
                        BUTTON_HEIGHT
                    };
                    
                    if (SDL_PointInRect(&(SDL_Point){e->button.x, e->button.y}, &btn_rect)) {
                        /* Animation de clic */
                        SDL_SetRenderDrawColor(ren, 180, 200, 255, 255);
                        SDL_RenderFillRect(ren, &btn_rect);
                        SDL_RenderPresent(ren);
                        SDL_Delay(100);
                        
                        /* Actions */
                        switch (i) {
                            case 0: /* New game */
                                game_delete(env->current_game);
                                env->initial_game = game_random(4, 4, 1, 0, 0);
                                game_shuffle_orientation(env->initial_game);
                                env->current_game = game_copy(env->initial_game);
                                env->grid_width = game_nb_cols(env->current_game);
                                env->grid_height = game_nb_rows(env->current_game);
                                break;
                            case 1: /* Restart game */
                                game_delete(env->current_game);
                                env->current_game = game_copy(env->initial_game);
                                break;
                            case 2: /* Solve game */
                                game_solve(env->current_game);
                                break;
                            case 3: /* Undo */
                                game_undo(env->current_game);
                                break;
                            case 4: /* Redo */
                                game_redo(env->current_game);
                                break;
                            case 5: /* Quit */
                                game_delete(env->current_game);
                                return true;
                        }
                        break;
                    }
                }
            } else {
                /* Clic dans la grille de jeu */
                int available_width = env->window_width - 40;
                int available_height = env->window_height - TOOLBAR_HEIGHT - 40;
                int cell_size = fmin(
                    available_width / env->grid_width, 
                    available_height / env->grid_height
                );
                int grid_width = env->grid_width * cell_size;
                int grid_height = env->grid_height * cell_size;
                int grid_x = (env->window_width - grid_width) / 2;
                int grid_y = TOOLBAR_HEIGHT + (env->window_height - TOOLBAR_HEIGHT - grid_height) / 2;
                
                int x = (e->button.x - grid_x) / cell_size;
                int y = (e->button.y - grid_y) / cell_size;
                
                if (x >= 0 && x < env->grid_width && y >= 0 && y < env->grid_height) {
                    game_play_move(env->current_game, y, x, 1);
                }
            }
            break;
            
        case SDL_KEYDOWN:
            switch (e->key.keysym.sym) {
                case SDLK_r: /* Réinitialiser */
                    game_delete(env->current_game);
                    env->current_game = game_copy(env->initial_game);
                    break;
                case SDLK_z: /* Annuler */
                    game_undo(env->current_game);
                    break;
                case SDLK_y: /* Rétablir */
                    game_redo(env->current_game);
                    break;
                case SDLK_ESCAPE: /* Quitter */
                    return true;
            }
            break;
    }
    return false;
}

void clean(SDL_Window* win, SDL_Renderer* ren, Env* env) {
    /* Nettoyage des textures */
    if (env->background) SDL_DestroyTexture(env->background);
    
    for (int i = 0; i < NB_SHAPES; i++) {
        if (env->pieces[i]) SDL_DestroyTexture(env->pieces[i]);
    }
    
    for (int i = 0; i < BUTTON_COUNT; i++) {
        if (env->button_textures[i]) SDL_DestroyTexture(env->button_textures[i]);
    }
    
    /* Nettoyage du jeu */
    if (env->current_game) game_delete(env->current_game);
    if (env->initial_game) game_delete(env->initial_game);
    
    free(env);
}