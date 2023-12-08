

#include "olcPixelGameEngine.h"
#include "olcPGEX_TransformedView.h"
#include "Rectangle.h"
#include "Entities.h"
#include "srpg_data.h"
#include "Managers.h"

//extern QuadTree* gameObjects ;
//extern olc::TransformedView* viewer;

// class DecalManager{
    DecalManager::DecalManager( float world) : worldRadius(world){};
    DecalManager::~DecalManager(){};
    void DecalManager::initalize(olc::PixelGameEngine* game){
        srand(time(NULL));
        int failure = 0;
        float grass_size = 0.01f;
        olc::vi2d Size = srpg_data::viewer->ScaleToScreen({grass_size*2,grass_size*2});
        image = new olc::Sprite(Size.x+1,Size.y+1);
        makeRender(image,Size,game,olc::DARK_GREEN);

        while(failure < 10000){//lawn.size() < worldRadius * worldRadius * 4 * 2.5){ // 4 quadrents * # grass per unit area

            olc::vf2d attempting;
            attempting.x = worldRadius*2 * ((float)rand() / (float)RAND_MAX) - worldRadius;
            attempting.y = worldRadius*2 * ((float)rand() / (float)RAND_MAX) - worldRadius;
            //using a poison distribution
            bool valid = true;
            float toxcity = 0.0;
            for(auto const& blade : lawn){
                float distance = (attempting - blade->getLocal()).mag();
                if(distance < grass_size * 4){
                    valid = false;
                    break;
                }
                if(distance < 1.0){
                    toxcity += distance; //This is actually backwards - originaly a bug the effect is better
                    if (toxcity > 2.0){
                        valid = false;
                        break;
                    }
                }
            }
            if(valid){
                std::shared_ptr<Decoration> temp = std::shared_ptr<Decoration>(new Decoration( attempting, grass_size));
                temp->setRender(image);
                lawn.push_back( temp);
                srpg_data::gameObjects->insertItem((std::shared_ptr<Entity>)temp);

            } else {
                failure ++;
            }
        }
    }

    void DecalManager::update(float fElapsedTime,olc::vf2d movement){
        for(int i = 0;i < lawn.size();i++){
            lawn[i]->movement(movement);
            if(lawn[i]->getLocal().x > worldRadius)
                lawn[i]->movement({-worldRadius * 2,0});
            if(lawn[i]->getLocal().x < -worldRadius)
                lawn[i]->movement({worldRadius * 2,0});
            if(lawn[i]->getLocal().y > worldRadius)
                lawn[i]->movement({0, -worldRadius * 2});
            if(lawn[i]->getLocal().y < -worldRadius)
                lawn[i]->movement({0, worldRadius * 2});

		}
    }
    void DecalManager::makeRender(olc::Sprite* tSprite,olc::vf2d area,olc::PixelGameEngine* game,olc::Pixel lineColourL){
            game->SetDrawTarget(tSprite);
            game->Clear(olc::BLANK);

            game->DrawLine( 0,area.y,0,area.y*0.5f,lineColourL);
            game->DrawLine(area.x *0.5f,area.y,area.x*0.5f,0.0f,lineColourL);
            game->DrawLine(area.x,area.y,area.x,area.y*0.5f,lineColourL);

        }
    int DecalManager::size(){return lawn.size();}


//Foe Manager is designed around enemies and will maintain their numbers, stats growth and overall difficulty
// class FoeManager

    FoeManager::FoeManager( float world):worldRadius(world){};
    FoeManager::~FoeManager(){};

    //
    void FoeManager::initalize(int numFoes,olc::PixelGameEngine* game){
        float foeSize = 0.05f;
        olc::vf2d area = {foeSize*2,foeSize*2};
        olc::vi2d Size = srpg_data::viewer->ScaleToScreen(area) ;

        image = new olc::Sprite(Size.x+1,Size.y+1);
        makeRender(image,Size,game);
        //visage = std::shared_ptr<olc::Decal>(new olc::Decal(image));

        for(int i = 0; i< numFoes;i++){
            olc::vf2d attempt;
            float spawnRad = 20;
            attempt.x = spawnRad*2 * ((float)rand() / (float)RAND_MAX) - spawnRad;
            attempt.y = spawnRad*2 * ((float)rand() / (float)RAND_MAX) - spawnRad;

            std::shared_ptr<Foe> theEvil = std::shared_ptr<Foe>(new Foe(attempt,foeSize));
            theEvil->setRender(image);
            mainVillain.push_back(theEvil);

        }


        for(auto V = mainVillain.begin(); V != mainVillain.end();V++){
            srpg_data::gameObjects->insertItem((std::shared_ptr<Entity>)(*V));
        }
    }
    void FoeManager::makeRender(olc::Sprite* sprite,olc::vf2d area,olc::PixelGameEngine* game){

            game->SetDrawTarget(sprite);
            game->Clear(olc::BLANK);

            olc::Pixel colours = olc::DARK_RED;
            game->FillCircle(area.x * 0.50 ,area.y * 0.12 , area.x * 0.12 ,olc::BLACK);// Head
            game->DrawCircle(area.x * 0.50 ,area.y * 0.12 , area.x * 0.12,colours );


            game->FillRect( area.x * 0.25 ,area.y *0.25 , area.x *0.10 , area.y *0.30 ,olc::BLACK);// Left Arm
            game->DrawRect( area.x * 0.25 ,area.y *0.25 , area.x *0.10 , area.y *0.30 ,colours);

            game->FillRect(  area.x *0.65 , area.y *0.25 , area.x *0.10 , area.y *0.30 ,olc::BLACK);// Right Arm
            game->DrawRect(  area.x *0.65 , area.y *0.25 , area.x *0.10 , area.y *0.30 ,colours);

            game->FillRect( area.x *0.35 ,area.y *0.25 , area.x *0.30 , area.y *0.35 ,olc::BLACK); //torso
            game->DrawRect( area.x *0.35 ,area.y *0.25 , area.x *0.30 , area.y *0.35 ,colours);

            game->FillRect( area.x *0.38 , area.y *0.60 , area.x *0.12 , area.y *0.40 ,olc::BLACK);// Left Leg
            game->DrawRect( area.x *0.38 , area.y *0.60 , area.x *0.12 , area.y *0.40 ,colours);

            game->FillRect( area.x *0.50 , area.y *0.60 , area.x *0.12 , area.y *0.40 ,olc::BLACK);// Right Leg
            game->DrawRect( area.x *0.50 , area.y *0.60 , area.x *0.12 , area.y *0.40 ,colours);


            game->SetDrawTarget(nullptr);;
        }

    void FoeManager::update(float fElapsedTime,olc::vf2d movement){

        for( auto V = mainVillain.begin(); V!=mainVillain.end(); ){
            if((*V)->isAlive()){
                (*V)->update(fElapsedTime,movement);
                V++;
            } else { //V is not alive
                (*V).reset();
                mainVillain.erase(V++);
            }
        }
        for(int i = 0; i < 2; i++){
            for( auto V = mainVillain.begin(); V!=mainVillain.end();V++ ){
                std::list<std::shared_ptr<Entity>> impacts;
                srpg_data::gameObjects->getOverlapItems((*V)->getBoxCollider(),impacts);
                for(auto t = impacts.begin();t != impacts.end();t++){
                    (*V)->onOverlap((*t)) ;
                }
            }
        }
    }
    int FoeManager::size(){return mainVillain.size();}


