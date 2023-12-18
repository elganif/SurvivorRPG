#include "olcPixelGameEngine.h"
#include "olcPGEX_TransformedView.h"
#include "Rectangle.h"
#include "Entities.h"
#include "srpg_data.h"
#include "Managers.h"
#include "Engine.h"



GameWorld::GameWorld(float worldSize,olc::PixelGameEngine* game) : worldRadius(worldSize), srpg(game)
    {
       srpg_data::gameObjects = std::make_unique<QuadTree>(olc::vf2d(-worldRadius,-worldRadius),olc::vf2d(worldRadius*2,worldRadius*2),0);
    }
    GameWorld::~GameWorld(){
        delete heroicImage;
    }
    bool GameWorld::gameOver(){
        return (mainChar != nullptr && !mainChar->isAlive());

    }

    void GameWorld::start(){
        if(running){return;}// guard against calls to start() while game is already initialized

        mainChar = std::make_shared<Hero>(Hero({0,0},0.07));

        olc::vi2d Size = srpg_data::viewer->ScaleToScreen(mainChar->getBoxCollider().sides);
        heroicImage = new olc::Sprite(Size.x+1,Size.y+1);
        mainChar->makeRender(heroicImage,Size,srpg);
        mainChar->setRender(heroicImage);

        srpg_data::gameObjects->insertItem((std::shared_ptr<Entity>)mainChar);
        villians = std::make_unique<FoeManager>(worldRadius);
        villians->initalize(250,srpg);
        lawn =  std::make_unique<DecalManager>(worldRadius);
        lawn->initalize(srpg);
        running = true;

    }
    bool GameWorld::run(float fElapsedTime, srpg_data::controls& inputs){
        /// If game is not running then nothing will update
        if(!running){
            return running;
        }

        engineTime += fElapsedTime;
        int ticks = 0;
        while ( engineTime >= tickSize && ticks <= maxTicks){
                engineTime -= tickSize;   ticks++;

            /// Move camera gradualy toward Center of world
            olc::vf2d one = {(float)srpg->ScreenWidth()/(float)srpg->ScreenHeight(),1.0};
            olc::vf2d misalign = (srpg_data::viewer->GetWorldOffset() + one) * tickSize * -10;
            srpg_data::viewer->MoveWorldOffset(misalign);

            olc::vf2d movement = {0,0};
            if(mainChar->isAlive()){
                /// slide world and mainChar to keep player at center
                movement = mainChar->getLocal() + inputs.movement;
                mainChar->placement({0,0});

            /// move world objects
                movement *= tickSize;
                srpg_data::viewer->MoveWorldOffset(movement);

            /// update entity management containers

                mainChar->update(tickSize,movement);

                villians->update(tickSize,movement);
                lawn->update(tickSize,movement);



                std::shared_ptr<Projectile> temp;
                olc::vf2d target = inputs.target;
                if(inputs.aim.mag() > 0.5f){
                        inputs.rapidFire = true;
                        target = inputs.aim.norm();
                }
                if(inputs.mainAttack && mainChar->projectileReady()){
                        mainChar->fireProjectile(target,bullets,srpg);
                        inputs.mainAttack = false;

                }
                while(inputs.rapidFire && mainChar->fireProjectile(target,bullets,srpg)){
                     // TODO: code to update each bullet as its fired to compensate for time passing between shots within a single tick.
                }

                // Loop through and process all bullet objects
                auto entity = bullets.begin();

                while(entity != bullets.end()){
                    // check for bullet being dead, if dead erase and move on
                    if (!(*entity)->isAlive()){
                        bullets.erase(entity++);
                        continue;
                    } // else entity is alive
                    // call the tick update on each bullet
                    (*entity)->update(tickSize,movement);
                    entity++;
                }
            }

        srpg_data::gameObjects->validateLocations();
        if (ticks == maxTicks){ // if game is rendering too slow dont store extra game engine time.
                engineTime = 0.0f;
        }

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
        srpg->DrawString({200,100},"items on screen:" + std::to_string(renderables.size()),olc::BLUE);
        srpg->SetDrawTarget(nullptr);

    }
    void GameWorld::pause(){

    }


