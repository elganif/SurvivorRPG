#ifndef MENUS_H_INCLUDED
#define MENUS_H_INCLUDED
//struct Rectangle; // from Rectangle.h

class UI
{
    protected:
    std::unique_ptr<olc::Sprite> sprite = nullptr;
    std::unique_ptr<olc::Decal> decal = nullptr;
    struct theme{
        olc::Pixel text;
        olc::Pixel bg;
        olc::Pixel border;
        olc::Pixel highlight;
    };
    olc::PixelGameEngine* game;
    olc::vf2d area;
    olc::vi2d borderZone = {1,1};
    theme colours = {olc::MAGENTA,olc::DARK_MAGENTA,olc::MAGENTA,olc::WHITE};
    public:
    UI(olc::PixelGameEngine* gameObj, olc::vf2d sides);
    virtual ~UI() = default;

    virtual bool update(srpg_data::controls& inputs,olc::vf2d tl, olc::vf2d drawArea) = 0;
    virtual void draw(olc::vf2d tl, olc::vf2d drawArea) = 0;
    virtual void updateSprite() = 0;

    void setTheme(olc::Pixel textColour,olc::Pixel background,olc::Pixel border,olc::Pixel highlight);
    void setTheme(theme newTheme);

    void resize(olc::vf2d newSize);
    olc::vi2d getSize() {return area;};


    enum ALIGN {LEFT,JUST,RIGHT};
};

class UIContainer : public UI
{
public:
    enum LAYOUT {VERT,HORIZ,MANUAL};
protected:
    struct component {
        std::shared_ptr<UI> item = nullptr;
        olc::vf2d location;
        olc::vf2d area;
    };
    std::vector<component> components;
    LAYOUT type;

public:
    UIContainer(olc::PixelGameEngine* gameObj,olc::vf2d sides,LAYOUT layout = MANUAL);
    virtual ~UIContainer() = default;

    void resize(olc::vf2d newSize);
    void updateSprite();
    void setTheme(olc::Pixel textColour,olc::Pixel background,olc::Pixel border,olc::Pixel highlight);
    void setTheme(theme newTheme);

    bool update(srpg_data::controls& inputs,olc::vf2d tl, olc::vf2d drawArea);
    void draw(olc::vf2d tl, olc::vf2d area);
    void addContainer(std::unique_ptr<UIContainer>& container,int index = -1,olc::vf2d coverage = {-1,-1});
    void addContainer(std::unique_ptr<UIContainer>& container,olc::vf2d loc,olc::vf2d coverage,int index = -1);

    void addTitle(std::string name, int fontSize,olc::vi2d spriteSize = {-1,-1},int index = -1);

    void addButton(std::string name,std::function<void()> task,olc::vi2d spriteSize = {-1,-1},int index = -1);

    void addDynamicText(std::function<std::string()> task,int maxLength,ALIGN alignment, int index = -1, olc::vi2d spriteSize = {-1,-1});
    void addDynamicText(std::function<std::string()> task,int maxLength,ALIGN alignment, olc::vf2d location, olc::vi2d spriteSize = {-1,-1});

private:
    void insert(std::unique_ptr<UI>& container,int order,olc::vf2d loc,olc::vf2d coverage);
    void updateLayout();
};

class Screen : public UIContainer
{
private:
uint8_t spriteLayer = 0;
public:
    Screen(olc::PixelGameEngine* gameObj,uint8_t drawLayer);
    ~Screen() = default;
    void display(srpg_data::controls& inputs);

};

class Element : public UI
{
public:
    Element(olc::PixelGameEngine* gameObj,olc::vf2d sides,theme colour);
    virtual ~Element() = default;

    virtual bool update(srpg_data::controls& inputs,olc::vf2d tl, olc::vf2d drawArea);
    virtual void draw(olc::vf2d tl, olc::vf2d drawSize);

    virtual void updateSprite() = 0;
};

class TitlePlate : public Element
{
protected:
    std::string name;
    int magnitude;
    public:
    TitlePlate(olc::PixelGameEngine* gameObj,theme colour,std::string name, olc::vf2d padding,int fontSize);
    ~TitlePlate() = default;
    void updateSprite();
};

class Button : public Element
{
    std::string name;
    std::function<void()> execute;
    olc::vf2d textCorner;
    bool highlighted = false;

    public:
    Button(olc::PixelGameEngine* gameObj,theme colour, std::string name, olc::vf2d area,std::function<void()> task);
    ~Button() = default;

    bool update(srpg_data::controls& inputs,olc::vf2d tl, olc::vf2d drawArea);
    void draw(olc::vf2d tl, olc::vf2d drawSize);
    void updateSprite();

};

class DynamicText : public Element
{
    std::function<std::string()> stringDisplay;

    ALIGN align;
public:
    DynamicText(olc::PixelGameEngine* gameObj,theme colour, olc::vf2d area, std::function<std::string()> task,ALIGN alignment = UI::LEFT);
    ~DynamicText() = default;
    void updateSprite();
    void draw(olc::vf2d tl, olc::vf2d drawSize);
};

#endif // MENUS_H_INCLUDED
