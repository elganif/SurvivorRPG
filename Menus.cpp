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

/// class Button
Button::Button(olc::PixelGameEngine* game,std::string title, olc::vf2d area,std::function<void()> task)
                :UIElement(game,area),name(title),execute(task){
    olc::Sprite* oldDrawTarget = srpg->GetDrawTarget();

    olc::vi2d center = sides / 2;
    olc::vi2d textCorner = {center.x - (int)name.size()*4,center.y-4};

    srpg->SetDrawTarget(surface.get());
    srpg->Clear(olc::BLACK);
    srpg->DrawRect(0,0,sides.x-1,sides.y-1,olc::GREEN);
    srpg->DrawString(textCorner,name,olc::CYAN);

    srpg->SetDrawTarget(oldDrawTarget);
}

olc::vi2d Button::render(olc::vi2d tl,srpg_data::controls& inputs){
    if(Rectangle(tl,sides).contains(inputs.UItarget)){
        /// If the cursor is within this button area and click is detected then execute designated function.
        if(inputs.mainAttack){
            inputs.mainAttack = false;
            execute();
        }
    }
    srpg->DrawSprite(tl,surface.get());

    return sides;
}

/// class TitlePlate
TitlePlate::TitlePlate(olc::PixelGameEngine* game,std::string title, olc::vf2d padding,int fontSize = 1)
                :UIElement(game,padding),name(title),magnitude(fontSize){

    /// recalculate proper size from fontSize and string size.
    int fontRadius = magnitude * 4; /// Default charecter set uses an 8 by 8 mono space
    sides.x = (name.size() * fontRadius + sides.x) * 2;
    sides.y = (fontRadius + sides.y) * 2;

    olc::Sprite* oldDrawTarget = srpg->GetDrawTarget();

    surface = std::make_shared<olc::Sprite>(sides.x,sides.y);
    srpg->SetDrawTarget(surface.get());
    srpg->Clear(olc::BLANK);
    olc::vi2d center = sides / 2;
    //olc::vi2d textCorner = padding;
    srpg->DrawString(padding,name,olc::CYAN,magnitude);

    srpg->SetDrawTarget(oldDrawTarget);
}

olc::vi2d TitlePlate::render(olc::vi2d tl,srpg_data::controls& inputs){
    srpg->DrawSprite(tl,surface.get());
    return sides;
}

/// class MenuContainer
MenuContainer::MenuContainer(olc::PixelGameEngine* game, olc::vf2d area): UIElement(game,area){
    olc::Sprite* oldDrawTarget = srpg->GetDrawTarget();
    srpg->SetDrawTarget(surface.get());
    srpg->Clear(olc::VERY_DARK_BLUE);
    srpg->DrawRect(0,0,area.x,area.y,olc::DARK_BLUE);
    srpg->SetDrawTarget(oldDrawTarget);
}

olc::vi2d MenuContainer::render(olc::vi2d tl,srpg_data::controls& inputs){
    // Remeber previous draw target and set to draw to this sprite
    olc::Sprite* temp = srpg->GetDrawTarget();
    srpg->SetDrawTarget(surface.get());
    // If mouse is within this frame record original location and calculate offset from corner

    olc::vf2d tempaim;
    /// store mouse location and set it to be in relation to this container
    tempaim = inputs.UItarget;
    inputs.UItarget -= tl;

    // render each item within frame
    for(auto comp = components.begin(); comp != components.end(); comp++){

        (*comp)->render({1,1},inputs);

    }
    /// set render target back to callers target and draw generated sprite
    srpg->SetDrawTarget(temp);
    srpg->DrawSprite(tl,surface.get());

    inputs.UItarget = tempaim;


    return sides;
}

void MenuContainer::addItem(std::unique_ptr<UIElement> element){
    components.push_back(std::move(element));
    prepareRender();
}

/// class Menu
Menu::Menu(olc::PixelGameEngine* game, olc::vi2d centerP,olc::vi2d area): MenuContainer(game,area),center(centerP){
    borderZone = 3;
    drawNewBackground();
}

void MenuContainer::prepareRender(){
    olc::vi2d newArea = {0,0};
    for(auto iter = components.begin();iter != components.end();++iter){
        olc::vi2d objSize = (*iter)->getSize();
        newArea.x = objSize.x > newArea.x ? objSize.x : newArea.x;
        newArea.y += objSize.y + borderZone;
    }
    newArea.x += borderZone * 2;
    newArea.y += borderZone;
    sides = newArea;
    surface = std::make_shared<olc::Sprite>(newArea.x,newArea.y);
    drawNewBackground();
}

void MenuContainer::drawNewBackground(){
    olc::Sprite* oldDrawTarget = srpg->GetDrawTarget();
    srpg->SetDrawTarget(surface.get());
    srpg->Clear(olc::VERY_DARK_BLUE);
    srpg->DrawRect(0,0,sides.x-1,sides.y-1,olc::DARK_BLUE);
    srpg->SetDrawTarget(oldDrawTarget);

}


void Menu::render(srpg_data::controls& inputs){
    render(center - (sides/2),inputs);
}

olc::vi2d Menu::render(olc::vi2d tl, srpg_data::controls& inputs){
    // Remeber previous draw target and set to draw to this sprite
    olc::Sprite* oldDrawTarget = srpg->GetDrawTarget();
    srpg->SetDrawTarget(surface.get());

    /// store mouse location and set it to be in relation to this container
    olc::vf2d tempaim;
    tempaim = inputs.UItarget;
    inputs.UItarget -= tl;

    // render each item within frame
    olc::vi2d elemCorn = {borderZone,borderZone};
    for(auto comp = components.begin(); comp != components.end(); comp++){

        (*comp)->render(elemCorn, inputs);
        elemCorn.y += (*comp)->getSize().y + borderZone;
    }

    /// Set render to the UI layer, draw the sprite for this surface and reset to stored draw target
    srpg->SetDrawTarget(srpg_data::renderLayerUI);
    srpg->DrawSprite(tl,surface.get());
    srpg->SetDrawTarget(oldDrawTarget);

    inputs.UItarget = tempaim;

    return sides;
};

/// class hud
hud::hud(olc::PixelGameEngine* game,olc::vi2d topL ,olc::vi2d area): MenuContainer(game,area),tl(topL){}

hud::~hud() = default;

void hud::render(srpg_data::controls& inputs){

}

/// class valueBar
    valueBar::valueBar(olc::PixelGameEngine* game,olc::vi2d topL ,olc::vi2d area) : hudDisplay(game,topL, area){}

    void valueBar::update(){}

    void valueBar::render(){}





