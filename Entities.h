#ifndef ENTITIES_H_INCLUDED
#define ENTITIES_H_INCLUDED
class Projectile;

class Entity
    {
        protected:
        olc::Decal* image;
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
            DECORATION,
            PROJECTILE,
            I_DONT_KNOW

        };

        public:
        Entity( olc::vf2d spawn, float newSize):location(spawn),entSize(newSize){}
        virtual ~Entity(){};

        void placement(olc::vf2d destiny){
            location = destiny;
        }
        void movement(olc::vf2d destiny){
            location += destiny;
        }
        olc::vf2d getLocal(){return location;}
        float getSpeed(){return speed;};
        float getSize(){return entSize;}
        virtual void update(float fElapsedTime, olc::vf2d worldMove){};
        virtual bool isAlive(){return true;};
        virtual void render(){};


        void setRender(olc::Sprite* sprite){
            image = new olc::Decal(sprite);
        }
        static void makeRender(olc::Sprite* sprite, olc::vi2d area, olc::PixelGameEngine* game){};

        virtual TYPE whoAreYou(){return I_DONT_KNOW;};

        Rectangle getBoxCollider(){
            return Rectangle({location.x - (entSize),location.y - (entSize)},{entSize*2.0f,entSize*2.0f});
        }

        // rotates a point around center
        olc::vf2d rotatePt(olc::vf2d point,olc::vf2d angle){
            olc::vf2d updatedpoint = point - location;

            updatedpoint.x = (point.y * angle.x) + (point.x * angle.y);
            updatedpoint.y = -(point.x * angle.x) + (point.y * angle.y);
            updatedpoint += location;
            return updatedpoint;
        }

    };

class Hero : public Entity
    {
    private:
        float fireRate = 1000; // Projectiles per second
        float projectileCooldown = 0;
        bool solidCollision = true;
    public:
        Hero( olc::vf2d spawn, float newSize);
        ~Hero();
        TYPE whoAreYou();
        bool isAlive();
        void update(float fElapsedTime, olc::vf2d worldMove);
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
        //olc::Decal* image;
        bool solidCollision = true;
        float HP = 100.0f;

    public:
        Foe( olc::vf2d spawn, float newSize);
        ~Foe();
        TYPE whoAreYou();
        void update(float fElapsedTime, olc::vf2d worldMove);
        bool isAlive();
        void onOverlap(std::shared_ptr<Entity> other);
        olc::vf2d bump(olc::vf2d otherLoc,float otherSize);

        void render();
        static void makeRender(olc::Sprite* sprite, olc::vf2d area, olc::PixelGameEngine* game);
        // make Render should be moved out to its own module that will contain many different entity draw routines
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

};



#endif // ENTITIES_H_INCLUDED
