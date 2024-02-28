#include "olcPixelGameEngine.h"
#include "olcPGEX_TransformedView.h"
#include "srpg_data.h"
#include "Rectangle.h"
#include "Menus.h"
#include <functional>


UI::UI(olc::PixelGameEngine* game, olc::vi2d sides):pge(game),pixelArea(sides)
{
    sprite = std::make_unique<olc::Sprite>(pixelArea.x,pixelArea.y);
    decal = std::make_unique<olc::Decal>(sprite.get());
}


void UI::resize(olc::vf2d newSize)
{
    pixelArea = newSize;
    sprite = std::make_unique<olc::Sprite>(pixelArea.x,pixelArea.y);
    updateSprite();
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
UIContainer::UIContainer(olc::PixelGameEngine* game,olc::vi2d drawArea,olc::vi2d mapArea): UI(game,drawArea),mapSize(mapArea)
{
    updateSprite();
}

bool UIContainer::update(srpg::controls& inputs,olc::vf2d tl, olc::vf2d drawArea)
{


    draw(tl,drawArea);
    olc::vi2d comptl = tl + borderZone;
    olc::vi2d compArea  = drawArea - (2 * borderZone);
    olc::vf2d gridBlock = compArea/mapSize;
    for(auto& [key,comp] : componentMap){
        comp.item->update(inputs,comptl + (key * gridBlock), comp.area * gridBlock);
    }

    return false;
}

void UIContainer::draw(olc::vf2d tl, olc::vf2d drawArea)
{
    olc::vf2d scale = drawArea / pixelArea;

    pge->DrawDecal(tl,decal.get(),scale);
}

void UIContainer::updateSprite()
{
    olc::Sprite* oldDrawTarget = pge->GetDrawTarget();
    pge->SetDrawTarget(sprite.get());
    pge->Clear(colours.border);
    pge->FillRect(borderZone.x,borderZone.y,pixelArea.x - borderZone.x - 1,pixelArea.y - borderZone.y - 1,colours.bg);
    pge->SetDrawTarget(oldDrawTarget);
}

void UIContainer::addTitle(std::string name,int fontSize,olc::vi2d loc, olc::vi2d gridArea)
{
    olc::vi2d spriteSize = (pixelArea - borderZone * 2) / mapSize * gridArea;
    std::unique_ptr<UI> temp = std::make_unique<TitlePlate>(pge,colours,name,spriteSize,fontSize);
    componentMap[loc] = {std::move(temp),gridArea};
}

void UIContainer::addButton(std::string name,std::function<void()> task,olc::vi2d loc,olc::vi2d gridArea)
{
    olc::vi2d compArea = pixelArea - (2 * borderZone);
    olc::vi2d spriteSize = (compArea / mapSize) * gridArea;
    std::unique_ptr<UI> temp = std::make_unique<Button>(pge,colours,name,spriteSize,task);
    componentMap[loc] = {std::move(temp),gridArea};
}

void UIContainer::addDynamicText(std::function<std::string()> task,int maxLength,ALIGN alignment,olc::vf2d loc,olc::vi2d gridArea)
{
    olc::vi2d spriteSize = pixelArea / mapSize * gridArea;
    std::unique_ptr<UI> temp = std::make_unique<DynamicText>(pge,colours,spriteSize,task,alignment);
    componentMap[loc] = {std::move(temp),gridArea};
}

void UIContainer::addContainer(std::unique_ptr<UIContainer>& container,olc::vi2d loc,olc::vi2d gridArea)
{
    std::unique_ptr<UI> contain = std::move(container);

    componentMap[loc] = {std::move(contain),gridArea};
}


void UIContainer::setTheme(olc::Pixel textColour,olc::Pixel background,olc::Pixel border,olc::Pixel highlight)
{
    UI::setTheme(textColour,background,border,highlight);
    setTheme(colours);
}

void UIContainer::setTheme(theme newTheme)
{
    colours = newTheme;
    for(auto& [key,comp] : componentMap){
        comp.item->setTheme(colours);
    }
}


/// class Screen
/// represents an entire display screen. Objects are placed inside to be shown or hidden as a collection.
Screen::Screen(olc::PixelGameEngine* game,uint8_t drawLayer)
    : UIContainer(game,olc::vi2d(game->ScreenWidth(),game->ScreenHeight()),olc::vi2d(game->ScreenWidth(),game->ScreenHeight()) ),spriteLayer(drawLayer)
{
    setTheme(olc::BLANK,olc::BLANK,olc::BLANK,olc::BLANK);
    borderZone = {0,0};
}

void Screen::display(srpg::controls& inputs)
{
    olc::Sprite* oldDraw = pge->GetDrawTarget();
    pge->SetDrawTarget(spriteLayer);

    UIContainer::update(inputs,{0,0},pixelArea);

    pge->SetDrawTarget(oldDraw);

}


/// class Element
Element::Element(olc::PixelGameEngine* game,olc::vf2d sides,theme colour):UI(game,sides),lastDrawArea(sides)
{
    colours = colour;
}

bool Element::update(srpg::controls& inputs,olc::vf2d tl, olc::vf2d drawArea)
{
    draw(tl,drawArea);
    return false;
}

void Element::draw(olc::vf2d tl, olc::vf2d drawSize)
{
    olc::vf2d scale = drawSize / pixelArea;
    pge->DrawDecal(tl,decal.get(),scale);
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
    olc::Sprite* oldTarget = pge->GetDrawTarget();
    pge->SetDrawTarget(sprite.get());

    olc::vf2d textCorner = (pixelArea / 2) - olc::vf2d(name.size() * 4 * magnitude , 4 * magnitude);

    pge->Clear(colours.bg);
    pge->DrawRect(0,0,pixelArea.x-1,pixelArea.y-1,colours.border);
    pge->DrawString(textCorner,name,colours.text,magnitude);

    pge->SetDrawTarget(oldTarget);
}


/// class Button
Button::Button(olc::PixelGameEngine* game,theme colour, std::string title, olc::vf2d sides,std::function<void()> task)
                :Element(game,sides,colour),name(title),execute(task)
{
    updateSprite();
    decal->Update();
}

bool Button::update(srpg::controls& inputs,olc::vf2d tl, olc::vf2d drawArea)
{
    /// check if the button was clicked and execute its function.
    if(Rectangle(tl,drawArea).contains(inputs.UItarget) && inputs.mainAttack){
        execute();
    }
    bool updated = false;
    /// We check if the mouse of selection has changed since last frame and update highlighting if changed.
    if(Rectangle(tl,drawArea).contains(inputs.UItarget) != highlighted){
        highlighted = !highlighted;

        /// record old draw target and set buttons sprite for drawing.
        olc::Sprite* oldTarget = pge->GetDrawTarget();
        pge->SetDrawTarget(sprite.get());

        /// we are drawing because its changed, set the colour we need based on if its highlighted or not
        olc::Pixel newBorder = highlighted ? colours.highlight : colours.border;
        pge->DrawRect(0,0,pixelArea.x-1,pixelArea.y-1,newBorder);
        decal->Update();

        /// return draw target to prior state
        pge->SetDrawTarget(oldTarget);

        updated = true;
    }
    /// draw the button to the specified area, return if the sprite was updated or not.
    draw(tl,drawArea);
    return updated;
}

void Button::updateSprite()
{
    olc::Sprite* oldDraw = pge->GetDrawTarget();

    textCorner = -olc::vf2d(name.size() * 4,4);

    pge->SetDrawTarget(sprite.get());
    pge->Clear(colours.bg);
    pge->DrawRect(0,0,pixelArea.x-1,pixelArea.y-1,colours.border);
    decal->Update();

    pge->SetDrawTarget(oldDraw);
}

void Button::draw(olc::vf2d tl, olc::vf2d drawSize)
{
    olc::vf2d scale = drawSize / pixelArea;

    float textScale = std::min(scale.x,scale.y);
    pge->DrawDecal(tl,decal.get(),scale);
    pge->DrawStringDecal(tl + (textCorner*textScale + (pixelArea/2) * scale),name,colours.text,{textScale,textScale});

}


/// class DynamicText
DynamicText::DynamicText(olc::PixelGameEngine* game,theme colour, olc::vf2d sides,std::function<std::string()> task,ALIGN alignment)
                :Element(game,sides,colour),align(alignment), stringDisplay(task)
{
    updateSprite();
    decal->Update();
}

void DynamicText::updateSprite()
{
    olc::Sprite* oldDraw = pge->GetDrawTarget();

    pge->SetDrawTarget(sprite.get());
    pge->Clear(colours.bg);
    pge->DrawRect(0,0,pixelArea.x-1,pixelArea.y-1,colours.border);
    decal->Update();

    pge->SetDrawTarget(oldDraw);
}

void DynamicText::draw(olc::vf2d tl, olc::vf2d drawSize)
{
    olc::vf2d scale = drawSize / pixelArea;
    float textScale = std::min(scale.x,scale.y);
    olc::vf2d textCorner = {0,pixelArea.y/2 - 4};

    /// Call our saved function to generate the string being displayed
    std::string output = stringDisplay();

    /// Establish starting point based on alignment
    switch (align) {
        case LEFT :
            textCorner.x = borderZone.x;
        break;
        case JUST :
            textCorner.x = pixelArea.x/2 - (output.size() * 4);
        break;
        case RIGHT :
            textCorner.x = pixelArea.x - borderZone.x - (output.size() * 8);
        break;
    }
    /// Draw our elements to screen
    pge->DrawDecal(tl,decal.get(),scale);
    pge->DrawStringDecal(tl + (textCorner*textScale),output,colours.text,{textScale,textScale});
}

