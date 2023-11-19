#ifndef ENTITIES_H_INCLUDED
#define ENTITIES_H_INCLUDED

class Entity
    {
        public:
        float entSize;
        olc::vf2d location;
        olc::Pixel colour = olc::DARK_MAGENTA;
        olc::TransformedView* view;

        public:
        Entity(){}
        virtual ~Entity(){};

        void placement(olc::vf2d destiny){
            location = destiny;
        }
        void movement(olc::vf2d destiny){
            location += destiny;
        }
        virtual void render(){};

        olc::vf2d rotatePt(olc::vf2d point,olc::vf2d angle){
            olc::vf2d updatedpoint;

            updatedpoint.x = (point.y * angle.x) + (point.x * angle.y);
            updatedpoint.y = -(point.x * angle.x) + (point.y * angle.y);

            return updatedpoint;;
        }
    };

class Hero : public Entity
    {
    private:

    public:
        Hero(olc::TransformedView* window, olc::vf2d spawn, float newSize){
            location = spawn;
            entSize = newSize;
            view = window;
            colour = olc::BLUE;
        }
        ~Hero(){};

        void render(){
            view->DrawCircle(location,entSize,colour);
        }

        // Testing code, may use later for Charecter Sprite
        void render(olc::vf2d faceing){
            float looking = 0;
            if (faceing.x < 0.0)
                looking += 0.05;
            if (faceing.x > 0.0)
                looking -= 0.05;

            view->DrawCircle((0.0 + looking) *entSize,-1 *entSize, 0.25 *entSize); // Head

            view->DrawRect( -0.3 *entSize,-0.7 *entSize,-0.15*entSize, 0.6 *entSize,colour); // Left Arm
            view->DrawRect(  0.3 *entSize,-0.7 *entSize, 0.15*entSize, 0.6 *entSize,colour); // Right Arm
            view->DrawRect( -0.3 *entSize,-0.75 *entSize, 0.6 *entSize, 0.75 *entSize,colour); // Torso
            view->DrawRect(  0.0 *entSize, 0.0 *entSize,-0.25 *entSize, 0.8 *entSize,colour); // Left Leg
            view->DrawRect(  0.0 *entSize, 0.0 *entSize, 0.25 *entSize, 0.8 *entSize,colour); // Right Leg

            //collider Debug
            view->DrawCircle(location,entSize,olc::VERY_DARK_BLUE);
        }

    };

class Foe : public Entity
    {
    private:

    public:
        Foe(olc::TransformedView* window, olc::vf2d spawn, float newSize){
            location = spawn;
            entSize = newSize;
            view = window;
            colour = olc::DARK_RED;
        }
        ~Foe(){};

        void render(){
            view->DrawCircle(location,entSize,colour);
        }

        // Testing code, may use later for Charecter Sprite
        void render(olc::vf2d faceing){
            float shape = entSize/2;
            olc::vf2d tip =   {0,entSize/2};
            olc::vf2d left =  {shape/2.0, -entSize/2};
            olc::vf2d right = {-shape/2.0, -entSize/2};
            tip = rotatePt(tip,faceing);
            left = rotatePt(left,faceing);
            right = rotatePt(right,faceing);

            view->DrawTriangle(tip+location,left+location,right+location, colour);
        }
    };

class Projectile : public Entity
    {
    private:
        olc::vf2d direction;
        float shape;
        float travel;
        float lifespan;
    public:
        Projectile(olc::TransformedView* window, olc::vf2d spawn, float newSize, float width, float duration, float speed, olc::vf2d orientation){
            location = spawn;
            entSize = newSize;
            shape = width;
            view = window;
            lifespan = duration;
            travel = speed;
            direction = orientation.norm();
            colour = olc::DARK_GREY;
        }
        ~Projectile(){};

        bool update(float fElapsedTime){
            if (isAlive()) {
                location = location + (direction * travel * fElapsedTime);
                lifespan -= fElapsedTime;
                return 1;
            }
            return 0;
        }

        bool isAlive()
        {
            return lifespan > 0.0;
        }

        float getLife()
        {
            return lifespan;
        }

        void render(){
            // Set up Tips of Triangle
            olc::vf2d tip =   {0,entSize*0.5};
            olc::vf2d left =  {shape*0.5, -entSize*0.5};
            olc::vf2d right = {-shape*0.5, -entSize*0.5};
            // orient to follow movement
            tip = rotatePt(tip,direction);
            left = rotatePt(left,direction);
            right = rotatePt(right,direction);

            view->DrawTriangle(tip+location,left+location,right+location, colour);
        }


    };


class Grass : public Entity
{
    private:

    public:
        Grass(olc::TransformedView* window, olc::vf2d spawn, float newSize){
            location = spawn;
            entSize = newSize;
            view = window;
            colour = olc::DARK_GREEN;
        }
        ~Grass(){};

        olc::vf2d findLoc(){
            return location;
        }

        void render(){
            view->DrawLine(location.x + (entSize/2.0f),location.y,location.x + (entSize/2.0f),location.y - entSize,colour);
            view->DrawLine(location.x ,location.y,location.x ,location.y - entSize*0.6f,colour);
            view->DrawLine(location.x + entSize,location.y,location.x + entSize,location.y - entSize*0.6f,colour);
        }
    };

#endif // ENTITIES_H_INCLUDED
