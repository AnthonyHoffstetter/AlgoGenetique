#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <mpi.h>

const int TAILLE_POPULATION = 30;
const int NOMBRE_GENERATIONS = 100;
const double TAUX_MUTATION = 0.1;
const double TAUX_CROISEMENT = 0.5;
const int nbSolutionsEnvoi=2;
const int iterationsEnvoi=5;

const int LONGUEUR_ADN = 10; // Chaque case indique dans quel tas se trouve la carte

int evaluerFitness(const std::vector<int>& adn)
{
  int sommeTas1 = 0;
  int produitTas2 = 1;
  for (int i = 0; i < LONGUEUR_ADN; ++i) 
  {
    if (adn[i] == 1) 
    {
      sommeTas1 += (i + 1); // La carte i+1 est ajoutée au tas 1
    } 
    else if (adn[i] == 2) 
    {
      produitTas2 *= (i + 1); // La carte i+1 est ajoutée au tas 2
    }
  }

  int ecartSomme = std::abs(36 - sommeTas1);
  int ecartProduit = std::abs(360 - produitTas2);
  return -(ecartSomme + ecartProduit); // Négatif car on cherche à minimiser l'écart
}


std::vector<int> genererAdnAleatoire()
{
  std::vector<int> adn(LONGUEUR_ADN);
  for (int i = 0; i < LONGUEUR_ADN; ++i)
  {
    adn[i] = rand() % 2 + 1; // Chaque case vaut soit 1 (tas 1) soit 2 (tas 2)
  }
  return adn;
}

void affichageSommeEtProduit(const std::vector<int>& adn)
{
    int sommeTas1 = 0;
    int produitTas2 = 1;
    for (int i = 0; i < LONGUEUR_ADN; ++i)
    {
        if (adn[i] == 1)
        {
            sommeTas1 += (i + 1);
        }
        else if (adn[i] == 2)
        {
            produitTas2 *= (i + 1);
        }
    }
    std::cout << "Somme Tas 1: " << sommeTas1 << " | Produit Tas 2: " << produitTas2 << std::endl << std::endl;
}

int main(int argc , char * argv []) {
    int a, b, Winner, Loser, generation = 0;
    bool arret = false;

    int my_rank;
    int nb_procs;

    //Initialisation MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nb_procs);
    srand(time(0) + my_rank);

    //Initialisation de la population
    std::vector<std::pair<std::vector<int>, int>> population;
    for (int i = 0; i < TAILLE_POPULATION; ++i) 
    {
      std::vector<int> adn = genererAdnAleatoire();
      population.push_back({adn, evaluerFitness(adn)});
    }
    
    //Boucle des générations
    while (!arret && generation < NOMBRE_GENERATIONS)
    {
        if (population[0].second == 0)
        {
            arret = true;
        }

        //Utilisation de MPI_Allreduce pour propager l'information de "solution trouvée" à tous les processus
        int arret_local = arret ? 1 : 0;
        int arret_global = 0;
        MPI_Allreduce(&arret_local, &arret_global, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
        
        if (arret_global == 0) 
        {
          //Sélection et croisement des individus
          for (int j = 0; j < TAILLE_POPULATION / 2; j++)
          {
            //Sélection de deux membres de la population aléatoire
            a = rand() % TAILLE_POPULATION;
            b = rand() % TAILLE_POPULATION;

            //Comparaison des membres pour voir le meilleur des deux
            if (population[a].second > population[b].second)
            {
                Winner = a;
                Loser = b;
            }
            else
            {
                Winner = b;
                Loser = a;
            }

            //Changements possibles de gènes pour le membre Loser
            for (int i = 0; i < LONGUEUR_ADN; i++)
            {
                // Croisement
                if ((rand() * 1.0) / RAND_MAX < TAUX_CROISEMENT) {
                    population[Loser].first[i] = population[Winner].first[i];
                }

                // Mutation
                if ((rand() * 1.0) / RAND_MAX < TAUX_MUTATION)
                {
                    population[Loser].first[i] = (population[Loser].first[i] == 1) ? 2 : 1;
                }
            }

            //Réévaluation du membre Loser
            population[Loser].second = evaluerFitness(population[Loser].first);
          }

          if (generation % iterationsEnvoi == 0) 
          {
            int dest = (my_rank + 1) % nb_procs; // Processus suivant (boucle circulaire)
            for(int i=0;i<nbSolutionsEnvoi;i++)
            {
              MPI_Send(&population[i].first[0],LONGUEUR_ADN, MPI_INT, dest, 0, MPI_COMM_WORLD);
            }
            //std::cout << "Processus " << my_rank << " envoie les meilleures solutions au processus " << dest << std::endl;

            int src = (my_rank - 1 + nb_procs) % nb_procs;
            for(int i=0;i<nbSolutionsEnvoi;i++)
            {
              std::vector<int> solution_recue(LONGUEUR_ADN);
              MPI_Recv(&solution_recue[0], LONGUEUR_ADN, MPI_INT, src, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
              population[TAILLE_POPULATION-i-1].first=solution_recue;
              population[TAILLE_POPULATION-i-1].second=evaluerFitness(population[TAILLE_POPULATION-i].first);
            }
            //std::cout << "Processus " << my_rank << " recoit les meilleures solutions du processus " << src << std::endl;
          }

          //Tri par fitness décroissante
          std::sort(population.begin(), population.end(), [](const auto& a, const auto& b) {
              return a.second > b.second;
          });

          //Affichage de la meilleure solution
          std::cout << "Processus " << my_rank << " Generation " << generation << " - Meilleur fitness: " << population[0].second << std::endl;
          //affichageSommeEtProduit(population[0].first);

          generation++;
        }
        else
        {
          arret=true;
        }
          

    }
    
    MPI_Barrier(MPI_COMM_WORLD);

    if (my_rank != 0) 
    {
      //Chaque processus envoie sa meilleure solution au processus 0
      MPI_Send(&population[0].first[0], LONGUEUR_ADN, MPI_INT, 0, 0, MPI_COMM_WORLD);
      MPI_Send(&population[0].second, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
    } 
    else 
    {
      //Le processus 0 reçoit les meilleures solutions des autres processus
      for (int i = 1; i < nb_procs; i++) 
      {
        std::vector<int> solution_recue(LONGUEUR_ADN);
        int fitness_recue;
        
        MPI_Recv(&solution_recue[0], LONGUEUR_ADN, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&fitness_recue, 1, MPI_INT, i, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      
        population.push_back({solution_recue, fitness_recue});
      }
      
      //Tri
      std::sort(population.begin(), population.end(), [](const auto& a, const auto& b) 
      {
        return a.second > b.second;
      });

      //Affichage de la meilleure solution
      if (population[0].second == 0) 
      {
        std::cout << "Solution optimale trouvée à la génération " << generation - 1 << " :\n";
      } 
      else 
      {
        std::cout << "Meilleure solution non optimale trouvée à la génération " << generation - 1 << " :\n";
      }

      std::cout << "==============================\n";
      std::cout << "Les cartes du tas 1 sont : ";
      for (int i = 0; i < LONGUEUR_ADN; i++) 
      {
        if (population[0].first[i] == 1)
          std::cout << i + 1 << " ";
      }
      std::cout << "\nLes cartes du tas 2 sont : ";
      for (int i = 0; i < LONGUEUR_ADN; i++) 
      {
        if (population[0].first[i] == 2)
          std::cout << i + 1 << " ";
      }
      std::cout << "\nFitness : " << population[0].second << "\n";
    }

    MPI_Finalize();
    return 0;
}
