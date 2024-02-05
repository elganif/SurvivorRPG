

#include "olcPixelGameEngine.h"
#include "olcPGEX_TransformedView.h"
#include "srpg_data.h"
#include "Rectangle.h"
#include "Managers.h"
#include "Entities.h"

/// class Entity
/// Base object of all items in the world.

/// Declaration of static uuid variable. Used in generation of a unique identifier for each entity created.
std::atomic_uint32_t Entity::uuid;

Entity::Entity(olc::vf2d spawn, float newSize):_location(spawn),entSize(newSize),entID(uuid++){}

/// Move Entity to specified postion.
olc::vf2d Entity::location(olc::vf2d destiny)
{
    _location = destiny;
    if(hostTreeNode){
        hostTreeNode->validateEnt(hostTreeNode,myself);
    }
    return _location;
}

/// Moves Entity a specified distance from current location.
void Entity::movement(olc::vf2d destiny)
{
    location(_location + destiny);
}

/// frame update for Entity
//void Entity::update(float fElapsedTime,olc::vf2d worldMove)
//{
//    movement(worldMove);
//}

void Entity::update(float fElapsedTime, olc::vf2d worldMove){
    float friction = 0.75;

    olc::vf2d totalDist = walkingDirection + momentum;
    if(totalDist.mag2() > 1.0f){
     /// Set max speed limit currently half a screen per second.
        totalDist = totalDist.norm();
    }
    momentum -= momentum * fElapsedTime;
    momentum += totalDist * fElapsedTime;
    momentum *= friction;
    movement((totalDist * fElapsedTime) + worldMove);
    walkingDirection = {0,0};
}


/// Used to create entity decal out of sprite passed in
void Entity::setRender(std::shared_ptr<olc::Sprite> sprite){
    image = std::shared_ptr<olc::Decal>(new olc::Decal(sprite.get()));
}

/// Used to directly specify a Decal that may be shared by different entities
void Entity::setSharedDecal(std::shared_ptr<olc::Decal> tDecal){
    image = tDecal;
}

/// returns if entity is needed or can be removed from lists and other systems
bool Entity::isValid(){
    if(whatIsLife()){
        return true;
    } /// Else entity is dead, check if it is still in quadTree and remove if necessary.
    if (hostTreeNode){
        hostTreeNode->removeMe(hostTreeNode,myself);
    }
    return false;

}

bool Entity::operator == (const Entity& other) const{
    return entID == other.entID;
}

bool Entity::operator != (const Entity& other) const{
    return !( *this == other);
}

/// used by QuadTree to give the entity information about where it sits. Used for calling validation on the tree.
void Entity::setTreeLocation(QuadTree* treeNode, std::list<std::shared_ptr<Entity>>::iterator ent){
    hostTreeNode = treeNode;
    myself = ent;
}

// Make Render may be moved to its own class later to import instructions from config files.
void Entity::makeRender(std::shared_ptr<olc::Sprite> sprite, olc::vi2d area, olc::PixelGameEngine* game){
game->SetDrawTarget(sprite.get());
    game->Clear(olc::MAGENTA);
    game->SetDrawTarget(nullptr);
}

/// gets a rectangular collsion box for quick checking if entities are close enough to interact
Rectangle Entity::getBoxCollider(){
    return Rectangle({_location.x - (entSize), _location.y - (entSize)},{entSize*2.0f,entSize*2.0f});
}

/// rotates a point around center of the entity
olc::vf2d Entity::rotatePt(olc::vf2d point,olc::vf2d angle){
    olc::vf2d updatedpoint = point - _location;

    updatedpoint.x = (point.y * angle.x) + (point.x * angle.y);
    updatedpoint.y = -(point.x * angle.x) + (point.y * angle.y);
    updatedpoint += _location;
    return updatedpoint;
}



/// class Npc : public Entity

Npc::Npc( olc::vf2d spawn, float newSize):Entity(spawn,newSize){
    speed = 0.15f;
    fColour = olc::BLACK;
    lineColour = olc::DARK_RED;
}

Npc::~Npc(){};

Entity::TYPE Npc::whoAreYou(){ return NPC;}

float Npc::getHP(){
    return HP;
}

void Npc::update(float fElapsedTime,olc::vf2d worldMove){
    if (isValid()) {
        walkingDirection = -location().norm() * speed;
    }
    float friction = 0.75;

    olc::vf2d totalDist = walkingDirection + momentum;
    if(totalDist.mag2() > 1.0f){
     //Set max speed limit currently half a screen per second.
        totalDist = totalDist.norm();
    }
    momentum -= momentum * fElapsedTime;
    momentum += totalDist * fElapsedTime;
    momentum *= friction;
    movement((totalDist * fElapsedTime) + worldMove);
    walkingDirection = {0,0};

}

