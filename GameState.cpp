//
// Created by xcao2 on 11/29/2023.
//

#include "GameState.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <algorithm>
#include <random>
#include <iostream>
#include <fstream>
#include <sstream>
#include "Bomb.h"

//constructor without board
GameState::GameState(sf::Vector2i _dimensions, int _numberOfMines) : dimensions(_dimensions), numberOfMines(_numberOfMines), flagCount(0), playStatus(PLAYING) {
    //numbers of tiles in game
    int tilesLeft=static_cast<int>(dimensions.x*dimensions.y);
    for(int i = 0; i < dimensions.y; i++){
        for(int j = 0; j < dimensions.x; j++){
            //makes individual tiles
            sf::Vector2f vec(static_cast<float>(j*32), static_cast<float>(i*32));
            Tile *tile= new Tile(vec);
            tiles.push_back(tile);
        }
    }
    //copied tile for randomized bomb placement
    std::vector<Tile*> copied_tiles;
    copied_tiles.reserve(tiles.size());
    for(auto t:tiles){
        copied_tiles.push_back(t);
    }
    std::random_device rand;
    std::mt19937 g(rand());
    std::shuffle(copied_tiles.begin(), copied_tiles.end(), g);
    for(int i = 0; i < numberOfMines && i < tilesLeft; i++){
        //process replacing respective Tiles to bombs in tiles
        Tile *placeholder = copied_tiles[i];
        sf::Vector2f copy(placeholder->getLocation());
        Bomb *newBomb = new Bomb(copy);
        copied_tiles[i]=newBomb;
        int index=static_cast<int>(copy.x/32+copy.y/32*dimensions.x);
        tiles[index]=newBomb;
        delete placeholder;
    }
    //sets neighbors
    setTileMineStatus();
}


GameState::GameState(const char* filepath) : playStatus(PLAYING), flagCount(0) {
    //reads from file
    numberOfMines=0;
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filepath << std::endl;
        return;
    }
    //reads each line to know dimensions
    std::string line;
    int y=0;
    while (std::getline(file, line)) {
        if(line!=""){
            if(y==0){ //only checks first line; enough given board format
                dimensions.x=line.length();
            }
            std::istringstream iss(line);
            for (int i = 0; i < line.length(); i++) {
                sf::Vector2f vec(static_cast<float>(i*32), static_cast<float>(y*32));
                //if 1, makes a bomb
                if(line[i]=='1'){
                    Bomb *bomb = new Bomb(vec);
                    numberOfMines++;
                    tiles.push_back(bomb);
                    tiles[0]->draw();
                } //otherwise makes a tile
                else{
                    Tile *tile= new Tile(vec);
                    tiles.push_back(tile);
                }
            }
            y++;
        }
    }
    dimensions.y=y;
    //sets neighbor stuff
    setTileMineStatus();
}

//gets number of flags on the board, since it can't be accessed outside
int GameState::getFlagCount() {
    flagCount=0;
    for(Tile *t : tiles){
        if(t->getState()==Tile::FLAGGED){
            flagCount++;
        }
    }
    return flagCount;
}

//getters
int GameState::getMineCount() {
    return numberOfMines;
}

Tile* GameState::getTile(int x, int y) {
    // if exists
    if (x >= 0 && x < dimensions.x && y >= 0 && y < dimensions.y) {
        // calculate index 2D->1D
        int index = x + y * dimensions.x;
        return tiles[index];
    }
    return nullptr;
}

GameState::PlayStatus GameState::getPlayStatus() {
    return playStatus;
}

//sets current playstatus
void GameState::setPlayStatus(PlayStatus _status) {
    playStatus = _status;
}


//finds neighbors
void GameState::setTileMineStatus(){
    int current=0;
    for(Tile *t : tiles){
        std::array<Tile*, 8> neighbors;
        sf::Vector2f pos = t->getLocation();
        //considers borders
        if(pos.x-1<0){
            neighbors[0]=nullptr;
            neighbors[1]=nullptr;
            neighbors[2]=nullptr;
        }
        else if(pos.x+33>dimensions.x*32){
            neighbors[5]=nullptr;
            neighbors[6]=nullptr;
            neighbors[7]=nullptr;
        }
        if(pos.y-1<0){
            neighbors[0]=nullptr;
            neighbors[3]=nullptr;
            neighbors[5]=nullptr;
        }
        else if(pos.y+33>dimensions.y*32){
            neighbors[2]=nullptr;
            neighbors[4]=nullptr;
            neighbors[7]=nullptr;
        }
        for(int i =0; i < 8; i++){
            if(neighbors[i]!=nullptr){
                if(i==0){
                    neighbors[i]=tiles[current-dimensions.x-1];
                }
                else if(i==1){
                    neighbors[i]=tiles[current-1];
                }
                else if(i==2){
                    neighbors[i]=tiles[current+dimensions.x-1];
                }
                else if(i==3){
                    neighbors[i]=tiles[current-dimensions.x];
                }
                else if(i==4){
                    neighbors[i]=tiles[current+dimensions.x];
                }
                else if(i==5){
                    neighbors[i]=tiles[current-dimensions.x+1];
                }
                else if(i==6){
                    neighbors[i]=tiles[current+1];
                }
                else if(i==7){
                    neighbors[i]=tiles[current+dimensions.x+1];
                }
            }
        }
        //sets neighbors
        t->setNeighbors(neighbors);
        //seems to be necessary, otherwise memory issues
        neighbors.fill(t);
        //index+1
        current++;
    }
}

