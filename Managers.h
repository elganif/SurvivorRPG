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

template <class E>
class Manager {
protected:
    olc::PixelGameEngine* srpg;
    float worldRadius;

    std::list<std::shared_ptr<E>> items;
    std::shared_ptr<olc::Sprite> image;
    std::shared_ptr<olc::Decal> drawing;

    Manager<E>(olc::PixelGameEngine* srpg, float worldRadius);
    ~Manager<E>();
    int size();
    int updateAndClear(float fElapsedTime,olc::vf2d movement);
    void spawn();

};

///Foe Manager is designed around enemies and will maintain their numbers, stats growth and overall difficulty
class FoeManager : Manager<Foe>{
    private:
    int maxPop;
    float foeSize = 0.05f;

    int deadFoes = 0;
    public:
    FoeManager(olc::PixelGameEngine* game, float world);
    ~FoeManager();

    int getKills();

    void spawn(float foeSize);
    void update(float fElapsedTime,olc::vf2d movement);

    void initalize(int numFoes);
    void makeRender();

};

/// Manages projectiles
class ProjectileManager : Manager<Projectile>{
private:

public:
    ProjectileManager(olc::PixelGameEngine* srpg, float worldRadius);
    ~ProjectileManager();
    //void initalize(int numFoes);
    void spawn(float bulletSize);
    void update(float fElapsedTime,olc::vf2d movement);
    void makeRender(std::shared_ptr<olc::Sprite> sprite,olc::vf2d area,olc::PixelGameEngine* game);



};


/// A manager for all things part of the enviroment. Grass, trees(eventually) or possibly buildings
class DecalManager : Manager<Decoration> {
private:
    float grassSize = 0.01f;
    olc::Pixel grassColour = olc::DARK_GREEN;
public:
    DecalManager(olc::PixelGameEngine* srpg, float worldRadius);
    ~DecalManager();

    void initalize();
    void update(float fElapsedTime,olc::vf2d movement);
    void makeRender();
};




#endif // MANAGERS_H_INCLUDED
