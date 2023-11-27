#ifndef ENTITIES_H_INCLUDED
#define ENTITIES_H_INCLUDED

#include "quadTree.h"

using std::list;
using std::shared_ptr;

 struct rectangle{
    olc::vf2d tl;
    olc::vf2d sides;

    rectangle(const olc::vf2d& loc, const olc::vf2d& area) : tl(loc),sides(area){}

    bool contains(const rectangle& other){
        if(tl.x <= other.tl.x && tl.x + sides.x > other.tl.x + other.sides.x &&
           tl.y <= other.tl.y && tl.y + sides.y > other.tl.y + other.sides.y){
           return true;
        }
        return false;


    }
    bool overlaps(const rectangle& other){
        if(tl.x < other.tl.x + other.sides.x && tl.x + sides.x >= other.tl.x &&
           tl.y < other.tl.y + other.sides.y && tl.y + sides.y >= other.tl.y){
            return true;
        }
        return false;
    }
};

class Entity
    {
        protected:
        float entSize;
        olc::vf2d location;
        olc::Pixel colour = olc::DARK_MAGENTA;
        olc::TransformedView* view;
        bool solidCollision = false;
        enum TYPE {
            HERO,
            FOE,
            DECORATION,
            PROJECTILE,
            I_DONT_KNOW

        };

        public:
        Entity(){}
        virtual ~Entity(){};

        void placement(olc::vf2d destiny){
            location = destiny;
        }
        void movement(olc::vf2d destiny){
            location += destiny;
        }
        virtual void render(){};
        virtual void render(olc::vf2d x){};

        virtual TYPE whoAreYou(){ return I_DONT_KNOW;}

        rectangle getBoxCollider(){
            return rectangle({location.x - (entSize/2.0),location.y - (entSize/2.0)},{entSize*2.0,entSize*2.0});
        }

        // rotates a point around center
        olc::vf2d rotatePt(olc::vf2d point,olc::vf2d angle){
            olc::vf2d updatedpoint = point - location;

            updatedpoint.x = (point.y * angle.x) + (point.x * angle.y);
            updatedpoint.y = -(point.x * angle.x) + (point.y * angle.y);
            updatedpoint += location;
            return updatedpoint;
        }
        virtual bool isAlive(){return true;}
    };

class Hero : public Entity
    {
    private:

        bool solidCollision = true;
    public:
        Hero(olc::TransformedView* window, olc::vf2d spawn, float newSize){
            location = spawn;
            entSize = newSize;
            view = window;
            colour = olc::BLUE;
        }
        ~Hero(){};
        TYPE whoAreYou(){return HERO;}

        void render(){
            view->DrawCircle(location,entSize,colour);
        }

        void render(olc::vf2d faceing){
            float looking = 0;
            if (faceing.x < 0.0)
                looking += 0.05;
            if (faceing.x > 0.0)
                looking -= 0.05;

            view->FillCircle((0.0 + looking) * entSize,-1 * entSize, 0.25 * entSize,olc::BLACK);// Head
            view->DrawCircle((0.0 + looking) * entSize,-1 * entSize, 0.25 * entSize);


            view->FillRect( -0.45 * entSize,-0.7 * entSize, 0.15 * entSize, 0.6 * entSize,olc::BLACK);// Left Arm
            view->DrawRect( -0.45 * entSize,-0.7 * entSize, 0.15 * entSize, 0.6 * entSize,colour);

            view->FillRect(  0.3 * entSize, -0.7 * entSize, 0.15 * entSize, 0.6 * entSize,olc::BLACK);// Right Arm
            view->DrawRect(  0.3 * entSize, -0.7 * entSize, 0.15 * entSize, 0.6 * entSize,colour);

            view->FillRect( -0.3 * entSize,-0.75 * entSize, 0.6 * entSize, 0.75 * entSize,olc::BLACK); //torso
            view->DrawRect( -0.3 * entSize,-0.75 * entSize, 0.6 * entSize, 0.75 * entSize,colour);

            view->FillRect( -0.25 * entSize, 0.0 * entSize, 0.25 * entSize, 0.8 * entSize,olc::BLACK);// Left Leg
            view->DrawRect( -0.25 * entSize, 0.0 * entSize, 0.25 * entSize, 0.8 * entSize,colour);

            view->FillRect(  0.0 * entSize, 0.0 * entSize, 0.25 * entSize, 0.8 * entSize,olc::BLACK);// Right Leg
            view->DrawRect(  0.0 * entSize, 0.0 * entSize, 0.25 * entSize, 0.8 * entSize,colour);


            //collider Debug
            view->DrawCircle(location,entSize,olc::VERY_DARK_BLUE);
        }

    };

