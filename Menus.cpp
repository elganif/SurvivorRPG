#include "olcPixelGameEngine.h"
#include "olcPGEX_TransformedView.h"
#include "srpg_data.h"
#include "Rectangle.h"
#include "Menus.h"
#include <functional>


UI::UI(olc::PixelGameEngine* gameObj, olc::vf2d sides):game(gameObj),area(sides)
{
    sprite = std::make_unique<olc::Sprite>(area.x,area.y);
    decal = std::make_unique<olc::Decal>(sprite.get());
}


void UI::resize(olc::vf2d newSize)
{
    area = newSize;
    sprite = std::make_unique<olc::Sprite>(area.x,area.y);
    decal = std::make_unique<olc::Decal>(sprite.get());
}

void UI::setTheme(olc::Pixel textColour,olc::Pixel background,olc::Pixel border,olc::Pixel highlight)
{
    setTheme({textColour,background,border,highlight});
}

void UI::setTheme(theme newTheme)
{
    colours = newTheme;
    updateSprite();
    decal->Update();
}

/// class Container
UIContainer::UIContainer(olc::PixelGameEngine* gameObj,olc::vf2d sides,LAYOUT layout): UI(gameObj,sides),type(layout)
{
    updateSprite();
}

bool UIContainer::update(srpg_data::controls& inputs,olc::vf2d tl, olc::vf2d drawArea)
{
    olc::vf2d tempMouse = inputs.UItarget;
    draw(tl,drawArea);
    for(auto& comp : components){
        inputs.UItarget -= comp.location;
        comp.item->update(inputs,comp.location + tl,comp.area);
        inputs.UItarget = tempMouse;
    }
    return false;
}

void UIContainer::draw(olc::vf2d tl, olc::vf2d drawArea)
{
    olc::vf2d scale = drawArea / area;

    game->DrawDecal(tl,decal.get(),scale);
}

void UIContainer::updateSprite()
{
    olc::Sprite* oldDrawTarget = game->GetDrawTarget();
    game->SetDrawTarget(sprite.get());
    game->Clear(colours.bg);
    game->DrawRect(0,0,area.x-1,area.y-1,colours.border);
    game->SetDrawTarget(oldDrawTarget);
}

void UIContainer::addTitle(std::string name,int fontSize,olc::vi2d spriteSize,int index)
{

    olc::vf2d location = {0,0};
    if(spriteSize.x == -1){
        spriteSize.x = area.x - (borderZone.x * 2);
    }
    if(spriteSize.y == -1){
        spriteSize.y = fontSize * 8 + borderZone.y * 2;
    }
    std::unique_ptr<UI> temp = std::make_unique<TitlePlate>(game,colours,name,spriteSize,fontSize);
    insert(temp,index,location,spriteSize );
}

void UIContainer::addButton(std::string name,std::function<void()> task,olc::vi2d spriteSize,int index)
{
    if(spriteSize.x == -1){
        spriteSize.x = area.x - (borderZone.x * 2);
    }
    if(spriteSize.y == -1){
        spriteSize.y = 8 + borderZone.y * 2;
    }
    olc::vi2d location = borderZone;

    std::unique_ptr<UI> temp = std::make_unique<Button>(game,colours,name,spriteSize,task);
    insert(temp, index,location,spriteSize);
}

void UIContainer::addDynamicText(std::function<std::string()> task,int maxLength,ALIGN alignment,olc::vf2d location,olc::vi2d spriteSize)
{
    std::unique_ptr<UI> temp = std::make_unique<DynamicText>(game,colours,spriteSize,task,alignment);
    insert(temp,components.size(),location,spriteSize);
}

void UIContainer::addDynamicText(std::function<std::string()> task,int maxLength,ALIGN alignment,int index,olc::vi2d spriteSize)
{
    if(spriteSize.x == -1){
        spriteSize.x = area.x - (borderZone.x * 2);
    }
    if(spriteSize.y == -1){
        spriteSize.y = 8 + borderZone.y * 2;
    }
    olc::vi2d location = borderZone;

    std::unique_ptr<UI> temp = std::make_unique<DynamicText>(game,colours,spriteSize,task,UI::RIGHT);
    insert(temp, index,location,spriteSize);
}


