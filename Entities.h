#ifndef ENTITIES_H_INCLUDED
#define ENTITIES_H_INCLUDED
class Projectile;

class Entity
    {
        protected:
        olc::Decal* image = nullptr;
        float entSize;
        olc::vf2d location;
        olc::Pixel fColour = olc::DARK_MAGENTA;
        olc::Pixel lineColour = olc::Pixel(255,0,127);
        //olc::TransformedView* viewer;
        bool solidCollision = false;
        float speed = 0.0;
        enum TYPE {
            HERO,
            FOE,
            PROJECTILE,
            DECORATION
        };

        static int uuid;
        const int entID;

        public:
        Entity(olc::vf2d spawn, float newSize);

        virtual ~Entity(){delete image;}

        ///pure virtual functions that all derived must implement
        virtual void render()=0;
        virtual void update(float fElapsedTime, olc::vf2d worldMove)=0;
        virtual TYPE whoAreYou()=0;

        virtual bool isAlive(){return true;}

        int getUID(){return entID;}

        void placement(olc::vf2d destiny){location = destiny;}
        void movement(olc::vf2d destiny){location += destiny;}
        olc::vf2d getLocal(){return location;}

        float getSize(){return entSize;}
        float getSpeed(){return speed;}

        void setRender(olc::Sprite* sprite);

        static void makeRender(olc::Sprite* sprite, olc::vi2d area, olc::PixelGameEngine* game);

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
        bool solidCollision = true;
    public:
        Hero( olc::vf2d spawn, float newSize);
        ~Hero();
        TYPE whoAreYou();
        bool isAlive();
        void update(float fElapsedTime, olc::vf2d worldMove);
        float getHP();


        bool fireProjectile(olc::vf2d& target, std::list<std::shared_ptr<Projectile>>& bullets, olc::PixelGameEngine* game);
        bool projectileReady();
        olc::vf2d bump(olc::vf2d otherLoc,float otherSize);

        void setImage(olc::Decal* heroicImage) {image = heroicImage; };
        void render();
        void makeRender(olc::Sprite* sprite, olc::vf2d area, olc::PixelGameEngine* game);

    };

class Foe : public Entity
    {
    private:
        float maxHP = 100.0f;
        float HP = 100.0f;

    public:
        Foe( olc::vf2d spawn, float newSize);
        ~Foe();
        void update(float fElapsedTime, olc::vf2d worldMove);
        TYPE whoAreYou();
        bool isAlive();
        void onOverlap(std::shared_ptr<Entity> other);
        olc::vf2d bump(olc::vf2d otherLoc,float otherSize);

        void render();
        static void makeRender(olc::Sprite* sprite, olc::vf2d area, olc::PixelGameEngine* game);
        // make Render should be moved out to its own module that will contain many routines for many different things
    };

class Projectile : public Entity
    {
    private:
        olc::vf2d direction;
        float shape;
        float lifespan;
        int hits;
        olc::Sprite* sprite;
    public:
        Projectile( olc::vf2d spawn, float newSize, float width, float duration,olc::vf2d orientation,int hitCount, olc::PixelGameEngine* game);
        ~Projectile();

        TYPE whoAreYou(){ return PROJECTILE;}

        void update(float fElapsedTime, olc::vf2d worldMove);

        bool isAlive();

        float impact(Entity* other);
        void kill();
        float getLife();

        void render();
        void makeRender(olc::Sprite* sprite, olc::vf2d area, olc::PixelGameEngine* game);


};


class Decoration : public Entity
{
    private:

        bool solidCollision = false;
    public:
        Decoration( olc::vf2d spawn, float newSize);
        ~Decoration();
        TYPE whoAreYou();
        void render();

        void update(float fElapsedTime, olc::vf2d worldMove);

};



#endif // ENTITIES_H_INCLUDED
