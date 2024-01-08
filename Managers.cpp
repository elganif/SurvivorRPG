

#include "olcPixelGameEngine.h"
#include "olcPGEX_TransformedView.h"
#include "Rectangle.h"
#include "Entities.h"
#include "srpg_data.h"
#include "Managers.h"
#include "Menus.h"



/// class Manager
template <class E>
Manager<E>::Manager(olc::PixelGameEngine* game, float world) : srpg(game),worldRadius(world) {}

template <class E>
Manager<E>::~Manager() {
    drawing.reset();
    image.reset();
}

template <class E>
int Manager<E>::size(){
    return items.size();
}
template <class E>
int Manager<E>::updateAndClear(float fElapsedTime){
    int removed = 0;
    for( auto ent = items.begin(); ent != items.end(); ) /// no iterator due to erase call - all paths must increment.
    {
        if((*ent)->isValid()){
            (*ent)->update(fElapsedTime);
            ent++;
            continue;
        } // else ent is not alive and can be removed
        (*ent).reset();
        items.erase(ent++);
        removed++;
    }
    return removed;
}

template <class E>
void Manager<E>::eofUpdate(float fElapsedTime, olc::vf2d movement){
    for( auto ent = items.begin(); ent != items.end();ent++ ){
        (*ent)->eofUpdate(fElapsedTime,movement);
    }
}

/// Foe Manager is designed around enemies and will maintain their numbers, stats growth and overall difficulty
/// class FoeManager

FoeManager::FoeManager(olc::PixelGameEngine* game, float world):Manager(game, world){};
FoeManager::~FoeManager(){}


void FoeManager::initalize(int numFoes) {
    maxPop = numFoes;

    /// Set up the sprite & decal for foes
    makeRender();

    for(int i = 0; i< maxPop;i++){
        spawn(foeSize);

    }
}

/// Code for creating a new unit
void FoeManager::spawn(float foeSize){
    olc::vf2d attempt;
    float spawnRad = worldRadius;
    /// Get a random point, equal distributed around center by disreguarding corners
    do{
    attempt.x =  (float)rand() / (float)RAND_MAX - 0.5f; // these subtractions give a range around zero and save a divide by 2 later
    attempt.y = (float)rand() / (float)RAND_MAX - 0.5f;
    } while(attempt.mag2() > 1.0f && attempt.mag2() != 0.0f);

    /// establish spawn distance
    attempt *= worldRadius; // attempt is already between 0 and .5 units so this is only half way to edge of the world
    attempt += attempt.norm() * 2; // move away from center 2 units to ensure not spawning on screen

    /// Add spawned unit to the list and the quad tree
    std::shared_ptr<Foe> theEvil = std::make_shared<Foe>(attempt,foeSize);
    theEvil->setSharedDecal(drawing);
    items.push_back(theEvil);
    srpg_data::gameObjects->insertItem(std::move(theEvil));
}


void FoeManager::makeRender(){
    /// prepare the sprite object for drawing
    olc::vi2d area = srpg_data::viewer->ScaleToScreen({foeSize*2,foeSize*2} ) ;
    image = std::make_shared<olc::Sprite>(area.x+1,area.y+1);

    srpg->SetDrawTarget(image.get());
    srpg->Clear(olc::BLANK);

    olc::Pixel colours = olc::DARK_RED;
    srpg->FillCircle(area.x * 0.50 ,area.y * 0.12 , area.x * 0.12 ,olc::BLACK);// Head
    srpg->DrawCircle(area.x * 0.50 ,area.y * 0.12 , area.x * 0.12,colours );


    srpg->FillRect( area.x * 0.25 ,area.y *0.25 , area.x *0.10 , area.y *0.30 ,olc::BLACK);// Left Arm
    srpg->DrawRect( area.x * 0.25 ,area.y *0.25 , area.x *0.10 , area.y *0.30 ,colours);

    srpg->FillRect(  area.x *0.65 , area.y *0.25 , area.x *0.10 , area.y *0.30 ,olc::BLACK);// Right Arm
    srpg->DrawRect(  area.x *0.65 , area.y *0.25 , area.x *0.10 , area.y *0.30 ,colours);

    srpg->FillRect( area.x *0.35 ,area.y *0.25 , area.x *0.30 , area.y *0.35 ,olc::BLACK); //torso
    srpg->DrawRect( area.x *0.35 ,area.y *0.25 , area.x *0.30 , area.y *0.35 ,colours);

    srpg->FillRect( area.x *0.38 , area.y *0.60 , area.x *0.12 , area.y *0.40 ,olc::BLACK);// Left Leg
    srpg->DrawRect( area.x *0.38 , area.y *0.60 , area.x *0.12 , area.y *0.40 ,colours);

    srpg->FillRect( area.x *0.50 , area.y *0.60 , area.x *0.12 , area.y *0.40 ,olc::BLACK);// Right Leg
    srpg->DrawRect( area.x *0.50 , area.y *0.60 , area.x *0.12 , area.y *0.40 ,colours);

    /// Set decal and return draw target to default
    drawing = std::make_shared<olc::Decal>(image.get());
    srpg->SetDrawTarget(nullptr);;
}

void FoeManager::update(float fElapsedTime){

    deadFoes += updateAndClear(fElapsedTime);

    /// after updates and removal check collisions.
    for( auto V = items.begin(); V!=items.end();V++ ){
        if((*V)->getLocal().mag() > worldRadius){
            (*V)->movement((*V)->getLocal().norm() * worldRadius * -2);
        }
        std::list<std::shared_ptr<Entity>> impacts;
        srpg_data::gameObjects->getOverlapItems((*V)->getBoxCollider(),impacts);
        for(auto t = impacts.begin();t != impacts.end();t++){
            if( (*V) != (*t))
                (*V)->onOverlap((*t)) ;
        }

    }

    /// On each frame that enemies are below desired population add one more.
    if(items.size() < maxPop){
        spawn(foeSize);
    }
    items.sort();
}

