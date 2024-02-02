#ifndef SRPG_DATA_H_INCLUDED
#define SRPG_DATA_H_INCLUDED
#include "Rectangle.h"  //include needed here for constructors in childArea initalizer
#include "Entities.h"
#include <deque>

class Entity;

class QuadTree
{
    public:
    static int depthLimit;

    private:
    int depth;
    Rectangle quadArea;
    olc::vf2d quadrentSize = quadArea.sides * 0.5;
    std::vector<Rectangle> childArea =
            {Rectangle(quadArea.tl,quadrentSize),
             Rectangle({quadArea.tl.x + quadrentSize.x,quadArea.tl.y},quadrentSize),
             Rectangle({quadArea.tl.x,quadArea.tl.y + quadrentSize.y},quadrentSize),
             Rectangle(quadArea.tl + quadrentSize,quadrentSize)};

    QuadTree* parentNode;
    std::vector<QuadTree*> quads = {nullptr,nullptr,nullptr,nullptr};
    std::list<std::shared_ptr<Entity>> entStored;


    QuadTree(Rectangle newArea, int newdepth = 0,QuadTree* parent = nullptr);

    public:
    QuadTree(olc::vf2d newtl, olc::vf2d newbr);

    ~QuadTree();

    void insertItem(const std::shared_ptr<Entity>& newEnt);

    void getOverlapItems(Rectangle area, std::list<std::shared_ptr<Entity>>& returns);

    enum TARG {
        CLOSE,
        WEAK,
        STRONG
    };
    int getFoes(olc::vf2d targetLoc, float range, int numTarg, std::list<std::shared_ptr<Entity>>& returns, TARG targType);
    void getFoes(olc::vf2d targetLoc, float range, int numTarg, std::list<std::shared_ptr<Entity>>& returns, std::function<bool(const std::shared_ptr<Entity> f, const std::shared_ptr<Entity> s)> targType);

    void validateEnt(QuadTree*& treeNode, std::list<std::shared_ptr<Entity>>::iterator& entIT);
    void upEscalator(QuadTree*& treeNode, std::list<std::shared_ptr<Entity>>::iterator& entIT);
    void downEscalator(QuadTree*& treeNode, std::list<std::shared_ptr<Entity>>::iterator& entIT);
    void removeMe(QuadTree*& treeNode, std::list<std::shared_ptr<Entity>>::iterator& entIT);

    bool clean();

    int size();
    int activity();
    int curDepth();
    void drawTree(Rectangle area, olc::Pixel item, olc::Pixel noItem );

    /// Testing code for profiling different depth limits.
    void adjustDepth(int change){depthLimit += change; if(depthLimit < 0) depthLimit = 0;}
};

class Profiler {
public:

    std::string defaultName = "CoreFrame";
    int frameCounter = 0;
    Profiler();
    ~Profiler() = default;
    struct Event{
        std::string identity;
        int frameNum;
        std::chrono::_V2::steady_clock::time_point startT;
        std::chrono::_V2::steady_clock::time_point stopT;
        float passedTime(){ return std::chrono::duration<float>(stopT - startT).count();}
        Event(std::string name,int frameOn):identity(name),frameNum(frameOn) {startT = std::chrono::_V2::steady_clock::now();}
    };

    std::unordered_map<std::string, std::list<Event>> events;

    void frameMark();
    void start(std::string timerID);
    float stop(std::string timerID);
    bool running(std::string timerID);

    void drawDebug(olc::PixelGameEngine* game);
};

namespace srpg_data{
    const bool debugTools = true;
    extern std::unique_ptr<Profiler> timers;

    extern std::unique_ptr<QuadTree> gameObjects;
    extern olc::TransformedView* viewer;
    extern uint8_t renderLayerFloor;
    extern uint8_t renderLayerEntities;
    extern uint8_t renderLayerUI;
    extern uint8_t renderLayerMenu;

    struct controls{
            olc::vf2d movement;
            olc::vf2d target;
            olc::vf2d UItarget;
            bool mainAttack;
            bool rapidFire;
            bool escapeKey;
    };
};

#endif // SRPG_DATA_H_INCLUDED
