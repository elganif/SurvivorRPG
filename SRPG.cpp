/************************************************
Survivor RPG program, tentitive release title "Roles of Survival"
A Survivorlike game designed around more classical RPG elements like
core stats, abilties, and interesting class - race combinations.

Solo project by Elganif (AKA Allen Hardonk)

Built on the Pixel Game Engine develouped by:
OneLoneCoder https://github.com/OneLoneCoder
Plugin Transformed View also by OneLoneCoder used in this project

************************************************/

#define OLC_PGE_GAMEPAD
#include "olcPixelGameEngine.h"
#include "olcPGEX_TransformedView.h"
#include "olcPGEX_Gamepad.h"
#include "srpg_data.h"
#include "Managers.h"
#include "Entities.h"
#include "Menus.h"
#include "Engine.h"


using std::vector;
using std::list;
using std::shared_ptr;

std::unique_ptr<QuadTree> srpg_data::gameObjects;
olc::TransformedView* srpg_data::viewer;
olc::GamePad* controller = nullptr;

uint8_t srpg_data::renderLayerFloor;
uint8_t srpg_data::renderLayerEntities;
uint8_t srpg_data::renderLayerUI;
uint8_t srpg_data::renderLayerMenu;


class SurvivorRPG : public olc::PixelGameEngine
{
bool gameOpen = true;
float screenRatio;
float worldRadius = 10.0;
std::unique_ptr<Interactable> buttons;

std::unique_ptr<Menu> mainMenu = nullptr;
std::unique_ptr<Menu> gameOverScreen = nullptr;
std::unique_ptr<GameWorld> gamePlay;

bool menus = true;

std::chrono::_V2::high_resolution_clock::time_point startTime;

public:
	SurvivorRPG()
	{
		sAppName = "Roles of Survival";
	}
    ~SurvivorRPG()
    {

    }
public:
	bool OnUserCreate() override
	{
		// Called once at the start, so create things here
        olc::GamePad::init();
        //create needed rendering layers back to front
        SetPixelMode(olc::Pixel::MASK);
        Clear(olc::BLANK);
        srpg_data::renderLayerMenu = CreateLayer();
        EnableLayer(srpg_data::renderLayerMenu,true);

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

        // Object inialization


        gamePlay = std::make_unique<GameWorld>(worldRadius,this);

        olc::vf2d mtl = {ScreenWidth() * 0.4f, ScreenHeight() * 0.2f};
        olc::vf2d marea = {ScreenWidth() *0.2f , ScreenHeight() * 0.5f};
        float heighttoadd = 0;
        mainMenu = std::make_unique<Menu>(this,mtl,marea);
        mainMenu->addItem(std::unique_ptr<Interactable>(new TitlePlate(this,"Roles of",olc::vf2d(5,heighttoadd+=5),{marea.x-10,30},3)));
        mainMenu->addItem(std::unique_ptr<Interactable>(new TitlePlate(this,"Survival",olc::vf2d(5,heighttoadd+=35),{marea.x-10,30},3)));

        mainMenu->addItem(std::unique_ptr<Interactable>(new Button(this,"START",{3,heighttoadd+=35},{marea.x - 6,30},
                        [&]{gamePlay->start();menus = false;})));

        mainMenu->addItem(std::unique_ptr<Interactable>(new Button(this,"Restart",{3,heighttoadd+=35},{marea.x - 6,30},
                        [&]{gamePlay = std::make_unique<GameWorld>(worldRadius,this);gamePlay->start();menus = false;})));

        mainMenu->addItem(std::unique_ptr<Interactable>(new Button(this,"EXIT",{3,heighttoadd+=35},{marea.x - 6,30},
                        [&]{gameOpen = false;menus = false;})));

        mtl = {(ScreenWidth() - ScreenHeight())*0.5f + (ScreenHeight() * 0.25f), ScreenHeight() * 0.25f};
        marea = {ScreenHeight() *0.5f , ScreenHeight() * 0.5f};

        heighttoadd = 0;
        gameOverScreen = std::make_unique<Menu>(this,mtl,marea);

        gameOverScreen->addItem(std::unique_ptr<Interactable>(new TitlePlate(this,"GAME",olc::vf2d(5,heighttoadd+=5),{marea.x-10,30},2)));
        gameOverScreen->addItem(std::unique_ptr<Interactable>(new TitlePlate(this,"OVER",olc::vf2d(5,heighttoadd+=35),{marea.x-10,30},2)));

        gameOverScreen->addItem(std::unique_ptr<Interactable>(new Button(this,"Restart",{3,heighttoadd+=35},{marea.x - 6,30},
                        [&]{gamePlay = std::make_unique<GameWorld>(worldRadius,this);gamePlay->start();menus = false;})));
        gameOverScreen->addItem(std::unique_ptr<Interactable>(new Button(this,"EXIT",{3,heighttoadd+=35},{marea.x - 6,30},
                        [&]{gameOpen = false;menus = false;})));
        startTime = std::chrono::high_resolution_clock::now();
		return true;
	}


