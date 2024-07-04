#include "olcPixelGameEngine.h"
#include "olcPGEX_TransformedView.h"
#include "srpg_data.h"
#include "Rectangle.h"
#include "Menus.h"
#include <functional>

/// Class UI. Only has one static method.
std::unique_ptr<Display> UI::makeDisplay(olc::PixelGameEngine* game, olc::vi2d loc, olc::vi2d area)
{
    return std::move(std::make_unique<Display>(game,loc,area) );
}

/// class Display
/// represents a primary display object. Objects are placed inside to be shown or hidden as a collection.
Display::Display(olc::PixelGameEngine* game,olc::vi2d loc, olc::vi2d area)
    : Element(game,area),tl(loc)
{
    borderZone = {0,0};
}

void Display::display(srpg::controls& inputs)
{
    this->update(inputs,tl,pixelArea);
    draw(tl,pixelArea);
}

/// class Element
Element::Element(olc::PixelGameEngine* game, olc::vf2d sides) : pge(game),pixelArea(sides)
{}

/// addSprite adds sprite & decal only if needed. Sets spriteCurrent bool false in all cases
void Element::addSprite()
{
    spriteCurrent = false;
    if(sprite)  /// Guard against recreateing Sprite multiple times
        return;
    sprite = std::make_unique<olc::Sprite>(pixelArea.x,pixelArea.y);
    //decal = std::make_unique<olc::Decal>(sprite.get() );
}

bool Element::update(srpg::controls& inputs,olc::vf2d tl, olc::vf2d drawArea)
{
    bool changed = false;

    for(auto& [key,comp] : features){
        if(comp->update(inputs,tl,drawArea) )
            changed = true;
    }

    if(!spriteCurrent || changed){
        updateSprite();
    }
    return changed;
}

void Element::updateSprite()
{
    /// draw each feature to the sprite & update decal
    if(sprite){
        olc::Sprite* oldDraw = pge->GetDrawTarget();
        pge->SetDrawTarget(sprite.get());
        pge->Clear(olc::BLANK);
        for(auto& [key,comp]:features){
            comp->draw(borderZone, pixelArea-(2*borderZone) );
        }
        pge->SetDrawTarget(oldDraw);
        //decal->Update();
        spriteCurrent = true;
        return;
    }
}

void Element::draw(olc::vf2d tl, olc::vf2d drawSize)
{
    if(sprite){
        pge->DrawSprite(tl,sprite.get() );//,drawSize / pixelArea);
    }
}

bool Element::isCurrent()
{
    return spriteCurrent;
}

/// class Container

Element* Element::makeGrid(int x, int y)
{
    addSprite();
    std::unique_ptr<Feature> temp = std::make_unique<Container>(pge,olc::vi2d(x,y));
    features[CONTAINER] = std::move(temp);
    return this;
}

Element* Element::setBlock(int x, int y, olc::vi2d blocks)
{
    if(features.count(CONTAINER) == 0)
        return nullptr;
    std::shared_ptr<Container> containPtr = std::static_pointer_cast<Container>(features[CONTAINER]);

    olc::vf2d area = (pixelArea / containPtr->mapSize) * blocks;
    std::shared_ptr<Element> elem = std::make_shared<Element>(pge,area);
    containPtr->insertBlock(elem,olc::vi2d(x,y),blocks);
    return elem.get();
}

Element* Element::getBlock(int x, int y)
{
    std::map<funct,std::shared_ptr<Feature>>::iterator contain = features.find(CONTAINER);
    if(contain == features.end());
        return nullptr;
    std::shared_ptr<Container> containPtr = std::static_pointer_cast<Container>(contain->second);
    std::shared_ptr<Element> element = containPtr->getBlock(x,y);
    return element.get();
}

Element::Container::Container(olc::PixelGameEngine* game, olc::vi2d mapArea) : Feature(game),mapSize(mapArea)
{}

bool Element::Container::update(srpg::controls& inputs,olc::vf2d tl, olc::vf2d drawArea)
{
    bool changed = false;
    olc::vi2d comptl = tl;
    olc::vi2d compArea  = drawArea;
    olc::vf2d gridBlock = compArea/mapSize;
    for(auto& [key,comp] : elements){
        if(comp.item->update(inputs,comptl + (key * gridBlock), comp.area * gridBlock) ){ /// run update as condition. if it returns true set changed true for return.
            changed = true;
        }
    }
    return changed;
}

void Element::Container::draw(olc::vi2d borderArea, olc::vf2d drawSize)
{
    olc::Sprite* test = pge->GetDrawTarget();
    olc::vi2d comptl = borderArea;
    olc::vi2d compArea  = drawSize;
    olc::vf2d gridBlock = compArea/mapSize;
    for(auto& [key,comp] : elements){
        comp.item->draw(comptl + (key * gridBlock), comp.area * gridBlock);
    }
}

std::shared_ptr<Element> Element::Container::insertBlock(std::shared_ptr<Element> newBlock,olc::vi2d loc, olc::vi2d blocks)
{
    elements[loc] = {newBlock,blocks};
    return elements[loc].item;
}

