# Projet en Algorithmie Parallèle

Le projet consiste à implémenter un algorithme génétique parallèle en utilisant l'API MPI (Message Passing Interface), où chaque processus exécute indépendamment l'algorithme génétique sur sa propre population d'individus. L'objectif est de tirer parti de la parallélisation pour améliorer l'efficacité de la recherche de solutions optimales. La spécificité de ce projet réside dans la communication entre processus, qui doit être gérée de manière efficace pour garantir une convergence rapide vers la meilleure solution.

Objectifs et fonctionnement :

# 1) Algorithme génétique parallèle :

Chaque processus MPI exécute l'algorithme génétique indépendamment sur sa propre population.

À chaque itération, les processus doivent évaluer et modifier leurs populations en fonction des principes de sélection, croisement, mutation, et évaluation de la fitness.

# 2) Communication entre processus :

Échange de solutions : Tous les i itérations, chaque processus envoie ses deux meilleures solutions (les individus ayant la meilleure fitness) au processus suivant. Cette communication permet d’assurer une certaine homogénéisation des populations entre processus et de favoriser une exploration plus large de l’espace de recherche.

# 3) Détection de la solution optimale :

Lorsqu'un processus trouve la solution optimale (c'est-à-dire un individu dont la fitness dépasse un seuil de performance prédéfini ou atteint un critère d'arrêt), il doit envoyer cette solution au processus 0.

Le processus ayant trouvé la solution optimale doit également notifier tous les autres processus afin de leur indiquer d’arrêter leurs calculs. Cela permet d’éviter des calculs inutiles et d’optimiser l’utilisation des ressources.

# 4) Affichage de la solution optimale :

Le processus 0 est chargé d’afficher la solution optimale une fois qu’elle a été trouvée par un processus quelconque. Ce processus central recueille les informations et affiche le résultat final du calcul.
