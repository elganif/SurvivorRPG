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

std::unique_ptr<Profiler> srpg_data::timers;

class SurvivorRPG : public olc::PixelGameEngine
{
private:
bool gameOpen = true;
float screenRatio;
float worldRadius = 10.0;


enum MENUS{
    TITLE,
    OPTION,
    GAMEOVER
};

std::unordered_map<MENUS, std::unique_ptr<Screen>> menuDisplay;
//std::unique_ptr<Screen> title = nullptr;
//std::unique_ptr<Screen> mainMenu = nullptr;
//std::unique_ptr<Screen> gameOverScreen = nullptr;
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

        menuDisplay.emplace(TITLE,constructMain() );

        menuDisplay.emplace(GAMEOVER ,constructGameOver());

        srpg_data::timers = std::make_unique<Profiler>();
		return true;
	}

    std::unique_ptr<Screen> constructMain(){

        olc::vf2d titleArea = {ScreenWidth() * 0.35f, ScreenHeight() *0.2f};
        olc::vf2d titleLoc = olc::vi2d(ScreenWidth() * 0.5f, ScreenHeight() *0.25f) - titleArea/2;

        std::unique_ptr<Screen> screen = std::make_unique<Screen>(this,srpg_data::renderLayerMenu);

        std::unique_ptr<UIContainer> title = std::make_unique<UIContainer>(this,titleArea,UIContainer::VERT);
        title->setTheme(olc::CYAN ,olc::BLANK ,olc::BLANK ,olc::BLANK );

        title->addTitle("Roles of",3,{(int)titleArea.x,(int)titleArea.y/2});
        title->addTitle("Survival",3,{(int)titleArea.x,(int)titleArea.y/2});

        screen->addContainer(title,titleLoc,titleArea);

        olc::vf2d menuArea = {ScreenWidth() * 0.2f, ScreenHeight() * 0.2f};
        olc::vf2d menuLoc = olc::vf2d(ScreenWidth() *0.5f , ScreenHeight() * 0.6f) - menuArea/2;

        std::unique_ptr<UIContainer> main = std::make_unique<UIContainer>(this,menuArea,UIContainer::VERT);
        main->setTheme(olc::CYAN ,olc::DARK_BLUE ,olc::DARK_BLUE ,olc::BLUE );

        main->addButton("START",
                        [&]{if(state.game == STATE::GAME::NONE){
                                gamePlay = std::make_unique<GameWorld>(worldRadius,this);
                                gamePlay->start();
                            }
                            if(gamePlay){
                            state.game = STATE::GAME::PLAY;
                            state.menu = STATE::MENU::CLOSED;
                            }});

        main->addButton("RESTART",
                        [&]{gamePlay = std::make_unique<GameWorld>(worldRadius,this);
                                gamePlay->start();
                                state.game = STATE::GAME::PLAY;
                                state.menu = STATE::MENU::CLOSED;

                            });

        main->addButton("EXIT",
                        [&]{gameOpen = false;});

        screen->addContainer(main,menuLoc,menuArea);

        return screen;
    }

    std::unique_ptr<Screen> constructGameOver(){
        olc::vf2d menuArea = {ScreenHeight() * 0.5f , ScreenHeight() * 0.5f};
        olc::vf2d menuLoc = olc::vf2d(ScreenWidth() * 0.5f , ScreenHeight() * 0.5f) - menuArea;


        std::unique_ptr<UIContainer> gameOver = std::make_unique<UIContainer>(this,menuArea,UIContainer::VERT);

        gameOver->addTitle("GAME",2);

        gameOver->addTitle("OVER",2);

        gameOver->addButton("Main Menu",
                        [&]{gamePlay.reset();
                            state.menu = STATE::MENU::MAIN;
                            state.game = STATE::GAME::NONE;
                            });

        gameOver->addButton("Restart",
                        [&]{gamePlay = std::make_unique<GameWorld>(worldRadius,this);
                            gamePlay->start();
                            state.menu = STATE::MENU::CLOSED;
                            state.game = STATE::GAME::PLAY;
                            });

        gameOver->addButton("Exit",[&]{gameOpen = false;});

        std::unique_ptr<Screen> screen = std::make_unique<Screen>(this,srpg_data::renderLayerUI);
        screen->addContainer(gameOver,menuLoc,menuArea);
        return screen;

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
		    movement.y--;
		}
		if(GetKey(olc::Key::DOWN).bHeld){
            movement.y++;
		}
		if(GetKey(olc::Key::LEFT).bHeld){
		    movement.x--;
		}
		if(GetKey(olc::Key::RIGHT).bHeld){
            movement.x++;
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
                inputs.target = aim.mag2() < 1 ? aim : aim.norm();
            }

        }


	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		// called once per frame
        srpg_data::timers->frameMark();
		srpg_data::timers->start("MainLoop");

		SetDrawTarget(nullptr);
        Clear(olc::BLANK);
        SetDrawTarget(srpg_data::renderLayerMenu);
        Clear(olc::BLANK);
        SetDrawTarget(srpg_data::renderLayerUI);
        Clear(olc::BLANK);
        SetDrawTarget(srpg_data::renderLayerEntities);
        Clear(olc::BLANK);
        SetDrawTarget(srpg_data::renderLayerFloor);
        Clear(olc::Pixel(0,16,0));

        // Clear all layers for new frame draw.


        //Gather input
        srpg_data::controls inputs;
		takeInput(inputs);

		if(inputs.escapeKey){
            if(state.menu == STATE::MENU::CLOSED){
                state.menu = STATE::MENU::MAIN;
                if(gamePlay){
                    state.game = STATE::GAME::PAUSED;
                }
            } else {
                state.menu = STATE::MENU::CLOSED;
                if(gamePlay){
                    state.game = STATE::GAME::PLAY;
                }
            }
        }

        if(state.menu == STATE::MENU::MAIN){
            SetDrawTarget(srpg_data::renderLayerMenu);
            menuDisplay[TITLE]->display(inputs);
            menuDisplay[TITLE]->display(inputs);
            SetDrawTarget(nullptr);
        }

        if(gamePlay){
        SetDrawTarget(srpg_data::renderLayerMenu);
        std::list<std::shared_ptr<Entity>> closeTest;
        srpg_data::gameObjects->getFoes(inputs.target,20, 5,closeTest,QuadTree::WEAK);
        for(auto ent = closeTest.begin(); ent != closeTest.end();ent++)
            srpg_data::viewer->DrawLine((*ent)->location(),inputs.target);
        }


        switch (state.game){
        case STATE::GAME::PLAY :
            gamePlay->run(fElapsedTime,inputs);
            if(gamePlay->gameOver()){
               state.game = STATE::GAME::OVER;
            }
        break;
        case STATE::GAME::PAUSED :
            gamePlay->pause();
        break;
        case STATE::GAME::OVER :
            menuDisplay[GAMEOVER]->display(inputs); //gameOverScreen->display(inputs);
        break;

        }
        if(gamePlay){
            gamePlay->gameHudDraw(inputs);
            gamePlay->draw();
        }

        SetDrawTarget(nullptr);

        srpg_data::timers->stop("MainLoop");
        if(srpg_data::debugTools)
            srpg_data::timers->drawDebug(this);
		return gameOpen; // if gameOpen becomes false this will close the program
	}


    bool OnUserDestroy(){
        //clean up memory on exit
        delete srpg_data::viewer;
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

