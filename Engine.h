#ifndef ENGINE_H_INCLUDED
#define ENGINE_H_INCLUDED

class Menu;
class gameClock {
    int hours = 0;
    int minutes = 0;
    int seconds = 0;
    float fractions = 0;
    public:
    gameClock() = default;
    gameClock(float t) : fractions(t){ run(0);}
    void operator += (const float& t) {run(t);}

    void run(float time){
        fractions += time;
        while (fractions >= 1.0f){
            fractions -= 1.0f;
            seconds++;
        }
        while(seconds >= 60){
            seconds -= 60;
            minutes++;
        }
        while(minutes>= 60){
            minutes -= 60;
            hours++;
        }
    }

    std::string print(){
        std::string out = "";

        enum time {MM,HH};
        time ts;
        if (minutes > 0)
            ts = MM;
        if (hours > 0)
            ts = HH;

        std::string temp;

        /// starting at the largest time that is greater than 0 assemble the string.
        /// After the starting point always include all smaller steps (don't break out of cases)
        switch(ts){
        case HH:
            out += std::to_string(hours) + ":";
        case MM:
            temp = std::to_string(minutes);
            if (temp.length() == 1){
                temp = "0" + temp + ":";
            }
            out += temp;
        default:
            temp = std::to_string(seconds);
            if (temp.length() == 1){
                temp = "0" + temp;
            }
            out += temp + std::to_string(fractions).substr(1);
        }
        return out;
    };
};


class GameWorld{
private:
    olc::PixelGameEngine* srpg;
    olc::vi2d screenArea;
    float worldRadius;

    bool running = false;
    gameClock worldTime;
    float engineTime = 0.0f;
    float tickSize = 1.0f/60.0f;
    int maxTicks = 5;

    std::shared_ptr<olc::Sprite> heroicImage = nullptr;
    std::shared_ptr<Hero> mainChar;
    std::unique_ptr<FoeManager> villians;
    std::unique_ptr<DecalManager> lawn;
    std::unique_ptr<ProjectileManager> bulletList;
    std::list<std::shared_ptr<Projectile>> bullets;



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
