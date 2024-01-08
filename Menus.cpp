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
/// class Container
Container::Container(olc::PixelGameEngine* game, olc::vi2d area,LAYOUT layout): UI(game,area), type(layout) {
    prepareRender();
}

void Container::addItem(std::unique_ptr<UI> element,olc::vi2d position){
    components.push_back({std::move(element),position});
    prepareRender();
}

bool Container::update(){
    for(auto comp = components.begin();comp != components.end();comp++){
        if((*comp).item->update())
            return true;
    }
    return false;
}

olc::vi2d Container::prepareRender(){
    if(type == VERTICAL){
        return prepareVertRender();
    }
   return prepareArrangedRender();
}

std::shared_ptr<olc::Sprite> Container::onDisplay(srpg_data::controls& inputs) {
    // Remeber previous draw target and set to draw to this sprite
    olc::Sprite* temp = srpg->GetDrawTarget();
    srpg->SetDrawTarget(sprite.get());

    olc::vf2d originalTarget;
    /// store mouse location and set it to be in relation to this container
    originalTarget = inputs.UItarget;

    // render each item within frame
    for(auto comp = components.begin(); comp != components.end(); comp++){
        inputs.UItarget -= (*comp).pos;
        if((*comp).item->update()){
            (*comp).item->prepareRender();
        }
        srpg->DrawSprite((*comp).pos, (*comp).item->onDisplay(inputs).get());
        inputs.UItarget = originalTarget;
    }
    /// set render target back to callers target and draw generated sprite
    srpg->SetDrawTarget(temp);

    inputs.UItarget = originalTarget;

    return sprite;
}

void Container::drawNewBackground(){
    olc::Sprite* oldDrawTarget = srpg->GetDrawTarget();
    srpg->SetDrawTarget(sprite.get());
    srpg->Clear(olc::VERY_DARK_BLUE);
    srpg->DrawRect(0,0,sides.x-1,sides.y-1,olc::DARK_BLUE);
    srpg->SetDrawTarget(oldDrawTarget);
}

olc::vi2d Container::prepareVertRender(){
    olc::vi2d newArea = {0,0};
    olc::vi2d itemLoc = borderZone;
    for(auto comp = components.begin();comp != components.end();++comp){
        (*comp).item->prepareRender();
        olc::vi2d objSize = (*comp).item->getSize();
        (*comp).pos = itemLoc;
        newArea.x = objSize.x < newArea.x ? newArea.x : objSize.x;
        newArea.y += objSize.y + borderZone.y;
        itemLoc.y += objSize.y + borderZone.y;
    }

    newArea.x += borderZone.x * 2;
    newArea.y += borderZone.y;
    sides = newArea;
    sprite = std::make_shared<olc::Sprite>(newArea.x,newArea.y);
    drawNewBackground();
    return sides;
}

olc::vi2d Container::prepareArrangedRender(){
    /// Sizes and locations should be defined by user. Only need to call prepareRender() on each item.
    for(auto comp = components.begin(); comp != components.end(); comp++){
        (*comp).item->prepareRender();
    }
    drawNewBackground();
    return sides;
}

/// class Menu
Menu::Menu(olc::PixelGameEngine* game, olc::vi2d centerP,olc::vi2d area): Container(game,area),center(centerP){
    borderZone = {3,3};
}

void Menu::render(srpg_data::controls& inputs){
    olc::vi2d tl = center - (sides / 2);
    olc::vi2d origTarget = inputs.UItarget;
    inputs.UItarget -= tl;
    onDisplay(inputs);
    srpg->DrawSprite( tl ,sprite.get() );
    inputs.UItarget = origTarget;
}


/// class Panel
//Panel::Panel(olc::PixelGameEngine* game,int left,int top,int right,int bottom) : Container(game,{right,bottom},ARRANGED),tl({left,top}) {}
Panel::Panel(olc::PixelGameEngine* game,olc::vi2d topL ,olc::vi2d area): Container(game,area,ARRANGED),tl(topL) {}


void Panel::render(srpg_data::controls& inputs){
    olc::vi2d origTarget = inputs.UItarget;
    inputs.UItarget -= tl;
    onDisplay(inputs);
    srpg->DrawSprite( tl ,sprite.get() );
    inputs.UItarget = origTarget;
}

/// class Button
Button::Button(olc::PixelGameEngine* game,std::string title, olc::vf2d area,std::function<void()> task)
                :Element(game,area),name(title),execute(task){}

bool Button::update(){
    return false;
}

olc::vi2d Button::prepareRender(){

    /// Remember callers draw target and reset it on exit.
    olc::Sprite* callerTarget = srpg->GetDrawTarget();

    olc::vi2d center = sides / 2;
    olc::vi2d textCorner = {center.x - (int)name.size()*4,center.y-4};

    srpg->SetDrawTarget(sprite.get());
    srpg->Clear(olc::BLACK);
    srpg->DrawRect(0,0,sides.x-1,sides.y-1,olc::GREEN);
    srpg->DrawString(textCorner,name,olc::CYAN);

    srpg->SetDrawTarget(callerTarget);
    return sides;
}

std::shared_ptr<olc::Sprite> Button::onDisplay(srpg_data::controls& inputs){

    if(Rectangle({0,0},sides).contains(inputs.UItarget)){
        /// If the cursor is within this button area and click is detected then execute designated function.
        if(inputs.mainAttack){
            inputs.mainAttack = false;
            execute();
        }
    }

    return sprite;
}

/// class TitlePlate
TitlePlate::TitlePlate(olc::PixelGameEngine* game,std::string title, olc::vf2d padding,int fontSize = 1)
                :Element(game,padding),name(title),magnitude(fontSize){

    /// recalculate proper size from fontSize and string size.
    int fontRadius = magnitude * 4; /// Default charecter set uses an 8 diameter mono space font
    sides.x = (name.size() * fontRadius + sides.x) * 2;
    sides.y = (fontRadius + sides.y) * 2;
    sprite = std::make_shared<olc::Sprite>(sides.x,sides.y); ///manually make a correct size sprite becasue we dont use area in the default way
}

bool TitlePlate::update(){
    return false;
}

olc::vi2d TitlePlate::prepareRender(){
    olc::Sprite* oldDrawTarget = srpg->GetDrawTarget();
    srpg->SetDrawTarget(sprite.get());

    olc::vi2d center = sides / 2;
    olc::vi2d padding = center - olc::vi2d(name.size() * 4 * magnitude,4*magnitude);
    srpg->DrawString(padding,name,olc::CYAN,magnitude);

    srpg->SetDrawTarget(oldDrawTarget);

    return sides;
}

std::shared_ptr<olc::Sprite> TitlePlate::onDisplay(srpg_data::controls& inputs){
    return sprite;
}

/// class textDisplay
textDisplay::textDisplay(olc::PixelGameEngine* game,olc::vi2d topL ,olc::vi2d area) : Element(game, area){}

void textDisplay::onDisplay(){}

/// class valueBar
valueBar::valueBar(olc::PixelGameEngine* game,olc::vi2d topL ,olc::vi2d area) : Element(game,area){}

bool valueBar::update(){return false;}

void valueBar::onDisplay(){}





