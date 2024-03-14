#include "olcPixelGameEngine.h"
#include "olcPGEX_TransformedView.h"
#include "srpg_data.h"
#include "Rectangle.h"
#include "Entities.h"
#include "Managers.h"
#include "Engine.h"
#include "Menus.h"



GameWorld::GameWorld(float worldSize,olc::PixelGameEngine* game) : worldRadius(worldSize), pge(game)
{
   srpg::gameObjects = std::make_unique<QuadTree>(olc::vf2d(-worldRadius,-worldRadius),olc::vf2d(worldRadius*2,worldRadius*2));
}

GameWorld::~GameWorld()
{
    heroicImage.reset();
    HUD.reset();
}

bool GameWorld::gameOver()
{

    return (mainChar == nullptr || !mainChar->isValid() );

}

void GameWorld::start()
{
    if(running){return;}// guard against calls to start() while game is already initialized

    mainChar = std::make_shared<Hero>(olc::vf2d(0,0),0.07,pge,worldRadius);

    olc::vi2d Size = srpg::viewer->ScaleToScreen(mainChar->getBoxCollider().sides);
    heroicImage = std::make_shared<olc::Sprite>(Size.x+1,Size.y+1);
    mainChar->makeRender(heroicImage,Size,pge);
    mainChar->setRender(heroicImage);
    srpg::gameObjects->insertItem((std::shared_ptr<Entity>)mainChar);

    villians = std::make_unique<FoeManager>(pge,worldRadius);
    villians->initalize(250);
    lawn =  std::make_unique<DecalManager>(pge,worldRadius);
    lawn->initalize();

    gameHudGenerate();
    running = true;

    /// initialize games epoch time and first frame time
    epochTime = std::chrono::_V2::steady_clock::now();
    engineClock = frame{0};
}

bool GameWorld::run(float fElapsedTime, srpg::controls& inputs)
{
    std::chrono::time_point fStart = std::chrono::_V2::steady_clock::now();

    while(engineClock < std::chrono::_V2::steady_clock::now() - epochTime
        && frame{10} > std::chrono::_V2::steady_clock::now() - fStart ){

        srpg::timers->start("gameTick");

        engineClock++;
        float tickSize = std::chrono::duration<float>(frame{1}).count();

        /// Move camera gradualy toward Center of world
        olc::vf2d tc = {(float)pge->ScreenWidth()/(float)pge->ScreenHeight(),1.0};
        olc::vf2d misalign = (srpg::viewer->GetWorldOffset() + tc) * tickSize * -10;
        srpg::viewer->MoveWorldOffset(misalign);

        olc::vf2d worldMove = {0,0};
        if(mainChar->isValid()){
            /// slide world and mainChar to keep player at center
            worldMove = -mainChar->location();
            mainChar->location({0,0});

        /// update entity management containers
            std::list<std::shared_ptr<Entity>> impacts;
            srpg::gameObjects->getOverlapItems(mainChar->getBoxCollider(),impacts);
            for(std::shared_ptr<Entity> other : impacts){
                mainChar->onOverlap(other);
            }
            villians->collision();

            mainChar->update(tickSize,worldMove,inputs);
            villians->update(tickSize,worldMove);

            lawn->update(tickSize,worldMove);
        }
        srpg::timers->stop("gameTick");
    }
    /// clean up empty nodes in quad tree
    srpg::gameObjects->clean();

    /// update current time
    frameTime =  std::chrono::_V2::steady_clock::now();

    return running;
}

void GameWorld::pause()
{
    /// advance the epoch time forward by the amount of time passing per frame so timer is not running ahead of game.
    std::chrono::_V2::steady_clock::time_point newTime =  std::chrono::_V2::steady_clock::now();
    epochTime += newTime - frameTime;
    frameTime = newTime;
}

