#include "olcPixelGameEngine.h"
#include "olcPGEX_TransformedView.h"
#include "Rectangle.h"
#include "Entities.h"
#include "srpg_data.h"
#include "Managers.h"
#include "Engine.h"



GameWorld::GameWorld(float worldSize,olc::PixelGameEngine* game) : worldRadius(worldSize), srpg(game)
    {
       srpg_data::gameObjects = new QuadTree({-worldRadius,-worldRadius},{worldRadius*2,worldRadius*2},0);
    }
    GameWorld::~GameWorld(){}
    void GameWorld::start(){
        mainChar = std::shared_ptr<Hero> (new Hero({0,0},0.07));

        olc::vi2d Size = srpg_data::viewer->ScaleToScreen(mainChar->getBoxCollider().sides);
        olc::Sprite* heroicImage = new olc::Sprite(Size.x+1,Size.y+1);
        mainChar->makeRender(heroicImage,Size,srpg);
        mainChar->setRender(heroicImage);

        srpg_data::gameObjects->insertItem((std::shared_ptr<Entity>)mainChar);
        villians = std::unique_ptr<FoeManager>(new FoeManager(worldRadius));
        villians->initalize(100,srpg);
        lawn =  std::unique_ptr<DecalManager>(new DecalManager(worldRadius));
        lawn->initalize(srpg);
    }
    void GameWorld::run(float fElapsedTime, srpg_data::controls& inputs){
        // Gradually move view to center on Player
        olc::vf2d one = {(float)srpg->ScreenWidth()/(float)srpg->ScreenHeight(),1.0};
		olc::vf2d misalign = (srpg_data::viewer->GetWorldOffset() + one) * fElapsedTime * -10;
        srpg_data::viewer->MoveWorldOffset(misalign);

        olc::vf2d movement = mainChar->getLocal() + inputs.movement;
        mainChar->placement({0,0}); // slide world and mainChar to keep player at center

        //move world objects
        movement *= fElapsedTime;
        srpg_data::viewer->MoveWorldOffset(movement);

        //update entity management containers
        mainChar->update(fElapsedTime,movement);
        villians->update(fElapsedTime,movement);
        lawn->update(fElapsedTime,movement);


		// Shooty buttons for testing
        std::shared_ptr<Projectile> temp;
		if(inputs.mainAttack && mainChar->projectileReady()){
            mainChar->fireProjectile(inputs.target,bullets,srpg);

		}
		while(inputs.rapidFire && mainChar->fireProjectile(inputs.target,bullets,srpg)){

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

    }
    void GameWorld::pause(){

    }


