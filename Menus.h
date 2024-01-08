#ifndef MENUS_H_INCLUDED
#define MENUS_H_INCLUDED
struct Rectangle; // from Rectangle.h

class UI{

    protected:
    olc::PixelGameEngine* srpg;
    std::shared_ptr<olc::Sprite> sprite = nullptr;
    olc::vi2d sides;

    public:
    UI(olc::PixelGameEngine* game, olc::vi2d area):srpg(game),sides(area){
        sprite = std::make_shared<olc::Sprite>(sides.x,sides.y);
    }
    virtual ~UI() = default;
    virtual bool update() = 0;
    virtual olc::vi2d prepareRender() = 0;
    virtual std::shared_ptr<olc::Sprite> onDisplay(srpg_data::controls& inputs) = 0;

    olc::vi2d getSize(){return sides;}
};

class Container : public UI {
protected:
    struct UIcomponent {
        std::unique_ptr<UI> item;
        olc::vi2d pos;
    };
    std::vector<UIcomponent> components;

public:
    enum LAYOUT {VERTICAL, ARRANGED};
protected:
    LAYOUT type;

    olc::vi2d borderZone = {0,0};

public:
    Container(olc::PixelGameEngine* game, olc::vi2d area,LAYOUT layout = VERTICAL);
    virtual ~Container() = default;

    void addItem(std::unique_ptr<UI> element,olc::vi2d position = {0,0});
    bool update();
    virtual olc::vi2d prepareRender();
    virtual std::shared_ptr<olc::Sprite> onDisplay(srpg_data::controls& inputs);
    void drawNewBackground();

private:
    olc::vi2d prepareVertRender();
    olc::vi2d prepareArrangedRender();
};

class Element : public UI {

public:
    Element(olc::PixelGameEngine* game, olc::vi2d area):UI(game,area){
        sprite = std::make_shared<olc::Sprite>(sides.x,sides.y);
    }
    virtual ~Element() = default;
};

class Menu : public Container {
    olc::vi2d center;

    public:

    Menu(olc::PixelGameEngine* game, olc::vi2d centerP, olc::vi2d area = {1,1});
    ~Menu() = default;
    void render(srpg_data::controls& inputs);

};

class TitlePlate : public Element {
    std::string name;
    int magnitude;
    public:
    TitlePlate(olc::PixelGameEngine* game,std::string name, olc::vf2d padding,int fontSize);
    ~TitlePlate(){};

    bool update();
    olc::vi2d prepareRender();
    std::shared_ptr<olc::Sprite> onDisplay(srpg_data::controls& inputs);

};

class Button : public Element {
    std::string name;
    std::function<void()> execute;

    public:
    Button(olc::PixelGameEngine* game,std::string name, olc::vf2d area,std::function<void()> task);
    ~Button(){};

    bool update();
    olc::vi2d prepareRender();
    std::shared_ptr<olc::Sprite> onDisplay(srpg_data::controls& inputs);

};


class Panel : public Container{
 private:
    olc::vi2d tl;
public:
    //Panel(olc::PixelGameEngine* game,int left,int top,int right,int bottom);
    Panel(olc::PixelGameEngine* game,olc::vi2d topL ,olc::vi2d area);

    ~Panel()= default;

    void render(srpg_data::controls& inputs);

};

class hudDisplay : public Element {
public:
    hudDisplay(olc::PixelGameEngine* game,olc::vf2d area) : Element(game,area){}
    virtual ~hudDisplay() = default;
    virtual bool update() = 0;
    virtual void onDisplay() = 0;
};

class textDisplay : public Element{
    textDisplay(olc::PixelGameEngine* game,olc::vi2d topL ,olc::vi2d area);
    ~textDisplay() = default;

    void onDisplay();

};
class valueBar : public Element{
public:
    valueBar(olc::PixelGameEngine* game,olc::vi2d topL ,olc::vi2d area);
    ~valueBar() = default;

    bool update();
    void onDisplay();

};
#endif // MENUS_H_INCLUDED
