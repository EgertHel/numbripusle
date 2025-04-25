# Sliding Number Puzzle

## Project Description
This project is a school assignment developed for the C++ Programming course, created by authors Annabel and Egert.

### Project Idea:
The Sliding Number Puzzle is a classic logic game where the player has to rearrange numbered tiles in a grid by sliding them into an empty space. The goal is to order the numbers in ascending order using as few moves as possible.

The game consists of an NxN grid, where all the cells except one are filled with numbered tiles. The empty cell is placed randomly.
For example, in a 4x4 grid, the numbers range from [1, 15]. When the player clicks on a tile adjacent to the empty space, that tile moves into the empty spot. If the tile is not adjacent to the empty space, nothing happens. The tiles can only move horizontally or vertically, not diagonally. At the beginning, the numbers are scrambled, and the goal of the game is to arrange the numbers in ascending order.

## Project Structure
### Graphical User Interface:
* The user interface is built using Dear ImGui library along with SDL3 and OpenGL3 for rendering.
* The puzzle grid is displayed graphically.
* The player can move tiles by clicking on them.
* A label next to the game grid displays the number of moves made.

## Solvability Check

## How to Run
