//
// Created by annabel7 on 17.04.2025.
//

#include "grid.h"
#include <random>
#include <algorithm>
#include <iostream>
#include <vector>

using namespace std;

// Funktsioon segatud ruudustiku genereerimiseks
vector<int> GenerateShuffledGrid(int gridSize) {
    // vektor N*N
    vector<int> grid(gridSize * gridSize);
    for (int i = 0; i < gridSize * gridSize - 1; i++) {
        grid[i] = i + 1;
    }

    random_device rd;
    mt19937 g(rd());
    //Segame
    shuffle(grid.begin(), grid.end(), g);

    return grid;
}

// Funktsioon mis loendab invatitsoone
int getInvCount(vector<int> &arr, int N) {
    int inv_count = 0;
    for (int i = 0; i < N * N - 1; i++) {
        for (int j = i + 1; j < N * N; j++) {
            // Loendame paare (arr[i], arr[j]), kus i < j ja arr[i] > arr[j]
            if (arr[j] && arr[i] && arr[i] > arr[j]) // Ignome tühja kohta (0)
                inv_count++;
        }
    }
    return inv_count;
}

// Funktsioon, mis leiab tühja koha (0) asukoha alt ülespoole
int findXPosition(vector<int> &puzzle, int N) {
    for (int i = N - 1; i >= 0; i--) {
        for (int j = N - 1; j >= 0; j--) {
            if (puzzle[i * N + j] == 0)  // Tühi koht on 0
                return N - i;
        }
    }
    return -1;
}

// Kas on lahendatav?
bool isSolvable(vector<int> &puzzle, int N) {
    int invCount = getInvCount(puzzle, N);

    // Kui ruudustik on paaritu, siis puzle on lahendatav, kui invatsioonide arv on paaris
    if (N & 1) {
        return !(invCount & 1);
    } else {  // Kui ruudustik on paaris
        int pos = findXPosition(puzzle, N); // Leiame tühja koha asukoha
        if (pos & 1) // Kui tühja koha asukoht on paaritu
            return !(invCount & 1);
        else // Kui tühja koha asukoht on paaris
            return invCount & 1;
    }
}

// Funktsioon mis kuvab ruudustiku
void printGrid(const vector<int> &grid, int N) {
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            int index = i * N + j;
            cout << grid[index] << "\t";
        }
        cout << endl;
    }
}


// Funktsioon, mis annab lahendatava
std::vector<int> GenerateSolvablePuzzle(int gridSize) {
    std::vector<int> grid;
    bool solvable = false;

    while (!solvable) {
        grid = GenerateShuffledGrid(gridSize);
        solvable = isSolvable(grid, gridSize);
    }

    return grid;
}