std::shared_ptr<Element> Element::Container::getBlock(int x, int y)
{
    olc::vi2d xy = {x,y};
    if(elements.find(xy) == elements.end())
        return nullptr;
    return elements[xy].item;
}

/// class Background
Element& Element::background(olc::Pixel bgColour)
{
    addSprite();
    std::unique_ptr<Feature> temp = std::make_unique<Background>(pge,bgColour);
    features[BG] = std::move(temp);
    return *this;
}

Element::Background::Background(olc::PixelGameEngine* game,olc::Pixel bgColour):Feature(game),colour(bgColour)
{}

void Element::Background::draw(olc::vi2d borderArea, olc::vf2d drawSize)
{
    olc::Sprite* test = pge->GetDrawTarget();
    pge->Clear(colour);
    pge->FillRect(borderArea.x,borderArea.y,drawSize.x - borderArea.x - borderArea.x - 1,drawSize.y - borderArea.y- borderArea.y - 1,colour);
}

/// class TextPlate
Element& Element::text(std::string name,olc::Pixel textColour,UI::Align alignment)
{
    addSprite();
    std::unique_ptr<Feature> temp = std::make_unique<TextPlate>(pge,name,textColour,alignment);
    features[TEXT] = std::move(temp);
    return *this;
}

Element::TextPlate::TextPlate(olc::PixelGameEngine* game, std::string title, olc::Pixel textColour,UI::Align alignment)
                :Feature(game),name(title),colour(textColour),align(alignment)
{}

void Element::TextPlate::draw(olc::vi2d borderArea, olc::vf2d drawSize)
{

    olc::vi2d textArea = {(int)name.size() * 8,8};
    olc::vf2d scale = (drawSize - borderArea*2) /textArea;
    float textScale = std::min(scale.x,scale.y);
    textArea = textArea * textScale;

    olc::vi2d textCorner;
    /// Establish starting point based on alignment
    switch (align) {
        case UI::LEFT :
            textCorner.x = borderArea.x;
        break;
        case UI::CENTER :
            textCorner.x = (drawSize.x - textArea.x)/2;
        break;
        case UI::RIGHT :
            textCorner.x = drawSize.x - textArea.x - borderArea.x;
        break;
    }
    textCorner.y = (drawSize.y - textArea.y)/2;
    /// Draw our elements to screen
    pge->DrawString(textCorner,name,colour,textScale);
}

/// class DynamicText
Element& Element::addDynamicText(std::function<std::string()> task,olc::Pixel textColour,int maxLength,UI::Align alignment)
{
    addSprite();
    std::unique_ptr<Feature> temp = std::make_unique<DynamicText>(pge,task,textColour,maxLength,alignment);
    features[TEXT] = std::move(temp);
    return *this;
}

Element::DynamicText::DynamicText(olc::PixelGameEngine* game, std::function<std::string()> task,olc::Pixel textColour,int expectedLength, UI::Align alignment)
                :Feature(game), stringDisplay(task),colour(textColour),length(expectedLength), align(alignment)
{}

bool Element::DynamicText::update(srpg::controls& inputs,olc::vf2d tl, olc::vf2d drawArea)
{
    return true; ///Text should be refreshed each update.
}

void Element::DynamicText::draw(olc::vi2d borderArea, olc::vf2d drawSize)
{
    std::string display = stringDisplay();
    int numChar = display.size();
    int maxLength = std::max(length,numChar);
    olc::vi2d textArea = drawSize - borderArea*2;
    olc::vf2d areaScale = textArea / olc::vf2d(maxLength*8,8);
    float scale = std::min(areaScale.x,areaScale.y);
    olc::vf2d textPixels = olc::vf2d(numChar, 1) * 8 * scale;
    olc::vi2d textCorner;
    /// Establish starting point based on alignment
    switch (align) {
        case UI::LEFT :
            textCorner.x = borderArea.x;
        break;
        case UI::CENTER :
            textCorner.x = (drawSize.x - textPixels.x) / 2;
        break;
        case UI::RIGHT :
            textCorner.x = drawSize.x - borderArea.x - textPixels.x;
        break;
    }
    textCorner.y = (drawSize.y - textPixels.y)/2;
    /// Draw our elements to screen
    pge->DrawString(textCorner,display,colour,scale);
}

/// class Button
Element& Element::addButton(std::function<void()> task)
{
    std::unique_ptr<Feature> temp = std::make_unique<Button>(pge,task);
    features[CLICK] = std::move(temp);
    return *this;
}

Element::Button::Button(olc::PixelGameEngine* game, std::function<void()> task)
                :Feature(game),execute(task)
{}

bool Element::Button::update(srpg::controls& inputs,olc::vf2d tl, olc::vf2d drawArea)
{
    /// check if the button was clicked and execute its function.
    if(Rectangle(tl,drawArea).contains(inputs.UItarget) && inputs.mainAttack){
        execute();
    }
    return false;
}