void GameWorld::draw()
{
    srpg::timers->start("Render");
    pge->SetDrawTarget(srpg::renderLayerEntities);

    std::list<std::shared_ptr<Entity>> renderables;
    /// Use quadTree to find all entities that have overlap with the screen space. Only these need to be drawn.
    Rectangle screen = Rectangle({srpg::viewer->GetWorldTL(),srpg::viewer->GetWorldVisibleArea()});
    srpg::gameObjects->getOverlapItems(screen,renderables);

    /// Sort the list of items needing to be rendered ordered from top of screen to bottom of screen.
    renderables.sort([](const std::shared_ptr<Entity> f, const std::shared_ptr<Entity> s){return (f->location().y+f->getSize()) < (s->location().y+s->getSize()); });
    for(auto& it : renderables){
        it->render();
    }
    pge->SetDrawTarget(nullptr);
    srpg::timers->stop("Render");
}


std::string timeIntoString(std::chrono::duration<double> time){
    double seconds = time.count();
    int intTime = 0;

    enum clockUnit{HOUR,MIN,SEC,NONE};
    clockUnit unit = NONE;
    /// determing what the largets unit in use is.
    if(seconds >= 1.0f){
        unit = SEC;
    }
    if(seconds >= 60.0){
        unit = MIN;
    }
    if(seconds >= 3600.0){
        unit = HOUR;
    }

    /// calculat and convert each signifigant unit in use to a string and assemble the final result
    /// Switch to decide where to start, once any givin unit is in use all smaller units will be needed.
    std::string result = "";
    std::string addon = "";
    switch (unit){
    case HOUR :
        intTime = std::chrono::duration_cast<std::chrono::hours>(time).count();
        result += std::to_string(intTime) + ":";
    case MIN :
        intTime = std::chrono::duration_cast<std::chrono::minutes>(time).count();
        intTime = intTime % 60;
        addon = std::to_string(intTime);
        if(result.size() > 0 && addon.size() == 1){
            addon = "0" + addon;
        }
        result += addon + ":";
    case SEC:
        intTime = std::chrono::duration_cast<std::chrono::seconds>(time).count();
        intTime = intTime % 60;
        addon = std::to_string(intTime);
        if(result.size() > 0 && addon.size() == 1){
            addon = "0" + addon;
        }
        result += addon;
    default : /// add milliseconds. This is always needed
        float subSec = std::chrono::duration<float>(time).count();
        addon = std::to_string(subSec);
        addon = addon.substr(addon.find(".") , 3);
        result += addon;
    }

    return result;
}

void GameWorld::gameHudDraw(srpg::controls& inputs){
    ///prepare areas for Side bar UI interface
    pge->SetDrawTarget(srpg::renderLayerUI);
    HUD->display(inputs);

    ///crosshair for targeting - TODO: figure out where this code actually fits. Design proper crosshairs or cursor.
    pge->SetDrawTarget(nullptr);
    srpg::viewer->DrawLine( inputs.target.x+0.01,inputs.target.y,inputs.target.x-0.01,inputs.target.y,olc::DARK_MAGENTA);
    srpg::viewer->DrawLine( inputs.target.x,inputs.target.y+0.01,inputs.target.x,inputs.target.y-0.01,olc::DARK_MAGENTA);
}

void GameWorld::gameHudGenerate(){
    int uiWidth = (pge->ScreenWidth() - pge->ScreenHeight())/2;
    int uiHeight = (pge->ScreenHeight());

    HUD = std::make_unique<Screen>(pge,srpg::renderLayerUI);

    std::unique_ptr<UIContainer> leftHUD = std::make_unique<UIContainer>(pge,olc::vi2d(uiWidth,uiHeight));
    leftHUD->editContainerElement()->background(olc::DARK_BLUE);
    HUD->addContainer(leftHUD,olc::vi2d(0,0),olc::vi2d(uiWidth,uiHeight));

    std::unique_ptr<UIContainer> rightHUD = std::make_unique<UIContainer>(pge,olc::vi2d(uiWidth,uiHeight),olc::vi2d(1,10));
    rightHUD->addElement(olc::vi2d(0,0))->addDynamicText([&](){return timeIntoString(engineClock); },olc::CYAN,14,RIGHT).background(olc::DARK_BLUE);
    rightHUD->editContainerElement()->background(olc::DARK_BLUE);
    HUD->addContainer(rightHUD,olc::vi2d(pge->ScreenWidth() - uiWidth,0),olc::vi2d(uiWidth,uiHeight));

}




