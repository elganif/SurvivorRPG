#ifndef MANAGERS_H_INCLUDED
#define MANAGERS_H_INCLUDED

/**************************
This file contains the managers for the various entity types.
These structures are designed to centralize functionality for groups of entities

**************************/
class Hero;
class Foe;
class Projectile;
class Decoration;

// A manager for all things part of the enviroment. Grass, trees(eventually) or possibly buildings
class DecalManager{
    private:
    std::vector<std::shared_ptr<Decoration>> lawn;
    olc::Sprite* image;
    float worldRadius;
    public:
    DecalManager( float world);
    ~DecalManager();

    void initalize(olc::PixelGameEngine* game);
    void update(float fElapsedTime,olc::vf2d movement);
    void makeRender(olc::Sprite* tSprite,olc::vf2d area,olc::PixelGameEngine* game,olc::Pixel lineColourL);
    int size();
};

//Foe Manager is designed around enemies and will maintain their numbers, stats growth and overall difficulty
class FoeManager{
    private:
    std::list<std::shared_ptr<Foe>> mainVillain;
    olc::Sprite* image;
    float worldRadius;
    public:
    FoeManager(float world);
    ~FoeManager();

    void update(float fElapsedTime,olc::vf2d movement);
    void initalize(int numFoes,olc::PixelGameEngine* game);
    void makeRender(olc::Sprite* sprite,olc::vf2d area,olc::PixelGameEngine* game);
    int size();
};


#endif // MANAGERS_H_INCLUDED
