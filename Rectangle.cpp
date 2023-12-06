 #include "olcPixelGameEngine.h"

#include "Rectangle.h"
///struct Rectangle


        Rectangle::Rectangle(const olc::vf2d& loc, const olc::vf2d& area) : tl(loc),sides(area){}

        Rectangle::~Rectangle(){}

        bool Rectangle::contains(const Rectangle& other){
            if(tl.x <= other.tl.x && tl.x + sides.x > other.tl.x + other.sides.x &&
               tl.y <= other.tl.y && tl.y + sides.y > other.tl.y + other.sides.y){
               return true;
            }
            return false;
        }

        bool Rectangle::overlaps(const Rectangle& other){
            if(tl.x < other.tl.x + other.sides.x && tl.x + sides.x >= other.tl.x &&
               tl.y < other.tl.y + other.sides.y && tl.y + sides.y >= other.tl.y){
                return true;
            }
            return false;
        }

