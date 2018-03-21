#include "CrazyTanksGame.h"
#include <algorithm>
#include <ctime>
#include <random>

#undef max
#undef min

const int TANK_N = 5;
const std::vector<double> TANK_ACTION_PROBABILITIES{ 0.1, 0.1, 0.1, 0.1, 0.3, 0.3 };

const int TOTAL_HEALTH = 3;
const int WALL_N = 5;
const int WALL_STRENGTH = 2;

const int FIELD_HEIGHT = 12;
const int FIELD_WIDTH = 16;
const int CELL_SIZE = 3;
const int TIME_STEP = 400;
const int INFO_SIZE = 10;

const char TANK_SYMBOL = 'o';
const char PLAYER_SYMBOL = '#';
const char BULLET_SYMBOL = '.';
const char WALL_SYMBOL = 'x';
const char DIRECTION_SYMBOL = '*';
const char GOLD_SYMBOL = '@';


CrazyTanksGame::CrazyTanksGame () : health_(TOTAL_HEALTH), 
                                    screen_height_ (FIELD_HEIGHT * CELL_SIZE + INFO_SIZE), 
                                    screen_width_ (FIELD_WIDTH * CELL_SIZE)
{
    console_handler_ = std::make_shared<ConsoleHandler> (screen_height_, screen_width_, "Crazy Tanks");
}

void CrazyTanksGame::Play () {
    bool start_game = Menu_ ();
    while (start_game) {
        PlayGame_ ();
        ShowResult_ ();
        start_game = Menu_ ();
    }
}

bool CrazyTanksGame::Menu_ () {
    auto screen = console_handler_->GetEmptyScreen ();
    bool start_game = true;
    screen[3] = console_handler_->MakeSelected ("Play");
    screen[5] = console_handler_->MakeCentred ("Exit");
    console_handler_->Show (screen);
    Key key = console_handler_->GetPressedKey ();
    while (key != Key::ENTER) {
        if (key == Key::UP || key == Key::DOWN) {
            if (start_game) {
                screen[3] = console_handler_->MakeCentred ("Play");
                screen[5] = console_handler_->MakeSelected ("Exit");
            } else {
                screen[3] = console_handler_->MakeSelected ("Play");
                screen[5] = console_handler_->MakeCentred ("Exit");
            }
            start_game = !start_game;
            console_handler_->Show (screen);
        }
        key = console_handler_->GetPressedKey ();
    }
    return start_game;
}

void CrazyTanksGame::PlayGame_ () {
    bool continue_game = true;
    CreateField_ ();
    ShowField_ ();
    while (continue_game) {
        continue_game = Tik_ ();
        ShowField_ ();
    }
}

bool CrazyTanksGame::Tik_ () {
    clock_t tik_start = clock ();

    Action player_action = GetPlayerAction_ ();
    if (player_action != Action::NO_ACTION) {
        if (player_action == Action::PAUSE) {
            time_ += (clock () - tik_start);
            Key key = console_handler_->GetPressedKey ();
            if (key == Key::ECS) {
                return false;
            }
            tik_start = clock ();
        } else if (player_action == Action::SHOT) {
            AddPlayerBullet_ (player_->x_coordinate, player_->y_coordinate, player_->direction);
        } else {
            Move_ (player_, GetDirectionFromAction (player_action));
        }
    }

    for (auto& tank : tanks_) {
        Action tank_action = GetRandomTankAction_ ();
        if (tank_action != Action::NO_ACTION) {
            if (tank_action == Action::SHOT) {
                AddBullet_ (tank.second->x_coordinate, tank.second->y_coordinate, tank.second->direction);
            } else {
                Move_ (tank.second, GetDirectionFromAction (tank_action));
            }
        }
    }

    std::vector<int> player_bullet_ids;
    for (auto& bullet : player_bullets_) {
        player_bullet_ids.push_back (bullet.first);
    }
    for (auto& i : player_bullet_ids) {
        Move_ (player_bullets_[i], player_bullets_[i]->direction);
    }

    std::vector<int> bullet_ids;
    for (auto& bullet : bullets_) {
        bullet_ids.push_back (bullet.first);
    }
    for (auto& i : bullet_ids) {
        Move_ (bullets_[i], bullets_[i]->direction);
    }

    console_handler_->Wait (TIME_STEP);
    time_ += (clock () - tik_start);

    if (health_ == 0) {
        return false;
    }
    if (!gold_in_fortress_) {
        return false;
    }
    if (tanks_.empty ()) {
        return false;
    }
    return true;
}