void FoeManager::eofUpdate(float fElapsedTime, olc::vf2d movement){
    Manager<Foe>::eofUpdate(fElapsedTime, movement);
}

int FoeManager::getKills(){return deadFoes;}



/// class ProjectileManager
ProjectileManager::ProjectileManager(olc::PixelGameEngine* srpg, float worldRadius) : Manager(srpg,worldRadius){};
ProjectileManager::~ProjectileManager(){

}

//void initalize(int numFoes);
void ProjectileManager::spawn(olc::vf2d origin, olc::vf2d momentum, olc::vf2d bulletSize){
    std::shared_ptr<Projectile> temp = std::make_shared<Projectile>(origin,bulletSize,2.0f,momentum,1,srpg);
    temp->setSharedDecal(drawing);
    items.push_front(temp);
    srpg_data::gameObjects->insertItem(temp);
}

void ProjectileManager::update(float fElapsedTime){
    updateAndClear(fElapsedTime);
    while(items.size() > 5000){
        items.back()->kill();
        items.back()->isValid();
        items.pop_back();
    }
}

void ProjectileManager::eofUpdate(float fElapsedTime, olc::vf2d movement){
    Manager<Projectile>::eofUpdate(fElapsedTime, movement);
}

void ProjectileManager::makeRender(olc::vf2d pSize){
    /// prepare the sprite object for drawing
    olc::vi2d area = srpg_data::viewer->ScaleToScreen({pSize.x,pSize.y}) ;
    image = std::make_shared<olc::Sprite>(area.x+1,area.y+1);

    srpg->SetDrawTarget(image.get());
    srpg->Clear(olc::BLANK);

    // Set up Tips of Triangle
    olc::vi2d tip =   {area.x / 2,0};
    olc::vi2d left =  {0,area.y};
    olc::vi2d right = {area.x,area.y};

    olc::Pixel fColour = olc::Pixel(0,0,0);
    olc::Pixel lineColour = olc::Pixel(128,0,63); /// temp until I figure out where these should live and how they get here
    srpg->FillTriangle(tip,left,right,fColour);
    srpg->DrawTriangle(tip,left,right,lineColour);

    /// Set decal and return draw target to default
    drawing = std::make_shared<olc::Decal>(image.get());
    srpg->SetDrawTarget(nullptr);
}


/// class DecalManager
/// Designed around background objects and static elements.
DecalManager::DecalManager( olc::PixelGameEngine* srpg, float worldRadius) : Manager(srpg,worldRadius){}

DecalManager::~DecalManager(){}

void DecalManager::initalize(){
    srand(time(nullptr));
    int failure = 0;
    makeRender();
    drawing->Update();

    while(failure < 10000){

        olc::vf2d attempting;
        attempting.x = worldRadius*2 * ((float)rand() / (float)RAND_MAX) - worldRadius;
        attempting.y = worldRadius*2 * ((float)rand() / (float)RAND_MAX) - worldRadius;

        //using a poison distribution
        bool valid = true;
        float toxcity = 0.0;
        for(auto const& blade : items){
            float distance = (attempting - blade->getLocal()).mag();
            if(distance < grassSize * 2){
                valid = false;
                break;
            }
            if(distance < 1.0){
                toxcity += distance; //Not 1 - distance because this created a good clumping effect
                if (toxcity > 2.0){
                    valid = false;
                    break;
                }
            }
        }
        if(valid){
            std::shared_ptr<Decoration> temp = std::make_shared<Decoration>(attempting, grassSize);
            temp->setSharedDecal(drawing);
            srpg_data::gameObjects->insertItem(temp);
            items.push_back(std::move(temp));

        } else {
            failure ++;
        }
    }
}

void DecalManager::update(float fElapsedTime){
    for(auto it = items.begin() ; it != items.end() ; it++ ){
        (*it)->update(fElapsedTime);

        if((*it)->getLocal().x > worldRadius)
            (*it)->movement({-worldRadius * 2,0});

        if((*it)->getLocal().x < -worldRadius)
            (*it)->movement({worldRadius * 2,0});

        if((*it)->getLocal().y > worldRadius)
            (*it)->movement({0, -worldRadius * 2});

        if((*it)->getLocal().y < -worldRadius)
            (*it)->movement({0, worldRadius * 2});
    }
}

void DecalManager::eofUpdate(float fElapsedTime, olc::vf2d movement){
    Manager<Decoration>::eofUpdate(fElapsedTime, movement);
}

void DecalManager::makeRender(){


    olc::vi2d area = srpg_data::viewer->ScaleToScreen({grassSize * 2,grassSize * 2});
    image = std::make_shared<olc::Sprite>(area.x+1,area.y+1);

    srpg->SetDrawTarget(image.get());
    srpg->Clear(olc::BLANK);

    srpg->DrawLine( 0,area.y*0.5f,0,area.y,grassColour);
    srpg->DrawLine(area.x *0.5f,0.0f,area.x*0.5f,area.y,grassColour);
    srpg->DrawLine(area.x,area.y*0.5f,area.x,area.y,grassColour);

    srpg->SetDrawTarget(nullptr);

    drawing = std::make_shared<olc::Decal>(image.get());
}
