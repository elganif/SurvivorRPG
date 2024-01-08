#include "olcPixelGameEngine.h"
#include "olcPGEX_TransformedView.h"
#include "Rectangle.h"
#include "Entities.h"
#include "srpg_data.h"
#include "Managers.h"
#include "Engine.h"
#include "Menus.h"



GameWorld::GameWorld(float worldSize,olc::PixelGameEngine* game) : worldRadius(worldSize), srpg(game)
    {
       srpg_data::gameObjects = std::make_unique<QuadTree>(olc::vf2d(-worldRadius,-worldRadius),olc::vf2d(worldRadius*2,worldRadius*2),0);
    }
    GameWorld::~GameWorld(){
        heroicImage.reset();
    }
    bool GameWorld::gameOver(){
        return (mainChar != nullptr && !mainChar->isValid());

    }

    void GameWorld::start(){
        if(running){return;}// guard against calls to start() while game is already initialized

        mainChar = std::make_shared<Hero>(olc::vf2d(0,0),0.07,srpg,worldRadius);

        olc::vi2d Size = srpg_data::viewer->ScaleToScreen(mainChar->getBoxCollider().sides);
        heroicImage = std::shared_ptr<olc::Sprite>(new olc::Sprite(Size.x+1,Size.y+1));
        mainChar->makeRender(heroicImage,Size,srpg);
        mainChar->setRender(heroicImage);

        srpg_data::gameObjects->insertItem((std::shared_ptr<Entity>)mainChar);
        villians = std::make_unique<FoeManager>(srpg,worldRadius);
        villians->initalize(250);
        lawn =  std::make_unique<DecalManager>(srpg,worldRadius);
        lawn->initalize();

        gameHudGenerate();
        running = true;

    }

    bool GameWorld::run(float fElapsedTime, srpg_data::controls& inputs){
        /// If game is not running then nothing will update
        if(!running){
            return false;
        }
        engineTime += fElapsedTime;
        int ticks = 0;
        if ( engineTime >= tickSize && ticks <= maxTicks){
                engineTime -= tickSize;   ticks++;

            worldTime += tickSize;
            /// Move camera gradualy toward Center of world
            olc::vf2d tc = -(srpg_data::viewer->GetWorldScale() / 2);
            olc::vf2d one = {(float)srpg->ScreenWidth()/(float)srpg->ScreenHeight(),1.0};
            olc::vf2d misalign = (srpg_data::viewer->GetWorldOffset() + one) * tickSize * -10;
            srpg_data::viewer->MoveWorldOffset(misalign);

            olc::vf2d worldMove = {0,0};
            if(mainChar->isValid()){
                /// slide world and mainChar to keep player at center
                worldMove = -mainChar->getLocal();
                mainChar->placement({0,0});

            /// move world objects
                //worldMove *= tickSize;
                srpg_data::viewer->MoveWorldOffset(worldMove);

            /// update entity management containers

                mainChar->update(tickSize,inputs);
                std::list<std::shared_ptr<Entity>> impacts;
                srpg_data::gameObjects->getOverlapItems(mainChar->getBoxCollider(),impacts);
                for(auto t = impacts.begin();t != impacts.end();t++){
                    if( mainChar != (*t))
                        mainChar->onOverlap((*t));
                }
                villians->update(tickSize);

                lawn->update(tickSize);

                mainChar->eofUpdate(tickSize,worldMove);

                villians->eofUpdate(tickSize,worldMove);

                lawn->eofUpdate(tickSize,worldMove);


            }

        //srpg_data::gameObjects->clean();

        //if (engineTime > tickSize * 5){ // if game is rendering too slow skip ticks
            while (engineTime > tickSize){
                engineTime -= tickSize;
            }
        //}
        srpg_data::gameObjects->clean();
        }
        return running;
    }
    void GameWorld::draw(){
        srpg->SetDrawTarget(srpg_data::renderLayerEntities);
        //srpg->Clear(olc::BLANK);

        std::list<std::shared_ptr<Entity>> renderables;
        // Use quadTree to find all entities that have overlap with the screen space. Only these need to be drawn.
        Rectangle screen = Rectangle({srpg_data::viewer->GetWorldTL(),srpg_data::viewer->GetWorldVisibleArea()});
        srpg_data::gameObjects->getOverlapItems(screen,renderables);

        // Sort the list of items needing to be rendered ordered from top of screen to bottom of screen.
        renderables.sort([](const std::shared_ptr<Entity> f, const std::shared_ptr<Entity> s){return (f->getLocal().y+f->getSize()) < (s->getLocal().y+s->getSize()); });
        for(auto it = renderables.begin();it!= renderables.end();it++){
            (*it)->render();
        }
        if(srpg_data::debugTools){

            srpg_data::gameObjects->drawTree(screen, olc::YELLOW,olc::VERY_DARK_CYAN);
            srpg->DrawString({200,100},"items on screen:" + std::to_string(renderables.size()),olc::BLUE);
        }
        srpg->SetDrawTarget(nullptr);

    }
    void GameWorld::pause(){

    }

    std::unique_ptr<Panel> leftHUD;
    std::unique_ptr<Panel> rightHUD;
    void GameWorld::gameHudDraw(srpg_data::controls& inputs){
        //prepare areas for Side bar UI interface
        srpg->SetDrawTarget(srpg_data::renderLayerUI);

        int uiWidth = (srpg->ScreenWidth() - srpg->ScreenHeight())/2;
        leftHUD->render(inputs);
        rightHUD->render(inputs);
        //srpg->FillRect(0, 0, uiWidth, srpg->ScreenHeight(), olc::VERY_DARK_BLUE);
        //srpg->FillRect(srpg->ScreenWidth() - uiWidth, 0, srpg->ScreenHeight(), srpg->ScreenHeight(), olc::VERY_DARK_BLUE);

        std::string timer = worldTime.print();
        olc::vi2d clockpos = {(uiWidth/2) - ((int)timer.length()*4), 8};
        srpg->DrawString(clockpos,timer);

        if(srpg_data::debugTools){

            //Debug data code
            srpg->DrawString({50,80},"tot Quads:" + std::to_string(srpg_data::gameObjects->activity()),olc::BLUE);
            srpg->DrawString({50,90},"quad depth:" + std::to_string(srpg_data::gameObjects->curDepth()),olc::BLUE);
        }
        //crosshair for targeting - TODO: figure out where this code actually fits. Design proper crosshairs?
        srpg->SetDrawTarget(nullptr);
        srpg->Clear(olc::BLANK);
        srpg_data::viewer->DrawLine( inputs.target.x+0.01,inputs.target.y,inputs.target.x-0.01,inputs.target.y,olc::DARK_MAGENTA);
        srpg_data::viewer->DrawLine( inputs.target.x,inputs.target.y+0.01,inputs.target.x,inputs.target.y-0.01,olc::DARK_MAGENTA);


    }

    void GameWorld::gameHudGenerate(){
        int uiWidth = (srpg->ScreenWidth() - srpg->ScreenHeight())/2;
        int uiHeight = (srpg->ScreenHeight());

        leftHUD = std::make_unique<Panel>(srpg,olc::vi2d(0,0),olc::vi2d(uiWidth,uiHeight));





        rightHUD = std::make_unique<Panel>(srpg,olc::vi2d(srpg->ScreenWidth() - uiWidth,0),olc::vi2d(uiWidth,uiHeight));



        srpg->SetDrawTarget(nullptr);
    }