void CrazyTanksGame::Move_ (std::shared_ptr<Entity> entity, Direction direction)
{
    int old_x = entity->x_coordinate;
    int old_y = entity->y_coordinate;
    EntityName entity_name = entity->GetName ();
    if (entity->direction != direction) {
        entity->direction = direction;
        if (entity_name == EntityName::PLAYER) {
            return;
        }
    }
    int new_x = old_x;
    int new_y = old_y;
    switch (direction) {
        case Direction::DOWN:
            ++new_x;
            break;
        case Direction::UP:
            --new_x;
            break;
        case Direction::LEFT:
            --new_y;
            break;
        case Direction::RIGHT:
            ++new_y;
            break;
    }
    if (!map_[new_x][new_y]) {
        if (map_[old_x][old_y] == entity){
            map_[old_x][old_y].reset ();
        }
        map_[new_x][new_y] = entity;
        entity->x_coordinate = new_x;
        entity->y_coordinate = new_y;
        return;
    }
    EntityName target_name = map_[new_x][new_y]->GetName ();


    if (entity_name == EntityName::BULLET) {
        if (target_name == EntityName::PLAYER) {
            --health_;
            bullets_.erase (dynamic_cast<Bullet*>(entity.get ())->id);
            if (map_[old_x][old_y] == entity) {
                map_[old_x][old_y].reset ();
            }
            return;
        }
        if (target_name == EntityName::WEAK_WALL) {
            auto wall = dynamic_cast<WeakWall*>(map_[new_x][new_y].get ());
            --(wall->strength);
            if (wall->strength == 0) {
                map_[new_x][new_y].reset ();
            }
            bullets_.erase (dynamic_cast<Bullet*>(entity.get ())->id);
            if (map_[old_x][old_y] == entity) {
                map_[old_x][old_y].reset ();
            }
            return;
        }
        if (target_name == EntityName::GOLD) {
            gold_in_fortress_ = false;
            return;
        }
        if (target_name == EntityName::TANK || target_name == EntityName::PLAYER_BULLET || 
            target_name == EntityName::WALL || target_name == EntityName::BULLET) {
            bullets_.erase (dynamic_cast<Bullet*>(entity.get ())->id);
            if (map_[old_x][old_y] == entity) {
                map_[old_x][old_y].reset ();
            }
            return;
        }
    }
    if (entity_name == EntityName::PLAYER_BULLET) {
        if (target_name == EntityName::TANK) {
            ++score_;
            player_bullets_.erase (dynamic_cast<PlayerBullet*>(entity.get ())->id);
            if (map_[old_x][old_y] == entity) {
                map_[old_x][old_y].reset ();
            }
            tanks_.erase (dynamic_cast<Tank*>(map_[new_x][new_y].get ())->id);
            map_[new_x][new_y].reset ();
            return;
        }
        if (target_name == EntityName::WEAK_WALL) {
            auto wall = dynamic_cast<WeakWall*>(map_[new_x][new_y].get ());
            --(wall->strength);
            if (wall->strength == 0) {
                map_[new_x][new_y].reset ();
            }
            player_bullets_.erase (dynamic_cast<PlayerBullet*>(entity.get ())->id);
            if (map_[old_x][old_y] == entity) {
                map_[old_x][old_y].reset ();
            }
            return;
        }
        if (target_name == EntityName::WALL || target_name == EntityName::PLAYER_BULLET || 
            target_name == EntityName::PLAYER || target_name == EntityName::GOLD) {
            player_bullets_.erase (dynamic_cast<PlayerBullet*>(entity.get ())->id);
            if (map_[old_x][old_y] == entity) {
                map_[old_x][old_y].reset ();
            }
            return;
        }
    }
}

