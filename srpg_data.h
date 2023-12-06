#ifndef SRPG_DATA_H_INCLUDED
#define SRPG_DATA_H_INCLUDED
#include "Rectangle.h"

class Entity;



    class QuadTree
    {
        public:
        static int depthLimit;
        private:
        int depth;
        Rectangle quadArea;
        QuadTree* parentNode;

        std::vector<QuadTree*> quads = {NULL,NULL,NULL,NULL};
        std::list<std::shared_ptr<Entity>> entStored;

        olc::vf2d quadrentSize = quadArea.sides * 0.5;
        std::vector<Rectangle> childArea =
                {Rectangle(quadArea.tl,quadrentSize),
                 Rectangle({quadArea.tl.x + quadrentSize.x,quadArea.tl.y},quadrentSize),
                 Rectangle({quadArea.tl.x,quadArea.tl.y + quadrentSize.y},quadrentSize),
                 Rectangle(quadArea.tl + quadrentSize,quadrentSize)};


        public:
        QuadTree(olc::vf2d newtl, olc::vf2d newbr, int newdepth = 0);

        QuadTree(Rectangle newArea, int newdepth = 0,QuadTree* parent = nullptr);
        ~QuadTree();

        void insertItem(const std::shared_ptr<Entity>& newEnt);

        void getOverlapItems(Rectangle area, std::list<std::shared_ptr<Entity>>& returns);


        void validateLocations();
        void upElevator(std::list<std::shared_ptr<Entity>> &riders);
        void prune();

        int size();
        int activity();
        int curDepth();
        void drawTree(olc::Pixel item,olc::Pixel noItem );

        void adjustDepth(int change){depthLimit += change;}
    };

namespace srpg_data{
        extern QuadTree* gameObjects ;
        extern olc::TransformedView* viewer;
};
#endif // SRPG_DATA_H_INCLUDED
