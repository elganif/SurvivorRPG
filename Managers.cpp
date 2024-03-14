

#include "olcPixelGameEngine.h"
#include "olcPGEX_TransformedView.h"
#include "srpg_data.h"
#include "Rectangle.h"
#include "Entities.h"
#include "Managers.h"
#include "Menus.h"



/// class Manager
template <class E>
Manager<E>::Manager(olc::PixelGameEngine* game, float world) : pge(game),worldRadius(world) {}

template <class E>
Manager<E>::~Manager()
{
    drawing.reset();
    image.reset();
}

template <class E>
int Manager<E>::size(){
    return items.size();
}

template <class E>
int Manager<E>::update(float fElapsedTime,olc::vf2d worldMove){
    int removed = 0;
    items.remove_if([](auto &value) {return !value->isValid();} );
    for( auto& ent : items)
    {
        ent->update(fElapsedTime,worldMove);
    }
    return removed;
}



/// Foe Manager is designed around enemies and will maintain their numbers, stats growth and overall difficulty
/// class FoeManager

FoeManager::FoeManager(olc::PixelGameEngine* game, float world):Manager(game, world){};

FoeManager::~FoeManager(){}


void FoeManager::initalize(int numFoes)
{
    maxPop = numFoes;

    /// Set up the sprite & decal for foes
    makeRender();

    for(int i = 0; i< maxPop;i++){
        spawn(foeSize);
    }
}

/// Code for creating a new unit
void FoeManager::spawn(float foeSize)
{
    olc::vf2d attempt;
    float spawnRad = worldRadius;
    /// Get a random point, equal distributed around center by disreguarding corners
    do{
    attempt.x = (float)rand() / (float)RAND_MAX - 0.5f; // gives range between -0.5 and +0.5
    attempt.y = (float)rand() / (float)RAND_MAX - 0.5f; // this saves a divide by 2 calculation later
    } while(attempt.mag2() > (0.5f * 0.5f) && attempt.mag2() != 0.0f);

    /// establish spawn distance
    attempt *= worldRadius; // attempt is already at radius between 0 and .5 units so this is goes half way to edge of world
    attempt += attempt.norm() * 2; // move away from center 2 units to prevent visible spawning on screen

    /// Add spawned unit to the list and the quad tree
    std::shared_ptr<Npc> theEvil = std::make_shared<Npc>(attempt,foeSize);
    theEvil->setSharedDecal(drawing);
    srpg::gameObjects->insertItem(theEvil);
    items.push_back(std::move(theEvil));
}

void FoeManager::makeRender()
{
    /// prepare the sprite object for drawing
    olc::vi2d area = srpg::viewer->ScaleToScreen({foeSize*2,foeSize*2} ) ;
    image = std::make_shared<olc::Sprite>(area.x+1,area.y+1);

    pge->SetDrawTarget(image.get());
    pge->Clear(olc::BLANK);

    olc::Pixel colours = olc::DARK_RED;
    pge->FillCircle(area.x * 0.50 ,area.y * 0.12 , area.x * 0.12 ,olc::BLACK);// Head
    pge->DrawCircle(area.x * 0.50 ,area.y * 0.12 , area.x * 0.12,colours );


    pge->FillRect( area.x * 0.25 ,area.y *0.25 , area.x *0.10 , area.y *0.30 ,olc::BLACK);// Left Arm
    pge->DrawRect( area.x * 0.25 ,area.y *0.25 , area.x *0.10 , area.y *0.30 ,colours);

    pge->FillRect(  area.x *0.65 , area.y *0.25 , area.x *0.10 , area.y *0.30 ,olc::BLACK);// Right Arm
    pge->DrawRect(  area.x *0.65 , area.y *0.25 , area.x *0.10 , area.y *0.30 ,colours);

    pge->FillRect( area.x *0.35 ,area.y *0.25 , area.x *0.30 , area.y *0.35 ,olc::BLACK); //torso
    pge->DrawRect( area.x *0.35 ,area.y *0.25 , area.x *0.30 , area.y *0.35 ,colours);

    pge->FillRect( area.x *0.38 , area.y *0.60 , area.x *0.12 , area.y *0.40 ,olc::BLACK);// Left Leg
    pge->DrawRect( area.x *0.38 , area.y *0.60 , area.x *0.12 , area.y *0.40 ,colours);

    pge->FillRect( area.x *0.50 , area.y *0.60 , area.x *0.12 , area.y *0.40 ,olc::BLACK);// Right Leg
    pge->DrawRect( area.x *0.50 , area.y *0.60 , area.x *0.12 , area.y *0.40 ,colours);

    /// Set decal and return draw target to default
    drawing = std::make_shared<olc::Decal>(image.get());
    pge->SetDrawTarget(nullptr);
}