void UIContainer::addContainer(std::unique_ptr<UIContainer>& container,olc::vf2d loc,olc::vf2d coverage,int index)
{
    std::unique_ptr<UI> contain = std::move(container);
    insert(contain,index,loc,coverage);
}

/// Adds objects to the internal data strctures. Private method called by add___() functions.
void UIContainer::insert(std::unique_ptr<UI>& object, int index,olc::vf2d location,olc::vf2d coverage)
{
    std::shared_ptr<UI> compObj = std::move(object);
    if(index <= -1 || index >= components.size()){
        components.push_back({compObj,location,coverage});
    } else {
        components.insert(components.begin() + index,{compObj,location,coverage});
    }
    if(type == HORIZ || type == VERT)
        updateLayout();
}

/// When using automatic horizontal or vertical layouts this will arrange items through the visual space of the container.
/// Private function called by insert
void UIContainer::updateLayout()
{
    int numItem = components.size();
    olc::vf2d placementPoint = borderZone;
    olc::vf2d itemAdjust = {0,0};
    olc::vf2d itemSize = {0,0};
    if(type == HORIZ){
        // horizontal spacing math along x axis
    }
    if(type == VERT){
        //vertical spacing math along y axis
        float distance = area.y - borderZone.y * 2;
        float elementspaceing = distance / numItem;
        itemAdjust.y = elementspaceing;
        itemSize.x = area.x - borderZone.x * 2;
        itemSize.y = itemAdjust.y;

    }
    for(component& comp : components){
        comp.location = placementPoint;
        comp.area = itemSize;
        placementPoint += itemAdjust;
    }
}

void UIContainer::setTheme(olc::Pixel textColour,olc::Pixel background,olc::Pixel border,olc::Pixel highlight)
{
    UI::setTheme(textColour,background,border,highlight);
    setTheme(colours);
}

void UIContainer::setTheme(theme newTheme)
{
    colours = newTheme;
    for(auto& comp : components){
        comp.item->setTheme(colours);
    }
}


/// class Screen
/// represents an entire display screen. Objects are placed inside to be shown or hidden as a collection.
Screen::Screen(olc::PixelGameEngine* gameObj,uint8_t drawLayer)
    : UIContainer(gameObj,olc::vi2d(gameObj->ScreenWidth(),gameObj->ScreenHeight()) ),spriteLayer(drawLayer)
{
    setTheme(olc::BLANK,olc::BLANK,olc::BLANK,olc::BLANK);
}

void Screen::display(srpg_data::controls& inputs)
{
    olc::Sprite* oldDraw = game->GetDrawTarget();
    game->SetDrawTarget(spriteLayer);

    UIContainer::update(inputs,{0,0},area);

    game->SetDrawTarget(oldDraw);

}


/// class Element
Element::Element(olc::PixelGameEngine* gameObj,olc::vf2d sides,theme colour):UI(gameObj,sides)
{
    colours = colour;
}

bool Element::update(srpg_data::controls& inputs,olc::vf2d tl, olc::vf2d drawArea)
{
    draw(tl,drawArea);
    return false;
}

void Element::draw(olc::vf2d tl, olc::vf2d drawSize)
{
    olc::vf2d scale = drawSize / area;
    game->DrawDecal(tl,decal.get(),scale);
}



/// class TitlePlate
TitlePlate::TitlePlate(olc::PixelGameEngine* game,theme colour,std::string title, olc::vf2d sides,int fontSize = 1)
                :Element(game,sides,colour),name(title),magnitude(fontSize)
{
    updateSprite();
    decal->Update();
}

