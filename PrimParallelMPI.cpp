#include "mpi.h"
#include <iostream>
#include <vector>
#include <climits>
#include <fstream>
#include <string>

using namespace std;

// Functie care imparte nodurile grafului intre procese
void computeRange(int V, int rank, int P, int& start, int& end)
{
    int base = V / P;        // cate noduri primeste fiecare proces 
    int rem = V % P;         // noduri ramase 

    // Procesele cu rank mai mic primesc cate un nod in plus
    start = rank * base + min(rank, rem);
    end = start + base + (rank < rem);
}

// Citeste graful din fisier, dar pastreaza doar partea care apartine procesului curent
vector<vector<pair<int, int>>> loadLocalGraph(const string& file, int V, int start, int end)
{
    ifstream f(file);

    // Daca fisierul nu exista, oprim toate procesele
    if (!f)
    {
        cout << "Error: graph file not found: " << file << endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    int n;
    f >> n; // citim numarul de noduri 

    // Fiecare proces isi creeaza doar subgraful sau (nodurile din intervalul [start, end))
    vector<vector<pair<int, int>>> graph(end - start);

    // Citim toata matricea din fisier
    for (int i = 0; i < V; i++)
    {
        for (int j = 0; j < V; j++)
        {
            int w;
            f >> w; // greutatea muchiei i -> j

            // Daca nodul i apartine procesului curent si exista muchie valida
            if (i >= start && i < end && i != j && w != INT_MAX)
            {
                // Salvam muchia in lista de adiacenta locala
                graph[i - start].push_back({ j, w });
            }
        }
    }

    return graph;
}

// Algoritmul Prim paralel folosind MPI
long long primMPI(const vector<vector<pair<int, int>>>& graph, int V, int rank, int P, int start, int end)
{
    int localN = end - start; // cate noduri are acest proces

    // key[i] = cost minim pentru nodul i
    vector<int> key(localN, INT_MAX);

    // inMST[i] = 1 daca nodul este deja in MST
    vector<int> inMST(localN, 0);

    // Pornim din nodul 0 
    if (0 >= start && 0 < end)
        key[0 - start] = 0;

    // Buffer pentru muchiile grafului 
    vector<int> row(V, INT_MAX);

    // Construim arborele MST
    for (int step = 0; step < V - 1; step++)
    {
        int localMin = INT_MAX;
        int localIndex = -1;

        // Fiecare proces cauta minimul local
        for (int i = 0; i < localN; i++)
        {
            if (!inMST[i] && key[i] < localMin)
            {
                localMin = key[i];
                localIndex = start + i; // index global
            }
        }

        // Trimitem (cost minim, nod), iar MPI gaseste minimul global
        struct { int val, idx; } local = { localMin, localIndex }, global;

        // Determinam minimul global dintre toate procesele
        MPI_Allreduce(&local, &global, 1, MPI_2INT, MPI_MINLOC, MPI_COMM_WORLD);

        int u = global.idx; // nodul ales global

        // Daca nu exista nod valid (graful nu este conex)
        if (u == -1)
            return -1;

        // Procesul care detine nodul u il marcheaza ca fiind in MST
        if (u >= start && u < end)
        {
            inMST[u - start] = 1;

            // Initializam linia
            fill(row.begin(), row.end(), INT_MAX);

            // Copiem vecinii nodului u
            for (auto& edge : graph[u - start])
                row[edge.first] = edge.second;
        }

        // Determinam procesul care detine nodul u
        int owner = -1;

        for (int r = 0; r < P; r++)
        {
            int s, e;
            computeRange(V, r, P, s, e);

            if (u >= s && u < e)
            {
                owner = r;
                break;
            }
        }

        // Trimitem catre toate procesele muchiile nodului u
        MPI_Bcast(row.data(), V, MPI_INT, owner, MPI_COMM_WORLD);

        // Fiecare proces isi actualizeaza valorile key
        for (int i = 0; i < localN; i++)
        {
            if (!inMST[i] && row[start + i] < key[i])
                key[i] = row[start + i];
        }
    }

    // Fiecare proces calculeaza costul partial
    long long localCost = 0;

    for (int i = 0; i < localN; i++)
    {
        if (start + i != 0) // excludem nodul 0
            localCost += key[i];
    }

    // Combinam rezultatele din toate procesele
    long long totalCost = 0;
    MPI_Reduce(&localCost, &totalCost, 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

    return totalCost;
}

// Ruleaza experimentul pentru un graf
void runExperimentMPI(int V, int rank, int P)
{
    string file = "graphs/graph_" + to_string(V) + ".txt";

    int start, end;
    computeRange(V, rank, P, start, end);

    // Afisam doar din procesul 0
    if (rank == 0)
    {
        cout << "\n";
        cout << "Graph size: " << V << " nodes\n";
        cout << "Loading graph from file...\n";
        cout.flush();
    }

    // Fiecare proces isi incarca doar partea lui din graf (subgraful local)
    auto graph = loadLocalGraph(file, V, start, end);

    if (rank == 0)
    {
        cout << "Running Prim algorithm...\n";
        cout.flush();
    }

    // Sincronizam procesele inainte de masurare
    MPI_Barrier(MPI_COMM_WORLD);
    double t1 = MPI_Wtime();

    // Rulam algoritmul
    long long mstCost = primMPI(graph, V, rank, P, start, end);

    // Sincronizam dupa executie
    MPI_Barrier(MPI_COMM_WORLD);
    double t2 = MPI_Wtime();

    // Afisam rezultatul doar o data (procesul 0)
    if (rank == 0)
    {
        if (mstCost == -1)
        {
            cout << "Graph is not connected. MST cannot be computed.\n";
        }
        else
        {
            cout << "MST cost: " << mstCost << "\n";
            cout << "Execution time: " << (t2 - t1) << " seconds\n";
        }

        cout << "\n";
        cout.flush();
    }
}

// Functia principala
int main(int argc, char* argv[])
{
    // Initializam MPI
    MPI_Init(&argc, &argv);

    int rank, P;

    // Obtinem rank-ul procesului curent
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Obtinem numarul total de procese
    MPI_Comm_size(MPI_COMM_WORLD, &P);

    // Dimensiunile grafurilor testate
    vector<int> sizes = { 2000, 4000, 8000, 12000, 32000, 40000 };

    if (rank == 0)
    {
        cout << "Parallel Prim Algorithm - MPI - Performance Test\n";
        cout << "Number of processes: " << P << "\n";
    }

    // Rulam experimentele
    for (int V : sizes)
    {
        runExperimentMPI(V, rank, P);

        // Sincronizare intre rulari
        MPI_Barrier(MPI_COMM_WORLD);
    }

    // Inchidem MPI
    MPI_Finalize();

    return 0;
}