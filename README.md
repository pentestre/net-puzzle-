# net-a45

Projet C de jeu de grille avec une version texte et une version SDL.

## Prérequis

- CMake
- un compilateur C compatible C99
- bibliothèques SDL2, SDL2_image et SDL2_ttf pour lancer la version graphique

## Compilation

Depuis la racine du projet :

```sh
mkdir -p build
cd build
cmake ..
make
```

## Lancer le jeu depuis le terminal

### Version texte (la plus simple dans un terminal)

```sh
cd build
./game_text
```

### Version graphique SDL

```sh
cd build
./game_sdl
```

Si la fenêtre SDL ne s’ouvre pas correctement dans votre terminal, utilisez d’abord la version texte.

### Version web sur serveur

La version web est disponible dans le dossier [web/](web/). Pour la tester dans un navigateur, démarrez un petit serveur local :

```sh
cd web
python3 -m http.server 8080
```

Puis ouvrez :

```text
http://127.0.0.1:8080/game.html
```

Cette interface utilise la version WebAssembly du projet et peut aussi être déployée sur un serveur web standard.

## Tests

```sh
cd build
ctest --output-on-failure
```

## Description rapide

Le projet contient :

- une bibliothèque de logique de jeu,
- une interface texte pour jouer dans le terminal,
- une interface SDL pour jouer avec une fenêtre graphique,
- des tests CTest pour valider les fonctionnalités principales.

# Projet de Jeu

Ce projet implémente un jeu de grille en C. Il comprend des fonctions de base pour créer, manipuler et vérifier l'état du jeu. Le jeu utilise une grille où chaque case peut contenir une pièce avec une forme et une orientation spécifiques. Les joueurs peuvent manipuler les pièces pour résoudre le jeu en respectant certaines règles de connexion et d'orientation.

## Fonctionnalités

- Création d'un jeu vide ou pré-initialisé
- Manipulation des pièces (formes et orientations)
- Vérification de l'état du jeu (gagné ou non, bien apparié, connecté)
- Impression de l'état du jeu
- Réinitialisation et mélange des orientations des pièces

## Interface Graphique

L'interface graphique du jeu "Net" permet de jouer via la souris ou le toucher (sur Android). Les fonctionnalités suivantes sont disponibles :
- Rotation des pièces en cliquant/touchant une case.
- Annuler/refaire un coup avec les touches `Z` et `Y`.
- Redémarrer le jeu avec la touche `R`.
- Quitter le jeu avec la touche `Échap`.

Les erreurs sont affichées à l'écran lorsque l'utilisateur tente de jouer un coup invalide.

## Auteurs

- M'BAKOU Alan
- KOUAKOU Paul-Henri
- GBALLOU Yann-Kevin

## Notes

- La version texte est recommandée si vous voulez tester le jeu dans un terminal sans interface graphique.
- La version SDL exige un environnement graphique disponible.
- Les tests sont disponibles via la commande :

```sh
cd build
ctest --output-on-failure
```