void FoeManager::collision()
{

    for(std::shared_ptr<Npc> foe : items){
        std::list<std::shared_ptr<Entity>> impacts;
        srpg::gameObjects->getOverlapItems(foe->getBoxCollider(),impacts);
        for(std::shared_ptr<Entity> other : impacts){
            foe->onOverlap(other);
        }
    }
}

void FoeManager::update(float fElapsedTime,olc::vf2d worldMove)
{

    for(auto foe = items.begin() ; foe != items.end();){ /// iteration inside loop due to erase call affecting position.
        if( !(*foe)->isValid()){
            deadFoes++;
            items.erase(foe++);
            continue;
        }
        /// if foes are too far from player teleport them to opposite side
        if((*foe)->location().mag2() > worldRadius * worldRadius){
            (*foe)->movement((*foe)->location().norm() * worldRadius * -2);
        }
        (*foe)->update(fElapsedTime,worldMove);
        foe++; /// iteration
    }

    /// On each frame that enemies are below desired population add one more. Collision solves overlaps next frame
    if(items.size() < maxPop){
        spawn(foeSize);
    }
}

int FoeManager::getKills()
{
    return deadFoes;
}



/// class ProjectileManager
ProjectileManager::ProjectileManager(olc::PixelGameEngine* game, float world) : Manager(game,world){};

ProjectileManager::~ProjectileManager(){}

void ProjectileManager::setProjectileStats(float lifeTime, float travelSpeed, int numHits){
    life = lifeTime;
    speed = travelSpeed;
    hits = numHits;
}

void ProjectileManager::spawn(olc::vf2d origin, olc::vf2d target, olc::vf2d bulletSize){
    olc::vf2d momentum = target.norm() * speed;
    std::shared_ptr<Projectile> temp = std::make_shared<Projectile>(origin,bulletSize,life,momentum,hits,pge);
    temp->setSharedDecal(drawing);
    items.push_front(temp);
    srpg::gameObjects->insertItem(temp);
}

void ProjectileManager::update(float fElapsedTime,olc::vf2d worldMove){
    Manager<Projectile>::update(fElapsedTime,worldMove);
}

void ProjectileManager::makeRender(olc::vf2d pSize){
    /// prepare the sprite object for drawing
    olc::vi2d area = srpg::viewer->ScaleToScreen({pSize.x,pSize.y}) ;
    image = std::make_shared<olc::Sprite>(area.x+1,area.y+1);

    pge->SetDrawTarget(image.get());
    pge->Clear(olc::BLANK);

    // Set up Tips of Triangle
    olc::vi2d tip =   {area.x / 2,0};
    olc::vi2d left =  {0,area.y};
    olc::vi2d right = {area.x,area.y};

    olc::Pixel fColour = olc::Pixel(0,0,0);
    olc::Pixel lineColour = olc::Pixel(128,0,63); /// temp until I figure out where these should live and how they get here
    pge->FillTriangle(tip,left,right,fColour);
    pge->DrawTriangle(tip,left,right,lineColour);

    /// Set decal and return draw target to default
    drawing = std::make_shared<olc::Decal>(image.get());
    pge->SetDrawTarget(nullptr);
}


/// class DecalManager
/// Designed around background objects and static elements.
DecalManager::DecalManager( olc::PixelGameEngine* game, float world) : Manager(game,world){}

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
            float distance = (attempting - blade->location()).mag();
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
            srpg::gameObjects->insertItem(temp);
            items.push_back(std::move(temp));

        } else {
            failure ++;
        }
    }
}

void DecalManager::update(float fElapsedTime, olc::vf2d worldMove){
    for(auto it : items ){
        it->update(fElapsedTime,worldMove);

        if(it->location().x > worldRadius)
            it->movement({-worldRadius * 2,0});

        if(it->location().x < -worldRadius)
            it->movement({worldRadius * 2,0});

        if(it->location().y > worldRadius)
            it->movement({0, -worldRadius * 2});

        if(it->location().y < -worldRadius)
            it->movement({0, worldRadius * 2});
    }
}

void DecalManager::makeRender(){

    olc::vi2d area = srpg::viewer->ScaleToScreen({grassSize * 2,grassSize * 2});
    image = std::make_shared<olc::Sprite>(area.x+1,area.y+1);

    pge->SetDrawTarget(image.get());
    pge->Clear(olc::BLANK);

    pge->DrawLine( 0,area.y*0.5f,0,area.y,grassColour);
    pge->DrawLine(area.x *0.5f,0.0f,area.x*0.5f,area.y,grassColour);
    pge->DrawLine(area.x,area.y*0.5f,area.x,area.y,grassColour);

    pge->SetDrawTarget(nullptr);

    drawing = std::make_shared<olc::Decal>(image.get());
    pge->SetDrawTarget(nullptr);
}
