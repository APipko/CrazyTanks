#include "GameEntities.h"

const char TANK_SYMBOL = 'o';
const char PLAYER_SYMBOL = '#';
const char BULLET_SYMBOL = '.';
const char WALL_SYMBOL = 'x';
const char DIRECTION_SYMBOL = '*';
const char GOLD_SYMBOL = '@';

Entity::Entity (int x, int y, Direction d, int health) : x_ (x), y_ (y), direction_ (d), health_(health) {}

Entity::~Entity () {}

void Entity::Hit () {
    --health_;
}

bool Entity::IsAlive () {
    return health_ > 0;
}

int Entity::GetX () {
    return x_;
}

int Entity::GetY () {
    return y_;
}

Direction Entity::GetDirection () {
    return direction_;
}

void Entity::SetX (int x) {
    x_ = x;
}

void Entity::SetY (int y) {
    y_ = y;
}

void Entity::SetDirection (Direction direction) {
    direction_ = direction;
}

Player::Player (int x, int y, Direction d, int health) : Entity (x, y, d, health) {}

int Player::GetHealth ()
{
    return health_;
}

std::vector<std::string> Player::GetPatch (int cellSize) {
    std::vector<std::string> patch (cellSize, std::string (cellSize, PLAYER_SYMBOL));
    if (direction_ == Direction::UP) {
        patch[0][cellSize / 2] = DIRECTION_SYMBOL;
    }
    if (direction_ == Direction::DOWN) {
        patch[cellSize - 1][cellSize / 2] = DIRECTION_SYMBOL;
    }
    if (direction_ == Direction::LEFT) {
        patch[cellSize / 2][0] = DIRECTION_SYMBOL;
    }
    if (direction_ == Direction::RIGHT) {
        patch[cellSize / 2][cellSize - 1] = DIRECTION_SYMBOL;
    }
    return patch;
}

EntityName Player::GetName () {
    return EntityName::PLAYER;
}

Gold::Gold (int x, int y) : Entity (x, y) {}

std::vector<std::string> Gold::GetPatch (int cellSize) {
    std::vector<std::string> patch (cellSize, std::string (cellSize, ' '));
    patch[cellSize / 2][cellSize / 2] = GOLD_SYMBOL;
    return patch;
}

EntityName Gold::GetName () {
    return EntityName::GOLD;
}

Tank::Tank (int x, int y, Direction d, int i) : Entity (x, y, d), id_ (i) {}

int Tank::GetId () {
    return id_;
}

std::vector<std::string> Tank::GetPatch (int cellSize) {
    std::vector<std::string> patch (cellSize, std::string (cellSize, TANK_SYMBOL));
    if (direction_ == Direction::UP) {
        patch[0][cellSize / 2] = DIRECTION_SYMBOL;
    }
    if (direction_ == Direction::DOWN) {
        patch[cellSize - 1][cellSize / 2] = DIRECTION_SYMBOL;
    }
    if (direction_ == Direction::LEFT) {
        patch[cellSize / 2][0] = DIRECTION_SYMBOL;
    }
    if (direction_ == Direction::RIGHT) {
        patch[cellSize / 2][cellSize - 1] = DIRECTION_SYMBOL;
    }
    return patch;
}

EntityName Tank::GetName () {
    return EntityName::TANK;
}

Bullet::Bullet (int x, int y, Direction d, int i) : Entity (x, y, d), id_ (i) {}

int Bullet::GetId () {
    return id_;
}

std::vector<std::string> Bullet::GetPatch (int cellSize) {
    std::vector<std::string> patch (cellSize, std::string (cellSize, ' '));
    patch[cellSize / 2][cellSize / 2] = BULLET_SYMBOL;
    return patch;
}

PlayerBullet::PlayerBullet (int x, int y, Direction d, int i) : Bullet (x, y, d, i){}

EntityName PlayerBullet::GetName () {
    return EntityName::PLAYER_BULLET;
}

TankBullet::TankBullet (int x, int y, Direction d, int i) : Bullet (x, y, d, i) {}

EntityName TankBullet::GetName () {
    return EntityName::TANK_BULLET;
}

Wall::Wall (int x, int y) : Entity (x, y) {}

std::vector<std::string> Wall::GetPatch (int cellSize) {
    return std::vector<std::string> (cellSize, std::string (cellSize, WALL_SYMBOL));
}

EntityName Wall::GetName () {
    return EntityName::WALL;
}

WeakWall::WeakWall (int x, int y, int health) : Wall (x, y) {
    health_ = health;
}

EntityName WeakWall::GetName ()
{
    return EntityName::WEAK_WALL;
}