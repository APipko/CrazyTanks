#include "CrazyTanksGame.h"
#include <algorithm>
#include <random>

#undef max
#undef min

const int TANK_N = 5;
const std::vector<double> TANK_ACTION_PROBABILITIES{ 0.1, 0.1, 0.1, 0.1, 0.3, 0.3 };

const int WALL_N = 5;
const int TOTAL_HEALTH = 3;
const int WALL_STRENGTH = 2;


const int FIELD_HEIGHT = 12;
const int FIELD_WIDTH = 16;
const int CELL_SIZE = 3;
const int TIME_STEP = 400;
const int INFO_SIZE = 10;

CrazyTanksGame::CrazyTanksGame () {
    int screenHeight = FIELD_HEIGHT * CELL_SIZE + INFO_SIZE;
    int screenWidth = FIELD_WIDTH * CELL_SIZE;
    consoleHandler_ = std::make_shared<ConsoleHandler> (screenHeight, screenWidth, "Crazy Tanks");
}

void CrazyTanksGame::Play () {
    bool start_game = StartMenuChoice_ ();
    while (start_game) {
        PlayGame_ ();
        ShowResult_ ();
        start_game = StartMenuChoice_ ();
    }
}

bool CrazyTanksGame::StartMenuChoice_ () {
    auto screen = consoleHandler_->GetEmptyScreen ();
    bool start_game = true;
    screen[3] = consoleHandler_->MakeSelected ("Play");
    screen[5] = consoleHandler_->MakeCentred ("Exit");
    consoleHandler_->Show (screen);
    Key key = consoleHandler_->GetPressedKey ();
    while (key != Key::ENTER) {
        if (key == Key::UP || key == Key::DOWN) {
            if (start_game) {
                screen[3] = consoleHandler_->MakeCentred ("Play");
                screen[5] = consoleHandler_->MakeSelected ("Exit");
            } else {
                screen[3] = consoleHandler_->MakeSelected ("Play");
                screen[5] = consoleHandler_->MakeCentred ("Exit");
            }
            start_game = !start_game;
            consoleHandler_->Show (screen);
        }
        key = consoleHandler_->GetPressedKey ();
    }
    return start_game;
}

void CrazyTanksGame::PlayGame_ () {
    bool continue_game = true;
    InitializeGame_ ();
    ShowField_ ();
    while (continue_game) {
        continue_game = Tik_ ();
        ShowField_ ();
    }
}

void CrazyTanksGame::InitializeGame_ ()
{
    time_ = 0;
    score_ = 0;
    bulletCounter_ = 0;
    tanks_.clear ();
    bullets_.clear ();

    CreateField_ ();
}

void CrazyTanksGame::CreateField_ () {
    map_ = Map (FIELD_HEIGHT, std::vector<std::shared_ptr<Entity>> (FIELD_WIDTH));
    for (int i = 0; i < FIELD_WIDTH; ++i) {
        map_[0][i] = std::make_shared<Wall> (0, i);
        map_[FIELD_HEIGHT - 1][i] = std::make_shared<Wall> (FIELD_HEIGHT - 1, i);
    }
    for (int i = 1; i < FIELD_HEIGHT - 1; ++i) {
        map_[i][0] = std::make_shared<Wall> (i, 0);
        map_[i][FIELD_WIDTH - 1] = std::make_shared<Wall> (i, FIELD_WIDTH - 1);
    }

    SetGold_ ();
    SetPlayer_ ();
    CreateWalls_ (WALL_N);
    CreateTanks_ (TANK_N);
}

void CrazyTanksGame::SetPlayer_ ()
{
    int xPlayer = gold_->GetX () - 2;
    int yPlayer = gold_->GetY ();
    player_ = std::make_shared<Player> (xPlayer, yPlayer, Direction::UP, TOTAL_HEALTH);
    map_[xPlayer][yPlayer] = player_;
}

