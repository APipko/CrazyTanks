#pragma once
#include <vector>
#include <string>

enum class Key {
    UP, DOWN, LEFT, RIGHT, SPACE, ECS, ENTER, UNDEFINED
};

class ConsoleHandler {
public:
    ConsoleHandler (int screenHeight, int screenWidth, const std::string& title);
    void Show (const std::vector<std::string>& screen);
    Key GetPressedKey ();
    bool KeyPressed ();
    void Wait (int duration);
    std::vector<std::string> GetEmptyScreen ();
    std::string MakeCentred (const std::string& string);
    std::string MakeSelected (const std::string& string);

private:
    int screenHeight_;
    int screenWidth_;
    std::string title_;
};