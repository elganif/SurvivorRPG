#ifndef SRPG_DATA_H_INCLUDED
#define SRPG_DATA_H_INCLUDED
#include "Rectangle.h"  //include needed here for constructors in childArea initalizer

class Entity;

class QuadTree
{
    public:
    static int depthLimit;

    private:
    int depth;
    Rectangle quadArea;
    olc::vf2d centerPoint = quadArea.tl + quadArea.sides * 0.5;
    olc::vf2d quadrentSize = quadArea.sides * 0.5;
    std::vector<Rectangle> childArea =
            {Rectangle(quadArea.tl,quadrentSize),
             Rectangle({quadArea.tl.x + quadrentSize.x,quadArea.tl.y},quadrentSize),
             Rectangle({quadArea.tl.x,quadArea.tl.y + quadrentSize.y},quadrentSize),
             Rectangle(quadArea.tl + quadrentSize,quadrentSize)};

    QuadTree* parentNode;
    std::vector<QuadTree*> quads = {nullptr,nullptr,nullptr,nullptr};
    std::map<int,std::shared_ptr<Entity>> entStored;

    /// private constructor for making subquads.
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

    void validateEnt(QuadTree*& treeNode,int entID);
    void removeMe(QuadTree*& treeNode,int entID);
    private: /// used as part of validation process to relocate items.
    void upEscalator(QuadTree*& treeNode, std::shared_ptr<Entity>& entIT);
    void downEscalator(QuadTree*& treeNode,std::shared_ptr<Entity>& entIT);

    public:
    bool clean();

    int size();
    int activity();
    int curDepth();
    void drawTree(Rectangle area, olc::Pixel item, olc::Pixel noItem );
//
//    /// Testing method for profiling different depth limits.
//    void adjustDepth(int change){depthLimit += change; if(depthLimit < 0) depthLimit = 0;}
};

class Profiler {
public:
    std::string defaultName = "CoreFrame";
    int frameCounter = 0;
    Profiler();
    ~Profiler() = default;
    struct Event{
        std::string identity;
        const int frameNum;
        int openCount = 1;
        std::chrono::_V2::steady_clock::time_point startT;
        std::chrono::_V2::steady_clock::time_point stopT;

        Event(std::string name,int frameOn):identity(name),frameNum(frameOn) {startT = std::chrono::_V2::steady_clock::now(); }
        std::chrono::_V2::steady_clock::duration passedTime(){ return std::chrono::duration(stopT - startT); }

    };

    std::map<int,Event> coreFrame;
    std::unordered_map<std::string, std::list<Event>> events;

    void frameMark();
    void start(std::string timerID);
    std::chrono::_V2::steady_clock::duration stop(std::string timerID);

    void drawDebug(olc::PixelGameEngine* game);
};

namespace srpg{
    const bool debugTools = true;
    extern std::unique_ptr<Profiler> timers;

    extern std::unique_ptr<QuadTree> gameObjects;
    extern std::unique_ptr<olc::TransformedView> viewer;
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
