#include <iostream>   
#include <vector>     
#include <climits>    
#include <cstdlib>    
#include <ctime>      
#include <chrono>     
#include <fstream>    
#include <string>     
#include <direct.h>   

using namespace std;

/*
   Verifica daca fisierul exista
*/
bool fileExists(string filename)
{
    ifstream file(filename);   // deschidem fisierul
    return file.good();        // verificam daca exista
}

/*
   Salveaza graful in fisier
*/
void saveGraphToFile(const vector<vector<int>>& graph, int V, string filename)
{
    ofstream file(filename);   // deschidem fisierul pentru scriere

    file << V << "\n";         // scriem numarul de noduri

    // Parcurgem matricea
    for (int i = 0; i < V; i++)
    {
        for (int j = 0; j < V; j++)
        {
            file << graph[i][j] << " "; // scriem fiecare valoare
        }
        file << "\n"; // trecem pe linie noua
    }

    file.close(); // inchidem fisierul
}

/*
   Citeste graful din fisier
*/
vector<vector<int>> loadGraphFromFile(string filename, int& V)
{
    ifstream file(filename); // deschidem fisierul

    file >> V; // citim numarul de noduri

    // Cream matricea
    vector<vector<int>> graph(V, vector<int>(V));

    // Citim matricea
    for (int i = 0; i < V; i++)
    {
        for (int j = 0; j < V; j++)
        {
            file >> graph[i][j]; // citim fiecare valoare
        }
    }

    file.close(); // inchidem fisierul

    return graph; // returnam graful
}

/*
   Gaseste nodul cu cost minim care nu este in MST
*/
int findMinKey(const vector<int>& key, const vector<bool>& inMST, int V)
{
    int minValue = INT_MAX; // valoarea minima
    int minIndex = -1;      // indexul nodului

    // Parcurgem toate nodurile
    for (int i = 0; i < V; i++)
    {
        // Verificam daca nu este in MST si are cost mai mic
        if (!inMST[i] && key[i] < minValue)
        {
            minValue = key[i]; // actualizam minimul
            minIndex = i;      // salvam indexul
        }
    }

    return minIndex; // returnam nodul
}

/*
   Genereaza graf aleator (matrice de adiacenta)
*/
vector<vector<int>> generateGraph(int V)
{
    // Initializam matricea cu INT_MAX (nu exista muchie)
    vector<vector<int>> graph(V, vector<int>(V, INT_MAX));

    // Parcurgem perechile de noduri
    for (int i = 0; i < V; i++)
    {
        for (int j = i + 1; j < V; j++)
        {
            // Probabilitate 30% sa existe muchie
            if (rand() % 100 < 30)
            {
                int w = rand() % 100000 + 1; // greutate aleatoare

                graph[i][j] = w; // muchie i -> j
                graph[j][i] = w; // muchie j -> i
            }
        }
    }

    return graph; // returnam graful
}

/*
   Algoritmul Prim (secvential)
*/
long long primAlgorithm(const vector<vector<int>>& graph, int V)
{
    vector<int> key(V, INT_MAX);   // cost minim pentru fiecare nod
    vector<bool> inMST(V, false);  // daca nodul este in MST
    vector<int> parent(V, -1);     // parintele fiecarui nod

    key[0] = 0; // pornim din nodul 0

    // Construim MST
    for (int step = 0; step < V - 1; step++)
    {
        int u = findMinKey(key, inMST, V); // alegem nodul minim

        inMST[u] = true; // il adaugam in MST

        // Actualizam vecinii
        for (int v = 0; v < V; v++)
        {
            // Daca nu e in MST si costul e mai mic
            if (!inMST[v] && graph[u][v] < key[v])
            {
                key[v] = graph[u][v]; // actualizam costul
                parent[v] = u;        // salvam parintele
            }
        }
    }

    long long totalCost = 0; // cost total

    // Calculam suma costurilor
    for (int i = 1; i < V; i++)
        totalCost += key[i];

    return totalCost; // returnam costul MST
}

/*
   Ruleaza experimentul pentru un graf
*/
void runExperiment(int V)
{
    cout << "Graph size: " << V << " nodes\n"; // afisam dimensiunea

    // Cream folderul graphs daca nu exista
    system("mkdir graphs >nul 2>&1");

    // Construim numele fisierului
    string filename = "graphs/graph_" + to_string(V) + ".txt";

    vector<vector<int>> graph; // graful

    // Verificam daca fisierul exista
    if (fileExists(filename))
    {
        cout << "Loading graph from file...\n";
        graph = loadGraphFromFile(filename, V); // citim din fisier
    }
    else
    {
        cout << "Generating graph and saving to file...\n";
        graph = generateGraph(V);              // generam graf
        saveGraphToFile(graph, V, filename);   // salvam in fisier
    }

    // Pornim cronometrarea
    auto start = chrono::high_resolution_clock::now();

    long long mstCost = primAlgorithm(graph, V); // rulam Prim

    // Oprim cronometrarea
    auto end = chrono::high_resolution_clock::now();

    chrono::duration<double> duration = end - start; // calculam durata

    cout << "MST cost: " << mstCost << endl; // afisam costul
    cout << "Execution time: " << duration.count() << " seconds\n\n"; // afisam timpul
}

int main()
{
    srand(time(NULL)); // initializam rand()

    // Dimensiunile grafurilor
    vector<int> testSizes =
    {
        2000,
        4000,
        8000,
        12000,
        32000,
        40000
    };

    cout << "Sequential Prim Algorithm - Performance Test\n\n";

    // Rulam pentru fiecare dimensiune
    for (int size : testSizes)
    {
        runExperiment(size); // apelam experimentul
    }

    return 0; 
}