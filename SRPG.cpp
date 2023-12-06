#include "olcPixelGameEngine.h"
#include "olcPGEX_TransformedView.h"
#include "srpg_data.h"
#include "Managers.h"
#include "Entities.h"

using std::vector;
using std::list;
using std::shared_ptr;

QuadTree* srpg_data::gameObjects ;
olc::TransformedView* srpg_data::viewer;

class SurvivorRPG : public olc::PixelGameEngine
{
float screenRatio;
float worldRadius = 10.0;

shared_ptr<Hero> mainChar;
std::unique_ptr<FoeManager> villians;

std::unique_ptr<DecalManager> lawn;
list<shared_ptr<Projectile>> bullets;



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
        olc::vf2d screentl = {ScreenWidth(),ScreenHeight()};
        olc::vf2d screenscale = {ScreenHeight()/2.0f,ScreenHeight()/2.0f};
		srpg_data::viewer = new olc::TileTransformedView(screentl,screenscale);
		srpg_data::viewer->MoveWorldOffset({-1*screenRatio,-1});

        // Object inialization
        srpg_data::gameObjects = new QuadTree({-worldRadius,-worldRadius},{worldRadius*2,worldRadius*2});

        SetPixelMode(olc::Pixel::MASK);
        mainChar = std::shared_ptr<Hero> (new Hero({0,0},0.07));

        olc::vi2d Size = srpg_data::viewer->ScaleToScreen(mainChar->getBoxCollider().sides) ;
        olc::Sprite* heroicImage = new olc::Sprite(Size.x+1,Size.y+1);
        mainChar->makeRender(heroicImage,Size,this);
        mainChar->setRender(heroicImage);

        srpg_data::gameObjects->insertItem((std::shared_ptr<Entity>)mainChar);
        villians = std::unique_ptr<FoeManager>(new FoeManager(worldRadius));
        villians->initalize(100,this);
        lawn =  std::unique_ptr<DecalManager>(new DecalManager(worldRadius));
        lawn->initalize();
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
		olc::vf2d misalign = (srpg_data::viewer->GetWorldOffset() + one) * fElapsedTime * -10;
        srpg_data::viewer->MoveWorldOffset(misalign);

    	Clear(olc::Pixel(10,50,50));

		olc::vf2d movement = takeInput(); // Should be upgraded return a data structure holding status on all valid inputs
        movement -= mainChar->getLocal();
        mainChar->placement({0,0}); // slide world and mainChar to keep player at center
        olc::vf2d target = srpg_data::viewer->ScreenToWorld({GetMouseX(),GetMouseY()});
        //move world objects
        movement *= fElapsedTime;
        srpg_data::viewer->MoveWorldOffset(movement);

        //update entity management containers
        villians->update(fElapsedTime,movement);
        lawn->update(fElapsedTime,movement);



		// Shooty buttons for testing - needs moving into control scheeme
		if(GetMouse(olc::Mouse::LEFT).bPressed || GetMouse(olc::Mouse::RIGHT).bHeld)
		{
                std::shared_ptr<Projectile> temp(new Projectile({0,0},0.05f,0.025f,((float)rand() / (float)RAND_MAX) * 100.0f,((float)rand() / (float)RAND_MAX) * 0.5f,target,1));

		        bullets.push_back(temp);
		        srpg_data::gameObjects->insertItem((shared_ptr<Entity>)temp);

		}




        // Loop through and process all bullet objects
        auto entity = bullets.begin();

        while(entity != bullets.end()){

            // call the tick update on each bullet
            (*entity)->update(fElapsedTime,movement);

            // check alive or dead, if alive draw, if dead erase
            if ((*entity)->isAlive()){
                //(*entity)->render();
                entity++;
            } else { // if(!(*entity)->isAlive()){

                bullets.erase(entity++);
                //delete temp;
            }
        }






        // Brute force render entities - todo:Move each type into its own manager and draw layer
        list<shared_ptr<Entity>> renderables;
        Rectangle screen = Rectangle({srpg_data::viewer->GetWorldTL(),srpg_data::viewer->GetWorldVisibleArea()});
        srpg_data::gameObjects->getOverlapItems(screen,renderables);
        for(auto it = renderables.begin();it!= renderables.end();it++){
            (*it)->render();
        }
        if(GetKey(olc::Key::A).bReleased){
		    srpg_data::gameObjects->adjustDepth(1);
		}if(GetKey(olc::Key::Z).bReleased){
		    srpg_data::gameObjects->adjustDepth(-1);
		}
        srpg_data::gameObjects->validateLocations();

        srpg_data::gameObjects->drawTree(olc::DARK_YELLOW,olc::DARK_RED);
        srpg_data::viewer->HandlePanAndZoom();
		renderUI(target); //To build
        DrawString({50,80},std::to_string(srpg_data::gameObjects->activity()),olc::BLUE);
        DrawString({50,90},std::to_string(srpg_data::gameObjects->curDepth()),olc::BLUE);
        DrawString({50,100},std::to_string(renderables.size()),olc::BLUE);
		return true;
	}

    void renderUI(olc::vf2d target){
        //prepare areas for Side bar UI interface
        int uiWidth = (ScreenWidth() - ScreenHeight())/2;

        FillRect(0, 0, uiWidth, ScreenHeight(), olc::VERY_DARK_BLUE);
        FillRect(ScreenWidth() - uiWidth, 0, ScreenHeight(), ScreenHeight(), olc::VERY_DARK_BLUE);

        //Debug code for checking size of bullet list during runtime
        DrawString({50,50},std::to_string(bullets.size()),olc::BLUE);

        //crosshair for targeting
        srpg_data::viewer->DrawLine( target.x+0.01,target.y,target.x-0.01,target.y,olc::DARK_MAGENTA);
        srpg_data::viewer->DrawLine( target.x,target.y+0.01,target.x,target.y-0.01,olc::DARK_MAGENTA);

    }
};




int main()
{
	SurvivorRPG game;
	if (game.Construct(1024, 640, 2, 2))
		game.Start();

	return 0;
}

