#pragma once

namespace brisk {

struct Size {
    int width, height;
    Size(int w = 0, int h = 0) : width(w), height(h) {}
};

struct Point {
    int x, y;
    Point(int x = 0, int y = 0) : x(x), y(y) {}
};

enum class TextAlign {
    Left,
    Center,
    Right
};

enum class WindowStyle {
    Dialog,
    Normal
};

}