void CrazyTanksGame::CreateField_ () {
    health_ = TOTAL_HEALTH;
    gold_in_fortress_ = true;
    time_ = 0;
    score_ = 0;
    bullet_counter_ = 0;
    player_bullet_counter_ = 0;
    tanks_.clear ();
    bullets_.clear ();
    player_bullets_.clear ();
    map_ = Map (FIELD_HEIGHT, std::vector<std::shared_ptr<Entity>> (FIELD_WIDTH));
    for (int i = 0; i < FIELD_WIDTH; ++i) {
        map_[0][i] = std::make_shared<Wall> (0, i);
        map_[FIELD_HEIGHT - 1][i] = std::make_shared<Wall> (FIELD_HEIGHT - 1, i);
    }
    for (int i = 1; i < FIELD_HEIGHT - 1; ++i) {
        map_[i][0] = std::make_shared<Wall> (i, 0);
        map_[i][FIELD_WIDTH - 1] = std::make_shared<Wall> (i, FIELD_WIDTH - 1);
    }
    if (player_) {
        player_.reset ();
    }
   
    int x_gold = FIELD_HEIGHT - 2;
    int y_gold = FIELD_WIDTH / 2;
    map_[x_gold][y_gold] = std::make_shared<Gold> (x_gold, y_gold);
    for (int i = 0; i <= 1; ++i) {
        for (int j = -1; j <= 1; ++j) {
            if (i != 0 || j != 0) {
                map_[x_gold - i][y_gold - j] = std::make_shared<WeakWall> (x_gold - i, y_gold - j);
            }
        }
    }
    player_ = std::make_shared<Player> (x_gold - 2, y_gold, Direction::UP);
    map_[x_gold - 2][y_gold] = player_;
    CreateWalls_ (WALL_N);
    CreateTanks_ (TANK_N);
}

void CrazyTanksGame::CreateWalls_ (int wall_n) {
    std::random_device rd;
    std::mt19937 random_generator (rd ());
    std::uniform_int_distribution<> x_generator (1, FIELD_HEIGHT - 2);
    std::uniform_int_distribution<> y_generator (1, FIELD_WIDTH - 2);
    std::uniform_int_distribution<> length_generator (2, 3);
    std::uniform_int_distribution<> direction_generator (0, 1);
    int x;
    int y;
    int length;
    Direction direction;
    std::vector<Direction> directions{ Direction::DOWN, Direction::RIGHT };
    for (int i = 0; i < wall_n; ++i) {
        bool correct = false;
        while (!correct) {
            correct = true;
            x = x_generator (random_generator);
            y = y_generator (random_generator);
            length = length_generator (random_generator);
            direction = directions[direction_generator (random_generator)];
            if (map_[x][y]) {
                correct = false;
            }
            if (direction == Direction::DOWN) {
                for (int j = 0; j < length; ++j) {
                    int neighbour_x = std::min (FIELD_HEIGHT - 2, std::max (1, x + j));
                    if (map_[neighbour_x][y]) {
                        correct = false;
                    }
                }
                if (correct) {
                    for (int j = 0; j < length; ++j) {
                        int neighbour_x = std::min (FIELD_HEIGHT - 2, std::max (1, x + j));
                        map_[neighbour_x][y] = std::make_shared<WeakWall> (neighbour_x, y);
                    }
                }

            }
            if (direction == Direction::RIGHT) {
                for (int j = 0; j < length; ++j) {
                    int neighbour_y = std::min (FIELD_WIDTH - 2, std::max (1, y + j));
                    if (map_[x][neighbour_y]) {
                        correct = false;
                    }
                }
                if (correct) {
                    for (int j = 0; j < length; ++j) {
                        int neighbour_y = std::min (FIELD_WIDTH - 2, std::max (1, y + j));
                        map_[x][neighbour_y] = std::make_shared<WeakWall> (x, neighbour_y);
                    }
                }
            }
        }
    }
}

void CrazyTanksGame::CreateTanks_ (int tank_n) {
    std::random_device rd;
    std::mt19937 random_generator (rd ());
    std::uniform_int_distribution<> x_generator (1, FIELD_HEIGHT - 2);
    std::uniform_int_distribution<> y_generator (1, FIELD_WIDTH - 2);
    int x;
    int y;
    for (int i = 0; i < tank_n; ++i) {
        bool correct = false;
        while (!correct) {
            correct = true;
            x = x_generator (random_generator);
            y = y_generator (random_generator);
            if (map_[x][y]) {
                correct = false;
            }
            for (int j = -2; j <= 2; ++j) {
                for (int k = -2; k <= 2; ++k) {
                    int neighbour_x = std::min (FIELD_HEIGHT - 2, std::max (1, x + j));
                    int neighbour_y = std::min (FIELD_WIDTH - 2, std::max (1, y + k));
                    if (map_[neighbour_x][neighbour_y] && map_[neighbour_x][neighbour_y]->GetName () == EntityName::TANK) {
                        correct = false;
                    }
                }
            }
        }
        std::shared_ptr<Tank> tank = std::make_shared<Tank> (x, y, Direction::DOWN, i);
        tanks_[i] = tank;
        map_[x][y] = tank;
    }
}

void CrazyTanksGame::AddBullet_ (int x, int y, Direction direction) {
    ++bullet_counter_;
    bullets_[bullet_counter_] = std::make_shared<Bullet> (x, y, direction, bullet_counter_);
}

