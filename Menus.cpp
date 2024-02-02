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
    drawSprite();
    decal->Update();
}

/// class Container
UIContainer::UIContainer(olc::PixelGameEngine* gameObj,olc::vf2d sides,LAYOUT layout): UI(gameObj,sides),type(layout)
{
    drawSprite();
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

void UIContainer::drawSprite()
{
    olc::Sprite* oldDrawTarget = game->GetDrawTarget();
    game->SetDrawTarget(sprite.get());
    game->Clear(colours.bg);
    game->DrawRect(0,0,area.x-1,area.y-1,colours.border);
    game->SetDrawTarget(oldDrawTarget);
}

void UIContainer::addTitle(std::string name,int fontSize,olc::vi2d spriteSize,int order)
{

    olc::vf2d location = {0,0};
    if(spriteSize.x == -1){
        spriteSize.x = area.x - (borderZone.x * 2);
    }
    if(spriteSize.y == -1){
        spriteSize.y = fontSize * 8 + borderZone.y * 2;
    }
    std::unique_ptr<UI> temp = std::make_unique<TitlePlate>(game,colours,name,spriteSize,fontSize);
    insert(temp,location,spriteSize, order);
}

void UIContainer::addButton(std::string name,std::function<void()> task,olc::vi2d spriteSize,int order)
{
    if(spriteSize.x == -1){
        spriteSize.x = area.x - (borderZone.x * 2);
    }
    if(spriteSize.y == -1){
        spriteSize.y = 8 + borderZone.y * 2;
    }
    olc::vi2d location = borderZone;

    std::unique_ptr<UI> temp = std::make_unique<Button>(game,colours,name,spriteSize,task);
    insert(temp,location,spriteSize, order);
}

void UIContainer::addDynamicText(std::function<std::string()> task,int maxLength,ALIGN alignment,olc::vi2d spriteSize,int order)
{
    if(spriteSize.x == -1){
        spriteSize.x = area.x - (borderZone.x * 2);
    }
    if(spriteSize.y == -1){
        spriteSize.y = 8 + borderZone.y * 2;
    }
    olc::vi2d location = borderZone;

    std::unique_ptr<UI> temp = std::make_unique<DynamicText>(game,colours,spriteSize,task,UI::RIGHT);
    insert(temp,location,spriteSize, order);
}

void UIContainer::addContainer(std::unique_ptr<UIContainer>& container,olc::vf2d loc,olc::vf2d coverage,int order)
{
    std::unique_ptr<UI> contain = std::move(container);
    insert(contain,loc,coverage,order);

}

void UIContainer::insert(std::unique_ptr<UI>& object,olc::vf2d location,olc::vf2d coverage, int order)
{
    std::shared_ptr<UI> compObj = std::move(object);
    if(order <= -1 || order >= components.size()){
        components.push_back({compObj,location,coverage});
    } else {
        components.insert(components.begin() + order,{compObj,location,coverage});
    }
    if(type == HORIZ || type == VERT)
        updateLayout();
}

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
    drawSprite();
    decal->Update();
}

void TitlePlate::drawSprite()
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
    drawSprite();
    decal->Update();
}

bool Button::update(srpg_data::controls& inputs,olc::vf2d tl, olc::vf2d drawArea)
{
    if(Rectangle({0,0},drawArea).contains(inputs.UItarget) && inputs.mainAttack){
        execute();
        return true;
    }
    if(Rectangle({0,0},drawArea).contains(inputs.UItarget) != highlighted){
        highlighted = !highlighted;

        olc::Sprite* oldTarget = game->GetDrawTarget();
        game->SetDrawTarget(sprite.get());

        olc::Pixel newBorder = highlighted ? colours.highlight : colours.border;
        game->DrawRect(0,0,area.x-1,area.y-1,newBorder);
        decal->Update();

        game->SetDrawTarget(oldTarget);
        draw(tl,drawArea);
        return true;
    }
    draw(tl,drawArea);
    return false;
}

void Button::draw(olc::vf2d tl, olc::vf2d drawSize)
{
    olc::vf2d scale = drawSize / area;

    float textScale = std::min(scale.x,scale.y);
    game->DrawDecal(tl,decal.get(),scale);
    game->DrawStringDecal(tl + (textCorner*textScale + (area/2) * scale),name,colours.text,{textScale,textScale});

}

void Button::drawSprite()
{
    olc::Sprite* oldDraw = game->GetDrawTarget();

    textCorner = -olc::vf2d(name.size() * 4,4);

    game->SetDrawTarget(sprite.get());
    game->Clear(colours.bg);
    game->DrawRect(0,0,area.x-1,area.y-1,colours.border);
    decal->Update();

    game->SetDrawTarget(oldDraw);
}

/// class DynamicText
DynamicText::DynamicText(olc::PixelGameEngine* gameObj,theme colour, olc::vf2d sides,std::function<std::string()> task,ALIGN alignment)
                :Element(gameObj,sides,colour),align(alignment), stringDisplay(task)
{
    drawSprite();
    decal->Update();
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

void DynamicText::drawSprite()
{
    olc::Sprite* oldDraw = game->GetDrawTarget();

    game->SetDrawTarget(sprite.get());
    game->Clear(colours.bg);
    game->DrawRect(0,0,area.x-1,area.y-1,colours.border);
    decal->Update();

    game->SetDrawTarget(oldDraw);
}