    // collects key inputs from user and returns The desired actions. ::todo make extendable for more commands, and figure out remapable inputs.
	void takeInput(srpg_data::controls& inputs)
	{
        bool gamepad;
        if(controller == nullptr || !controller->stillConnected){
            controller = olc::GamePad::selectWithAnyButton();
            gamepad = false;
        } else {
            gamepad = true;
        }

        inputs.escapeKey = GetKey(olc::Key::ESCAPE).bPressed;

	    inputs.mainAttack = GetMouse(olc::Mouse::LEFT).bPressed;
        inputs.rapidFire = GetMouse(olc::Mouse::RIGHT).bHeld;
        inputs.target = srpg_data::viewer->ScreenToWorld({(float)GetMouseX(),(float)GetMouseY()});
        inputs.UItarget = {(float)GetMouseX(),(float)GetMouseY()};

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
        if(gamepad){
            movement.x = (controller->getAxis(olc::GPAxes::LX)-0.5f) * -2;
            movement.y = (controller->getAxis(olc::GPAxes::LY)-0.5f) * -2;
            if(movement.x != 0.0f || movement.y !=0.0f){
                inputs.movement = movement.mag2() < 1 ? movement : movement.norm();
            }
            olc::vf2d aim = {0.0f,0.0f};
            aim.x = (controller->getAxis(olc::GPAxes::RX)-0.5f) * 2;
            aim.y = (controller->getAxis(olc::GPAxes::RY)-0.5f) * 2;
            if(aim.x != 0.0f || aim.y != 0.0f){
                inputs.aim = aim;
            }


            SetDrawTarget(srpg_data::renderLayerMenu);
            DrawString({30,30},"controller:" + std::to_string(inputs.aim.x)+
                " "+ std::to_string(inputs.aim.y)+
                " "+ std::to_string(inputs.aim.mag()) ,olc::YELLOW);
        }


	}

	int wholetimemax = 1;
	int PGEtimemax = 0;
	int menutimemax = 0;
	int enginetimemax = 0;

	bool OnUserUpdate(float fElapsedTime) override
	{
		std::chrono::_V2::high_resolution_clock::time_point endPGETime = std::chrono::high_resolution_clock::now();
		// called once per frame
        // Clear all layers for new frame draw.
        SetDrawTarget(srpg_data::renderLayerMenu);
        Clear(olc::BLANK);
        SetDrawTarget(srpg_data::renderLayerUI);
        Clear(olc::BLANK);
        SetDrawTarget(srpg_data::renderLayerEntities);
        Clear(olc::BLANK);
        SetDrawTarget(srpg_data::renderLayerFloor);
        Clear(olc::Pixel(0,16,0));

        //Gather input
        srpg_data::controls inputs;
		takeInput(inputs);

		if(inputs.escapeKey){
		    menus = !menus;
        }

        if(menus){
            mainMenu->render(inputs);
        }
        if(gamePlay->gameOver()){
            gameOverScreen->render(inputs);

        }
        renderUI(inputs.target); //TODO: build properly
        std::chrono::_V2::high_resolution_clock::time_point endMenuTime = std::chrono::high_resolution_clock::now();
        if(!menus){
            gamePlay->run(fElapsedTime,inputs);
        }
        gamePlay->draw();

        std::chrono::_V2::high_resolution_clock::time_point endGameTime = std::chrono::high_resolution_clock::now();

        SetDrawTarget(nullptr);
        std::string temp = "";
        int wholetime = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - startTime).count();
        temp = std::to_string(wholetime) + ": Full";
        FillRect(ScreenWidth()-300,10,100*((float)wholetime/(float)wholetimemax),10,olc::DARK_GREY);
        DrawRect(ScreenWidth()-300,10,100*((float)wholetimemax/(float)wholetimemax),10,olc::GREY);
        DrawString(ScreenWidth()-(temp.size()*8),10,temp);
        wholetimemax = wholetimemax > wholetime ? wholetimemax : wholetime;
        DrawString(ScreenWidth()-(temp.size()*8),20,std::to_string(wholetimemax));

