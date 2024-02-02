#ifndef MANAGERS_H_INCLUDED
#define MANAGERS_H_INCLUDED

/**************************
This file contains the managers for the various entity types.
These structures are designed to centralize functionality for groups of entities

**************************/
class Hero;
class Npc;
class Projectile;
class Decoration;

template <class E>
class Manager {
protected:
    olc::PixelGameEngine* srpg;
    float worldRadius;

    std::list<std::shared_ptr<E>> items;
    std::shared_ptr<olc::Sprite> image = nullptr;
    std::shared_ptr<olc::Decal> drawing = nullptr;

    Manager<E>(olc::PixelGameEngine* srpg, float worldRadius);
    ~Manager<E>();
    int size();
    void spawn();
    int update(float fElapsedTime,olc::vf2d movement);
    public:
};

///Foe Manager is designed around enemies and will maintain their numbers, stats growth and overall difficulty
class FoeManager : public Manager<Npc>{
    private:
    int maxPop;
    float foeSize = 0.05f;

    int deadFoes = 0;
    public:
    FoeManager(olc::PixelGameEngine* game, float world);
    ~FoeManager();

    int getKills();

    void spawn(float foeSize);
    void update(float fElapsedTime,olc::vf2d worldMove);
    void collision();

    void initalize(int numFoes);
    void makeRender();

};

/// Manages projectiles
class ProjectileManager : public Manager<Projectile>{
private:
    float life = 0;
    float speed = 0;
    int hits = 0;
public:
    ProjectileManager(olc::PixelGameEngine* srpg, float worldRadius);
    ~ProjectileManager();

    void setProjectileStats(float life, float speed,int hits);
    void spawn(olc::vf2d origin, olc::vf2d momentum, olc::vf2d bulletSize);
    void update(float fElapsedTime,olc::vf2d worldMove);

    void makeRender(olc::vf2d pSize);
};


/// A manager for all things part of the enviroment. Grass, trees(eventually) or possibly buildings
class DecalManager : public Manager<Decoration> {
private:
    float grassSize = 0.01f;
    olc::Pixel grassColour = olc::DARK_GREEN;
public:
    DecalManager(olc::PixelGameEngine* srpg, float worldRadius);
    ~DecalManager();

    void initalize();
    void update(float fElapsedTime,olc::vf2d worldMove);

    void makeRender();
};




#endif // MANAGERS_H_INCLUDED
