

#include "olcPixelGameEngine.h"
#include "olcPGEX_TransformedView.h"
#include "Rectangle.h"
#include "srpg_data.h"
#include "Entities.h"

/// class Entity
 void Entity::setRender(olc::Sprite* sprite){
    image = new olc::Decal(sprite);
}

// Make Render may be moved to its own function to import instructions from config files.
void Entity::makeRender(olc::Sprite* sprite, olc::vi2d area, olc::PixelGameEngine* game){
game->SetDrawTarget(sprite);
    game->Clear(olc::MAGENTA);
    game->SetDrawTarget(nullptr);
}

Rectangle Entity::getBoxCollider(){
    return Rectangle({location.x - (entSize),location.y - (entSize)},{entSize*2.0f,entSize*2.0f});
}

// rotates a point around center
olc::vf2d Entity::rotatePt(olc::vf2d point,olc::vf2d angle){
    olc::vf2d updatedpoint = point - location;

    updatedpoint.x = (point.y * angle.x) + (point.x * angle.y);
    updatedpoint.y = -(point.x * angle.x) + (point.y * angle.y);
    updatedpoint += location;
    return updatedpoint;
}

/// class Hero : public Entity

Hero::Hero( olc::vf2d spawn, float newSize):Entity(spawn,newSize){

    fColour = olc::BLACK;
    lineColour = olc::BLUE;

}
Hero::~Hero(){};

Entity::TYPE Hero::whoAreYou(){return HERO;}

bool Hero::isAlive(){return HP > 0.0f;}

float Hero::getHP(){
    return HP;
}

void Hero::update(float fElapsedTime, olc::vf2d worldMove){
    projectileCooldown = projectileCooldown >= 0 ? fElapsedTime :  projectileCooldown + fElapsedTime;
}

bool Hero::fireProjectile(olc::vf2d& target, std::list<std::shared_ptr<Projectile>>& bullets, olc::PixelGameEngine* game){
    if(projectileReady()){
        std::shared_ptr<Projectile> temp = std::make_shared<Projectile>(location,0.05f,0.025f,((float)rand() / (float)RAND_MAX) * 100.0f,target,1,game);
        projectileCooldown -= 1.0/fireRate;
        bullets.push_back(temp);
        srpg_data::gameObjects->insertItem(temp);

        return true;
    }
    return false;

}

bool Hero::projectileReady(){
    return projectileCooldown >= 0;
}

olc::vf2d Hero::bump(olc::vf2d otherLoc,float otherSize){

    if(otherLoc == location){
        return {0,0};//assume same entity and skip collision
    }
    float entDist2 = (otherLoc - location).mag(); // magnitude of distance
    float colideDist2 = (otherSize + entSize); // also squared to skip squareroots
    if(colideDist2 > entDist2){
        float overLap = colideDist2 - entDist2;
        olc::vf2d bumpDirection = (location - otherLoc).norm();
        olc::vf2d thisMove = bumpDirection*overLap*0.5;
        HP = HP - ( 1.0f/60.0f ); // VERY BAD HACK: TODO Do better - for now remove hp equivelent to time tick
        movement(thisMove*0.8);
        return (-thisMove*1.2);

    } // else not collideing, return 0
    return{0,0};
}


void Hero::render(){
    srpg_data::viewer->DrawDecal(getBoxCollider().tl + olc::vf2d(0.0,-0.2)*entSize,image);

    srpg_data::viewer->FillRect(getBoxCollider().tl.x,getBoxCollider().tl.y + getBoxCollider().sides.y,
    getBoxCollider().sides.x*(HP/MaxHP),0.2f*entSize,olc::GREEN);
    srpg_data::viewer->DrawRect(getBoxCollider().tl.x,getBoxCollider().tl.y + getBoxCollider().sides.y,
    getBoxCollider().sides.x,0.2f*entSize,olc::DARK_GREY);

    //collider Debug
    srpg_data::viewer->DrawCircle(location,entSize,olc::VERY_DARK_BLUE);
    //srpg_data::viewer->DrawRect(getBoxCollider().tl,getBoxCollider().sides);
}

void Hero::makeRender(olc::Sprite* sprite,olc::vf2d area,olc::PixelGameEngine* game){

    game->SetDrawTarget(sprite);
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



/// class Foe : public Entity

Foe::Foe( olc::vf2d spawn, float newSize):Entity(spawn,newSize){
    speed = 0.2f;
    fColour = olc::BLACK;
    lineColour = olc::DARK_RED;
}

Foe::~Foe(){};

Entity::TYPE Foe::whoAreYou(){ return FOE;}

void Foe::update(float fElapsedTime, olc::vf2d worldMove){
    if (isAlive()) {
        location += (-location.norm() * speed * fElapsedTime);


    }
    location += worldMove;
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

/// class Projectile : public Entity
Projectile::Projectile( olc::vf2d spawn, float newSize, float width, float duration,
                        olc::vf2d orientation,int hitCount, olc::PixelGameEngine* game)
                    :Entity(spawn,newSize),shape(width),lifespan(duration),direction(orientation),hits(hitCount)
{
        lineColour = olc::DARK_RED;
        fColour = olc::DARK_GREY;

        olc::vi2d Size = srpg_data::viewer->ScaleToScreen({shape,entSize});
        sprite = new olc::Sprite(Size.x+1,Size.y+1);
        Projectile::makeRender(sprite,Size,game);
        setRender(sprite);
}

Projectile::~Projectile(){}//delete sprite;};

void Projectile::update(float fElapsedTime, olc::vf2d worldMove){
    if (isAlive()) {
        location = location + (direction * fElapsedTime) + worldMove;
        lifespan -= fElapsedTime;

    }

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

    float rad = atan2(direction.x, -direction.y);

    olc::vf2d proLoc = rotatePt(olc::vf2d( shape*0.5, entSize*0.5),direction.norm());

    srpg_data::viewer->DrawRotatedDecal(proLoc,image,rad);


}

void Projectile::makeRender(olc::Sprite* tSprite,olc::vf2d area,olc::PixelGameEngine* game){
    game->SetDrawTarget(tSprite);
    game->Clear(olc::BLANK);


    // Set up Tips of Triangle
    olc::vf2d tip =   {area.x * 0.5f,0.0f};
    olc::vf2d left =  {0.0,area.y};
    olc::vf2d right = {area.x,area.y};
    game->FillTriangle(tip,left,right,fColour);
    game->DrawTriangle(tip,left,right,lineColour );

    game->SetDrawTarget(nullptr);
}





/// class Decoration : public Entity
Decoration::Decoration::Decoration( olc::vf2d spawn, float newSize):Entity(spawn,newSize){

    lineColour = olc::DARK_GREEN;
}
Decoration::~Decoration(){};
Entity::TYPE Decoration::whoAreYou(){ return DECORATION;}

void Decoration::render(){
    srpg_data::viewer->DrawDecal(getBoxCollider().tl,image);

}