void CrazyTanksGame::SetGold_ ()
{
    int xGold = FIELD_HEIGHT - 2;
    int yGold = FIELD_WIDTH / 2;
    gold_ = std::make_shared<Gold> (xGold, yGold);
    map_[xGold][yGold] = gold_;
    for (int i = 0; i <= 1; ++i) {
        for (int j = -1; j <= 1; ++j) {
            if (i != 0 || j != 0) {
                map_[xGold - i][yGold - j] = std::make_shared<WeakWall> (xGold - i, yGold - j, WALL_STRENGTH);
            }
        }
    }
}

void CrazyTanksGame::CreateWalls_ (int nWalls) {
    std::random_device randomDevice;
    std::mt19937 randomGenerator (randomDevice ());
    std::uniform_int_distribution<> xGenerator (1, FIELD_HEIGHT - 2);
    std::uniform_int_distribution<> yGenerator (1, FIELD_WIDTH - 2);
    std::uniform_int_distribution<> lengthGenerator (2, 3);
    std::uniform_int_distribution<> directionGenerator (0, 1);
    int x;
    int y;
    int length;
    Direction direction;
    std::vector<Direction> directions{ Direction::DOWN, Direction::RIGHT };
    for (int i = 0; i < nWalls; ++i) {
        bool correct = false;
        while (!correct) {
            correct = true;
            x = xGenerator (randomGenerator);
            y = yGenerator (randomGenerator);
            length = lengthGenerator (randomGenerator);
            direction = directions[directionGenerator (randomGenerator)];
            if (map_[x][y]) {
                correct = false;
            }
            if (direction == Direction::DOWN) {
                for (int j = 0; j < length; ++j) {
                    int neighbourX = std::min (FIELD_HEIGHT - 2, std::max (1, x + j));
                    if (map_[neighbourX][y]) {
                        correct = false;
                    }
                }
                if (correct) {
                    for (int j = 0; j < length; ++j) {
                        int neighbourX = std::min (FIELD_HEIGHT - 2, std::max (1, x + j));
                        map_[neighbourX][y] = std::make_shared<WeakWall> (neighbourX, y, WALL_STRENGTH);
                    }
                }

            }
            if (direction == Direction::RIGHT) {
                for (int j = 0; j < length; ++j) {
                    int neighbourY = std::min (FIELD_WIDTH - 2, std::max (1, y + j));
                    if (map_[x][neighbourY]) {
                        correct = false;
                    }
                }
                if (correct) {
                    for (int j = 0; j < length; ++j) {
                        int neighbourY = std::min (FIELD_WIDTH - 2, std::max (1, y + j));
                        map_[x][neighbourY] = std::make_shared<WeakWall> (x, neighbourY, WALL_STRENGTH);
                    }
                }
            }
        }
    }
}

