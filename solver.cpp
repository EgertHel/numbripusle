//
// Created by annabel7 on 20.05.2025.
//

#include "solver.h"
#include <queue>
#include <algorithm>
#include <map>

struct PuzzleNode {
    std::vector<int> seis;
    int g; // tee pikkus algusest
    int h; // hinnang jäänud sammudele
    PuzzleNode* eelmine; // Viide eelnevale seisundile

    // Hindamisfunktsioon f = g + h, mida A* algoritm kasutab otsustamiseks
    int f() const { return g + h; }

    bool operator>(const PuzzleNode& other) const {
        return f() > other.f();
    }
};

// Manhattan kaugus
int Manhattan(const std::vector<int>& seis, int gridSize) {
    int summa = 0;
    for (int i = 0; i < seis.size(); ++i) {
        if (seis[i] == 0) continue; // Tühi koht (0) ei mõjuta kaugust
        int õigeRow = (seis[i] - 1) / gridSize;
        int õigeCol = (seis[i] - 1) % gridSize;
        int curRow = i / gridSize;
        int curCol = i % gridSize;
        summa += std::abs(õigeRow - curRow) + std::abs(õigeCol - curCol);
    }
    return summa;
}

// Funktsioon, mis genereerib antud seisundi kõik naabrid
std::vector<std::vector<int>> TeeNaabrid(const std::vector<int>& seis, int gridSize) {
    std::vector<std::vector<int>> naabrid;
    int nullIndex = std::find(seis.begin(), seis.end(), 0) - seis.begin();
    int row = nullIndex / gridSize;
    int col = nullIndex % gridSize;

    // Võimalikud liikumissuundad
    std::vector<std::pair<int, int>> suunad = {{-1,0},{1,0},{0,-1},{0,1}}; // üleval, all, vasak, parem

    for (auto [dr, dc] : suunad) {
        int uusRow = row + dr;
        int uusCol = col + dc;
        // kas uus koht on mängulaua piires?
        if (uusRow >= 0 && uusRow < gridSize && uusCol >= 0 && uusCol < gridSize) {
            int uusIndex = uusRow * gridSize + uusCol;
            std::vector<int> uusSeis = seis;
            std::swap(uusSeis[nullIndex], uusSeis[uusIndex]);
            naabrid.push_back(uusSeis);
        }
    }

    return naabrid;
}

// Peamine A* algoritm
std::vector<std::vector<int>> LahendaAStar(const std::vector<int>& algSeis, int gridSize) {
    auto cmp = [](PuzzleNode* a, PuzzleNode* b) { return a->f() > b->f(); };
    std::priority_queue<PuzzleNode*, std::vector<PuzzleNode*>, decltype(cmp)> open(cmp);

    std::map<std::vector<int>, int> kulud;

    auto* algNode = new PuzzleNode{algSeis, 0, Manhattan(algSeis, gridSize), nullptr};
    open.push(algNode);
    kulud[algSeis] = 0;

    std::vector<int> eesmärk(algSeis.size());
    for (int i = 0; i < eesmärk.size() - 1; ++i) eesmärk[i] = i + 1;
    eesmärk.back() = 0;

    while (!open.empty()) {
        PuzzleNode* current = open.top();
        open.pop();

        if (current->seis == eesmärk) {
            std::vector<std::vector<int>> tee;
            for (PuzzleNode* n = current; n != nullptr; n = n->eelmine) {
                tee.push_back(n->seis);
            }
            std::reverse(tee.begin(), tee.end());
            return tee;
        }

        for (auto& naaber : TeeNaabrid(current->seis, gridSize)) {
            int uusG = current->g + 1;
            if (!kulud.count(naaber) || uusG < kulud[naaber]) {
                kulud[naaber] = uusG;
                auto* uusNode = new PuzzleNode{naaber, uusG, Manhattan(naaber, gridSize), current};
                open.push(uusNode);
            }
        }
    }

    return {}; // Pole lahendust
}