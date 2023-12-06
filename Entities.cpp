

#include "olcPixelGameEngine.h"
#include "olcPGEX_TransformedView.h"
#include "Rectangle.h"
#include "srpg_data.h"
#include "Entities.h"
//#include "quadTree.h" currently included in this file. todo: fix that

using std::list;
using std::shared_ptr;

/// class Entity


/// class Hero : public Entity

        Hero::Hero( olc::vf2d spawn, float newSize):Entity(spawn,newSize){

            colour = olc::BLUE;
        }
        Hero::~Hero(){};
        Entity::TYPE Hero::whoAreYou(){return HERO;}
        bool Hero::isAlive(){return true;}
        olc::vf2d Hero::bump(olc::vf2d otherLoc,float otherSize){

            if(otherLoc == location){
                return {0,0};//assume same entity and skip collision
            }
            float entDist2 = (otherLoc - location).mag(); // magnitude of distance
            float colideDist2 = (otherSize + entSize); // also squared for pythagoras without roots
            if(colideDist2 > entDist2){
                float overLap = colideDist2 - entDist2;
                olc::vf2d bumpDirection = (location - otherLoc).norm();
                olc::vf2d thisMove = bumpDirection*overLap*0.5;
                movement(thisMove*0.5);
                return (-thisMove*1.5);

            } // else not collideing, return 0
            return{0,0};


        }

        void Hero::render(){
            srpg_data::viewer->DrawDecal(getBoxCollider().tl + olc::vf2d(0.0,-0.2)*entSize,image);



            //collider Debug
            srpg_data::viewer->DrawCircle(location,entSize,olc::VERY_DARK_BLUE);
            //srpg_data::viewer->DrawRect(getBoxCollider().tl,getBoxCollider().sides);
        }
        void Hero::setRender(olc::Sprite* sprite){
            image = new olc::Decal(sprite);
        }
        void Hero::makeRender(olc::Sprite* sprite,olc::vf2d area,olc::PixelGameEngine* game){

            game->SetDrawTarget(sprite);
            game->Clear(olc::BLANK);

            olc::Pixel colours = olc::BLUE;
            game->FillCircle(area.x * 0.50 ,area.y * 0.12 , area.x * 0.12 ,olc::BLACK);// Head
            game->DrawCircle(area.x * 0.50 ,area.y * 0.12 , area.x * 0.12,colours );


            game->FillRect( area.x * 0.25 ,area.y *0.25 , area.x *0.10 , area.y *0.30 ,olc::BLACK);// Left Arm
            game->DrawRect( area.x * 0.25 ,area.y *0.25 , area.x *0.10 , area.y *0.30 ,colours);

            game->FillRect(  area.x *0.65 , area.y *0.25 , area.x *0.10 , area.y *0.30 ,olc::BLACK);// Right Arm
            game->DrawRect(  area.x *0.65 , area.y *0.25 , area.x *0.10 , area.y *0.30 ,colours);

            game->FillRect( area.x *0.35 ,area.y *0.25 , area.x *0.30 , area.y *0.35 ,olc::BLACK); //torso
            game->DrawRect( area.x *0.35 ,area.y *0.25 , area.x *0.30 , area.y *0.35 ,colours);

            game->FillRect( area.x *0.38 , area.y *0.60 , area.x *0.12 , area.y *0.40 ,olc::BLACK);// Left Leg
            game->DrawRect( area.x *0.38 , area.y *0.60 , area.x *0.12 , area.y *0.40 ,colours);

            game->FillRect( area.x *0.50 , area.y *0.60 , area.x *0.12 , area.y *0.40 ,olc::BLACK);// Right Leg
            game->DrawRect( area.x *0.50 , area.y *0.60 , area.x *0.12 , area.y *0.40 ,colours);


            game->SetDrawTarget(nullptr);;
        }



