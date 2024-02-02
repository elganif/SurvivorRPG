#ifndef ENGINE_H_INCLUDED
#define ENGINE_H_INCLUDED

class Screen;
class UIContainer;

class GameWorld{
private:
    olc::PixelGameEngine* srpg;
    olc::vi2d screenArea;
    float worldRadius;

    bool running = false;
    std::chrono::_V2::steady_clock::time_point epochTime;
    std::chrono::time_point<std::chrono::_V2::steady_clock> frameTime;

    float engineTime = 0;
    float tickSize = 1.0f/60.0f;
    int maxTicks = 5;

    std::shared_ptr<olc::Sprite> heroicImage = nullptr;
    std::shared_ptr<Hero> mainChar;
    std::unique_ptr<FoeManager> villians;
    std::unique_ptr<DecalManager> lawn;
    std::unique_ptr<ProjectileManager> bulletList;
    std::list<std::shared_ptr<Projectile>> bullets;

    std::unique_ptr<Screen> HUD;

public:
    GameWorld(float worldSize,olc::PixelGameEngine* game);
    ~GameWorld();

    void start();
    bool gameOver();
    bool run(float fElapsedTime,srpg_data::controls& input);
    void draw();
    void pause();
    void gameHudDraw(srpg_data::controls& inputs);
    void gameHudGenerate();
};



#endif // ENGINE_H_INCLUDED
