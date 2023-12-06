#ifndef ENTITIES_H_INCLUDED
#define ENTITIES_H_INCLUDED

class Entity
    {
        protected:
        float entSize;
        olc::vf2d location;
        olc::Pixel colour = olc::DARK_MAGENTA;
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

        virtual bool isAlive(){return true;};
        virtual void render(){};
        virtual void setRender(olc::Sprite* sprite){};
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
    public:
        olc::Decal* image;
        bool solidCollision = true;
    public:
        Hero( olc::vf2d spawn, float newSize);
        ~Hero();
        TYPE whoAreYou();
        bool isAlive();
        olc::vf2d bump(olc::vf2d otherLoc,float otherSize);
        void setImage(olc::Decal* heroicImage) {image = heroicImage; };
        void render();

        void setRender(olc::Sprite* sprite);
        void makeRender(olc::Sprite* sprite, olc::vf2d area, olc::PixelGameEngine* game);

    };

class Foe : public Entity
    {
    private:
        olc::Decal* image;
        bool solidCollision = true;
        float HP = 100.0f;

    public:
        Foe( olc::vf2d spawn, float newSize);
        ~Foe();
        TYPE whoAreYou();
        bool update(float fElapsedTime, olc::vf2d worldMove);
        void render();
        bool isAlive();
        void onOverlap(std::shared_ptr<Entity> other);
        olc::vf2d bump(olc::vf2d otherLoc,float otherSize);
        // Testing code, may use later for Charecter Sprite
        void setRender(olc::Decal* sprite);
        static void makeRender(olc::Sprite* sprite, olc::vf2d area, olc::PixelGameEngine* game);
    };

class Projectile : public Entity
    {
    private:
        olc::vf2d direction;
        float shape;
        float travel;
        float lifespan;
        int hits;
    public:
        Projectile( olc::vf2d spawn, float newSize, float width, float duration,
                float speed, olc::vf2d orientation,int hitCount):Entity(spawn,newSize){

            shape = width;
            lifespan = duration;
            travel = speed;
            direction = orientation.norm();
            colour = olc::DARK_GREY;
            hits = hitCount;
        }
        ~Projectile(){};

        TYPE whoAreYou(){ return PROJECTILE;}

        bool update(float fElapsedTime, olc::vf2d worldMove);

        bool isAlive();

        float impact(Entity* other);
        void kill();
        float getLife();

        void render();


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
