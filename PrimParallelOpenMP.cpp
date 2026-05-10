#include <iostream>   
#include <fstream>    
#include <vector>     
#include <climits>    
#include <chrono>     
#include <string>     
#include <omp.h>      

using namespace std;

// Functie care citeste graful din fisier si il transforma in lista de adiacenta
vector<vector<pair<int, int>>> loadGraph(const string& file, int& V)
{
    ifstream f(file); // deschidem fisierul

    // Verificam daca fisierul exista
    if (!f)
    {
        cout << "Error: graph file not found: " << file << endl;
        exit(1); // oprim programul daca nu exista
    }

    f >> V; // citim numarul de noduri

    // Cream lista de adiacenta 
    vector<vector<pair<int, int>>> graph(V);

    // Citim matricea de adiacenta din fisier
    for (int i = 0; i < V; i++)
    {
        for (int j = 0; j < V; j++)
        {
            int w;  // costul muchiei
            f >> w; 

            // Daca exista muchie valida
            if (i != j && w != INT_MAX && w > 0)
            {
                // Adaugam in lista de adiacenta
                graph[i].push_back({ j, w });
            }
        }
    }

    return graph; // returnam graful
}

// Algoritmul Prim paralelizat folosind OpenMP
long long primParallelOpenMP(vector<vector<pair<int, int>>>& graph, int V, int numThreads)
{
    vector<int> key(V, INT_MAX); // cost minim pentru fiecare nod
    vector<char> inMST(V, 0);    // 1 daca nodul este deja in MST 

    key[0] = 0; // pornim din nodul 0
    long long totalCost = 0; // costul total al MST

    // Construim MST
    for (int count = 0; count < V; count++)
    {
        // Determinam nodul cu cost minim 
        int u = -1;
        int minVal = INT_MAX;

        for (int i = 0; i < V; i++)
        {
            // Cautam nodul cu cost minim care nu este in MST
            if (!inMST[i] && key[i] < minVal)
            {
                minVal = key[i];
                u = i;
            }
        }

        // Daca nu gasim nod valid, graful nu este conex
        if (u == -1)
            return -1;

        inMST[u] = 1;         // adaugam nodul in MST
        totalCost += key[u]; // adaugam costul

        // Actualizam vecinii 
        const auto& adj = graph[u]; // lista de vecini ai lui u
        int deg = adj.size();       // numarul de vecini

        // OpenMP imparte vecinii nodului curent intre thread-uri
#pragma omp parallel for num_threads(numThreads)
        for (int i = 0; i < deg; i++)
        {
            int v = adj[i].first;   // nod vecin
            int w = adj[i].second;  // cost muchie

            // Daca nu este in MST si gasim cost mai mic
            if (!inMST[v] && w < key[v])
            {
                key[v] = w; // actualizam costul
            }
        }
    }

    return totalCost; // returnam costul total
}

// Functie care ruleaza un test pentru un graf
void runExperiment(int V, int numThreads)
{
    // Construim numele fisierului
    string file = "../../../graphs/graph_" + to_string(V) + ".txt";

    cout << "Graph size: " << V << " nodes\n";
    cout << "Loading graph from file...\n";

    int realV;
    auto graph = loadGraph(file, realV); // citim graful

    cout << "Running Prim algorithm...\n";

    // Incepem masurarea timpului
    auto t1 = chrono::high_resolution_clock::now();

    // Rulam algoritmul
    long long mstCost = primParallelOpenMP(graph, realV, numThreads);

    // Oprim masurarea timpului
    auto t2 = chrono::high_resolution_clock::now();

    // Calculam durata
    chrono::duration<double> duration = t2 - t1;

    // Afisam rezultatul
    if (mstCost == -1)
    {
        cout << "Graph is not connected.\n\n";
    }
    else
    {
        cout << "MST cost: " << mstCost << "\n";
        cout << "Execution time: " << duration.count() << " seconds\n\n";
    }
}

int main()
{
    int numThreads = 4; // numarul de thread-uri folosite

    // Dimensiunile grafurilor testate
    vector<int> sizes = { 2000, 4000, 8000, 12000, 32000, 40000 };

    cout << "Parallel Prim Algorithm - OpenMP - Performance Test\n";
    cout << "Number of threads: " << numThreads << "\n\n";

    // Rulam testele pentru fiecare dimensiune
    for (int V : sizes)
    {
        runExperiment(V, numThreads);
    }

    return 0; 
}