class Foe : public Entity
    {
    private:

        bool solidCollision = true;
    public:
        Foe(olc::TransformedView* window, olc::vf2d spawn, float newSize){

            view = window;
            location = spawn;
            entSize = newSize;
            colour = olc::DARK_RED;
        }
        ~Foe(){};
        TYPE whoAreYou(){ return FOE;}
        void render(){
            view->DrawCircle(location,entSize,colour);
        }

        int onOverlap(Entity* other){




            }
        }
        // Testing code, may use later for Charecter Sprite
        void render(olc::vf2d faceing){
            float shape = entSize/2;
            olc::vf2d tip =   {0,entSize/2};
            olc::vf2d left =  {shape/2.0, -entSize/2};
            olc::vf2d right = {-shape/2.0, -entSize/2};
            tip = rotatePt(tip,faceing);
            left = rotatePt(left,faceing);
            right = rotatePt(right,faceing);


            view->DrawTriangle(tip+location,left+location,right+location, colour);
        }
    };

class Projectile : public Entity
    {
    private:
        olc::vf2d direction;
        float shape;
        float travel;
        float lifespan;
        int hits;
    public:
        Projectile(olc::TransformedView* window, olc::vf2d spawn, float newSize, float width, float duration,
                float speed, olc::vf2d orientation,int hitCount){
            location = spawn;
            entSize = newSize;
            shape = width;
            view = window;
            lifespan = duration;
            travel = speed;
            direction = orientation.norm();
            colour = olc::DARK_GREY;
            hits = hitCount;
        }
        ~Projectile(){};

        TYPE whoAreYou(){ return PROJECTILE;}

        bool update(float fElapsedTime, olc::vf2d worldMove){
            if (isAlive()) {
                location = location + (direction * travel * fElapsedTime) + worldMove;
                lifespan -= fElapsedTime;
                return 1;
            }
            return 0;
        }

        bool isAlive()
        {
            return (lifespan > 0.0 && hits > 0);
        }

        int impact(Entity& other){
            // eventually should check Entity for if it obliterates bullets (sets hits to zero) or not
            hits--;
            return 1;// Daamge for now, eventualy should be struct of effects
        }

        float getLife()
        {
            return lifespan;
        }

        void render(){
            // Set up Tips of Triangle
            olc::vf2d tip =   {0,entSize*0.5};
            olc::vf2d left =  {shape*0.5, -entSize*0.5};
            olc::vf2d right = {-shape*0.5, -entSize*0.5};
            // orient to follow movement
            tip = rotatePt(tip,direction);
            left = rotatePt(left,direction);
            right = rotatePt(right,direction);
            view->DrawTriangle(tip,left,right, colour);
        }


};


class Decoration : public Entity
{
    private:

        bool solidCollision = false;
    public:
        Decoration(olc::TransformedView* window, olc::vf2d spawn, float newSize){
            location = spawn;
            entSize = newSize;
            view = window;
            colour = olc::DARK_GREEN;
        }
        ~Decoration(){};
        TYPE whoAreYou(){ return DECORATION;}
        olc::vf2d findLoc(){
            return location;
        }

        void render(){
            view->DrawLine(location.x + (entSize/2.0f),location.y,location.x + (entSize/2.0f),location.y - entSize,colour);
            view->DrawLine(location.x ,location.y,location.x ,location.y - entSize*0.6f,colour);
            view->DrawLine(location.x + entSize,location.y,location.x + entSize,location.y - entSize*0.6f,colour);
        }
};





class QuadTree
{
    const int depthLimit = 5;

    //olc::vf2d tl;
    //olc::vf2d br;
    int depth;
    rectangle quadArea;// = {{-20,-20},{40,40}};
    QuadTree* parentNode = NULL;

    std::vector<QuadTree*> quads = {NULL,NULL,NULL,NULL};
    list<shared_ptr<Entity>> entStored;

    olc::vf2d quadrentSize = quadArea.sides * 0.5;
    std::array<rectangle,4> childArea = {
        rectangle(quadArea.tl,quadrentSize),
        rectangle({quadArea.tl.x + quadrentSize.x,quadArea.tl.y},quadrentSize),
        rectangle({quadArea.tl.x,quadArea.tl.y + quadrentSize.y},quadrentSize),
        rectangle(quadArea.tl + quadrentSize,quadrentSize)
    };


