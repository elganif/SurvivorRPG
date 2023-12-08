#ifndef ENGINE_H_INCLUDED
#define ENGINE_H_INCLUDED


class GameWorld{
private:
    olc::PixelGameEngine* srpg;
    olc::vi2d screenArea;
    float worldRadius;
    std::shared_ptr<Hero> mainChar;
    std::unique_ptr<FoeManager> villians;

    std::unique_ptr<DecalManager> lawn;
    std::list<std::shared_ptr<Projectile>> bullets;;

public:
    GameWorld(float worldSize,olc::PixelGameEngine* game);
    ~GameWorld();

    void start();

    void run(float fElapsedTime,srpg_data::controls& input);
    void pause();

};


#endif // ENGINE_H_INCLUDED
