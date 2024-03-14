#ifndef ENTITIES_H_INCLUDED
#define ENTITIES_H_INCLUDED

/// Forward Declarations for passing through functions.
namespace srpg{ struct controls; };
class Projectile;
class ProjectileManager;
class QuadTree;
struct Rectangle;



class Entity
{
private:
    /// Private variables require derived classes to use the accessor methods that ensure quadtree updates are called.
    olc::vf2d _location;

    /// for function interactions with QuadTree used in collision detection and rendering.
    QuadTree* hostTreeNode = nullptr;

protected:

    /// uuid and entID are for giving every entity created a unique identifier to eleminate duplicate or repeated checks.
    /// Useful in avoiding colide with self checks and for tracking recent interactions for hit cooldowns
    /// ie. to prevent projectile re-hitting the same entity.
    static std::atomic_uint32_t uuid;
    const uint32_t entID;

    /// Momentum and forces variables
    olc::vf2d walkingDirection;
    olc::vf2d momentum = {0,0};
    float speed = 0.0;
    float entSize;


    /// Bright default colours to make undefined colouring show up
    std::shared_ptr<olc::Decal> image = nullptr;
    olc::Pixel fColour = olc::DARK_MAGENTA;
    olc::Pixel lineColour = olc::Pixel(255,0,127);

    virtual bool whatIsLife() = 0;

public:
    /// Enum for type checking at runtime when Needed
    enum TYPE {
        HERO,
        NPC,
        PROJECTILE,
        DECORATION
    };

    Entity(olc::vf2d spawn, float newSize);

    virtual ~Entity(){image.reset();}

    ///pure virtual functions that all derived must implement
    virtual void render() = 0;
    virtual TYPE whoAreYou() = 0;

    bool isValid();

    bool operator == (const Entity& other) const;
    bool operator != (const Entity& other) const;

    virtual void setTreeLocation(QuadTree* hostTreeNode)final;

    virtual olc::vf2d location(olc::vf2d destiny) final;
    virtual olc::vf2d location() final {return _location;}
    void movement(olc::vf2d destiny);
    virtual void update(float fElapsedTime,olc::vf2d worldMove);

    int getUID(){return entID;}
    float getSize(){return entSize;}
    float getSpeed(){return speed;}

    void setRender(std::shared_ptr<olc::Sprite> sprite);
    void setSharedDecal(std::shared_ptr<olc::Decal> tDecal);

    Rectangle getBoxCollider();

    // rotates a point around center
    olc::vf2d rotatePt(olc::vf2d point,olc::vf2d angle);

};


class Npc : public Entity
    {
    protected:
        float maxHP = 100.0f;
        float HP = 100.0f;
        bool whatIsLife();

    public:
        Npc( olc::vf2d spawn, float newSize);
        ~Npc();
        void update(float fElapsedTime,olc::vf2d worldMove);
        TYPE whoAreYou();
        void onOverlap(std::shared_ptr<Entity> other);
        void bump(olc::vf2d otherLoc,float otherSize);
        float getHP();
        void render();

    bool operator < (const Npc& s) const;
    };

class Hero : public Npc
{
private:

    std::unique_ptr<ProjectileManager> bulletMan;
    float fireRate = 3; // Projectiles per second
    float fireCount = 5;
    float bulletSpeed = 0.5f;
    float bulletLife = 3.0f;
    float projectileCooldown = 0;
    olc::vf2d pSize = {0.025f,0.05f};

public:
    Hero( olc::vf2d spawn, float newSize,olc::PixelGameEngine* game, float world);
    ~Hero();
    TYPE whoAreYou();
    void update(float fElapsedTime);
    void update(float fElapsedTime, olc::vf2d worldMove, srpg::controls& inputs);

    bool fireProjectile(const olc::vf2d& target);
    bool projectileReady();
    void onOverlap(std::shared_ptr<Entity> other);

    void setImage(std::shared_ptr<olc::Decal> heroicImage) {image = heroicImage; };
    void render();
    void makeRender(std::shared_ptr<olc::Sprite> sprite, olc::vf2d area, olc::PixelGameEngine* game);

};

class Projectile : public Entity
    {
    private:
        olc::vf2d direction;
        float shape;
        float lifespan;
        int hits;
        std::shared_ptr<olc::Sprite> sprite;

    protected:
        bool whatIsLife();
    public:

        Projectile( olc::vf2d spawn, olc::vf2d projSize, float duration,olc::vf2d orientation,int hitCount, olc::PixelGameEngine* game);
        ~Projectile();

        TYPE whoAreYou(){ return PROJECTILE;}

        void update(float fElapsedTime,olc::vf2d worldMove);

        float impact(Entity* other);
        void kill();
        float getLife();

        void render();

};


class Decoration : public Entity
{
    protected:
        bool whatIsLife();

    public:
        Decoration( olc::vf2d spawn, float newSize);
        ~Decoration();
        TYPE whoAreYou();
        void render();
};



#endif // ENTITIES_H_INCLUDED
