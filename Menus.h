#ifndef MENUS_H_INCLUDED
#define MENUS_H_INCLUDED
//struct Rectangle; // from Rectangle.h

class Display;

class UI
{
public:
    UI()=delete;

    static std::unique_ptr<Display> makeDisplay(olc::PixelGameEngine* game, olc::vi2d loc, olc::vi2d area);
    enum Align {LEFT,CENTER,RIGHT};
};

class Element
{
protected:
    olc::PixelGameEngine* pge;
    olc::vf2d pixelArea;
    std::unique_ptr<olc::Sprite> sprite = nullptr;
    olc::vi2d borderZone = {5,5};
private:
    enum funct{ /// Types of components listed in order of processing and drawing. lower items draw overtop over higher items.
        CLICK, /// On click event
        BG, /// Background
        TEXT, /// Text display
        CONTAINER, /// Sub Grid of elements
        /// others to come
    };
    class Feature
    {
    protected:
        olc::PixelGameEngine* pge;
        Feature(olc::PixelGameEngine* game):pge(game){};
    public:
        virtual ~Feature() = default;
        virtual bool update(srpg::controls& inputs,olc::vf2d tl, olc::vf2d drawArea){return false;};
        virtual void draw(olc::vi2d borderArea, olc::vf2d drawSize){};

    };

    std::map<funct,std::shared_ptr<Feature>> features;
    bool spriteCurrent = true;
    void addSprite(); /// Should be called by elements needing to draw.

public:
    Element(olc::PixelGameEngine* game, olc::vf2d sides);
    ~Element() = default;

    bool update(srpg::controls& inputs,olc::vf2d tl, olc::vf2d drawArea);
    void draw(olc::vf2d tl, olc::vf2d drawSize);

    bool isCurrent();
    void updateSprite();


public: /// Container creation and access
    Element* makeGrid(int x, int y);
    Element* setBlock(int x, int y, olc::vi2d blocks = {1,1});
    Element* getBlock(int x, int y);
private:
    class Container : public Feature
    {
    friend Element;
    protected:
        struct component {
            std::shared_ptr<Element> item = nullptr;
            olc::vf2d area;
        };
        std::map<olc::vi2d,component> elements;
        olc::vi2d mapSize;
    public:
        Container(olc::PixelGameEngine* game, olc::vi2d mapArea);

        bool update(srpg::controls& inputs,olc::vf2d tl, olc::vf2d drawArea)override;
        void draw(olc::vi2d borderArea, olc::vf2d drawSize)override;

        std::shared_ptr<Element> insertBlock(std::shared_ptr<Element> newBlock,olc::vi2d loc,olc::vi2d blocks);
        std::shared_ptr<Element> getBlock(int x, int y);
    };

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
    Element& text(std::string name, olc::Pixel textColour, UI::Align alignment = UI::CENTER);
private:
    class TextPlate : public Feature
    {
    protected:
        std::string name;
        olc::Pixel colour;
        UI::Align align;
    public:
        TextPlate(olc::PixelGameEngine* game,std::string name,olc::Pixel textColour,UI::Align align = UI::CENTER);
        ~TextPlate() = default;
        void draw(olc::vi2d borderArea, olc::vf2d drawSize) override;
    };

public:
    Element& addDynamicText(std::function<std::string()> task,olc::Pixel textColour, int maxLength,UI::Align alignment = UI::CENTER);
private:
    class DynamicText : public Feature
    {
        std::function<std::string()> stringDisplay;
        olc::Pixel colour;
        int length;
        UI::Align align;
    public:
        DynamicText(olc::PixelGameEngine* game, std::function<std::string()> task,olc::Pixel textColour,int expectedLength, UI::Align alignment = UI::CENTER);
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

class Display : public Element
{
private:
    olc::vi2d tl;
    std::unique_ptr<olc::Decal> decal = nullptr;
public:
    Display(olc::PixelGameEngine* game,olc::vi2d loc, olc::vi2d area);
    ~Display() = default;
    void display(srpg::controls& inputs);
};
#endif // MENUS_H_INCLUDED
