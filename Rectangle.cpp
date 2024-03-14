 #include "olcPixelGameEngine.h"

#include "Rectangle.h"
///class Rectangle

Rectangle::Rectangle(const olc::vf2d& loc, const olc::vf2d& area) : tl(loc),sides(area){}

Rectangle::~Rectangle(){}

const olc::vi2d Rectangle::ufo(const olc::vf2d& other)
{
    olc::vi2d answer = {0,0};
    /// check if point is right of left wall, increment. Then check if left of right wall decriment.
    /// if point is in between both paths will happen and offset eachother for 0;
    /// after do same for y axis.
    if(tl.x <= other.x)
        answer.x += 1;
    if(tl.x + sides.x >= other.x)
        answer.x -= 1;

    if(tl.y <= other.y)
        answer.y += 1;
    if(tl.y + sides.y >= other.y)
        answer.y -= 1;

    return answer;
}

const olc::vi2d Rectangle::ufo(const Rectangle& other)
{
    olc::vi2d answer = {0,0};
    /// check if any part is right of left wall, increment. Then check if left of right wall decriment.
    /// if point is in between both paths will happen and offset eachother for 0;
    /// after do same for y axis.
    if(tl.x <= other.tl.x + other.sides.x)
        answer.x += 1;
    if(tl.x + sides.x >= other.tl.x)
        answer.x -= 1;

    if(tl.y <= other.tl.y + other.sides.x)
        answer.y += 1;
    if(tl.y + sides.y >= other.tl.y)
        answer.y -= 1;

    return answer;
}

bool Rectangle::contains(const olc::vf2d& point)
{
    if(tl.x < point.x && tl.x + sides.x > point.x &&
       tl.y < point.y && tl.y + sides.y > point.y ){
       return true;
    }
    return false;
}

bool Rectangle::contains(const Rectangle& other)
{
    if(tl.x < other.tl.x && tl.x + sides.x > other.tl.x + other.sides.x &&
       tl.y < other.tl.y && tl.y + sides.y > other.tl.y + other.sides.y){
       return true;
    }
    return false;
}

bool Rectangle::overlaps(const Rectangle& other)
{
    if(tl.x < other.tl.x + other.sides.x && tl.x + sides.x >= other.tl.x &&
       tl.y < other.tl.y + other.sides.y && tl.y + sides.y >= other.tl.y){
        return true;
    }
    return false;
}
