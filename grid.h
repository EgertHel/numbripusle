//
// Created by annabel7 on 17.04.2025.
//

#ifndef NUMBRIPUSLE_GRID_H
#define NUMBRIPUSLE_GRID_H

#include <vector>

std::vector<int> GenerateShuffledGrid(int gridSize);
int getInvCount(std::vector<int> &arr, int N);
int findXPosition(std::vector<int> &puzzle, int N);
bool isSolvable(std::vector<int> &puzzle, int N);
void printGrid(const std::vector<int> &grid, int N);
std::vector<int> GenerateSolvablePuzzle(int gridSize);
bool IsPuzzleSolved(std::vector<int> &grid, int N);

class grid {

};


#endif //NUMBRIPUSLE_GRID_H
