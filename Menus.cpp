#include "olcPixelGameEngine.h"
#include "olcPGEX_TransformedView.h"
#include "srpg_data.h"
#include "Rectangle.h"
#include "Menus.h"
#include <functional>


UI::UI(olc::PixelGameEngine* game,olc::vi2d sides):pge(game),pixelArea(sides)
{}

/// class Container
UIContainer::UIContainer(olc::PixelGameEngine* game, olc::vi2d drawArea,olc::vi2d mapArea): UI(game,drawArea),mapSize(mapArea)
{}

bool UIContainer::update(srpg::controls& inputs,olc::vf2d tl, olc::vf2d drawArea)
{
    if(element && !element->isCurrent())
        element->updateSprite();

    draw(tl,drawArea);

    bool changed = false;
    olc::vi2d comptl = tl + borderZone;
    olc::vi2d compArea  = drawArea - (2 * borderZone);
    olc::vf2d gridBlock = compArea/mapSize;
    for(auto& [key,comp] : componentMap){
        if(comp.item->update(inputs,comptl + (key * gridBlock), comp.area * gridBlock) )
        {changed = true;}
    }
    return changed;
}

void UIContainer::draw(olc::vf2d tl, olc::vf2d drawArea)
{
    olc::vf2d scale = drawArea / pixelArea;
    if(element)
        element->draw(tl,drawArea);
}

void UIContainer::addContainer(std::unique_ptr<UIContainer>& container,olc::vi2d loc,olc::vi2d gridArea)
{
    std::unique_ptr<UI> contain = std::move(container);

    componentMap[loc] = {std::move(contain),gridArea};
}

/// used to access an element behind the containers objects. will create new if needed.
Element* UIContainer::editContainerElement()
{
    if(!element)
        element = std::make_unique<Element>(pge,pixelArea);
    return element.get();
}

/// Adds new element to the container at the LOCation. Will replace any existing element with new blank
Element* UIContainer::addElement(olc::vi2d loc,olc::vi2d gridArea)
{
    olc::vf2d elementArea = pixelArea / mapSize * gridArea;

    std::unique_ptr<UI> contain = std::make_unique<Element>(pge,elementArea);
    componentMap[loc] = {std::move(contain),gridArea};
    return (Element*)componentMap[loc].item.get();
}

Element* UIContainer::editElement(olc::vi2d loc)
{
    if(componentMap.find(loc) == componentMap.end()) /// if element was not already added giveback null
        return nullptr;
    return (Element*)componentMap[loc].item.get();
}

/// class Screen
/// represents an entire display screen. Objects are placed inside to be shown or hidden as a collection.
Screen::Screen(olc::PixelGameEngine* game, uint8_t drawLayer)
    : UIContainer(game,olc::vi2d(game->ScreenWidth(),game->ScreenHeight()),olc::vi2d(game->ScreenWidth(),game->ScreenHeight()) )
{
    borderZone = {0,0};
}

void Screen::display(srpg::controls& inputs)
{
    UIContainer::update(inputs,{0,0},pixelArea);
}

/// class Element
Element::Element(olc::PixelGameEngine* game, olc::vf2d sides):UI(game,sides)
{
    sprite = std::make_unique<olc::Sprite>(pixelArea.x,pixelArea.y);
    decal = std::make_unique<olc::Decal>(sprite.get());
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
        spriteCurrent = true;
    }
    draw(tl,drawArea);
    return changed;
}

void Element::draw(olc::vf2d tl, olc::vf2d drawSize)
{
    olc::vf2d scale = drawSize / pixelArea;
    pge->DrawDecal(tl,decal.get(),scale);
}

bool Element::isCurrent()
{
    return spriteCurrent;
}

void Element::updateSprite()
{
    /// draw each feature to the sprite & decal
    olc::Sprite* oldDraw = pge->GetDrawTarget();
    pge->SetDrawTarget(sprite.get());

    for(auto& [key,comp]:features){
        comp->draw(borderZone, pixelArea);
    }
    pge->SetDrawTarget(oldDraw);
    decal->Update();
}

/// class Background
Element& Element::background(olc::Pixel bgColour)
{
    spriteCurrent = false;
    std::unique_ptr<Feature> temp = std::make_unique<Background>(pge,bgColour);
    features[Feature::BG] = std::move(temp);
    return *this;
}

Element::Background::Background(olc::PixelGameEngine* game,olc::Pixel bgColour):Feature(game)
{
    colour = bgColour;
}

void Element::Background::draw(olc::vi2d borderArea, olc::vf2d drawSize)
{
    pge->Clear(colour);
    pge->FillRect(borderArea.x,borderArea.y,drawSize.x - borderArea.x - borderArea.x - 1,drawSize.y - borderArea.y- borderArea.y - 1,colour);
}

/// class TextPlate
Element& Element::text(std::string name,olc::Pixel textColour,Align alignment)
{
    spriteCurrent = false;
    std::unique_ptr<Feature> temp = std::make_unique<TextPlate>(pge,name,textColour,alignment);
    features[Feature::TEXT] = std::move(temp);
    return *this;
}

Element::TextPlate::TextPlate(olc::PixelGameEngine* game, std::string title, olc::Pixel textColour,Align alignment)
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
        case LEFT :
            textCorner.x = borderArea.x;
        break;
        case CENTER :
            textCorner.x = (drawSize.x - textArea.x)/2;
        break;
        case RIGHT :
            textCorner.x = drawSize.x - textArea.x - borderArea.x;
        break;
    }
    textCorner.y = (drawSize.y - textArea.y)/2;
    /// Draw our elements to screen
    pge->DrawString(textCorner,name,colour,textScale);
}

/// class DynamicText
Element& Element::addDynamicText(std::function<std::string()> task,olc::Pixel textColour,int maxLength,Align alignment)
{
    spriteCurrent = false;
    std::unique_ptr<Feature> temp = std::make_unique<DynamicText>(pge,task,textColour,maxLength,alignment);
    features[Feature::TEXT] = std::move(temp);
    return *this;
}

Element::DynamicText::DynamicText(olc::PixelGameEngine* game, std::function<std::string()> task,olc::Pixel textColour,int expectedLength, Align alignment)
                :Feature(game), stringDisplay(task),colour(textColour),length(expectedLength), align(alignment)
{}

bool Element::DynamicText::update(srpg::controls& inputs,olc::vf2d tl, olc::vf2d drawArea)
{
    return true; ///Text needs to be refreshed each update.
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
        case LEFT :
            textCorner.x = borderArea.x;
        break;
        case CENTER :
            textCorner.x = (drawSize.x - textPixels.x) / 2;
        break;
        case RIGHT :
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
    spriteCurrent = false;
    std::unique_ptr<Feature> temp = std::make_unique<Button>(pge,task);
    features[Feature::CLICK] = std::move(temp);
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
    /// May include highlighting or mouse over response later or elsewhere
    /// for now only input capture
    return false;
}