/// class Foe : public Entity

        Foe::Foe( olc::vf2d spawn, float newSize):Entity(spawn,newSize){
            speed = 0.2f;
            colour = olc::DARK_RED;
        }
        Foe::~Foe(){};
        Entity::TYPE Foe::whoAreYou(){ return FOE;}
        bool Foe::update(float fElapsedTime, olc::vf2d worldMove){
            if (isAlive()) {
                location = location + (-location.norm() * speed * fElapsedTime) + worldMove;

                return 1;
            }
            return 0;
        }

        bool Foe::isAlive(){
            return HP > 0.0f;
        }
        void Foe::onOverlap(std::shared_ptr<Entity> other){
            if(other->whoAreYou() == HERO){
                //Collide with Hero
                if(other->isAlive()){
                    movement(std::dynamic_pointer_cast<Hero>(other)->bump(location,entSize));
                }
            }
            if(other->whoAreYou() == FOE){
                //Collide with Friendly
                if(other->isAlive()){
                    movement(std::dynamic_pointer_cast<Foe>(other)->bump(location,entSize));
                }
            }
            if(other->whoAreYou() == PROJECTILE){
                // subtract projectiles damage from hp
                HP = HP - std::dynamic_pointer_cast<Projectile>(other)->impact(this);
            }
        }
        olc::vf2d Foe::bump(olc::vf2d otherLoc,float otherSize){
            if(otherLoc == location){
                return {0.0f,0.0f};//assume same entity and skip collision
            }
            float entDist2 = (otherLoc - location).mag(); // magnitude of distance
            float colideDist2 = (otherSize + entSize); // also squared for pythagoras without roots
            if(colideDist2 > entDist2){
                float overLap = colideDist2 - entDist2;
                olc::vf2d bumpDirection = (location - otherLoc).norm();
                olc::vf2d thisMove = bumpDirection*overLap*0.5f;
                movement(thisMove);
                return -thisMove;

            } // else not collideing, return 0
            return{0.0f,0.0f};


        }

        void Foe::render(){
            srpg_data::viewer->DrawDecal(getBoxCollider().tl + olc::vf2d(0.0,-0.2)*entSize,image);

            // debug collider
            srpg_data::viewer->DrawCircle(location,entSize,olc::VERY_DARK_RED);
        }
        void Foe::setRender(olc::Decal* sprite){
            image = sprite;
        }
        void Foe::makeRender(olc::Sprite* sprite,olc::vf2d area,olc::PixelGameEngine* game){

            game->SetDrawTarget(sprite);
            game->Clear(olc::BLANK);

            olc::Pixel colours = olc::DARK_RED;
            game->FillCircle(area.x * 0.50 ,area.y * 0.12 , area.x * 0.12 ,olc::BLACK);// Head
            game->DrawCircle(area.x * 0.50 ,area.y * 0.12 , area.x * 0.12,colours );


            game->FillRect( area.x * 0.25 ,area.y *0.25 , area.x *0.10 , area.y *0.30 ,olc::BLACK);// Left Arm
            game->DrawRect( area.x * 0.25 ,area.y *0.25 , area.x *0.10 , area.y *0.30 ,colours);

            game->FillRect(  area.x *0.65 , area.y *0.25 , area.x *0.10 , area.y *0.30 ,olc::BLACK);// Right Arm
            game->DrawRect(  area.x *0.65 , area.y *0.25 , area.x *0.10 , area.y *0.30 ,colours);

            game->FillRect( area.x *0.35 ,area.y *0.25 , area.x *0.30 , area.y *0.35 ,olc::BLACK); //torso
            game->DrawRect( area.x *0.35 ,area.y *0.25 , area.x *0.30 , area.y *0.35 ,colours);

            game->FillRect( area.x *0.38 , area.y *0.60 , area.x *0.12 , area.y *0.40 ,olc::BLACK);// Left Leg
            game->DrawRect( area.x *0.38 , area.y *0.60 , area.x *0.12 , area.y *0.40 ,colours);

            game->FillRect( area.x *0.50 , area.y *0.60 , area.x *0.12 , area.y *0.40 ,olc::BLACK);// Right Leg
            game->DrawRect( area.x *0.50 , area.y *0.60 , area.x *0.12 , area.y *0.40 ,colours);


            game->SetDrawTarget(nullptr);;
        }


/// class Projectile : public Entity

//        Projectile(olc::TransformedView* window, olc::vf2d spawn, float newSize, float width, float duration,
//                float speed, olc::vf2d orientation,int hitCount):Entity(window,spawn,newSize){
//
//            shape = width;
//            lifespan = duration;
//            travel = speed;
//            direction = orientation.norm();
//            colour = olc::DARK_GREY;
//            hits = hitCount;
//        }
//        ~Projectile(){};
//
//        TYPE whoAreYou(){ return PROJECTILE;}

        bool Projectile::update(float fElapsedTime, olc::vf2d worldMove){
            if (isAlive()) {
                location = location + (direction * travel * fElapsedTime) + worldMove;
                lifespan -= fElapsedTime;
                return 1;
            }
            return 0;
        }

        bool Projectile::isAlive()
        {
            return (lifespan > 0.0f && hits > 0);
        }

        float Projectile::impact(Entity* other){
            // eventually should check Entity for if it obliterates bullets (sets hits to zero) or not
            if(isAlive() && (location - other->getLocal()).mag2() <= other->getSize() * other->getSize()){
                hits--;
                return 10.0f;// Damage for now, eventualy should be struct of effects
            }
            return 0.0f;
        }
        void Projectile::kill(){
            lifespan = 0.0f;
            hits = 0;
        }
        float Projectile::getLife()
        {
            return lifespan;
        }

        void Projectile::render(){
            // Set up Tips of Triangle
            olc::vf2d tip =   {0.0f,entSize*0.5f};
            olc::vf2d left =  {shape*0.5f, -entSize*0.5f};
            olc::vf2d right = {-shape*0.5f, -entSize*0.5f};
            // orient to follow movement
            tip = rotatePt(tip,direction);
            left = rotatePt(left,direction);
            right = rotatePt(right,direction);
            srpg_data::viewer->DrawTriangle(tip,left,right, colour);
        }