bool Npc::whatIsLife(){
    return HP > 0.0f;
}

void Npc::onOverlap(std::shared_ptr<Entity> other){
    if(*this == *other)
        return;
    if(other->whoAreYou() == HERO){
        //Collide with Hero
        std::dynamic_pointer_cast<Hero>(other)->bump(location(),entSize);
        return;
    }
    if(other->whoAreYou() == NPC){
        //Collide with Friendly
        std::dynamic_pointer_cast<Npc>(other)->bump(location(),entSize);
        return;
    }
    if(other->whoAreYou() == PROJECTILE){
        // subtract projectiles damage from hp
        HP = HP - std::dynamic_pointer_cast<Projectile>(other)->impact(this);
        return;
    }
}

void Npc::bump(olc::vf2d otherLoc,float otherSize){
    //srpg_data::timers->start("bumpin");
    olc::vf2d entRelLoc = (location() - otherLoc); // coordinate Dist
    float collideDist = (otherSize + entSize);

    if(collideDist * collideDist > entRelLoc.mag2()){
        float overLap = collideDist - entRelLoc.mag();

        float force = ((overLap * overLap) / entSize)*10;
        momentum += entRelLoc.norm()*force;

    }
    //srpg_data::timers->stop("bumpin");
}

void Npc::render(){
    srpg_data::viewer->DrawDecal(getBoxCollider().tl + olc::vf2d(0.0,-0.2)*entSize,image.get());

    // debug collider
    if(srpg_data::debugTools){
        srpg_data::viewer->DrawCircle(location(),entSize,olc::VERY_DARK_RED);
    }
}

bool Npc::operator < (const Npc& other) const
{
    return (HP < other.HP );
}

/// class Hero : public Entity

Hero::Hero( olc::vf2d spawn, float newSize,olc::PixelGameEngine* game, float world):Npc(spawn,newSize){
    speed = 0.5f;
    fColour = olc::BLACK;
    lineColour = olc::BLUE;
    projectileCooldown = 0;
    bulletMan = std::make_unique<ProjectileManager>(game,world);
    bulletMan->setProjectileStats(bulletLife,bulletSpeed,1);
    bulletMan->makeRender(pSize);

}
Hero::~Hero(){};

Entity::TYPE Hero::whoAreYou(){return HERO;}

void Hero::update(float fElapsedTime,olc::vf2d worldMove, srpg_data::controls& inputs){
    bulletMan->update(fElapsedTime,worldMove);

    walkingDirection = inputs.movement;

    projectileCooldown += fElapsedTime;

    if(projectileCooldown >= fElapsedTime){
        projectileCooldown = fElapsedTime;
    }
    while( projectileCooldown >= 0){
        projectileCooldown -= 1/fireRate;
        std::list<std::shared_ptr<Entity>> targets;
        srpg_data::gameObjects->getFoes(location(),bulletLife*bulletSpeed,fireCount, targets, QuadTree::CLOSE);

        for(auto ent = targets.begin(); ent != targets.end(); ent++){
            fireProjectile((*ent)->location());
        }
    }
    movement(walkingDirection * fElapsedTime);
}

bool Hero::fireProjectile(const olc::vf2d& target){

        bulletMan->spawn(location(),target,pSize);

        return true;
}

bool Hero::projectileReady(){
    return projectileCooldown >= 0;
}

void Hero::onOverlap(std::shared_ptr<Entity> other){
    if(*this == *other)
        return;
    if(other->whoAreYou() == HERO){
        //Collide with Hero
        std::dynamic_pointer_cast<Hero>(other)->bump(location(),entSize);
    }
    if(other->whoAreYou() == NPC){
        //Collide with Friendly
        std::dynamic_pointer_cast<Npc>(other)->bump(location(),entSize);
    }
    if(other->whoAreYou() == PROJECTILE){
        // subtract projectiles damage from hp
        //HP = HP - std::dynamic_pointer_cast<Projectile>(other)->impact(this);
    }
}



