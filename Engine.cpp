#include "olcPixelGameEngine.h"
#include "olcPGEX_TransformedView.h"
#include "srpg_data.h"
#include "Rectangle.h"
#include "Entities.h"
#include "Managers.h"
#include "Engine.h"
#include "Menus.h"



GameWorld::GameWorld(float worldSize,olc::PixelGameEngine* game) : worldRadius(worldSize), srpg(game)
{
   srpg_data::gameObjects = std::make_unique<QuadTree>(olc::vf2d(-worldRadius,-worldRadius),olc::vf2d(worldRadius*2,worldRadius*2));
}

GameWorld::~GameWorld()
{
    heroicImage.reset();
    HUD.reset();
}

bool GameWorld::gameOver()
{
    return (mainChar != nullptr && !mainChar->isValid());

}

void GameWorld::start()
{
    if(running){return;}// guard against calls to start() while game is already initialized

    mainChar = std::make_shared<Hero>(olc::vf2d(0,0),0.07,srpg,worldRadius);

    olc::vi2d Size = srpg_data::viewer->ScaleToScreen(mainChar->getBoxCollider().sides);
    heroicImage = std::make_shared<olc::Sprite>(Size.x+1,Size.y+1);
    mainChar->makeRender(heroicImage,Size,srpg);
    mainChar->setRender(heroicImage);

    srpg_data::gameObjects->insertItem((std::shared_ptr<Entity>)mainChar);
    villians = std::make_unique<FoeManager>(srpg,worldRadius);
    villians->initalize(250);
    lawn =  std::make_unique<DecalManager>(srpg,worldRadius);
    lawn->initalize();

    gameHudGenerate();
    running = true;

    /// initialize games epoch time and first frame time
    epochTime = std::chrono::_V2::steady_clock::now();
    frameTime =  std::chrono::_V2::steady_clock::now();

}

bool GameWorld::run(float fElapsedTime, srpg_data::controls& inputs)
{
    engineTime += fElapsedTime;
    int ticks = 0;
    while ( engineTime >= tickSize ){
            engineTime -= tickSize;
        srpg_data::timers->start("gameTick");

        /// Move camera gradualy toward Center of world
        olc::vf2d tc = -(srpg_data::viewer->GetWorldScale() / 2);
        olc::vf2d one = {(float)srpg->ScreenWidth()/(float)srpg->ScreenHeight(),1.0};
        olc::vf2d misalign = (srpg_data::viewer->GetWorldOffset() + one) * tickSize * -10;
        srpg_data::viewer->MoveWorldOffset(misalign);

        olc::vf2d worldMove = {0,0};
        if(mainChar->isValid()){
            /// slide world and mainChar to keep player at center
            worldMove = -mainChar->location();
            mainChar->location({0,0});

        /// move world objects
            srpg_data::viewer->MoveWorldOffset(worldMove);

        /// update entity management containers
            srpg_data::timers->start("collide");
            std::list<std::shared_ptr<Entity>> impacts;
            srpg_data::gameObjects->getOverlapItems(mainChar->getBoxCollider(),impacts);
            for(std::shared_ptr<Entity> other : impacts){ //auto t = impacts.begin();t != impacts.end();t++){
                mainChar->onOverlap(other);
            }
            villians->collision();

            srpg_data::timers->stop("collide");
            srpg_data::timers->start("update");
            mainChar->update(tickSize,worldMove,inputs);
            villians->update(tickSize,worldMove);

            lawn->update(tickSize,worldMove);

            srpg_data::timers->stop("update");

        }
        float tickTimer = srpg_data::timers->stop("gameTick");
        if(tickTimer > tickSize){
//            engineTime -= tickSize;
//            std::chrono::duration<float> chronoTick {tickSize};
//            epochTime += std::chrono::duration_cast<std::chrono::seconds>(chronoTick);
        }
    }
    srpg_data::gameObjects->clean();

    /// update current time
    frameTime =  std::chrono::_V2::steady_clock::now();


    return running;
}
void GameWorld::draw()
{
    srpg->SetDrawTarget(srpg_data::renderLayerEntities);
    //srpg->Clear(olc::BLANK);

    std::list<std::shared_ptr<Entity>> renderables;
    // Use quadTree to find all entities that have overlap with the screen space. Only these need to be drawn.
    Rectangle screen = Rectangle({srpg_data::viewer->GetWorldTL(),srpg_data::viewer->GetWorldVisibleArea()});
    srpg_data::gameObjects->getOverlapItems(screen,renderables);

    // Sort the list of items needing to be rendered ordered from top of screen to bottom of screen.
    renderables.sort([](const std::shared_ptr<Entity> f, const std::shared_ptr<Entity> s){return (f->location().y+f->getSize()) < (s->location().y+s->getSize()); });
    for(auto it = renderables.begin();it!= renderables.end();it++){
        (*it)->render();
    }
    srpg->SetDrawTarget(nullptr);
}

