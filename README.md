# Pricer de Produits Structurés

Application graphique en C++ pour le pricing de produits structurés (Autocalls, Cliquets) utilisant des simulations de Monte Carlo.

## Fonctionnalités

*   **Familles de produits** :
    *   **Autocall** : Simple, Phoenix, Memory Phoenix, Step-Down, Airbag.
    *   **Cliquet** : Max Return, Capped Coupons.
*   **Modèles de diffusion** : Black-Scholes (volatilité constante) et Heston (volatilité stochastique).
*   **Interface Graphique (GUI)** :
    *   Configuration complète des paramètres produits et modèles.
    *   Visualisation graphique du payoff à maturité.
    *   Calcul des grecques (Delta, Vega) et intervalles de confiance.

## Prérequis

*   Compilateur C++17
*   CMake (version 3.15 ou supérieure)
*   **Qt6** (Modules `Widgets` et `Charts` requis)

## Compilation et Exécution

1.  Créer le répertoire de build :
    ```bash
    mkdir build && cd build
    ```

2.  Configurer avec CMake :
    ```bash
    cmake ..
    ```

3.  Compiler le projet :
    ```bash
    make
    ```

4.  Lancer l'interface graphique :
    ```bash
    ./pricer_gui
    ```
    
