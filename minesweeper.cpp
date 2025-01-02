#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <unordered_set>
#include <thread>


#define MINE -1

struct Tile {
    int value; // MINE or #adjacent mines
    bool isRevealed = false;
    bool isFlagged = false;
};

// general variables
const int boardSize = 20;
const int tileSize = 64; // in px, depends on png sizes!
const int windowSize = boardSize * tileSize;
std::vector<std::vector<Tile>> board(boardSize, std::vector<Tile>(boardSize));

// game variables
bool gameOver = false;
bool gameLost = false;
bool showAnimation = false;
bool lastIteration = false;
const int numMines = 35;
int uncoveredTiles = 0;
int mineX = 0, mineY = 0; // buffer for last clicked mines, needed for lose animation
std::unordered_multiset<int> minePositions;
std::unordered_multiset<int> objectPositions;
const std::vector<std::pair<int, int>> directions = {
        {-1,-1}, {-1,0}, {-1,1},
        { 0,-1},         { 0,1},
        { 1,-1}, { 1,0}, { 1,1}
};

/* Returns the number of adjacent mines for a given tile */
int countAdjacentMines(const int row, const int col){     
    int count = 0;
    
    for (const auto& dir: directions){
        int newRow = row + dir.first;
        int newCol = col + dir.second;

        if (newRow >= 0 && newRow < boardSize && newCol >= 0 && newCol < boardSize){
            if (board[newRow][newCol].value == MINE)
                count++;
        }
    }
    return count;
}

/* Call at beginning. Places mines on to the board and computes adjacent mines for each tile*/
void initializeBoard(std::vector<sf::Sprite> &objects){
    // generate random minePositions
    std::srand(std::time(nullptr)); // set seed with current time
    for (int i = 0; i < numMines; i++){
        int position;
        do {
            position = std::rand() % (boardSize*boardSize);
        } while (minePositions.find(position) != minePositions.end());
        minePositions.insert(position);
    }

    // place mines
    for (int i = 0; i < boardSize; i++){
        for (int j = 0; j < boardSize; j++){
            if (minePositions.find(i*boardSize + j) != minePositions.end()){
                board[i][j].value = MINE;
            } else {
                board[i][j].value = 0;
            }
        }
    }

    // compute #adjacent mines for each tile
    for (int i = 0; i < boardSize; i++){
        for (int j = 0; j < boardSize; j++){
            if (board[i][j].value == MINE)
                continue;
            board[i][j].value = countAdjacentMines(i, j);
        }
    }

    // compute random decorative object positions for the map (only on tiles with soil)
    for (unsigned int i = 0; i < objects.size(); i++){
        int pos;
        do {
            pos = std::rand() % (boardSize*boardSize);
        } while (objectPositions.find(pos) != objectPositions.end() || board[pos/boardSize][pos%boardSize].value != 0);
        objectPositions.insert(pos);
    }
}

void uncoverTiles(int row, int col){
    Tile &tile = board[row][col];
    if (tile.isRevealed)
        return;
    tile.isRevealed = true;
    uncoveredTiles++;

    if (tile.value != 0)
        // tile has adjacent mines: reveal this tile only
        return;
    
    //no adjacent mines: reveal every tile until surrounded by tiles with numbers >= 1
    for (auto dir : directions){
        int newRow = row + dir.first;
        int newCol = col + dir.second;

        if (newRow >= 0 && newRow < boardSize && newCol >= 0 && newCol < boardSize)
            uncoverTiles(newRow, newCol);
    }
}

/**
 * @brief Adds a sprite to a vector of sprites.
 * 
 * This function loads a texture from a file, creates a sprite using the texture,
 * and then adds the sprite to the provided vector of sprites.
 * 
 * @param texture Reference to an sf::Texture object where the texture will be loaded.
 * @param filePath The file path of the texture to load.
 * @param vector Reference to a vector of sf::Sprite objects where the new sprite will be added.
 */
void addSprite(sf::Texture &texture, const std::string &filePath, std::vector<sf::Sprite> &vector){
    texture.loadFromFile(filePath);
    sf::Sprite sprite(texture);
    vector.push_back(sprite);
}