void CrazyTanksGame::CreateTanks_ (int nTanks) {
    std::random_device randomDevice;
    std::mt19937 randomGenerator (randomDevice ());
    std::uniform_int_distribution<> xGenerator (1, FIELD_HEIGHT - 2);
    std::uniform_int_distribution<> yGenerator (1, FIELD_WIDTH - 2);
    int x;
    int y;
    for (int i = 0; i < nTanks; ++i) {
        bool correct = false;
        while (!correct) {
            correct = true;
            x = xGenerator (randomGenerator);
            y = yGenerator (randomGenerator);
            if (map_[x][y]) {
                correct = false;
            }
            for (int j = -2; j <= 2; ++j) {
                for (int k = -2; k <= 2; ++k) {
                    int neighbourX = std::min (FIELD_HEIGHT - 2, std::max (1, x + j));
                    int neighbourY = std::min (FIELD_WIDTH - 2, std::max (1, y + k));
                    if (map_[neighbourX][neighbourY] && map_[neighbourX][neighbourY]->GetName () == EntityName::TANK) {
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

bool CrazyTanksGame::Tik_ () {
    clock_t tikStart = clock ();

    if (!PlayerMove_ (&tikStart)) {
        return false;
    }

    TanksMove_ ();
    BulletsMove_ ();

    consoleHandler_->Wait (TIME_STEP);
    time_ += (clock () - tikStart);
    return !(Win_() || GameOver_());
}

bool CrazyTanksGame::PlayerMove_ (clock_t* tikStart)
{
    Action playerAction = GetPlayerAction_ ();
    if (playerAction != Action::NO_ACTION) {
        if (playerAction == Action::PAUSE) {
            time_ += (clock () - *tikStart);
            Key key = consoleHandler_->GetPressedKey ();
            if (key == Key::ECS) {
                return false;
            }
            *tikStart = clock ();
        } else if (playerAction == Action::SHOT) {
            AddPlayerBullet_ (player_->GetX(), player_->GetY(), player_->GetDirection());
        } else {
            Step_ (player_, GetDirectionFromAction (playerAction));
        }
    }
    return true;
}

void CrazyTanksGame::TanksMove_ ()
{
    for (auto& tank : tanks_) {
        Action tank_action = GetRandomTankAction_ ();
        if (tank_action != Action::NO_ACTION) {
            if (tank_action == Action::SHOT) {
                AddBullet_ (tank.second->GetX (), tank.second->GetY (), tank.second->GetDirection ());
            } else {
                Step_ (tank.second, GetDirectionFromAction (tank_action));
            }
        }
    }
}

void CrazyTanksGame::BulletsMove_ ()
{
    std::vector<int> bullet_ids;
    for (auto& bullet : bullets_) {
        bullet_ids.push_back (bullet.first);
    }
    for (auto& i : bullet_ids) {
        if (!Step_ (bullets_[i], bullets_[i]->GetDirection())) {
            int newX = bullets_[i]->GetX();
            int newY = bullets_[i]->GetY ();
            ShiftPoint (&newX, &newY, bullets_[i]->GetDirection());
            Shot_ (bullets_[i], map_[newX][newY]);
        }
    }
}

bool CrazyTanksGame::Step_ (std::shared_ptr<Entity> entity, Direction direction)
{
    int oldX = entity->GetX();
    int oldY = entity->GetY();
    if (entity->GetDirection() != direction) {
        entity->SetDirection(direction);
        if (entity->GetName () == EntityName::PLAYER) {
            return false;
        }
    }
    int newX = oldX;
    int newY = oldY;
    ShiftPoint (&newX, &newY, direction);
    if (!map_[newX][newY]) {
        if (map_[oldX][oldY] == entity){
            map_[oldX][oldY].reset ();
        }
        map_[newX][newY] = entity;
        entity->SetX(newX);
        entity->SetY(newY);
        return true;
    }
    return false;
}

void CrazyTanksGame::Shot_ (std::shared_ptr<Bullet> bullet, std::shared_ptr<Entity> target)
{
    EntityName targetName = target->GetName ();
    
    if (bullet->GetName() == EntityName::TANK_BULLET) {
        if (targetName == EntityName::PLAYER || targetName == EntityName::GOLD) {
            target->Hit ();
        }
        if (targetName == EntityName::WEAK_WALL) {
            target->Hit ();
            if (!target->IsAlive ()) {
                map_[target->GetX ()][target->GetY ()].reset ();
            }
        }
    }
    
    if (bullet->GetName () == EntityName::PLAYER_BULLET) {
        if (targetName == EntityName::TANK) {
            target->Hit ();
            ++score_;
            tanks_.erase (dynamic_cast<Tank*>(target.get ())->GetId());
        }
        if (targetName == EntityName::WEAK_WALL) {
            target->Hit ();
        }
        if (!target->IsAlive ()) {
            map_[target->GetX ()][target->GetY ()].reset ();
        }
        
    }
    bullets_.erase (bullet->GetId ());
    if (map_[bullet->GetX ()][bullet->GetY ()] == bullet) {
        map_[bullet->GetX ()][bullet->GetY ()].reset ();
    }
}

void CrazyTanksGame::AddBullet_ (int x, int y, Direction direction) {
    ++bulletCounter_;
    bullets_[bulletCounter_] = std::make_shared<TankBullet> (x, y, direction, bulletCounter_);
}

void CrazyTanksGame::AddPlayerBullet_ (int x, int y, Direction direction) {
    ++bulletCounter_;
    bullets_[bulletCounter_] = std::make_shared<PlayerBullet> (x, y, direction, bulletCounter_);
}

bool CrazyTanksGame::Win_ ()
{
    return (player_->IsAlive () && gold_->IsAlive () && tanks_.empty ());
}

bool CrazyTanksGame::GameOver_ ()
{
    return ((!player_->IsAlive ()) || (!gold_->IsAlive ()));
}

void CrazyTanksGame::ShowResult_ () {
    auto screen = consoleHandler_->GetEmptyScreen ();
    std::string result;
    if (Win_()) {
        result = consoleHandler_->MakeCentred ("You win");
    } else if (GameOver_()) {
        result = consoleHandler_->MakeCentred ("Game over");
    } else {
        result = consoleHandler_->MakeCentred ("Your result");
        
    }
    screen[3] = result;
    screen[6] = consoleHandler_->MakeCentred ("Score: " + std::to_string (score_));
    screen[8] = consoleHandler_->MakeCentred ("Health: " + std::to_string (player_->GetHealth()));
    if (gold_->IsAlive()) {
        screen[10] = consoleHandler_->MakeCentred ("Gold in fortress");
    } else {
        screen[10] = consoleHandler_->MakeCentred ("Gold is captured");
    }
    screen[12] = consoleHandler_->MakeCentred ("Time: " + std::to_string (time_ / CLOCKS_PER_SEC));
    screen[15] = consoleHandler_->MakeCentred ("Press Enter");

    consoleHandler_->Show (screen);
    Key key = consoleHandler_->GetPressedKey ();
    while (key != Key::ENTER) {
        key = consoleHandler_->GetPressedKey ();
    }
}

void CrazyTanksGame::ShowField_ () {
    auto screen = consoleHandler_->GetEmptyScreen ();
    int row_offset = 0;
    int col_offset = 0;
    for (auto& row : map_) {
        col_offset = 0;
        for (auto& cell : row) {
            if (cell) {
                auto patch = cell->GetPatch (CELL_SIZE);
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

    screen[FIELD_HEIGHT * CELL_SIZE + 1] = "Health: " + std::to_string (player_->GetHealth());
    screen[FIELD_HEIGHT * CELL_SIZE + 2] = "Score: " + std::to_string (score_);
    screen[FIELD_HEIGHT * CELL_SIZE + 3] = "Time: " + std::to_string (time_ / CLOCKS_PER_SEC);
    screen[FIELD_HEIGHT * CELL_SIZE + 5] = "Pause: press ESC";
    screen[FIELD_HEIGHT * CELL_SIZE + 6] = "Exit: press ESC again";
    screen[FIELD_HEIGHT * CELL_SIZE + 7] = "Continue: press any key";

    consoleHandler_->Show (screen);
}

Action CrazyTanksGame::GetPlayerAction_ () {
    Key key;
    if (consoleHandler_->KeyPressed()) {
        key = consoleHandler_->GetPressedKey ();
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

int GetRandomIndex (const std::vector<double>& probabilities) {
    std::random_device randomDevice;
    std::mt19937 randomGenerator (randomDevice ());
    std::discrete_distribution<> discreteDistribution (probabilities.begin (), probabilities.end ());
    return discreteDistribution (randomGenerator);
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

void ShiftPoint (int* x, int* y, Direction direction) {
    switch (direction) {
        case Direction::DOWN:
            ++*x;
            break;
        case Direction::UP:
            --*x;
            break;
        case Direction::LEFT:
            --*y;
            break;
        case Direction::RIGHT:
            ++*y;
            break;
    }
}