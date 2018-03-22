#pragma once

#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <ctime>

#include "ConsoleHandler.h"
#include "GameEntities.h"

enum class Action {
    MOVE_UP, MOVE_DOWN, MOVE_LEFT, MOVE_RIGHT, SHOT, NO_ACTION, PAUSE
};

using Map = std::vector<std::vector<std::shared_ptr<Entity>>>;

class CrazyTanksGame {
public:
    CrazyTanksGame ();
    void Play ();

private:
    bool StartMenuChoice_ ();
    void PlayGame_ ();
    void InitializeGame_ ();
    void CreateField_ ();
    void SetPlayer_ ();
    void SetGold_ ();
    void CreateWalls_ (int nWalls);
    void CreateTanks_ (int nTanks);
    void ShowField_ ();
    bool Tik_ ();
    bool PlayerMove_ (clock_t* tikStart);
    void TanksMove_ ();
    void BulletsMove_ ();
    bool Step_ (std::shared_ptr<Entity> entity, Direction direction);
    void Shot_ (std::shared_ptr<Bullet> bullet, std::shared_ptr<Entity> target);
    bool Win_ ();
    bool GameOver_ ();
    void ShowResult_ ();
    
    Action GetPlayerAction_ ();
    Action GetRandomTankAction_ ();
    void AddBullet_ (int x, int y, Direction direction);
    void AddPlayerBullet_ (int x, int y, Direction direction);

    std::shared_ptr<Player> player_;
    std::shared_ptr<Gold> gold_;
    std::unordered_map<int, std::shared_ptr<Tank>> tanks_;
    std::unordered_map<int, std::shared_ptr<Bullet>> bullets_;
    int bulletCounter_;

    Map map_;
    int score_ = 0;
    int time_ = 0;

    std::shared_ptr<ConsoleHandler> consoleHandler_;
};

Direction GetDirectionFromAction (Action action);

int GetRandomIndex (const std::vector<double>& probabilities);

void ShiftPoint (int* x, int* y, Direction direction);
