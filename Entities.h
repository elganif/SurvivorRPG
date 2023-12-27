#ifndef ENTITIES_H_INCLUDED
#define ENTITIES_H_INCLUDED
class Projectile;
class QuadTree;

class Entity
    {
private:
    /// Private variables require derived classes to use the accessor methods that ensure quadtree updates
    olc::vf2d location;
    /// for function interactions with QuadTree used in collision detection and rendering.
    QuadTree* hostTreeNode = nullptr;
    std::list<std::shared_ptr<Entity>>::iterator myself;

    /// Only to be used by isValid() function
    virtual bool whatIsLife() = 0;


protected:
    /// uuid and entID are for giving every entity created a unique identifier to eleminate duplicate or repeated checks.
    /// Useful in avoiding colide with self checks and for tracking recent interactions for hit cooldowns
    /// ie. to prevent projectile hiting on every frame
    static int uuid;
    const int entID;


    std::shared_ptr<olc::Decal> image = nullptr;
    float entSize;
    float speed = 0.0;



    /// Bright default colours to make undefined colouring show up
    olc::Pixel fColour = olc::DARK_MAGENTA;
    olc::Pixel lineColour = olc::Pixel(255,0,127);


    /// Enum for type checking at runtime when Needed
    enum TYPE {
        HERO,
        FOE,
        PROJECTILE,
        DECORATION
    };



    public:
    Entity(olc::vf2d spawn, float newSize);

    virtual ~Entity(){image.reset();}

    ///pure virtual functions that all derived must implement
    virtual void render() = 0;
    virtual void update(float fElapsedTime, olc::vf2d worldMove)=0;
    virtual TYPE whoAreYou() = 0;

    bool isValid();
    bool operator == (const Entity& other) const;
    bool operator != (const Entity& other) const;

        void setTreeLocation(QuadTree* hostTreeNode, std::list<std::shared_ptr<Entity>>::iterator ent);

        int getUID(){return entID;}

        void placement(olc::vf2d destiny);
        void movement(olc::vf2d destiny);
        olc::vf2d getLocal(){return location;}

        float getSize(){return entSize;}
        float getSpeed(){return speed;}

        void setRender(std::shared_ptr<olc::Sprite> sprite);
        void setSharedDecal(std::shared_ptr<olc::Decal> tDecal);

        static void makeRender(std::shared_ptr<olc::Sprite> sprite, olc::vi2d area, olc::PixelGameEngine* game);

        Rectangle getBoxCollider();

        // rotates a point around center
        olc::vf2d rotatePt(olc::vf2d point,olc::vf2d angle);

    };

class Hero : public Entity
    {
    private:
        float MaxHP = 100.0f;
        float HP = 100.0f;
        float fireRate = 100; // Projectiles per second
        float projectileCooldown = 0;


        bool whatIsLife();

    public:
        Hero( olc::vf2d spawn, float newSize);
        ~Hero();
        TYPE whoAreYou();
        void update(float fElapsedTime, olc::vf2d worldMove);
        float getHP();


        bool fireProjectile(olc::vf2d& target, std::list<std::shared_ptr<Projectile>>& bullets, olc::PixelGameEngine* game);
        bool projectileReady();
        olc::vf2d bump(olc::vf2d otherLoc,float otherSize);

        void setImage(std::shared_ptr<olc::Decal> heroicImage) {image = heroicImage; };
        void render();
        void makeRender(std::shared_ptr<olc::Sprite> sprite, olc::vf2d area, olc::PixelGameEngine* game);

    };

class Foe : public Entity
    {
    private:
        float maxHP = 100.0f;
        float HP = 100.0f;

        bool whatIsLife();

    public:
        Foe( olc::vf2d spawn, float newSize);
        ~Foe();
        void update(float fElapsedTime, olc::vf2d worldMove);
        TYPE whoAreYou();
        void onOverlap(std::shared_ptr<Entity> other);
        olc::vf2d bump(olc::vf2d otherLoc,float otherSize);
        float getHP();
        void render();
        static void makeRender(olc::Sprite* sprite, olc::vf2d area, olc::PixelGameEngine* game);
        // make Render should be moved out to its own module that will contain many routines for many different things

    bool operator < (const Foe& s);
    };

class Projectile : public Entity
    {
    private:
        olc::vf2d direction;
        float shape;
        float lifespan;
        int hits;
        std::shared_ptr<olc::Sprite> sprite;

        bool whatIsLife();

    public:

        Projectile( olc::vf2d spawn, float newSize, float width, float duration,olc::vf2d orientation,int hitCount, olc::PixelGameEngine* game);
        ~Projectile();

        TYPE whoAreYou(){ return PROJECTILE;}

        void update(float fElapsedTime, olc::vf2d worldMove);


        float impact(Entity* other);
        void kill();
        float getLife();

        void render();
        void makeRender(std::shared_ptr<olc::Sprite> sprite, olc::vf2d area, olc::PixelGameEngine* game);

};


class Decoration : public Entity
{
    private:
        bool whatIsLife();

    public:
        Decoration( olc::vf2d spawn, float newSize);
        ~Decoration();
        TYPE whoAreYou();
        void render();

        void update(float fElapsedTime, olc::vf2d worldMove);
};



#endif // ENTITIES_H_INCLUDED
