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

std::unique_ptr<QuadTree> srpg::gameObjects = nullptr;
std::unique_ptr<olc::TransformedView> srpg::viewer = nullptr;
std::unique_ptr<olc::GamePad> controller = nullptr;

uint8_t srpg::renderLayerFloor;
uint8_t srpg::renderLayerEntities;
uint8_t srpg::renderLayerUI;
uint8_t srpg::renderLayerMenu;

std::unique_ptr<Profiler> srpg::timers;

class SurvivorRPG : public olc::PixelGameEngine
{
private:
bool gameOpen = true;
float screenRatio;
float worldRadius = 10.0;


enum Menus{
    TITLE,
    OPTION,
    GAMEOVER
};

std::unordered_map<Menus, std::unique_ptr<Screen>> menuDisplay;
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
        //create needed rendering layers front to back
        SetPixelMode(olc::Pixel::MASK);
        Clear(olc::BLANK);
        srpg::renderLayerMenu = CreateLayer();
        EnableLayer(srpg::renderLayerMenu,true);

        srpg::renderLayerUI = CreateLayer();
        EnableLayer(srpg::renderLayerUI,true);

        srpg::renderLayerEntities = CreateLayer();
        EnableLayer(srpg::renderLayerEntities,true);

        srpg::renderLayerFloor = CreateLayer();
        EnableLayer(srpg::renderLayerFloor,true);


        screenRatio = (float)ScreenWidth()/(float)ScreenHeight();
        olc::vf2d screentl = {(float)ScreenWidth(),(float)ScreenHeight()};
        olc::vf2d screenscale = {ScreenHeight()/2.0f,ScreenHeight()/2.0f};
		srpg::viewer = std::make_unique<olc::TileTransformedView>(screentl,screenscale);
		srpg::viewer->MoveWorldOffset({-1*screenRatio,-1});

        // Object inialization

        menuDisplay.emplace(TITLE,constructMain() );

        menuDisplay.emplace(GAMEOVER ,constructGameOver());