void GameWorld::pause()
{
    /// advance the epoch time forward by the amount of time passing per frame so timer is not running ahead of game.
    std::chrono::_V2::steady_clock::time_point newTime =  std::chrono::_V2::steady_clock::now();
    epochTime += newTime - frameTime;
    frameTime = newTime;
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
        result += addon + ".";
    default : /// add milliseconds. This is always needed
        intTime = std::chrono::duration_cast<std::chrono::microseconds>(time).count();
        intTime = intTime % 1000;
        addon = std::to_string(intTime);
        while(result.size() > 0 && addon.size() < 3){
            addon = "0" + addon;
        }
        result += addon;
    }

    return result;
}

void GameWorld::gameHudDraw(srpg_data::controls& inputs){
    ///prepare areas for Side bar UI interface
    srpg->SetDrawTarget(srpg_data::renderLayerUI);

    int uiWidth = (srpg->ScreenWidth() - srpg->ScreenHeight())/2;
    HUD->display(inputs);

    /// calculate time running based on difference between epoch and end of last update
    std::chrono::duration<double> time = frameTime - epochTime;
    std::string timer = timeIntoString(time);
    olc::vi2d clockpos = {(uiWidth/2) - ((int)timer.length()*4), 8};
    srpg->DrawString(clockpos,timer);

    ///crosshair for targeting - TODO: figure out where this code actually fits. Design proper crosshairs or cursor.
    srpg->SetDrawTarget(nullptr);
    srpg_data::viewer->DrawLine( inputs.target.x+0.01,inputs.target.y,inputs.target.x-0.01,inputs.target.y,olc::DARK_MAGENTA);
    srpg_data::viewer->DrawLine( inputs.target.x,inputs.target.y+0.01,inputs.target.x,inputs.target.y-0.01,olc::DARK_MAGENTA);
}

void GameWorld::gameHudGenerate(){
    int uiWidth = (srpg->ScreenWidth() - srpg->ScreenHeight())/2;
    int uiHeight = (srpg->ScreenHeight());
    HUD = std::make_unique<Screen>(srpg,srpg_data::renderLayerUI);

    std::unique_ptr<UIContainer> leftHUD = std::make_unique<UIContainer>(srpg,olc::vi2d(uiWidth,uiHeight));
    leftHUD->setTheme(olc::CYAN ,olc::DARK_BLUE ,olc::DARK_BLUE ,olc::DARK_GREEN );
    HUD->addContainer(leftHUD,olc::vi2d(0,0),olc::vi2d(uiWidth,uiHeight));

    std::unique_ptr<UIContainer> rightHUD = std::make_unique<UIContainer>(srpg,olc::vi2d(uiWidth,uiHeight));
    rightHUD->setTheme(olc::CYAN ,olc::DARK_BLUE ,olc::DARK_BLUE ,olc::DARK_GREEN );
    rightHUD->addDynamicText([&](){return timeIntoString(frameTime - epochTime); } ,14,UI::RIGHT );
    HUD->addContainer(rightHUD,olc::vi2d(srpg->ScreenWidth() - uiWidth,0),olc::vi2d(uiWidth,uiHeight));

}




