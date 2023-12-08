#include "olcPixelGameEngine.h"
#include "olcPGEX_TransformedView.h"
#include "srpg_data.h"
#include "Managers.h"
#include "Entities.h"
#include "Engine.h"

using std::vector;
using std::list;
using std::shared_ptr;

QuadTree* srpg_data::gameObjects ;
olc::TransformedView* srpg_data::viewer;
int srpg_data::renderLayerFloor = 0 ;
int srpg_data::renderLayerEntities= 0;
int srpg_data::renderLayerUI= 0;
class SurvivorRPG : public olc::PixelGameEngine
{
float screenRatio;
float worldRadius = 10.0;

GameWorld* gamePlay;




public:
	SurvivorRPG()
	{
		sAppName = "Roles of Survival";
	}

public:
	bool OnUserCreate() override
	{
		// Called once at the start, so create things here

        //create needed rendering layers back to front
        Clear(olc::BLANK);
        srpg_data::renderLayerUI = CreateLayer();
        EnableLayer(srpg_data::renderLayerUI,true);
        srpg_data::renderLayerEntities = CreateLayer();
        EnableLayer(srpg_data::renderLayerEntities,true);
        srpg_data::renderLayerFloor = CreateLayer();
        EnableLayer(srpg_data::renderLayerFloor,true);


        screenRatio = (float)ScreenWidth()/(float)ScreenHeight();
        olc::vf2d screentl = {(float)ScreenWidth(),(float)ScreenHeight()};
        olc::vf2d screenscale = {ScreenHeight()/2.0f,ScreenHeight()/2.0f};
		srpg_data::viewer = new olc::TileTransformedView(screentl,screenscale);
		srpg_data::viewer->MoveWorldOffset({-1*screenRatio,-1});
        SetPixelMode(olc::Pixel::MASK);

        // Object inialization
        srpg_data::gameObjects = new QuadTree({-worldRadius,-worldRadius},{worldRadius*2,worldRadius*2});

        gamePlay = new GameWorld(worldRadius,this);
        gamePlay->start();

		return true;
	}


    // collects key inputs from user and returns The desired actions. ::todo make extendable for more commands, and figure out remapable inputs.
	void takeInput(srpg_data::controls& inputs)
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
            inputs.movement = movement.norm();
        }
        inputs.mainAttack = GetMouse(olc::Mouse::LEFT).bPressed;
        inputs.rapidFire = GetMouse(olc::Mouse::RIGHT).bHeld;
        inputs.target = srpg_data::viewer->ScreenToWorld({(float)GetMouseX(),(float)GetMouseY()});

	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		// called once per frame




        srpg_data::controls inputs;
		takeInput(inputs); // Should be upgraded return a data structure holding status on all valid inputs



        gamePlay->run(fElapsedTime,inputs);



        // Collect all entities in the screen area for rendering


        SetDrawTarget(srpg_data::renderLayerFloor,true);
        Clear(olc::Pixel(5,25,25));

        SetDrawTarget(srpg_data::renderLayerEntities);
        list<shared_ptr<Entity>> renderables;

        Clear(olc::BLANK);
        Rectangle screen = Rectangle({srpg_data::viewer->GetWorldTL(),srpg_data::viewer->GetWorldVisibleArea()});
        srpg_data::gameObjects->getOverlapItems(screen,renderables);
        renderables.sort([](const std::shared_ptr<Entity> f, const std::shared_ptr<Entity> s){return (f->getLocal().y+f->getSize()) < (s->getLocal().y+s->getSize()); });
        for(auto it = renderables.begin();it!= renderables.end();it++){
            (*it)->render();
        }
        srpg_data::gameObjects->validateLocations();
        SetDrawTarget(nullptr);

        //srpg_data::gameObjects->drawTree(olc::DARK_YELLOW,olc::DARK_RED);
        //srpg_data::viewer->HandlePanAndZoom();
		renderUI(inputs.target,renderables.size()); //To build

		return true;
	}

    void renderUI(olc::vf2d target, int rendercount){
        //prepare areas for Side bar UI interface
        SetDrawTarget(srpg_data::renderLayerUI,true);
        Clear(olc::BLANK);
        int uiWidth = (ScreenWidth() - ScreenHeight())/2;

        FillRect(0, 0, uiWidth, ScreenHeight(), olc::VERY_DARK_BLUE);
        FillRect(ScreenWidth() - uiWidth, 0, ScreenHeight(), ScreenHeight(), olc::VERY_DARK_BLUE);

        //Debug code for checking size of bullet list during runtime
        //DrawString({50,50},"Bullet:" + std::to_string(bullets.size()),olc::BLUE);
        DrawString({50,80},"tot Quads:" + std::to_string(srpg_data::gameObjects->activity()),olc::BLUE);
        DrawString({50,90},"quad depth:" + std::to_string(srpg_data::gameObjects->curDepth()),olc::BLUE);
        DrawString({50,100},"items on screen:" + std::to_string(rendercount),olc::BLUE);
        //crosshair for targeting
        srpg_data::viewer->DrawLine( target.x+0.01,target.y,target.x-0.01,target.y,olc::DARK_MAGENTA);
        srpg_data::viewer->DrawLine( target.x,target.y+0.01,target.x,target.y-0.01,olc::DARK_MAGENTA);
        SetDrawTarget(nullptr);
    }
};




int main()
{
	SurvivorRPG game;
	if (game.Construct(1024, 640, 2, 2))
		game.Start();

	return 0;
}