void showLoseAnimation(sf::Sound &explosionSound, sf::Sprite &soil, sf::Sprite &explosion, sf::RenderWindow &window){
    std::this_thread::sleep_for(std::chrono::seconds(2));
    explosionSound.play();
    soil.setPosition(mineX, mineY);
    explosion.setPosition(mineX, mineY);
    window.draw(soil);
    window.draw(explosion);
    window.display();
    std::this_thread::sleep_for(std::chrono::milliseconds(2500));
}

void showWinAnimation(sf::Sprite &mine, sf::Sprite &meadow, sf::RenderWindow &window){
    for (int i = 0; i < 5; i++){
        for (int pos : minePositions){
            mine.setPosition(pos%boardSize*tileSize, pos/boardSize*tileSize);
            window.draw(mine);
        }
        window.display();
        std::this_thread::sleep_for(std::chrono::milliseconds(400));
        for (int pos : minePositions){
            meadow.setPosition(pos%boardSize*tileSize, pos/boardSize*tileSize);
            window.draw(meadow);
        }
        window.display();
        std::this_thread::sleep_for(std::chrono::milliseconds(400));
    }
}


void resetGame() {
    gameOver = false;
    gameLost = false;
    showAnimation = false;
    lastIteration = false;
    uncoveredTiles = 0;
    minePositions.clear();
    objectPositions.clear();
    board = std::vector<std::vector<Tile>>(boardSize, std::vector<Tile>(boardSize));
}


int main(){
    // Place window in middle of screen
    sf::RenderWindow window(sf::VideoMode({windowSize, windowSize}), "Minesweeper", sf::Style::Titlebar | sf::Style::Close); // disable resizing
    sf::VideoMode desktop = sf::VideoMode::getDesktopMode();   
    window.setFramerateLimit(10); // ! or CPU will blow up
    int winX = (desktop.width - windowSize) / 2;
    int winY = (desktop.height - windowSize) / 2;
    window.setPosition({winX, winY});
    
// setup graphics
    sf::Image icon;
    icon.loadFromFile("graphics/red-flag.png");
    window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());

    sf::Texture tileset;
    tileset.loadFromFile("graphics/tileset32x32px.png");
    
    // Play Again-Button
    sf::Font font;
    font.loadFromFile("fonts/PressStart2P-Regular.ttf");

    sf::Text playAgainText("Play Again", font, 80);
    playAgainText.setFillColor(sf::Color(200, 200, 200));
    playAgainText.setStyle(sf::Text::Bold);
    
    sf::FloatRect textBounds = playAgainText.getLocalBounds();
    playAgainText.setPosition((windowSize - textBounds.width)/2, (windowSize - textBounds.height)/2);
    
    sf::RectangleShape playAgainRect({textBounds.width + 20, textBounds.height + 20});
    playAgainRect.setFillColor(sf::Color(255, 255, 255, 40)); // make it transparent
    
    sf::FloatRect rectBounds = playAgainRect.getLocalBounds();
    playAgainRect.setPosition((windowSize - rectBounds.width)/2, (windowSize - rectBounds.height)/2);


    // environment
    sf::Sprite meadow(tileset);
    meadow.setTextureRect({0, 32, tileSize, tileSize}); // cut square located at coordinates (0px, 32px) from tileset

    sf::Sprite soil(tileset);
    soil.setTextureRect({2*96+2*32, 32, tileSize, tileSize});
    
    sf::Texture flagTexture;
    flagTexture.loadFromFile("graphics/red-flag.png"); // flag is 32x32px
    sf::Sprite flag(flagTexture);

    sf::Texture mineTexture;
    mineTexture.loadFromFile("graphics/bomb.png");
    sf::Sprite mine(mineTexture);

    sf::Texture explosionTexture;
    explosionTexture.loadFromFile("graphics/nuclear-explosion.png");
    sf::Sprite explosion(explosionTexture);
    

    // numbers
    sf::Texture one;
    sf::Texture two;
    sf::Texture three;
    sf::Texture four;
    sf::Texture five;
    sf::Texture six;
    sf::Texture seven;
    sf::Texture eight;
    
    std::vector<sf::Sprite> numbers;

    addSprite(one, "graphics/numbers/1.png", numbers);
    addSprite(two, "graphics/numbers/2.png", numbers);
    addSprite(three, "graphics/numbers/3.png", numbers);
    addSprite(four, "graphics/numbers/4.png", numbers);
    addSprite(five, "graphics/numbers/5.png", numbers);
    addSprite(six, "graphics/numbers/6.png", numbers);
    addSprite(seven, "graphics/numbers/7.png", numbers);
    addSprite(eight, "graphics/numbers/8.png", numbers);


    // objects lying around
    sf::Texture banana;
    sf::Texture ant;
    sf::Texture apple;
    sf::Texture compost;
    sf::Texture fossil;
    sf::Texture fossil2;
    sf::Texture fossil3;
    sf::Texture fossil4;
    sf::Texture fossil5;
    sf::Texture mole;
    sf::Texture rocks;
    sf::Texture stone_axe;
    sf::Texture stone;
    sf::Texture velociraptor;
    sf::Texture worm;
    sf::Texture worm2;
    sf::Texture worm3;

    std::vector<sf::Sprite> objects;

    addSprite(banana, "graphics/objects/banana.png", objects);
    addSprite(ant, "graphics/objects/ant.png", objects);
    addSprite(apple, "graphics/objects/apple.png", objects);
    addSprite(compost, "graphics/objects/compost.png", objects);
    addSprite(fossil, "graphics/objects/fossil.png", objects);
    addSprite(fossil2, "graphics/objects/fossil2.png", objects);
    addSprite(fossil3, "graphics/objects/fossil3.png", objects);
    addSprite(fossil4, "graphics/objects/fossil4.png", objects);
    addSprite(fossil5, "graphics/objects/fossil5.png", objects);
    addSprite(mole, "graphics/objects/mole.png", objects);
    addSprite(rocks, "graphics/objects/rocks.png", objects);
    addSprite(stone_axe, "graphics/objects/stone_axe.png", objects);
    addSprite(stone, "graphics/objects/stone.png", objects);
    addSprite(velociraptor, "graphics/objects/velociraptor.png", objects);
    addSprite(worm, "graphics/objects/worm.png", objects);
    addSprite(worm2, "graphics/objects/worm2.png", objects);
    addSprite(worm3, "graphics/objects/worm3.png", objects);


