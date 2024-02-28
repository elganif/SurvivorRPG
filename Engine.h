#ifndef ENGINE_H_INCLUDED
#define ENGINE_H_INCLUDED

class Screen;
class UIContainer;

class GameWorld{
private:
    olc::PixelGameEngine* pge;
    olc::vi2d screenArea;
    float worldRadius;

    bool running = false;
    std::chrono::_V2::steady_clock::time_point epochTime;
    std::chrono::_V2::steady_clock::time_point frameTime;

    using frame = std::chrono::duration<int64_t,std::ratio<1,60>>;
    frame engineClock;
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
    bool run(float fElapsedTime,srpg::controls& input);
    void draw();
    void pause();
    void gameHudDraw(srpg::controls& inputs);
    void gameHudGenerate();
};



#endif // ENGINE_H_INCLUDED
