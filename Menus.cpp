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
                :UIElement(game,topL,area),name(title),execute(task){
    olc::Sprite* oldDrawTarget = srpg->GetDrawTarget();

    olc::vi2d center = sides / 2;
    olc::vi2d textCorner = {center.x - (int)name.size()*4,center.y-4};

    srpg->SetDrawTarget(surface);
    srpg->Clear(olc::BLACK);
    srpg->DrawRect(0,0,sides.x-1,sides.y-1,olc::GREEN);
    srpg->DrawString(textCorner,name,olc::CYAN);

    srpg->SetDrawTarget(oldDrawTarget);
}

olc::vi2d Button::render(srpg_data::controls& inputs){
    if(Rectangle(tl,sides).contains(inputs.UItarget)){
        /// If the cursor is within this button area and click is detected then execute designated function.
        if(inputs.mainAttack){
            inputs.mainAttack = false;
            execute();
        }
    }
    srpg->DrawSprite(tl,surface);

    return sides;
}

TitlePlate::TitlePlate(olc::PixelGameEngine* game,std::string title,olc::vf2d topL, olc::vf2d area,int fontSize = 1)
                :UIElement(game,topL,area),name(title),magnitude(fontSize){
    olc::Sprite* oldDrawTarget = srpg->GetDrawTarget();

    //surface = new olc::Sprite(sides.x,sides.y);
    srpg->SetDrawTarget(surface);
    srpg->Clear(olc::BLANK);
    olc::vi2d center = sides / 2;
    olc::vi2d textCorner = {center.x - (int)name.size()*4*magnitude,center.y-4*magnitude};
    srpg->DrawString(textCorner,name,olc::CYAN,magnitude);

    srpg->SetDrawTarget(oldDrawTarget);
}

olc::vi2d TitlePlate::render(srpg_data::controls& inputs){


    srpg->DrawSprite(tl,surface);
    return sides;
}

MenuContainer::MenuContainer(olc::PixelGameEngine* game, olc::vf2d topL, olc::vf2d area): UIElement(game,topL,area){
    //surface = new olc::Sprite(area.x,area.y);
    olc::Sprite* oldDrawTarget = srpg->GetDrawTarget();
    srpg->SetDrawTarget(surface);
    srpg->Clear(olc::VERY_DARK_BLUE);
    srpg->DrawRect(0,0,area.x,area.y,olc::DARK_BLUE);
    srpg->SetDrawTarget(oldDrawTarget);
}

olc::vi2d MenuContainer::render(srpg_data::controls& inputs){
    // Remeber previous draw target and set to draw to this sprite
    olc::Sprite* temp = srpg->GetDrawTarget();
    srpg->SetDrawTarget(surface);
    // If mouse is within this frame record original location and calculate offset from corner

    olc::vf2d tempaim;
    /// store mouse location and set it to be in relation to this container
    tempaim = inputs.UItarget;
    inputs.UItarget -= tl;

    // render each item within frame
    for(auto comp = components.begin(); comp != components.end(); comp++){

        (*comp)->render(inputs);

    }
    /// set render target back to callers target and draw generated sprite
    srpg->SetDrawTarget(temp);
    srpg->DrawSprite(tl,surface);

    inputs.UItarget = tempaim;


    return sides;
}

void MenuContainer::addItem(std::unique_ptr<UIElement> element){
    components.push_back(std::move(element));

}

Menu::Menu(olc::PixelGameEngine* game, olc::vf2d topL, olc::vf2d area): UIElement(game,topL,area){
    olc::Sprite* oldDrawTarget = srpg->GetDrawTarget();
    srpg->SetDrawTarget(surface);
    srpg->Clear(olc::VERY_DARK_BLUE);
    srpg->DrawRect(0,0,area.x-1,area.y-1,olc::DARK_BLUE);
    srpg->SetDrawTarget(oldDrawTarget);
}

void Menu::addItem(std::unique_ptr<UIElement> element){
    components.push_back(std::move(element));

}
olc::vi2d Menu::render(srpg_data::controls& inputs){
    // Remeber previous draw target and set to draw to this sprite
    olc::Sprite* oldDrawTarget = srpg->GetDrawTarget();
    srpg->SetDrawTarget(surface);

    /// store mouse location and set it to be in relation to this container
    olc::vf2d tempaim;
    tempaim = inputs.UItarget;
    inputs.UItarget -= tl;

    // render each item within frame
    for(auto comp = components.begin(); comp != components.end(); comp++){

        (*comp)->render(inputs);

    }

    /// Set render to the UI layer, draw the sprite for this surface and reset to stored draw target
    srpg->SetDrawTarget(srpg_data::renderLayerUI);
    srpg->DrawSprite(tl,surface);
    srpg->SetDrawTarget(oldDrawTarget);

    inputs.UItarget = tempaim;

    return sides;
};

