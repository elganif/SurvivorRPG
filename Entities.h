#ifndef ENTITIES_H_INCLUDED
#define ENTITIES_H_INCLUDED
namespace srpg_data{ struct controls; };
class Projectile;
class ProjectileManager;
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
    /// Momentum and forces variables
    olc::vf2d walkingDirection;
    olc::vf2d momentum = {0,0};

    /// uuid and entID are for giving every entity created a unique identifier to eleminate duplicate or repeated checks.
    /// Useful in avoiding colide with self checks and for tracking recent interactions for hit cooldowns
    /// ie. to prevent projectile hiting on every frame
    static std::atomic_uint32_t uuid;
    const uint32_t entID;

    std::shared_ptr<olc::Decal> image = nullptr;
    float entSize;
    float speed = 0.0;

    /// Bright default colours to make undefined colouring show up
    olc::Pixel fColour = olc::DARK_MAGENTA;
    olc::Pixel lineColour = olc::Pixel(255,0,127);


    public:
    /// Enum for type checking at runtime when Needed
    enum TYPE {
        HERO,
        FOE,
        PROJECTILE,
        DECORATION
    };

    Entity(olc::vf2d spawn, float newSize);

    virtual ~Entity(){image.reset();}

    ///pure virtual functions that all derived must implement
    virtual void render() = 0;
    virtual TYPE whoAreYou() = 0;
    virtual void update(float fElapsedTime)=0;

    virtual void eofUpdate(float fElapsedTime, olc::vf2d worldMove);

    bool isValid();
    bool operator == (const Entity& other) const;
    bool operator != (const Entity& other) const;

    void setTreeLocation(QuadTree* hostTreeNode, std::list<std::shared_ptr<Entity>>::iterator ent);


    void placement(olc::vf2d destiny);
    void movement(olc::vf2d destiny);
    olc::vf2d getLocal(){return location;}

    int getUID(){return entID;}
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

        std::unique_ptr<ProjectileManager> bulletMan;
        float fireRate = 2; // Projectiles per second
        float fireCount = 5;
        float projectileCooldown = 0;
        olc::vf2d pSize = {0.025f,0.05f};


        bool whatIsLife();

    public:
        Hero( olc::vf2d spawn, float newSize,olc::PixelGameEngine* game, float world);
        ~Hero();
        TYPE whoAreYou();
        void update(float fElapsedTime);
        void update(float fElapsedTime, srpg_data::controls& inputs);
        void eofUpdate(float fElapsedTime, olc::vf2d worldMove);
        float getHP();


        bool fireProjectile(const olc::vf2d& target);
        bool projectileReady();
        void onOverlap(std::shared_ptr<Entity> other);
        void bump(olc::vf2d otherLoc,float otherSize);

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
        void update(float fElapsedTime);
        TYPE whoAreYou();
        void onOverlap(std::shared_ptr<Entity> other);
        void bump(olc::vf2d otherLoc,float otherSize);
        float getHP();
        void render();
        static void makeRender(olc::Sprite* sprite, olc::vf2d area, olc::PixelGameEngine* game);
        // make Render should be moved out to its own module that will contain many routines for many different things

    bool operator < (const Foe& s) const;
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

        Projectile( olc::vf2d spawn, olc::vf2d projSize, float duration,olc::vf2d orientation,int hitCount, olc::PixelGameEngine* game);
        ~Projectile();

        TYPE whoAreYou(){ return PROJECTILE;}

        void update(float fElapsedTime);

        float impact(Entity* other);
        void kill();
        float getLife();

        void render();
        //void makeRender(std::shared_ptr<olc::Sprite> sprite, olc::vf2d area, olc::PixelGameEngine* game);

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

        void update(float fElapsedTime);
};



#endif // ENTITIES_H_INCLUDED