void TitlePlate::updateSprite()
{
    olc::Sprite* oldTarget = game->GetDrawTarget();
    game->SetDrawTarget(sprite.get());

    olc::vf2d textCorner = (area / 2) - olc::vf2d(name.size() * 4 * magnitude, 4 * magnitude);

    game->Clear(colours.bg);
    game->DrawRect(0,0,area.x-1,area.y-1,colours.border);
    game->DrawString(textCorner,name,colours.text,magnitude);

    game->SetDrawTarget(oldTarget);
}


/// class Button
Button::Button(olc::PixelGameEngine* gameObj,theme colour, std::string title, olc::vf2d sides,std::function<void()> task)
                :Element(gameObj,sides,colour),name(title),execute(task)
{
    updateSprite();
    decal->Update();
}

bool Button::update(srpg_data::controls& inputs,olc::vf2d tl, olc::vf2d drawArea)
{
    /// check if the button was clicked and execute its function.
    if(Rectangle({0,0},drawArea).contains(inputs.UItarget) && inputs.mainAttack){
        execute();
    }
    bool updated = false;
    /// We check if the mouse of selection has changed since last frame and update highlighting if changed.
    if(Rectangle({0,0},drawArea).contains(inputs.UItarget) != highlighted){
        highlighted = !highlighted;

        /// record old draw target and set buttons sprite for drawing.
        olc::Sprite* oldTarget = game->GetDrawTarget();
        game->SetDrawTarget(sprite.get());

        /// we are drawing because its changed, set the colour we need based on if its highlighted or not
        olc::Pixel newBorder = highlighted ? colours.highlight : colours.border;
        game->DrawRect(0,0,area.x-1,area.y-1,newBorder);
        decal->Update();

        /// return draw target to prior state
        game->SetDrawTarget(oldTarget);

        updated = true;
    }
    /// draw the button to the specified area, return if the sprite was updated or not.
    draw(tl,drawArea);
    return updated;
}

void Button::updateSprite()
{
    olc::Sprite* oldDraw = game->GetDrawTarget();

    textCorner = -olc::vf2d(name.size() * 4,4);

    game->SetDrawTarget(sprite.get());
    game->Clear(colours.bg);
    game->DrawRect(0,0,area.x-1,area.y-1,colours.border);
    decal->Update();

    game->SetDrawTarget(oldDraw);
}

void Button::draw(olc::vf2d tl, olc::vf2d drawSize)
{
    olc::vf2d scale = drawSize / area;

    float textScale = std::min(scale.x,scale.y);
    game->DrawDecal(tl,decal.get(),scale);
    game->DrawStringDecal(tl + (textCorner*textScale + (area/2) * scale),name,colours.text,{textScale,textScale});

}


/// class DynamicText
DynamicText::DynamicText(olc::PixelGameEngine* gameObj,theme colour, olc::vf2d sides,std::function<std::string()> task,ALIGN alignment)
                :Element(gameObj,sides,colour),align(alignment), stringDisplay(task)
{
    updateSprite();
    decal->Update();
}

void DynamicText::updateSprite()
{
    olc::Sprite* oldDraw = game->GetDrawTarget();

    game->SetDrawTarget(sprite.get());
    game->Clear(colours.bg);
    game->DrawRect(0,0,area.x-1,area.y-1,colours.border);
    decal->Update();

    game->SetDrawTarget(oldDraw);
}

void DynamicText::draw(olc::vf2d tl, olc::vf2d drawSize)
{
    olc::vf2d scale = drawSize / area;
    float textScale = std::min(scale.x,scale.y);
    olc::vf2d textCorner = {0,area.y/2 - 4};

    /// Call our saved function to generate the string being displayed
    std::string output = stringDisplay();

    /// Establish starting point based on alignment
    switch (align) {
        case LEFT :
            textCorner.x = borderZone.x;
        break;
        case JUST :
            textCorner.x = area.x/2 - (output.size() * 4);
        break;
        case RIGHT :
            textCorner.x = area.x - borderZone.x - (output.size() * 8);
        break;
    }
    /// Draw our elements to screen
    game->DrawDecal(tl,decal.get(),scale);
    game->DrawStringDecal(tl + (textCorner*textScale),output,colours.text,{textScale,textScale});
}

