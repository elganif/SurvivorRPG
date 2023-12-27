#ifndef SRPG_DATA_H_INCLUDED
#define SRPG_DATA_H_INCLUDED
#include "Rectangle.h"  //include needed here for childArea constructors in vector initalizer


class Entity;



    class QuadTree
    {
        public:
        static int depthLimit;


        private:
        int depth;
        Rectangle quadArea;
        QuadTree* parentNode;

        std::vector<QuadTree*> quads = {nullptr,nullptr,nullptr,nullptr};
        std::list<std::shared_ptr<Entity>> entStored;

        olc::vf2d quadrentSize = quadArea.sides * 0.5;

        std::vector<Rectangle> childArea =
                {Rectangle(quadArea.tl,quadrentSize),
                 Rectangle({quadArea.tl.x + quadrentSize.x,quadArea.tl.y},quadrentSize),
                 Rectangle({quadArea.tl.x,quadArea.tl.y + quadrentSize.y},quadrentSize),
                 Rectangle(quadArea.tl + quadrentSize,quadrentSize)};


        QuadTree(Rectangle newArea, int newdepth = 0,QuadTree* parent = nullptr);

        public:
        QuadTree(olc::vf2d newtl, olc::vf2d newbr, int newdepth = 0);

        ~QuadTree();

        void insertItem(const std::shared_ptr<Entity>& newEnt);

        void getOverlapItems(Rectangle area, std::list<std::shared_ptr<Entity>>& returns);

        void validateEnt(QuadTree*& treeNode, std::list<std::shared_ptr<Entity>>::iterator& entIT);
        void upEscalator(QuadTree*& treeNode, std::list<std::shared_ptr<Entity>>::iterator& entIT);
        void downEscalator(QuadTree*& treeNode, std::list<std::shared_ptr<Entity>>::iterator& entIT);
        void removeMe(QuadTree*& treeNode, std::list<std::shared_ptr<Entity>>::iterator& entIT);

        void clean();

        int size();
        int activity();
        int curDepth();
        void drawTree(Rectangle area, olc::Pixel item, olc::Pixel noItem );

        /// Testing code for profiling different depth limits.
        void adjustDepth(int change){depthLimit += change; if(depthLimit < 0) depthLimit = 0;}
    };



namespace srpg_data{
        extern const bool debugTools;
        extern std::unique_ptr<QuadTree> gameObjects;
        extern olc::TransformedView* viewer;
        extern uint8_t renderLayerFloor;
        extern uint8_t renderLayerEntities;
        extern uint8_t renderLayerUI;
        extern uint8_t renderLayerMenu;

        struct controls{
                olc::vf2d movement;
                olc::vf2d aim;
                olc::vf2d target;
                olc::vf2d UItarget;
                bool mainAttack;
                bool rapidFire;
                bool escapeKey;

        };
};

#endif // SRPG_DATA_H_INCLUDED
