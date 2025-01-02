#include <SFML/Graphics.hpp>
#include <iostream>
#include <cstdlib>
#include <stdexcept>
#include <algorithm>
#include <ctime>
#include <unordered_set>

// TODO: use dependency injection(?)

class Logic {
public:
    #define BOMB 9
    #define NO_BOMB 0

    enum FieldState {
        Covered,
        Uncovered,
        Flagged
    };

    Logic(int boardSize = 10, unsigned short numBombs = 8){
        this->boardSize = boardSize;
        if (numBombs > boardSize * boardSize){
            throw std::invalid_argument("Number of bombs must be < boardSize * boardSize");
        } 
        this->numBombs = numBombs;

        board = new int*[boardSize];
        fieldStates = new int*[boardSize];
        for (int i = 0; i < boardSize; i++){
            board[i] = new int[boardSize];
            fieldStates[i] = new int[boardSize];
        }

        // generate different random bombPositions
        for (int i = 0; i < numBombs; i++){
            std::srand(std::time(nullptr)); // set seed with current time
            int position;
            do {
                position = std::rand() % (boardSize*boardSize);
            } while (bombPositions.find(position) != bombPositions.end());
            bombPositions.insert(position);
        }
    }
    ~Logic(){
        for (int i = 0; i < boardSize; i++){
            delete[] board[i];
            delete[] fieldStates[i];
        }
        delete[] board;
        delete[] fieldStates;
    }

    /* Call at beginning. Places bombs on to the board and computes adjacent bombs for each field*/
    void initializeBoard(){
        for (int i = 0; i < boardSize; i++){
            for (int j = 0; j < boardSize; j++){
                if (bombPositions.find(i*boardSize + j) != bombPositions.end()){
                    board[i][j] = BOMB;
                } else {
                    board[i][j] = NO_BOMB;
                }

                fieldStates[i][j] = FieldState::Covered;
            }
        }

        for (int i = 0; i < boardSize; i++){
            for (int j = 0; j < boardSize; j++){
                if (board[i][j] == BOMB) continue;
                board[i][j] = countAdjacentBombs(i, j);
            }
        }
    }

    int countAdjacentBombs(const int &row, const int &col) const {     
        int count = 0;
        
        for (const auto& dir: directions){
            int newRow = row + dir.first;
            int newCol = col + dir.second;

            if (newRow >= 0 && newRow < boardSize && newCol >= 0 && newCol < boardSize){
                if (board[newRow][newCol] == BOMB) count++;
            }
        }
        return count;
    }

    bool isCollision(const int &x, const int &y){
        if (board[x][y] == BOMB){
            return true;
        } else {
            return false;
        }
    }

    // TODO:
    void uncoverTiles(int row, int col){
        fieldStates[row][col] = FieldState::Uncovered;
        uncoveredFields++;

        if (board[row][col] != 0){
            // adjacent bomb: reveal only this field
            return;
        }
        // no adjacent bomb: reveal every field until surrounded by fields with numbers >= 1
        for (auto dir : directions){
            int newRow = row + dir.first;
            int newCol = col + dir.second;

            if (newRow >= 0 && newRow < boardSize && newCol >= 0 && newCol < boardSize){
                uncoverTiles(newRow, newCol);
            }
        }
    }

    /* handle user input at pos(x,y), flag == true, if user wants to set a flag */
    void handleInput(int x, int y, bool flag){
        if (flag){
            // mark field with flag or remove flag
            if (fieldStates[x][y] == FieldState::Covered){
                fieldStates[x][y] = FieldState::Flagged;
            } else if (fieldStates[x][y] == FieldState::Flagged){
                fieldStates[x][y] = FieldState::Covered;
            } // ignore if player tries to flag an uncovered field

        } else {
            uncoverTiles(x,y);
            if (isCollision(x,y)){
                gameWon = false;
                gameOver = true;
            } else if (uncoveredFields == boardSize*boardSize-numBombs){
                gameWon = true;
                gameOver = true;
            }
        }
    }

    // TODO:
    void printBoard() const {
        // TODO: show time at the top

        for (int i = 0; i < boardSize; i++){
            for (int j = 0; j < boardSize; j++){
                auto field = fieldStates[i][j];
                if (field == FieldState::Covered){
                    // print custom picture of covered field
                } else if (field == FieldState::Uncovered){
                    switch (board[i][j]){
                        case 0:
                            //print pic of uncovered field with no number on it
                            break;
                        case 1:
                            //print pic of uncovered field with number 1 on it
                            break;
                        case 2:
                            //print pic of uncovered field with number 2 on it
                            break;
                        case 3:
                            //print pic of uncovered field with number 3 on it
                            break;
                        case 4:
                            //print pic of uncovered field with number 4 on it
                            break;
                        case 5:
                            //print pic of uncovered field with number 5 on it
                            break;
                        case 6:
                            //print pic of uncovered field with number 6 on it
                            break;
                        case 7:
                            //print pic of uncovered field with number 7 on it
                            break;
                        case 8:
                            //print pic of uncovered field with number 8 on it
                            break;
                        case BOMB:
                            // print pic of bomb (last thing player sees before game over screen)
                            
                            //throw std::logic_error("Field is uncovered and contains a bomb");
                            break;
                    }
                } else {
                    // print pic of field with flag on it
                }
            }
        }
    }

    void dbg_printAll() const {
        std::cout << "Board: " << std::endl;
        for (int i = 0; i < boardSize; i++){
            for (int j = 0; j < boardSize; j++){
                if (j == 0) std::cout << " || ";
                if (board[i][j] == BOMB){
                    std::cout << "B || ";
                } else {
                    std::cout << board[i][j] << " || ";
                }
            }
            std::cout << std::endl;
        }
        
        std::cout << "=========================================" << std::endl;

        // std::cout << "Bomb positions: " << std::endl;
        // for (auto &val : bomPositions){
        //     std::cout << val << ", ";
        // }
        // std::cout << std::endl;
        
        // std::cout << "=========================================" << std::endl;
    }

    bool isGameOver(){
        return gameOver;
    }
    bool hasWon(){
        return gameWon;
    }

private:
    bool gameOver = false;
    bool gameWon = false;
    int boardSize;
    int **board;
    int **fieldStates;
    int numBombs;
    int uncoveredFields = 0;
    std::unordered_multiset<int> bombPositions;
    const std::vector<std::pair<int, int>> directions = {
        {-1,-1}, {-1,0}, {-1,1},
        { 0,-1},         { 0,1},
        { 1,-1}, { 1,0}, { 1,1}
    };
};