void CrazyTanksGame::AddPlayerBullet_ (int x, int y, Direction direction) {
    ++player_bullet_counter_;
    player_bullets_[player_bullet_counter_] = std::make_shared<PlayerBullet> (x, y, direction, player_bullet_counter_);
}

void CrazyTanksGame::ShowResult_ () {
    auto screen = console_handler_->GetEmptyScreen ();
    std::string result;
    if (tanks_.empty ()) {
        result = console_handler_->MakeCentred ("You win");
    } else if (health_ > 0 && gold_in_fortress_) {
        result = console_handler_->MakeCentred ("Your result");
    } else {
        result = console_handler_->MakeCentred ("Game over");
    }
    screen[3] = result;
    screen[6] = console_handler_->MakeCentred ("Score: " + std::to_string (score_));
    screen[8] = console_handler_->MakeCentred ("Health: " + std::to_string (health_));
    if (gold_in_fortress_) {
        screen[10] = console_handler_->MakeCentred ("Gold in fortress");
    } else {
        screen[10] = console_handler_->MakeCentred ("Gold is captured");
    }
    screen[12] = console_handler_->MakeCentred ("Time: " + std::to_string (time_ / CLOCKS_PER_SEC));
    screen[15] = console_handler_->MakeCentred ("Press Enter");

    console_handler_->Show (screen);
    Key key = console_handler_->GetPressedKey ();
    while (key != Key::ENTER) {
        key = console_handler_->GetPressedKey ();
    }
}

void CrazyTanksGame::ShowField_ () {
    auto screen = console_handler_->GetEmptyScreen ();
    int row_offset = 0;
    int col_offset = 0;
    for (auto& row : map_) {
        col_offset = 0;
        for (auto& cell : row) {
            if (cell) {
                auto patch = cell->Show ();
                for (int i = 0; i < CELL_SIZE; ++i) {
                    for (int j = 0; j < CELL_SIZE; ++j) {
                        screen[row_offset + i][col_offset + j] = patch[i][j];
                    }
                }
            }
            col_offset += CELL_SIZE;
        }
        row_offset += CELL_SIZE;
    }

    screen[FIELD_HEIGHT * CELL_SIZE + 1] = "Health: " + std::to_string (health_);
    screen[FIELD_HEIGHT * CELL_SIZE + 2] = "Score: " + std::to_string (score_);
    screen[FIELD_HEIGHT * CELL_SIZE + 3] = "Time: " + std::to_string (time_ / CLOCKS_PER_SEC);
    screen[FIELD_HEIGHT * CELL_SIZE + 5] = "Pause: press ESC";
    screen[FIELD_HEIGHT * CELL_SIZE + 6] = "Exit: press ESC again";
    screen[FIELD_HEIGHT * CELL_SIZE + 7] = "Continue: press any key";

    console_handler_->Show (screen);
}

int CrazyTanksGame::GetScreenHeight ()
{
    return screen_height_;
}

int CrazyTanksGame::GetScreenWidth ()
{
    return screen_width_;
}

Entity::Entity (int x, int y, Direction d) : x_coordinate (x), y_coordinate (y), direction (d) {}

Wall::Wall (int x, int y) : Entity (x, y, Direction::UNDEFINED)
{
}

std::vector<std::string> Wall::Show () {
    return std::vector<std::string> (CELL_SIZE, std::string (CELL_SIZE, WALL_SYMBOL));
}

Bullet::Bullet (int x, int y, Direction d, int i) : Entity (x, y, d), id (i)
{
}

std::vector<std::string> Bullet::Show () {
    std::vector<std::string> patch (CELL_SIZE, std::string (CELL_SIZE, ' '));
    patch[CELL_SIZE / 2][CELL_SIZE / 2] = BULLET_SYMBOL;
    return patch;
}

PlayerBullet::PlayerBullet (int x, int y, Direction d, int i) : Entity (x, y, d), id (i)
{
}

std::vector<std::string> PlayerBullet::Show () {
    std::vector<std::string> patch (CELL_SIZE, std::string (CELL_SIZE, ' '));
    patch[CELL_SIZE / 2][CELL_SIZE / 2] = BULLET_SYMBOL;
    return patch;
}


Player::Player (int x, int y, Direction d) : Entity (x, y, d)
{
}