    public:
    QuadTree(olc::vf2d newtl, olc::vf2d newbr, int newdepth = 0,QuadTree* parent = nullptr)
        :depth(newdepth),quadArea({newtl,newbr}),parentNode(parent)
    {
        //tl = newtl;
        //br = newbr;


    }
    QuadTree(rectangle newArea, int newdepth,QuadTree* parent = nullptr)
        :depth(newdepth),quadArea(newArea),parentNode(parent)
    {

    }
    ~QuadTree(){
        // to ensure shared pointers and memory are cleaned up properly delete each sub quad and then clear the local list
        for(int i = 0; i < 4; i++){
            delete quads[i];
        }
        while (entStored.begin() != entStored.end()){
            entStored.erase(entStored.begin());
        }
    };

    void insertItem(const shared_ptr<Entity>& newEnt){
         // Check if it belongs lower in the tree
         for(int i = 0;i < 4;i++){
            if(depth < depthLimit && childArea[i].contains(newEnt->getBoxCollider())){
                if(!quads[i]){
                    quads[i] = new QuadTree(childArea[i],depth+1,this);
                }
                quads[i]->insertItem(newEnt);
                return;
            }
        } // else we store it in this element
        entStored.push_back(newEnt);

        return;
    }

    void getOverlapItems(rectangle area, list<shared_ptr<Entity>>& returns){
        // collect overlaps from children
        for(int i = 0;i < 4;i++){
            if(childArea[i].overlaps(area) && quads[i]){
                 quads[i]->getOverlapItems(area,returns);
            }
        }
        // add any overlaped items from this layer
        for(auto it = entStored.begin(); it!= entStored.end(); it++){
            if(area.overlaps((*it)->getBoxCollider())){
                returns.push_back((*it));
            }
        }
        return;
    }

    // Allows external applications call validation without needing to provide an empty list
    void validateLocations(){
        list<shared_ptr<Entity>> newList;
        validateLocations(newList);
    }

    void validateLocations(list<shared_ptr<Entity>> &travelingList){
        // Verify current locations of objects, moving them up or down the tree as needed
        list<shared_ptr<Entity>> upwardBound;
        auto entity = entStored.begin();
        while (entity != entStored.end()){
            if(!(*entity)->isAlive()){  // Simply remove and expired entites that no longer need to exist
               entStored.erase(entity++);
               continue;
            }
            // if not root and entity has left the area of this quad put it in a list to travel up the tree - using local list lets children skip checking
            if(depth != 0 && !quadArea.contains((*entity)->getBoxCollider())){
                upwardBound.push_back((*entity));
                entStored.erase(entity++);
                continue;
            }
            // check if it fits any child area and place in Traveling List
            bool sentDown = false;
            for(int i = 0; i < 4; i++){
                if(depth < depthLimit && childArea[i].contains((*entity)->getBoxCollider())){

                    travelingList.push_back((*entity));
                    entStored.erase(entity++);
                    sentDown = true;
                    if(!quads[i]){
                        quads[i] = new QuadTree(childArea[i],depth+1,this);
                    }
                    break;
                }

            }
            if(sentDown)
                continue;
            entity++;

        }
        // pass list down tree for redistribution of items not in their proper place
        for(int i = 0; i < 4; i++){
            if(quads[i]){
                quads[i]->validateLocations(travelingList);
            }
        }
        // collect any items that belong in this quad before it passes back up the tree
        auto returned = travelingList.begin();
        while(returned != travelingList.end()){
            if(quadArea.contains((*returned)->getBoxCollider()) || depth == 0){
                entStored.push_back((*returned));
                travelingList.erase(returned++);
                continue;
            }
            returned++;
        }
        // add the list of entities headed back up to the traveling list
        travelingList.splice(travelingList.end(),upwardBound);
        return;
    }

    int size()
    {
        int thisCount = 0;
        for(int i = 0;i < 4;i++){
             if(quads[i]){
                thisCount += quads[i]->size();
            }
        }
        thisCount += entStored.size();
        return thisCount;
    }

    int curDepth(){
        int depthCharge = depth;
        for (int i = 0; i < 4; i++){
            if(quads[i]){
                if(quads[i]->curDepth() > depthCharge)
                    depthCharge = quads[i]->curDepth();
            }
        }
        return depthCharge;
    }
    void drawTree(olc::TransformedView* viewer,olc::Pixel colours = olc::VERY_DARK_YELLOW){
        if(entStored.size() > 0){
            viewer->DrawRect(quadArea.tl,quadArea.sides,colours);
        } else {
            viewer->DrawRect(quadArea.tl,quadArea.sides,olc::YELLOW);
        }
        for(int i = 0;i < 4; i++)
            if(quads[i])
                quads[i]->drawTree(viewer,colours);
    }
};
#endif // ENTITIES_H_INCLUDED
