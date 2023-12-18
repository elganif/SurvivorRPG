#ifndef ENGINE_H_INCLUDED
#define ENGINE_H_INCLUDED


class GameWorld{
private:
    bool running = false;
    float engineTime = 0.0f;
    float tickSize = 1.0f/60.0f;
    int maxTicks = 5;
    olc::PixelGameEngine* srpg;
    olc::vi2d screenArea;
    float worldRadius;

    olc::Sprite* heroicImage = nullptr;
    std::shared_ptr<Hero> mainChar;
    std::unique_ptr<FoeManager> villians;
    std::unique_ptr<DecalManager> lawn;
    std::list<std::shared_ptr<Projectile>> bullets;

public:
    GameWorld(float worldSize,olc::PixelGameEngine* game);
    ~GameWorld();

    void start();
    bool gameOver();
    bool run(float fElapsedTime,srpg_data::controls& input);
    void draw();
    void pause();

};


#endif // ENGINE_H_INCLUDED
