#pragma once

#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

#include "ConsoleHandler.h"

enum class Direction {
    UP, DOWN, LEFT, RIGHT, UNDEFINED
};

enum  class EntityName {
    PLAYER, GOLD, TANK, PLAYER_BULLET, BULLET, WALL, WEAK_WALL, EMPTY
};

enum class Action {
    MOVE_UP, MOVE_DOWN, MOVE_LEFT, MOVE_RIGHT, SHOT, NO_ACTION, PAUSE
};


class Entity {
public:
    Entity (int x, int y, Direction d);
    virtual std::vector<std::string> Show () = 0;
    virtual EntityName GetName () = 0;
    int x_coordinate;
    int y_coordinate;
    Direction direction;
};

class Player : public Entity {
public:
    Player (int x, int y, Direction d);
    std::vector<std::string> Show () override;
    EntityName GetName () override;
};

class Gold : public Entity {
public:
    Gold (int x, int y);
    std::vector<std::string> Show () override;
    EntityName GetName () override;
};

class Tank : public Entity {
public:
    Tank (int x, int y, Direction d, int i);
    std::vector<std::string> Show () override;
    EntityName GetName () override;
    int id;
};

class Bullet : public Entity {
public:
    Bullet (int x, int y, Direction d, int i);
    std::vector<std::string> Show () override;
    EntityName GetName () override;
    int id;
};

class PlayerBullet : public Entity {
public:
    PlayerBullet (int x, int y, Direction d, int i);
    std::vector<std::string> Show () override;
    EntityName GetName () override;
    int id;
};

class Wall : public Entity {
public:
    Wall (int x, int y);
    std::vector<std::string> Show () override;
    EntityName GetName () override;
};

class WeakWall : public Entity {
public:
    WeakWall (int x, int y);
    std::vector<std::string> Show () override;
    EntityName GetName () override;
    int strength;
};

using Map = std::vector<std::vector<std::shared_ptr<Entity>>>;

class CrazyTanksGame {
public:
    CrazyTanksGame ();
    void Play ();
    int GetScreenHeight ();
    int GetScreenWidth ();

private:
    void PlayGame_ ();
    bool Menu_ ();
    bool Tik_ ();
    void CreateField_ ();
    void ShowField_ ();
    void ShowResult_ ();
    void CreateWalls_ (int wall_n);
    void CreateTanks_ (int tank_n);
    Action GetPlayerAction_ ();
    Action GetRandomTankAction_ ();
    void Move_ (std::shared_ptr<Entity> entity, Direction direction);
    void AddBullet_ (int x, int y, Direction direction);
    void AddPlayerBullet_ (int x, int y, Direction direction);

    std::shared_ptr<Player> player_;
    std::unordered_map<int, std::shared_ptr<Tank>> tanks_;
    std::unordered_map<int, std::shared_ptr<PlayerBullet>> player_bullets_;
    std::unordered_map<int, std::shared_ptr<Bullet>> bullets_;
    int player_bullet_counter_;
    int bullet_counter_;

    Map map_;
    int score_ = 0;
    int health_;
    bool gold_in_fortress_ = true;
    int time_ = 0;

    int screen_height_;
    int screen_width_;
    std::shared_ptr<ConsoleHandler> console_handler_;
};

Direction GetDirectionFromAction (Action action);

int GetRandomIndex (const std::vector<double>& probabilities);
