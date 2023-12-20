#ifndef MENUS_H_INCLUDED
#define MENUS_H_INCLUDED
//#include "Rectangle.h"
struct Rectangle;

class UIElement{

    protected:
    olc::PixelGameEngine* srpg;
    olc::vi2d tl;
    olc::vi2d sides;
    olc::Sprite* surface = nullptr;

    public:
    UIElement(olc::PixelGameEngine* game, olc::vf2d topL, olc::vf2d area):srpg(game),tl(topL),sides(area){
        surface = new olc::Sprite(sides.x,sides.y);
    }

    virtual ~UIElement(){
        delete surface;
    }

    virtual olc::vi2d render(srpg_data::controls& inputs)=0;
};

class TitlePlate : public UIElement {
    std::string name;
    int magnitude;
    public:
    TitlePlate(olc::PixelGameEngine* game,std::string name,olc::vf2d topL, olc::vf2d area,int fontSize);
    ~TitlePlate(){};

    olc::vi2d render(srpg_data::controls& inputs);

};

class Button : public UIElement {
    std::string name;
    std::function<void()> execute;

    public:
    Button(olc::PixelGameEngine* game,std::string name,olc::vf2d topL, olc::vf2d area,std::function<void()> task);
    ~Button(){};

    olc::vi2d render(srpg_data::controls& inputs);

};

class MenuContainer : UIElement {

    std::vector<std::unique_ptr<UIElement>> components;

    public:
    MenuContainer(olc::PixelGameEngine* game, olc::vf2d topL, olc::vf2d area);
    ~MenuContainer(){};
    olc::vi2d render(srpg_data::controls& inputs);
    void addItem(std::unique_ptr<UIElement> element);

};

class Menu : UIElement {

    std::vector<std::unique_ptr<UIElement>> components;

    public:
    Menu(olc::PixelGameEngine* game, olc::vf2d topL, olc::vf2d area);
    ~Menu(){};

    olc::vi2d render(srpg_data::controls& inputs);
    void addItem(std::unique_ptr<UIElement> element);
};

#endif // MENUS_H_INCLUDED
