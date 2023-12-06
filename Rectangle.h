#ifndef RECTANGLE_H_INCLUDED
#define RECTANGLE_H_INCLUDED

 class Rectangle{
    public:
    olc::vf2d tl;
    olc::vf2d sides;

    Rectangle(const olc::vf2d& loc, const olc::vf2d& area);
    ~Rectangle();
    bool contains(const Rectangle& other);
    bool overlaps(const Rectangle& other);
};

#endif // RECTANGLE_H_INCLUDED