/// class Decoration : public Entity
        Decoration::Decoration::Decoration( olc::vf2d spawn, float newSize):Entity(spawn,newSize){

            colour = olc::DARK_GREEN;
        }
        Decoration::~Decoration(){};
        Entity::TYPE Decoration::whoAreYou(){ return DECORATION;}

        void Decoration::render(){
            srpg_data::viewer->DrawLine(location.x + (entSize/2.0f),location.y,location.x + (entSize/2.0f),location.y - entSize,colour);
            srpg_data::viewer->DrawLine(location.x ,location.y,location.x ,location.y - entSize*0.6f,colour);
            srpg_data::viewer->DrawLine(location.x + entSize,location.y,location.x + entSize,location.y - entSize*0.6f,colour);
        }





/*
class QuadTree
{
    const int depthLimit = 5;


    int depth;
    rectangle quadArea;// = {{-20,-20},{40,40}};
    QuadTree* parentNode = NULL;

    std::vector<QuadTree*> quads = {NULL,NULL,NULL,NULL};
    std::list<std::shared_ptr<Entity>> entStored;

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

    void insertItem(const std::shared_ptr<Entity>& newEnt){
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

    void getOverlapItems(rectangle area, std::list<std::shared_ptr<Entity>>& returns){
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

    // Validate items and relocate them to other quads if needed

    void validateLocations(){
        std::list<std::shared_ptr<Entity>> riders;
        for(auto it = entStored.begin(); it != entStored.end(); ){ //no incrementor, all paths will increment

            if(!(*it)->isAlive()){
                // remove expired entities
                it->reset();
                entStored.erase(it++);
                continue;
            }
            if (depth != 0 && !quadArea.contains((*it)->getBoxCollider())){
                // not at root and item does not fit. Send it up
                riders.push_back((*it));
                it->reset();
                entStored.erase(it++);
                continue;
            }
            int targetQuad = -1;
            if(depth < depthLimit){
                for(int i = 0; i < 4; i++)
                    if(childArea[i].contains((*it)->getBoxCollider()))
                        targetQuad = i;
            }
            if(targetQuad == -1){ // negitive 1 means item should remain at this level, move on
                it++;
                continue;
            } //else targetQuad contains the quad index to send this item
            if(!quads[targetQuad]){ //depth already checked
                quads[targetQuad] = new QuadTree(childArea[targetQuad],depth+1,this);
            }
            quads[targetQuad]->insertItem((*it));
            it->reset();
            entStored.erase(it++);
        }
        for(int i = 0; i < 4; i++){
            if(quads[i])
                quads[i]->validateLocations();
        }
        if(riders.size() > 0)
            parentNode->upElevator(riders);

        if(depth == 0){
            prune();
        }
    }
    void prune(){
        for(int i = 0; i < 4; i++){
            if(quads[i]){
                if(quads[i]->size() == 0){
                    delete quads[i];
                    quads[i] = nullptr;
                } else {
                    quads[i]->prune();
                }
            }
        }
    }
    void upElevator(std::list<std::shared_ptr<Entity>> &riders){
            for(auto it = riders.begin();it != riders.end(); ){
                if(depth == 0 || quadArea.contains((*it)->getBoxCollider())){
                    insertItem((*it));
                    it->reset();
                    riders.erase(it++);
                    continue;
                } //else
                it++;
            }
            if(riders.size() > 0)
                parentNode->upElevator(riders);
        }
    int size()
    {
        int thisCount = entStored.size();
        for(int i = 0;i < 4;i++){
            thisCount += quads[i] ? quads[i]->size() : 0;
        }
        return thisCount;
    }
    int activity(){
        int numQuads = 1;
        for(int i = 0; i < 4; i++){
            numQuads += (quads[i])?quads[i]->activity() : 0;
        }
        return numQuads;
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
*/
