#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#define OLC_PGEX_TRANSFORMEDVIEW
#include "olcPGEX_TransformedView.h"

#include "Entities.h"

using std::vector;
using std::list;

class SurvivorRPG : public olc::PixelGameEngine
{
float screenRatio;
float worldRadius = 10.0;

Hero* mainChar;
Foe* mainVillain;

vector<Grass*> lawn;
list<Projectile*> bullets;

olc::TileTransformedView viewer;

public:
	SurvivorRPG()
	{
		sAppName = "Roles of Survival";
	}

public:
	bool OnUserCreate() override
	{
		// Called once at the start, so create things here
        screenRatio = (float)ScreenWidth()/(float)ScreenHeight();

		viewer = olc::TileTransformedView({ScreenWidth(),ScreenHeight()},{ScreenHeight()/2,ScreenHeight()/2});
		viewer.MoveWorldOffset({-1*screenRatio,-1});

        // Object inialization
        mainChar = new Hero(&viewer,{0,0},0.07);
        mainVillain = new Foe(&viewer,{0.5,0.5},0.05);

        //place random grass for background
        srand(time(NULL));

        while(lawn.size() < worldRadius * worldRadius * 4 * 2.5){ // 4 quadrents * # grass per unit area
            olc::vf2d attempting;
            attempting.x = worldRadius*2 * ((float)rand() / (float)RAND_MAX) - worldRadius;
            attempting.y = worldRadius*2 * ((float)rand() / (float)RAND_MAX) - worldRadius;
            bool valid = true;

            for(auto const& blade : lawn){
                float distance = (attempting - blade->findLoc()).mag();
                if(distance < 0.5){
                    valid = false;
                    break;
                }
            }
            if(valid){
                lawn.push_back( new Grass(&viewer, attempting, 0.02));
            }
        }
        bullets.push_back(new Projectile(&viewer,{0.2,0.2},0.1,0.05,2.0,0.1,{0.5,0.5}));

		return true;
	}


    // collects key inputs from user and returns The desired actions. ::todo make extendable for more commands, and figure out remapable inputs.
	olc::vf2d takeInput()
	{
	    olc::vf2d movement = {0,0};
	    if(GetKey(olc::Key::UP).bHeld){
		    movement.y++;
		}
		if(GetKey(olc::Key::DOWN).bHeld){
            movement.y--;
		}
		if(GetKey(olc::Key::LEFT).bHeld){
		    movement.x++;
		}
		if(GetKey(olc::Key::RIGHT).bHeld){
            movement.x--;
		}
        if(movement.x != 0 || movement.y !=0){
            movement = movement.norm();
        }
        return movement;

	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		// called once per frame

		// Gradually move view to center on Player
        olc::vf2d one = {screenRatio,1.0};
		olc::vf2d misalign = (viewer.GetWorldOffset() + one) * fElapsedTime * -10;
        viewer.MoveWorldOffset(misalign);

    	Clear(olc::BLACK);

		olc::vf2d movement = takeInput(); // Should be upgraded return a data structure holding status on all valid inputs
        olc::vf2d target = viewer.ScreenToWorld({GetMouseX(),GetMouseY()});
        //move world objects
        movement *= fElapsedTime;
        viewer.MoveWorldOffset(movement);

        mainChar->render(movement);

        mainVillain->movement(movement);

        mainVillain->render();

        //confine the lawn to an area around the player
        for(int i = 0;i < lawn.size();i++){
            lawn[i]->movement(movement);
            if(lawn[i]->findLoc().x > worldRadius)
                lawn[i]->movement({-worldRadius * 2,0});
            if(lawn[i]->findLoc().x < -worldRadius)
                lawn[i]->movement({worldRadius * 2,0});
            if(lawn[i]->findLoc().y > worldRadius)
                lawn[i]->movement({0, -worldRadius * 2});
            if(lawn[i]->findLoc().y < -worldRadius)
                lawn[i]->movement({0, worldRadius * 2});
            lawn[i]->render();
		}
		if(GetMouse(olc::Mouse::LEFT).bPressed || GetMouse(olc::Mouse::RIGHT).bHeld)
		{

		        bullets.push_back(new Projectile(&viewer,{0,0},0.05,0.025,((float)rand() / (float)RAND_MAX) * 100.0,((float)rand() / (float)RAND_MAX) * 0.5,target));


		}

        for(auto const& entity : bullets){
            entity->movement(movement);

            if (entity->isAlive()){
                entity->update(fElapsedTime);
                entity->render();
            }

		}

		// clean up bullets that have expired by deleting from the front until a needed bullet is found
        while(bullets.size() > 0 && !bullets.front()->isAlive()){
            Projectile* temp = bullets.front();
            bullets.pop_front();
            delete temp;
        }
        // prevent any particularily long lived bullets from blocking clean up next frame by moving some to back of list
        for(int i = 0; i < bullets.size() / 2 && bullets.front()->getLife() > 1.0 ;i++){
            Projectile* temp = bullets.front();
            bullets.pop_front();
            bullets.push_back(temp);
        }

        viewer.DrawLine( target.x+0.01,target.y,target.x-0.01,target.y,olc::DARK_MAGENTA);
        viewer.DrawLine( target.x,target.y+0.01,target.x,target.y-0.01,olc::DARK_MAGENTA);
		//prepare areas for Side bar UI interface
		int uiWidth = (ScreenWidth() - ScreenHeight())/2;

		FillRect(0, 0, uiWidth, ScreenHeight(), olc::VERY_DARK_BLUE);
		FillRect(ScreenWidth() - uiWidth, 0, ScreenHeight(), ScreenHeight(), olc::VERY_DARK_BLUE);
		DrawString({50,50},std::to_string(bullets.size()),olc::BLUE);
        for(int i = 0;i < bullets.size();i++){
            DrawRect(3,i*3,2,2,olc::MAGENTA);
        }

		return true;
	}


};




int main()
{
	SurvivorRPG game;
	if (game.Construct(1024, 640, 2, 2))
		game.Start();

	return 0;
}

