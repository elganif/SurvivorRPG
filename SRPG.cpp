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
private:
bool gameOpen = true;
float screenRatio;
float worldRadius = 10.0;

std::unique_ptr<Menu> title = nullptr;
std::unique_ptr<Menu> mainMenu = nullptr;
std::unique_ptr<Menu> gameOverScreen = nullptr;
std::unique_ptr<GameWorld> gamePlay = nullptr;

struct STATE{
    enum MENU{
        CLOSED,
        MAIN,
        OPTIONS
    };
    enum GAME{
        NONE,
        LOADING,
        PLAY,
        PAUSED,
        OVER
    };
    MENU menu = MENU::MAIN;
    GAME game = GAME::NONE;
};

STATE state;
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

        title = constructTitle();

        mainMenu = constructMain();

        gameOverScreen = constructGameOver();

        if(srpg_data::debugTools)
            startTime = std::chrono::high_resolution_clock::now();
		return true;
	}

    std::unique_ptr<Menu> constructTitle(){
        olc::vf2d titleCenter = {ScreenWidth() * 0.5f, ScreenHeight() *0.3f};
        olc::vf2d titleArea = {ScreenWidth() * 0.2f, ScreenHeight() *0.5f};
        std::unique_ptr<Menu> title = std::make_unique<Menu>(this,titleCenter);
        title->addItem(std::unique_ptr<UIElement>(new TitlePlate(this,"Roles of",{5,5},3)));
        title->addItem(std::unique_ptr<UIElement>(new TitlePlate(this,"Survival",{5,5},3)));

        return title;

    }

	std::unique_ptr<Menu> constructMain(){

        olc::vf2d menuCenter = {ScreenWidth() *0.5f , ScreenHeight() * 0.6f};
        olc::vf2d menuArea = {ScreenWidth() * 0.2f, ScreenHeight() * 0.5f};

        std::unique_ptr<Menu> main = std::make_unique<Menu>(this,menuCenter,menuArea);

        main->addItem(std::unique_ptr<UIElement>(new Button(this,"START",{menuArea.x - 6,30},
                        [&]{if(state.game == STATE::GAME::NONE){
                                gamePlay->start();
                                state.menu = STATE::MENU::CLOSED;
                                state.game = STATE::GAME::PLAY;
                            }})));

        main->addItem(std::unique_ptr<UIElement>(new Button(this,"RESTART",{menuArea.x - 6,30},
                        [&]{gamePlay = std::make_unique<GameWorld>(worldRadius,this);
                                gamePlay->start();
                                state.menu = STATE::MENU::CLOSED;
                                state.game = STATE::GAME::PLAY;

                            })));

        main->addItem(std::unique_ptr<UIElement>(new Button(this,"EXIT",{menuArea.x - 6,30},
                        [&]{gameOpen = false;})));

        return main;
    }

    std::unique_ptr<Menu> constructGameOver(){
        olc::vf2d menuCenter = {ScreenWidth() * 0.5f , ScreenHeight() * 0.5f};
        olc::vf2d menuArea = {ScreenHeight() * 0.5f , ScreenHeight() * 0.5f};


        std::unique_ptr<Menu> gameOver = std::make_unique<Menu>(this,menuCenter,menuArea);

        gameOver->addItem(std::unique_ptr<UIElement>(new TitlePlate(this,"GAME",{10,10},2)));

        gameOver->addItem(std::unique_ptr<UIElement>(new TitlePlate(this,"OVER",{10,10},2)));

        gameOver->addItem(std::unique_ptr<UIElement>(new Button(this,"Main Menu",{200 - 6,30},
                        [&]{gamePlay.reset();
                            state.menu = STATE::MENU::MAIN;
                            state.game = STATE::GAME::NONE;
                            })));

        gameOver->addItem(std::unique_ptr<UIElement>(new Button(this,"Restart",{200 - 6,30},
                        [&]{gamePlay = std::make_unique<GameWorld>(worldRadius,this);
                            gamePlay->start();
                            state.menu = STATE::MENU::CLOSED;})));

        gameOver->addItem(std::unique_ptr<UIElement>(new Button(this,"Exit",{200 - 6,30},
                        [&]{gameOpen = false;})));
        return gameOver;

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
    std::chrono::_V2::high_resolution_clock::time_point endGameTime;
    std::chrono::_V2::high_resolution_clock::time_point endPGETime;
    std::chrono::_V2::high_resolution_clock::time_point endMenuTime;

	bool OnUserUpdate(float fElapsedTime) override
	{
		// called once per frame

		if(srpg_data::debugTools)
            endPGETime = std::chrono::high_resolution_clock::now();
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

            state.menu = state.menu == STATE::MENU::CLOSED ? STATE::MENU::MAIN : STATE::MENU::CLOSED;
        }

        if(state.menu == STATE::MENU::MAIN){
            title->render(inputs);
            mainMenu->render(inputs);
        }



        renderUI(inputs.target); //TODO: build properly

        if(srpg_data::debugTools)
            endMenuTime = std::chrono::high_resolution_clock::now();

        if(state.game == STATE::GAME::PLAY){
            gamePlay->run(fElapsedTime,inputs);
            if(gamePlay->gameOver()){
               state.game = STATE::GAME::OVER;
            }
        }
        if(state.game == STATE::GAME::OVER){
            gameOverScreen->render(inputs);
        }
        if(state.game != STATE::GAME::NONE){
            gamePlay->gameHudDraw(inputs);
            gamePlay->draw();
        }


        if(srpg_data::debugTools)
            endGameTime = std::chrono::high_resolution_clock::now();

        SetDrawTarget(nullptr);

        if(srpg_data::debugTools){
            std::string temp = "";
            std::string tempM = "";

            int wholetime = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - startTime).count();

            FillRect(ScreenWidth()-300,10,100*((float)wholetime/(float)wholetimemax),10,olc::DARK_GREY);
            DrawRect(ScreenWidth()-300,10,100*((float)wholetimemax/(float)wholetimemax),10,olc::GREY);
            wholetimemax = wholetimemax > wholetime ? wholetimemax : wholetime;

            temp = std::to_string(wholetime   ) + " :Full ";
            tempM= std::to_string(wholetimemax) + "       ";
            DrawString(ScreenWidth()-(temp.size()*8),10,temp);
            DrawString(ScreenWidth()-(tempM.size()*8),20,tempM);



            int PGEtime = std::chrono::duration_cast<std::chrono::nanoseconds>(endPGETime - startTime).count();

            FillRect(ScreenWidth()-300,30,100*((float)PGEtime/(float)wholetimemax),10,olc::DARK_GREY);
            DrawRect(ScreenWidth()-300,30,100*((float)PGEtimemax/(float)wholetimemax),10,olc::GREY);
            PGEtimemax = PGEtimemax > PGEtime ? PGEtimemax : PGEtime;

            temp = std::to_string(PGEtime   ) + " :PGE  ";
            tempM= std::to_string(PGEtimemax) + "       ";
            DrawString(ScreenWidth()-(temp.size()*8),30,temp);
            DrawString(ScreenWidth()-(tempM.size()*8),40,tempM);


            int menutime = std::chrono::duration_cast<std::chrono::nanoseconds>(endMenuTime - endPGETime).count();

            FillRect(ScreenWidth()-300,50,100*((float)menutime/(float)wholetimemax),10,olc::DARK_GREY);
            DrawRect(ScreenWidth()-300,50,100*((float)menutimemax/(float)wholetimemax),10,olc::GREY);
            menutimemax = menutimemax > menutime ? menutimemax : menutime;

            temp = std::to_string(menutime   ) + " :Menu ";
            tempM= std::to_string(menutimemax) + "       ";
            DrawString(ScreenWidth()-(temp.size()*8),50,temp);
            DrawString(ScreenWidth()-(tempM.size()*8),60,tempM);


            int enginetime = std::chrono::duration_cast<std::chrono::nanoseconds>(endGameTime - endPGETime).count();

            FillRect(ScreenWidth()-300,70,100*((float)enginetime/(float)wholetimemax),10,olc::DARK_GREY);
            DrawRect(ScreenWidth()-300,70,100*((float)enginetimemax/(float)wholetimemax),10,olc::GREY);
            enginetimemax = enginetimemax > enginetime ? enginetimemax : enginetime;

            temp = std::to_string(enginetime   ) + " :Engi ";
            tempM= std::to_string(enginetimemax) + "       ";
            DrawString(ScreenWidth()-(temp.size()*8),70,temp);
            DrawString(ScreenWidth()-(tempM.size()*8),80,tempM);


            startTime = std::chrono::high_resolution_clock::now();

            if(GetKey(olc::Key::F1).bReleased){
                wholetimemax = 1;
                PGEtimemax = 0;
                menutimemax = 0;
                enginetimemax = 0;
            }
        }

		return gameOpen; // if gameOpen becomes false this will close the program
	}

    bool OnUserDestroy(){
        //clean up memory on exit
        delete srpg_data::viewer;
        return true;
    }

    void renderUI(olc::vf2d target){

    }
};




int main()
{
	SurvivorRPG game;
	if (game.Construct(1024, 640, 2, 2))
		game.Start();

	return 0;
}

