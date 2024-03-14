#ifndef MENUS_H_INCLUDED
#define MENUS_H_INCLUDED
//struct Rectangle; // from Rectangle.h
//class Feature;

class UI
{
public:
    struct theme{
        olc::Pixel text;
        olc::Pixel bg;
        olc::Pixel border;
        olc::Pixel highlight;
    };
protected:
    olc::PixelGameEngine* pge;
    olc::vf2d pixelArea;
    olc::vi2d borderZone = {5,5};
public:
    UI(olc::PixelGameEngine* game, olc::vi2d sides);
    virtual ~UI() = default;

    virtual bool update(srpg::controls& inputs,olc::vf2d tl, olc::vf2d drawArea) = 0;
    virtual void draw(olc::vf2d tl, olc::vf2d drawArea) = 0;

    void setTheme(olc::Pixel textColour,olc::Pixel background,olc::Pixel border,olc::Pixel highlight);
    void setTheme(theme newTheme);

    olc::vi2d getSize() {return pixelArea;};


};
enum Align {LEFT,CENTER,RIGHT};

class Element;

class UIContainer : public UI
{
protected:
    std::unique_ptr<Element> element = nullptr;
    struct component {
        std::shared_ptr<UI> item = nullptr;
        olc::vf2d area;
    };

    std::map<olc::vi2d,component> componentMap;
    olc::vi2d mapSize;

public:
    UIContainer(olc::PixelGameEngine* game, olc::vi2d drawArea,olc::vi2d mapArea = {1,1});
    virtual ~UIContainer() = default;

    bool update(srpg::controls& inputs,olc::vf2d tl, olc::vf2d drawArea);
    void draw(olc::vf2d tl, olc::vf2d area);

    void addContainer(std::unique_ptr<UIContainer>& container,olc::vi2d loc,olc::vi2d gridArea = {1,1});
    UIContainer* accessContainer(olc::vi2d loc);

    Element* editContainerElement();

    Element* addElement(olc::vi2d loc,olc::vi2d gridArea = {1,1});
    Element* editElement(olc::vi2d loc);
};

class Screen : public UIContainer
{
private:
public:
    Screen(olc::PixelGameEngine* game, uint8_t drawLayer);
    ~Screen() = default;
    void display(srpg::controls& inputs);
};



class Element : public UI
{
private:
    class Feature
    {
    protected:
        olc::PixelGameEngine* pge;
        Feature(olc::PixelGameEngine* game):pge(game){};
    public:
        virtual ~Feature() = default;
        virtual bool update(srpg::controls& inputs,olc::vf2d tl, olc::vf2d drawArea){return false;};
        virtual void draw(olc::vi2d borderArea, olc::vf2d drawSize){};
        enum funct{ /// Types of components listed in order of processing and drawing. lower items appear over higher items.
            BG,
            TEXT,
            CLICK,
            /// others to come
        };
    };

    std::map<Feature::funct,std::shared_ptr<Feature>> features;
    bool spriteCurrent = false;
    std::unique_ptr<olc::Sprite> sprite = nullptr;
    std::unique_ptr<olc::Decal> decal = nullptr;
    friend UIContainer;
public:
    Element(olc::PixelGameEngine* game, olc::vf2d sides);
    ~Element() = default;

    bool update(srpg::controls& inputs,olc::vf2d tl, olc::vf2d drawArea)override;
    void draw(olc::vf2d tl, olc::vf2d drawSize)override;

    bool isCurrent();
    void updateSprite();

public:
    Element& background(olc::Pixel bgColour);
private:
    class Background : public Feature
    {
    protected:
        olc::Pixel colour;
    public:
        Background(olc::PixelGameEngine* game,olc::Pixel bgColour);
        ~Background() = default;
        void draw(olc::vi2d borderArea, olc::vf2d drawSize) override;
    };

public:
    Element& text(std::string name, olc::Pixel textColour, Align alignment = CENTER);
private:
    class TextPlate : public Feature
    {
    protected:
        std::string name;
        olc::Pixel colour;
        Align align;
    public:
        TextPlate(olc::PixelGameEngine* game,std::string name,olc::Pixel textColour,Align align = CENTER);
        ~TextPlate() = default;
        void draw(olc::vi2d borderArea, olc::vf2d drawSize) override;
    };

public:
    Element& addDynamicText(std::function<std::string()> task,olc::Pixel textColour, int maxLength,Align alignment = CENTER);
private:
    class DynamicText : public Feature
    {
        std::function<std::string()> stringDisplay;
        olc::Pixel colour;
        int length;
        Align align;
    public:
        DynamicText(olc::PixelGameEngine* game, std::function<std::string()> task,olc::Pixel textColour,int expectedLength, Align alignment = CENTER);
        ~DynamicText() = default;
        bool update(srpg::controls& inputs,olc::vf2d tl, olc::vf2d drawArea)override;
        void draw(olc::vi2d borderArea,olc::vf2d drawSize)override;
    };

public:
    Element& addButton(std::function<void()> task);
private:
    class Button : public Feature
    {
        std::function<void()> execute;

        public:
        Button(olc::PixelGameEngine* game,std::function<void()> task);
        ~Button() = default;

        bool update(srpg::controls& inputs,olc::vf2d tl, olc::vf2d drawArea) override;
    };

};
#endif // MENUS_H_INCLUDED
