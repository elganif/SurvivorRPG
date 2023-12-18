#include "olcPixelGameEngine.h"
#include "olcPGEX_TransformedView.h"
#include "Rectangle.h"
#include "srpg_data.h"
#include "Menus.h"
#include <functional>

enum ActionState{
    NONE,
    HOVER,
    CLICK
};


Button::Button(olc::PixelGameEngine* game,std::string title,olc::vf2d topL, olc::vf2d area,std::function<void()> task)
                :Interactable(game,topL,area),name(title),execute(task){

}

void Button::render(srpg_data::controls& inputs){
    if(Rectangle(tl,sides).contains(inputs.UItarget)){
        srpg->FillRect(tl,sides,olc::GREEN);
        if(inputs.mainAttack){
            inputs.mainAttack = false;
            execute();
        }
    } else {
        srpg->FillRect(tl,sides,olc::BLACK);
    }
    olc::vi2d center = tl + (sides / 2);
    olc::vi2d textCorner = {center.x - (int)name.size()*4,center.y-4};
    srpg->DrawString(textCorner,name,olc::CYAN);
    srpg->DrawRect(tl,sides,olc::GREEN);
}

TitlePlate::TitlePlate(olc::PixelGameEngine* game,std::string title,olc::vf2d topL, olc::vf2d area,int fontSize = 1)
                :Interactable(game,topL,area),name(title),magnitude(fontSize){

}

void TitlePlate::render(srpg_data::controls& inputs){

    srpg->FillRect(tl,sides,olc::BLACK);

    olc::vi2d center = tl + (sides / 2);
    olc::vi2d textCorner = {center.x - (int)name.size()*4*magnitude,center.y-4*magnitude};
    srpg->DrawString(textCorner,name,olc::CYAN,magnitude);

}

MenuContainer::MenuContainer(olc::PixelGameEngine* game, olc::vf2d topL, olc::vf2d area): Interactable(game,topL,area){
    surface = new olc::Sprite(area.x,area.y);
    srpg->SetDrawTarget(surface);
    srpg->Clear(olc::VERY_DARK_BLUE);
    srpg->DrawRect(0,0,area.x,area.y,olc::DARK_BLUE);
}

void MenuContainer::render(srpg_data::controls& inputs){
    // Remeber previous draw target and set to draw to this sprite
    olc::Sprite* temp = srpg->GetDrawTarget();
    srpg->SetDrawTarget(surface);
    // If mouse is within this frame record original location and calculate offset from corner
    bool mouseHere = false;
    olc::vf2d tempaim;
    if(Rectangle(tl,sides).contains(inputs.UItarget)){
        mouseHere = true;
        tempaim = inputs.UItarget;
        inputs.UItarget -= tl;
    }
    // render each item within frame
    for(auto comp = components.begin(); comp != components.end(); comp++){

        (*comp)->render(inputs);

    }
    // set render target back to callers target and draw generated sprite
    srpg->SetDrawTarget(temp);
    srpg->DrawSprite(tl,surface);
    if (mouseHere){
        inputs.UItarget = tempaim;
    }
}

void MenuContainer::addItem(std::unique_ptr<Interactable> element){
    components.push_back(std::move(element));

}

Menu::Menu(olc::PixelGameEngine* game, olc::vf2d topL, olc::vf2d area): Interactable(game,topL,area){
    surface = new olc::Sprite(area.x,area.y);
    srpg->SetDrawTarget(surface);
    srpg->Clear(olc::VERY_DARK_BLUE);
    srpg->DrawRect(0,0,area.x,area.y,olc::DARK_BLUE);
    srpg->SetDrawTarget(nullptr);
}

void Menu::addItem(std::unique_ptr<Interactable> element){
    components.push_back(std::move(element));

}
void Menu::render(srpg_data::controls& inputs){
    // Remeber previous draw target and set to draw to this sprite
    olc::Sprite* temp = srpg->GetDrawTarget();
    srpg->SetDrawTarget(surface);
    // If mouse is within this frame record original location and calculate offset from corner
    bool mouseHere = false;
    olc::vf2d tempaim;
    if(Rectangle(tl,sides).contains(inputs.UItarget)){
        mouseHere = true;
        tempaim = inputs.UItarget;
        inputs.UItarget -= tl;
    }
    // render each item within frame
    for(auto comp = components.begin(); comp != components.end(); comp++){

        (*comp)->render(inputs);

    }

    srpg->SetDrawTarget(srpg_data::renderLayerUI);
    srpg->DrawSprite(tl,surface);
    srpg->SetDrawTarget(temp);
    if (mouseHere){
        inputs.UItarget = tempaim;
    }
};