// setup audio
    sf::SoundBuffer flagBuffer;
    flagBuffer.loadFromFile("audio/pop.ogg");
    sf::Sound flagSound(flagBuffer);

    sf::SoundBuffer shovelBuffer;
    shovelBuffer.loadFromFile("audio/shovel.ogg");
    sf::Sound shovelSound(shovelBuffer);

    sf::SoundBuffer explosionBuffer;
    explosionBuffer.loadFromFile("audio/explosion.flac");
    sf::Sound explosionSound(explosionBuffer);

    sf::Music backgroundMusic;
    backgroundMusic.openFromFile("audio/rhythm_garden.ogg");
    backgroundMusic.setLoop(true);
    backgroundMusic.setVolume(60);
    backgroundMusic.play();

    sf::Music winMusic;
    winMusic.openFromFile("audio/laidback.ogg");
    winMusic.setVolume(60);

    initializeBoard(objects);


    while(window.isOpen()){
        sf::Event event;
        while (window.pollEvent(event)){ // event loop
            switch(event.type){
            
            case sf::Event::Closed:
                window.close();
                break;

            case sf::Event::MouseButtonPressed:{
                int x = event.mouseButton.x / tileSize;
                int y = event.mouseButton.y / tileSize;

                Tile &tile = board[y][x];

                if (event.mouseButton.button == sf::Mouse::Left){
                    if (gameOver && playAgainRect.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)){
                        // play again
                        winMusic.stop();
                        backgroundMusic.play();
                        resetGame();
                        initializeBoard(objects);
                        continue;
                    }

                    if (!gameOver && (!tile.isRevealed || tile.value == 0))
                        shovelSound.play();

                    if (tile.isRevealed && tile.value == 0){
                        // "dig out" object (if there is one)
                        for (unsigned int i = 0; i < objects.size(); i++){
                            auto object = objects.at(i);
                            if (object.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)){
                                objectPositions.erase(y*boardSize + x);
                                objects.erase(objects.begin() + i);
                                objects.push_back(object);
                            }
                        }
                    } else if (!gameOver && tile.value == MINE){
                        lastIteration = true;
                        tile.isRevealed = true;
                        gameLost = true;
                        showAnimation = true;
                    } else {
                        uncoverTiles(y, x);
                        if(uncoveredTiles == boardSize*boardSize - numMines){
                            lastIteration = true;
                            gameLost = false;
                            showAnimation = true;
                            uncoveredTiles = 0;
                        }
                    }
                } else if (event.mouseButton.button == sf::Mouse::Right){
                    if (!tile.isRevealed){
                        flagSound.play();
                        tile.isFlagged = !tile.isFlagged;
                    }
                }
                break;
            }
            default:
                break;
            }
        }


        if(!gameOver){
            // draw board
            window.clear(sf::Color::White);
            for (int i = 0; i < boardSize; i++){
                for (int j = 0; j < boardSize; j++){
                    Tile &tile = board[i][j];
                    int posY = i * tileSize;
                    int posX = j * tileSize;

                    if (tile.isRevealed){
                        soil.setPosition(posX, posY);
                        window.draw(soil);
                        switch(tile.value){
                            case MINE:
                                mine.setPosition(posX, posY);
                                window.draw(mine);
                                mineX = posX;
                                mineY = posY;
                                break;
                            case 1:
                                numbers.at(0).setPosition(posX, posY);
                                window.draw(numbers.at(0));
                                break;
                            case 2:
                                numbers.at(1).setPosition(posX, posY);
                                window.draw(numbers.at(1));
                                break;
                            case 3:
                                numbers.at(2).setPosition(posX, posY);
                                window.draw(numbers.at(2));
                                break;
                            case 4:
                                numbers.at(3).setPosition(posX, posY);
                                window.draw(numbers.at(3));
                                break;
                            case 5:
                                numbers.at(4).setPosition(posX, posY);
                                window.draw(numbers.at(4));
                                break;
                            case 6:
                                numbers.at(5).setPosition(posX, posY);
                                window.draw(numbers.at(5));
                                break;
                            case 7:
                                numbers.at(6).setPosition(posX, posY);
                                window.draw(numbers.at(6));
                                break;
                            case 8:
                                numbers.at(7).setPosition(posX, posY);
                                window.draw(numbers.at(7));
                                break;
                            case 0: {// just soil or object on soil
                                auto it = objectPositions.find(i * boardSize + j);
                                if (it != objectPositions.end()) {
                                    int index = std::distance(objectPositions.begin(), it);
                                    objects.at(index).setPosition(posX, posY);
                                    window.draw(objects.at(index));
                                }
                                break;
                            }
                            default:
                                break;
                        }
                    } else {
                        meadow.setPosition(posX, posY);
                        window.draw(meadow);

                        if (tile.isFlagged){
                            flag.setPosition(posX + tileSize/4, posY + tileSize/4);
                            window.draw(flag);
                        }
                    }
                }
            }
            if(lastIteration)
                gameOver = true;

        } else { // gameOver
            backgroundMusic.stop();
            if(showAnimation){
                if (gameLost){
                    showLoseAnimation(explosionSound, soil, explosion, window);
                } else {
                    winMusic.play();
                    showWinAnimation(mine, meadow, window);
                }
                showAnimation = false;
            }
            window.clear();
            for (int i = 0; i < boardSize; i++){
                for (int j = 0; j < boardSize; j++){
                    meadow.setPosition(j * tileSize, i * tileSize);
                    window.draw(meadow);
                }
            }
            window.draw(playAgainRect);
            window.draw(playAgainText);
        }

        window.display();
    }
}