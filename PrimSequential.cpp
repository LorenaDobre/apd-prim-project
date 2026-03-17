#include <iostream>  
#include <vector>     
#include <climits>   
#include <cstdlib>    
#include <ctime>      
#include <chrono>     

using namespace std;

/*
   Functie care gaseste nodul cu valoarea minima din vectorul key dintre nodurile care nu sunt deja incluse in MST
*/
int findMinKey(const vector<int>& key, const vector<bool>& inMST, int V)
{
    int minValue = INT_MAX;   // Retine cea mai mica valoare gasita
    int minIndex = -1;        // Retine indexul nodului cu valoare minima

    // Parcurgem toate nodurile
    for (int i = 0; i < V; i++)
    {
        // Verificam daca nodul nu este in MST si are cost mai mic
        if (!inMST[i] && key[i] < minValue)
        {
            minValue = key[i];   // Actualizam valoarea minima
            minIndex = i;        // Salvam indexul nodului
        }
    }

    return minIndex;   // Returnam nodul cu cost minim
}

/*
   Functie care genereaza un graf aleator
   Graful este reprezentat prin matrice de adiacenta
*/
vector<vector<int>> generateGraph(int V)
{
    // Cream matricea V x V si initializam cu INT_MAX (nu exista muchie)
    vector<vector<int>> graph(V, vector<int>(V, INT_MAX));

    // Parcurgem toate perechile de noduri
    for (int i = 0; i < V; i++)
    {
        for (int j = i + 1; j < V; j++)
        {
            // Generam probabilitatea ca muchia sa existe
            if (rand() % 100 < 30) // 30% probabilitate
            {
                int w = rand() % 100000 + 1; // Greutate aleatoare

                graph[i][j] = w;  // Muchie i -> j
                graph[j][i] = w;  // Muchie j -> i (graf neorientat)
            }
        }
    }

    return graph;   // Returnam graful generat
}

/*
   Implementarea secventiala a algoritmului Prim
   Determina arborele partial de cost minim (MST)
*/
long long primAlgorithm(const vector<vector<int>>& graph, int V)
{
    vector<int> key(V, INT_MAX);   // Cost minim pentru fiecare nod
    vector<bool> inMST(V, false);  // Indica daca nodul este in MST
    vector<int> parent(V, -1);     // Retine parintele fiecarui nod

    key[0] = 0;   // Primul nod este punctul de start

    // MST va avea V-1 muchii
    for (int step = 0; step < V - 1; step++)
    {
        // Gasim nodul cu cost minim
        int u = findMinKey(key, inMST, V);

        // Marcam nodul ca fiind inclus in MST
        inMST[u] = true;

        // Actualizam costurile pentru toate nodurile vecine
        for (int v = 0; v < V; v++)
        {
            // Verificam daca nodul nu este in MST si costul este mai mic
            if (!inMST[v] && graph[u][v] < key[v])
            {
                key[v] = graph[u][v]; // Actualizam costul minim
                parent[v] = u;        // Salvam parintele in arbore
            }
        }
    }

    long long totalCost = 0;   // Costul total al MST

    // Adunam costurile muchiilor din MST
    for (int i = 1; i < V; i++)
        totalCost += key[i];

    return totalCost;   // Returnam costul total
}

/*
   Functie care ruleaza algoritmul pentru un graf de dimensiune V
   si masoara timpul de executie
*/
void runExperiment(int V)
{
    cout << "Graph size: " << V << " nodes\n"; // Afisam dimensiunea grafului

    auto graph = generateGraph(V); // Generam graful aleator

    auto start = chrono::high_resolution_clock::now(); // Pornim cronometrarea

    long long mstCost = primAlgorithm(graph, V); // Rulam algoritmul Prim

    auto end = chrono::high_resolution_clock::now(); // Oprim cronometrarea

    chrono::duration<double> duration = end - start; // Calculam durata

    cout << "MST cost: " << mstCost << endl; // Afisam costul MST
    cout << "Execution time: " << duration.count() << " seconds\n\n"; // Afisam timpul
}

int main()
{
    srand(time(NULL)); // Initializam generatorul de numere aleatoare

    // Dimensiunile grafurilor pentru test
    vector<int> testSizes =
    {
        2000,
        4000,
        8000,
        12000,
        32000,
        40000
    };

    cout << "Sequential Prim Algorithm - Performance Test\n\n"; // Mesaj de start

    // Rulam experimentul pentru fiecare dimensiune
    for (int size : testSizes)
    {
        runExperiment(size); // Apelam functia de test
    }

    return 0; 
}