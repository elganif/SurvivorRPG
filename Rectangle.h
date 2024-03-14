#ifndef RECTANGLE_H_INCLUDED
#define RECTANGLE_H_INCLUDED

 class Rectangle{
    public:
    olc::vf2d tl;
    olc::vf2d sides;

    Rectangle(const olc::vf2d& loc, const olc::vf2d& area);
    ~Rectangle();
    /// ufo operators, but <=> does not exist in my compiler to make offical one.
    const olc::vi2d ufo(const olc::vf2d& other);
    const olc::vi2d ufo(const Rectangle& other);

    bool contains(const olc::vf2d& point);
    bool contains(const Rectangle& other);

    bool overlaps(const Rectangle& other);
};

#endif // RECTANGLE_H_INCLUDED