        srpg::timers = std::make_unique<Profiler>();
		return true;
	}

    std::unique_ptr<Screen> constructMain(){
        UI::theme titletheme =  {olc::CYAN ,olc::BLANK ,olc::BLANK ,olc::BLANK};
        UI::theme buttontheme = {olc::CYAN ,olc::DARK_BLUE ,olc::DARK_BLUE ,olc::BLUE};
        olc::vf2d titleArea = {ScreenWidth() * 0.35f, ScreenHeight() *0.2f};
        olc::vf2d titleLoc = olc::vi2d(ScreenWidth() * 0.5f, ScreenHeight() *0.25f) - titleArea/2;


        std::unique_ptr<UIContainer> title = std::make_unique<UIContainer>(this,titleArea,olc::vi2d(1,2));

        title->addElement({0,0})->text("Roles of",titletheme.text);
        title->addElement({0,1})->text("Survival",titletheme.text);

        std::unique_ptr<Screen> screen = std::make_unique<Screen>(this,srpg::renderLayerMenu);
        screen->addContainer(title,titleLoc,titleArea);

        olc::vf2d menuArea = {ScreenWidth() * 0.2f, ScreenHeight() * 0.16f};
        olc::vf2d menuLoc = olc::vf2d(ScreenWidth() *0.5f , ScreenHeight() * 0.6f) - menuArea/2;

        std::unique_ptr<UIContainer> main = std::make_unique<UIContainer>(this,menuArea,olc::vi2d(1,3));

        main->addElement({0,0})->text("START",buttontheme.text).background(buttontheme.bg)
                                .addButton([&]{if(state.game == STATE::GAME::NONE){
                                        gamePlay = std::make_unique<GameWorld>(worldRadius,this);
                                        gamePlay->start();
                                        }
                                        if(gamePlay){
                                        state.game = STATE::GAME::PLAY;
                                        state.menu = STATE::MENU::CLOSED;
                                        }}
                                    );



        main->addElement({0,1})->text("RESTART",buttontheme.text).background(buttontheme.bg)
                        .addButton( [&]{gamePlay = std::make_unique<GameWorld>(worldRadius,this);
                                    gamePlay->start();
                                    state.game = STATE::GAME::PLAY;
                                    state.menu = STATE::MENU::CLOSED;
                                });

        main->addElement({0,2})->text("EXIT",buttontheme.text).background(buttontheme.bg)
                             .addButton([&]{gameOpen = false;} );

        screen->addContainer(main,menuLoc,menuArea);

        return screen;

    }

    std::unique_ptr<Screen> constructGameOver(){
        olc::vf2d menuArea = {ScreenHeight() * 0.5f , ScreenHeight() * 0.5f};
        olc::vf2d menuLoc = olc::vf2d(ScreenWidth() * 0.5f, ScreenHeight() * 0.5f) - (menuArea / 2);

        std::unique_ptr<Screen> screen = std::make_unique<Screen>(this,srpg::renderLayerUI);


        std::unique_ptr<UIContainer> gameOver = std::make_unique<UIContainer>(this,menuArea,olc::vi2d(1,7));
        UI::theme gotheme = {olc::CYAN ,olc::DARK_BLUE ,olc::DARK_BLUE ,olc::BLUE };
        gameOver->editContainerElement()->background(gotheme.bg);

        gameOver->addElement({0,0},{1,2})->text("GAME",gotheme.text).background(gotheme.bg);

        gameOver->addElement({0,2},{1,2})->text("OVER",gotheme.text).background(gotheme.bg);

        gameOver->addElement({0,4})->text("Main Menu",gotheme.text).background(gotheme.bg)
                                        .addButton(
                                        [&]{gamePlay.reset();
                                        state.menu = STATE::MENU::MAIN;
                                        state.game = STATE::GAME::NONE;
                                        });

        gameOver->addElement({0,5})->text("Restart",gotheme.text).background(gotheme.bg)
                                    .addButton(
                                    [&]{gamePlay = std::make_unique<GameWorld>(worldRadius,this);
                                    gamePlay->start();
                                    state.menu = STATE::MENU::CLOSED;
                                    state.game = STATE::GAME::PLAY;
                                    });

        gameOver->addElement({0,6})->text("Exit",gotheme.text).background(gotheme.bg)
                                    .addButton([&]{gameOpen = false;});


        screen->addContainer(gameOver,menuLoc,menuArea);
        return screen;

    }

    // collects key inputs from user and returns The desired actions. ::todo make extendable for more commands, and figure out remapable inputs.
	void takeInput(srpg::controls& inputs)
	{
        bool gamepad;
        if(controller == nullptr || !controller->stillConnected){
            controller = std::unique_ptr<olc::GamePad>(olc::GamePad::selectWithAnyButton());
            gamepad = false;
        } else {
            gamepad = true;
        }

        inputs.escapeKey = GetKey(olc::Key::ESCAPE).bPressed;

	    inputs.mainAttack = GetMouse(olc::Mouse::LEFT).bPressed;
        inputs.rapidFire = GetMouse(olc::Mouse::RIGHT).bHeld;
        inputs.target = srpg::viewer->ScreenToWorld({(float)GetMouseX(),(float)GetMouseY()});
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
            movement.x = (controller->getAxis(olc::GPAxes::LX)-0.5f) * 2;
            movement.y = (controller->getAxis(olc::GPAxes::LY)-0.5f) * 2;
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
        srpg::timers->frameMark();
		srpg::timers->start("MainLoop");

		SetDrawTarget(nullptr);
        Clear(olc::BLANK);
        SetDrawTarget(srpg::renderLayerMenu);
        Clear(olc::BLANK);
        SetDrawTarget(srpg::renderLayerUI);
        Clear(olc::BLANK);
        SetDrawTarget(srpg::renderLayerEntities);
        Clear(olc::BLANK);
        SetDrawTarget(srpg::renderLayerFloor);
        Clear(olc::Pixel(0,16,0));

        // Clear all layers for new frame draw.


        //Gather input
        srpg::controls inputs;
		takeInput(inputs);

		if(inputs.escapeKey){

            if(gamePlay){
                /// if we have a game open we switch menu state, and set game to play if menus are all closed, pause the game otherwise.
                state.menu = state.menu == STATE::MENU::CLOSED ? STATE::MENU::MAIN : STATE::MENU::CLOSED;
            } else {
            /// if no game just ensure main menu is opened - eventually will be a main open or go back 1 level check
                state.menu = STATE::MENU::MAIN;
            }
        }

        if(state.menu == STATE::MENU::MAIN){
            SetDrawTarget(srpg::renderLayerMenu);
            menuDisplay[TITLE]->display(inputs);
            SetDrawTarget(nullptr);
        }

        if(gamePlay){
        SetDrawTarget(srpg::renderLayerMenu);
        std::list<std::shared_ptr<Entity>> closeTest;
        srpg::gameObjects->getFoes(inputs.target,20, 5,closeTest,QuadTree::CLOSE);
        for(auto& ent : closeTest)
            srpg::viewer->DrawLine(ent->location(),inputs.target);
        }

        switch (state.game){
            case STATE::GAME::PLAY :
                if(state.menu == STATE::MENU::CLOSED){
                    gamePlay->run(fElapsedTime,inputs);
                } else {
                    gamePlay->pause();
                }
                if(gamePlay->gameOver()){
                   state.game = STATE::GAME::OVER;
                }
            break;
            case STATE::GAME::OVER :
                menuDisplay[GAMEOVER]->display(inputs);
            break;
        }

        if(gamePlay){
            gamePlay->gameHudDraw(inputs);
            gamePlay->draw();
        }

        SetDrawTarget(nullptr);

        srpg::timers->stop("MainLoop");
        if(srpg::debugTools){
            srpg::timers->drawDebug(this);
            if(srpg::gameObjects && false){
                Rectangle screen = Rectangle({srpg::viewer->GetWorldTL(),srpg::viewer->GetWorldVisibleArea()});
                srpg::gameObjects->drawTree(screen,olc::YELLOW,olc::RED);
            }
        }
		return gameOpen; // if gameOpen becomes false this will close the program
	}


    bool OnUserDestroy(){
        //clean up memory on exit
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