        int PGEtime = std::chrono::duration_cast<std::chrono::nanoseconds>(endPGETime - startTime).count();
        temp = "PGE: " + std::to_string(PGEtime);
        FillRect(ScreenWidth()-300,30,100*((float)PGEtime/(float)wholetimemax),10,olc::DARK_GREY);
        DrawRect(ScreenWidth()-300,30,100*((float)PGEtimemax/(float)wholetimemax),10,olc::GREY);
        DrawString(ScreenWidth()-(temp.size()*8),30,temp);
        PGEtimemax = PGEtimemax > PGEtime ? PGEtimemax : PGEtime;
        DrawString(ScreenWidth()-(temp.size()*8),40,std::to_string(PGEtimemax));

        int menutime = std::chrono::duration_cast<std::chrono::nanoseconds>(endMenuTime - endPGETime).count();
        temp = "Menu: " + std::to_string(menutime);
        FillRect(ScreenWidth()-300,50,100*((float)menutime/(float)wholetimemax),10,olc::DARK_GREY);
        DrawRect(ScreenWidth()-300,50,100*((float)menutimemax/(float)wholetimemax),10,olc::GREY);
        DrawString(ScreenWidth()-(temp.size()*8),50,temp);
        menutimemax = menutimemax > menutime ? menutimemax : menutime;
        DrawString(ScreenWidth()-(temp.size()*8),60,std::to_string(menutimemax));

        int enginetime = std::chrono::duration_cast<std::chrono::nanoseconds>(endGameTime - endPGETime).count();
        temp = "Engine: " + std::to_string(enginetime);
        FillRect(ScreenWidth()-300,70,100*((float)enginetime/(float)wholetimemax),10,olc::DARK_GREY);
        DrawRect(ScreenWidth()-300,70,100*((float)enginetimemax/(float)wholetimemax),10,olc::GREY);
        DrawString(ScreenWidth()-(temp.size()*8),70,temp);
        enginetimemax = enginetimemax > enginetime ? enginetimemax : enginetime;
        DrawString(ScreenWidth()-(temp.size()*8),80,std::to_string(enginetimemax));

        startTime = std::chrono::high_resolution_clock::now();

        if(GetKey(olc::Key::F1).bReleased){
            wholetimemax = 1;
            PGEtimemax = 0;
            menutimemax = 0;
            enginetimemax = 0;
        }


		return gameOpen; // if gameOpen becomes false this will close the program
	}

    bool OnUserDestroy(){
        //clean up memory on exit
        delete srpg_data::viewer;
        return true;
    }

    void renderUI(olc::vf2d target){
        //prepare areas for Side bar UI interface
        SetDrawTarget(srpg_data::renderLayerUI);
        //Clear(olc::BLANK);
        int uiWidth = (ScreenWidth() - ScreenHeight())/2;

        FillRect(0, 0, uiWidth, ScreenHeight(), olc::VERY_DARK_BLUE);
        FillRect(ScreenWidth() - uiWidth, 0, ScreenHeight(), ScreenHeight(), olc::VERY_DARK_BLUE);

        //Debug data code
        DrawString({50,80},"tot Quads:" + std::to_string(srpg_data::gameObjects->activity()),olc::BLUE);
        DrawString({50,90},"quad depth:" + std::to_string(srpg_data::gameObjects->curDepth()),olc::BLUE);

        //crosshair for targeting
        SetDrawTarget(nullptr);
        Clear(olc::BLANK);
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

