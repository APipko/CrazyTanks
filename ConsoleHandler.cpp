#include "ConsoleHandler.h"

#include "windows.h"
#include "conio.h"
#include <iostream>

const int ESC_CODE = 27;
const int SPACE_CODE = 32;
const int ENTER_CODE = 13;
const int EXTENDED_CODE = 224;
const int UP_CODE = 72;
const int DOWN_CODE = 80;
const int LEFT_CODE = 75;
const int RIGHT_CODE = 77;

ConsoleHandler::ConsoleHandler (int screenHeight, int screenWidth, const std::string& title) : 
    screenHeight_(screenHeight), screenWidth_(screenWidth), title_(title) {
    
    system (("title " + title_).c_str());

    _COORD coordinates;
    coordinates.X = screenWidth_;
    coordinates.Y = screenHeight_;
    _SMALL_RECT rectangle;
    rectangle.Top = 0;
    rectangle.Left = 0;
    rectangle.Bottom = screenHeight_ - 1;
    rectangle.Right = screenWidth_ - 1;

    HANDLE hCons = GetStdHandle (STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursor = { 100, false };
    SetConsoleCursorInfo (hCons, &cursor);
    SetConsoleScreenBufferSize (hCons, coordinates);
    SetConsoleWindowInfo (hCons, TRUE, &rectangle);
}

std::vector<std::string> ConsoleHandler::GetEmptyScreen () {
    return std::vector<std::string> (screenHeight_, std::string (screenWidth_, ' '));
}

std::string ConsoleHandler::MakeCentred (const std::string& string) {
    int stringLength = string.size ();
    if (stringLength > screenWidth_) {
        stringLength = screenWidth_;
    }
    int offset = (screenWidth_ - stringLength) / 2;
    std::string centredString (screenWidth_, ' ');
    for (int i = 0; i < stringLength; ++i) {
        centredString[offset + i] = string[i];
    }
    return centredString;
}

std::string ConsoleHandler::MakeSelected (const std::string& string) {
    return MakeCentred ("<< " + string + " >>");
}

void ConsoleHandler::Show (const std::vector<std::string>& screen) {
    system ("cls");
    for (const auto& row : screen) {
        std::cout << row << "\n";
    }
}

Key ConsoleHandler::GetPressedKey () {
    int key;
    key = _getch ();
    if (key == ESC_CODE) {
        return Key::ECS;
    }
    if (key == SPACE_CODE) {
        return Key::SPACE;
    }
    if (key == ENTER_CODE) {
        return Key::ENTER;
    }
    if (key == EXTENDED_CODE) {
        key = _getch ();
        if (key == UP_CODE) {
            return Key::UP;
        }
        if (key == DOWN_CODE) {
            return Key::DOWN;
        }
        if (key == LEFT_CODE) {
            return Key::LEFT;
        }
        if (key == RIGHT_CODE) {
            return Key::RIGHT;
        }
    }
    return Key::UNDEFINED;
}

bool ConsoleHandler::KeyPressed ()
{
    return _kbhit ();
}

void ConsoleHandler::Wait (int duration)
{
    Sleep (duration);
}
