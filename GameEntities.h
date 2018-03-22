#pragma once

#include <vector>

enum class Direction {
    UP, DOWN, LEFT, RIGHT, UNDEFINED
};

enum  class EntityName {
    PLAYER, GOLD, TANK, PLAYER_BULLET, TANK_BULLET, WALL, WEAK_WALL, EMPTY
};

class Entity {
public:
    Entity (int x, int y, Direction d = Direction::UNDEFINED, int health = 1);
    virtual ~Entity ();
    void Hit ();
    bool IsAlive ();
    int GetX ();
    int GetY ();
    Direction GetDirection ();
    void SetX (int x);
    void SetY (int y);
    void SetDirection (Direction direction);
    virtual std::vector<std::string> GetPatch (int cellSize) = 0;
    virtual EntityName GetName () = 0;

protected:
    int x_;
    int y_;
    Direction direction_;
    int health_;
};

class Player : public Entity {
public:
    Player (int x, int y, Direction d, int health);
    int GetHealth ();
    std::vector<std::string> GetPatch (int cellSize) override;
    EntityName GetName () override;
};

class Gold : public Entity {
public:
    Gold (int x, int y);
    std::vector<std::string> GetPatch (int cellSize) override;
    EntityName GetName () override;
};

class Tank : public Entity {
public:
    Tank (int x, int y, Direction d, int i);
    int GetId ();
    std::vector<std::string> GetPatch (int cellSize) override;
    EntityName GetName () override;
private:
    int id_;
};

class Bullet : public Entity {
public:
    Bullet (int x, int y, Direction d, int i);
    int GetId ();
    std::vector<std::string> GetPatch (int cellSize) override;
private:
    int id_;
};

class PlayerBullet : public Bullet {
public:
    PlayerBullet (int x, int y, Direction d, int i);
    EntityName GetName () override;
};

class TankBullet : public Bullet {
public:
    TankBullet (int x, int y, Direction d, int i);
    EntityName GetName () override;
};

class Wall : public Entity {
public:
    Wall (int x, int y);
    std::vector<std::string> GetPatch (int cellSize) override;
    EntityName GetName () override;
};

class WeakWall : public Wall {
public:
    WeakWall (int x, int y, int health);
    EntityName GetName () override;
};