void Hero::render(){
    srpg_data::viewer->DrawDecal(getBoxCollider().tl + olc::vf2d(0.0,-0.2)*entSize,image.get());

    srpg_data::viewer->FillRect(getBoxCollider().tl.x,getBoxCollider().tl.y + getBoxCollider().sides.y,
                                getBoxCollider().sides.x*(HP/MaxHP),0.2f*entSize,olc::GREEN);

    srpg_data::viewer->DrawRect(getBoxCollider().tl.x,getBoxCollider().tl.y + getBoxCollider().sides.y,
                                getBoxCollider().sides.x,0.2f*entSize,olc::DARK_GREY);

    //collider Debug
    if(srpg_data::debugTools){
        srpg_data::viewer->DrawCircle(location(),entSize,olc::VERY_DARK_BLUE);
    }
    //srpg_data::viewer->DrawRect(getBoxCollider().tl,getBoxCollider().sides);
}

void Hero::makeRender(std::shared_ptr<olc::Sprite> sprite,olc::vf2d area,olc::PixelGameEngine* game){

    game->SetDrawTarget(sprite.get());
    game->Clear(olc::BLANK);

    olc::Pixel colours = olc::BLUE;
    game->FillCircle(area.x * 0.50 ,area.y * 0.12 , area.x * 0.12 ,fColour);// Head
    game->DrawCircle(area.x * 0.50 ,area.y * 0.12 , area.x * 0.12,lineColour );


    game->FillRect( area.x * 0.25 ,area.y *0.25 , area.x *0.10 , area.y *0.30 ,fColour);// Left Arm
    game->DrawRect( area.x * 0.25 ,area.y *0.25 , area.x *0.10 , area.y *0.30 ,lineColour);

    game->FillRect(  area.x *0.65 , area.y *0.25 , area.x *0.10 , area.y *0.30 ,fColour);// Right Arm
    game->DrawRect(  area.x *0.65 , area.y *0.25 , area.x *0.10 , area.y *0.30 ,lineColour);

    game->FillRect( area.x *0.35 ,area.y *0.25 , area.x *0.30 , area.y *0.35 ,fColour); //torso
    game->DrawRect( area.x *0.35 ,area.y *0.25 , area.x *0.30 , area.y *0.35 ,lineColour);

    game->FillRect( area.x *0.38 , area.y *0.60 , area.x *0.12 , area.y *0.40 ,fColour);// Left Leg
    game->DrawRect( area.x *0.38 , area.y *0.60 , area.x *0.12 , area.y *0.40 ,lineColour);

    game->FillRect( area.x *0.50 , area.y *0.60 , area.x *0.12 , area.y *0.40 ,fColour);// Right Leg
    game->DrawRect( area.x *0.50 , area.y *0.60 , area.x *0.12 , area.y *0.40 ,lineColour);


    game->SetDrawTarget(nullptr);
}

/// class Projectile : public Entity
Projectile::Projectile( olc::vf2d spawn, olc::vf2d projSize, float duration,
                        olc::vf2d orientation,int hitCount, olc::PixelGameEngine* game)
                    :Entity(spawn,projSize.x),shape(projSize.y),lifespan(duration),direction(orientation),hits(hitCount)
{
    lineColour = olc::DARK_RED;
    fColour = olc::DARK_GREY;
}

Projectile::~Projectile(){}

void Projectile::update(float fElapsedTime,olc::vf2d worldMove)
{
    if (isValid()) {
        movement(direction * fElapsedTime + worldMove);
    }

}

bool Projectile::whatIsLife()
{
    return (lifespan > 0.0f && hits > 0);
}

/// Sets values used to determine is Alive to 0
void Projectile::kill()
{
    lifespan = 0.0f;
    hits = 0;
}

/// What does a projectile do when it hits another entity
float Projectile::impact(Entity* other)
{
    if(isValid() && (location() - other->location()).mag2() <= other->getSize() * other->getSize()){
        hits--;
        return 10.0f;// Damage for now, eventualy should be struct of effects?
    }
    return 0.0f;
}



float Projectile::getLife()
{
    return lifespan;
}

void Projectile::render()
{

    float rad = atan2(direction.x, -direction.y);

    olc::vf2d proLoc = rotatePt(olc::vf2d( entSize*0.5, entSize*0.5),direction.norm());

    srpg_data::viewer->DrawRotatedDecal(location(),image.get(),rad,{entSize/2.0f,shape/2.0f});
}


/// class Decoration : public Entity
Decoration::Decoration( olc::vf2d spawn, float newSize):Entity(spawn,newSize)
{
    lineColour = olc::DARK_GREEN;
}
Decoration::~Decoration(){}

bool Decoration::whatIsLife()
{return true;}

Entity::TYPE Decoration::whoAreYou()
{ return DECORATION;}

void Decoration::render()
{
    srpg_data::viewer->DrawDecal(getBoxCollider().tl,image.get());
}