std::vector<std::string> Player::Show () {
    std::vector<std::string> patch (CELL_SIZE, std::string (CELL_SIZE, PLAYER_SYMBOL));
    if (direction == Direction::UP) {
        patch[0][CELL_SIZE / 2] = DIRECTION_SYMBOL;
    }
    if (direction == Direction::DOWN) {
        patch[CELL_SIZE - 1][CELL_SIZE / 2] = DIRECTION_SYMBOL;
    }
    if (direction == Direction::LEFT) {
        patch[CELL_SIZE / 2][0] = DIRECTION_SYMBOL;
    }
    if (direction == Direction::RIGHT) {
        patch[CELL_SIZE / 2][CELL_SIZE - 1] = DIRECTION_SYMBOL;
    }
    return patch;
}

Tank::Tank (int x, int y, Direction d, int i) : Entity (x, y, d), id (i)
{
}

std::vector<std::string> Tank::Show () {
    std::vector<std::string> patch (CELL_SIZE, std::string (CELL_SIZE, TANK_SYMBOL));
    if (direction == Direction::UP) {
        patch[0][CELL_SIZE / 2] = DIRECTION_SYMBOL;
    }
    if (direction == Direction::DOWN) {
        patch[CELL_SIZE - 1][CELL_SIZE / 2] = DIRECTION_SYMBOL;
    }
    if (direction == Direction::LEFT) {
        patch[CELL_SIZE / 2][0] = DIRECTION_SYMBOL;
    }
    if (direction == Direction::RIGHT) {
        patch[CELL_SIZE / 2][CELL_SIZE - 1] = DIRECTION_SYMBOL;
    }
    return patch;
}

EntityName Wall::GetName () {
    return EntityName::WALL;
}

EntityName Bullet::GetName () {
    return EntityName::BULLET;
}

EntityName PlayerBullet::GetName () {
    return EntityName::PLAYER_BULLET;
}

EntityName Player::GetName () {
    return EntityName::PLAYER;
}

EntityName Tank::GetName () {
    return EntityName::TANK;
}

Action CrazyTanksGame::GetPlayerAction_ () {
    Key key;
    if (console_handler_->KeyPressed()) {
        key = console_handler_->GetPressedKey ();
        if (key == Key::ECS) {
            return Action::PAUSE;
        }
        if (key == Key::SPACE) {
            return Action::SHOT;
        }
        if (key == Key::UP) {
            return Action::MOVE_UP;
        }
        if (key == Key::DOWN) {
            return Action::MOVE_DOWN;
        }
        if (key == Key::LEFT) {
            return Action::MOVE_LEFT;
        }
        if (key == Key::RIGHT) {
            return Action::MOVE_RIGHT;
        }
    }
    return Action::NO_ACTION;
}

Action CrazyTanksGame::GetRandomTankAction_ () {
    std::vector<Action> actions{ Action::MOVE_UP, Action::MOVE_DOWN,
        Action::MOVE_LEFT, Action::MOVE_RIGHT, Action::SHOT, Action::NO_ACTION };
    return actions[GetRandomIndex (TANK_ACTION_PROBABILITIES)];
}

Gold::Gold (int x, int y) : Entity(x, y, Direction::UNDEFINED)
{
}

std::vector<std::string> Gold::Show ()
{
    std::vector<std::string> patch (CELL_SIZE, std::string (CELL_SIZE, ' '));
    patch[CELL_SIZE / 2][CELL_SIZE / 2] = GOLD_SYMBOL;
    return patch;
}

EntityName Gold::GetName ()
{
    return EntityName::GOLD;
}

WeakWall::WeakWall (int x, int y) : Entity (x, y, Direction::UNDEFINED), strength(WALL_STRENGTH)
{
}

std::vector<std::string> WeakWall::Show ()
{
    return std::vector<std::string> (CELL_SIZE, std::string (CELL_SIZE, WALL_SYMBOL));
}

EntityName WeakWall::GetName ()
{
    return EntityName::WEAK_WALL;
}

int GetRandomIndex (const std::vector<double>& probabilities) {
    std::random_device rd;
    std::mt19937 random_generator (rd ());
    std::discrete_distribution<> d (probabilities.begin (), probabilities.end ());
    return d (random_generator);
}

Direction GetDirectionFromAction (Action action) {
    switch (action) {
        case Action::MOVE_DOWN:
            return Direction::DOWN;
        case Action::MOVE_LEFT:
            return Direction::LEFT;
        case Action::MOVE_RIGHT:
            return Direction::RIGHT;
        case Action::MOVE_UP:
            return Direction::UP;
    }
    return Direction::UNDEFINED;
}