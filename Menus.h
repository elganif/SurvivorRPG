#ifndef MENUS_H_INCLUDED
#define MENUS_H_INCLUDED
//#include "Rectangle.h"
struct Rectangle;

class Interactable{

    protected:
    olc::PixelGameEngine* srpg;
    olc::vf2d tl;
    olc::vf2d sides;
    olc::Sprite* surface = nullptr;

    public:
    Interactable(olc::PixelGameEngine* game, olc::vf2d topL, olc::vf2d area):srpg(game),tl(topL),sides(area){}

    ~Interactable(){
        delete surface;
    }

    virtual void render(srpg_data::controls& inputs){};
};

class TitlePlate : public Interactable {
    std::string name;
    std::function<void()> execute;
    int magnitude;
    public:
    TitlePlate(olc::PixelGameEngine* game,std::string name,olc::vf2d topL, olc::vf2d area,int fontSize);
    ~TitlePlate(){};

    void render(srpg_data::controls& inputs);

};

class Button : public Interactable {
    std::string name;
    std::function<void()> execute;

    public:
    Button(olc::PixelGameEngine* game,std::string name,olc::vf2d topL, olc::vf2d area,std::function<void()> task);
    ~Button(){};

    void render(srpg_data::controls& inputs);

};

class MenuContainer : Interactable {

    std::vector<std::unique_ptr<Interactable>> components;

    public:
    MenuContainer(olc::PixelGameEngine* game, olc::vf2d topL, olc::vf2d area);
    ~MenuContainer(){};
    void render(srpg_data::controls& inputs);
    void addItem(std::unique_ptr<Interactable> element);

};

class Menu : Interactable {

    std::vector<std::unique_ptr<Interactable>> components;

    public:
    Menu(olc::PixelGameEngine* game, olc::vf2d topL, olc::vf2d area);
    ~Menu(){};

    void render(srpg_data::controls& inputs);
    void addItem(std::unique_ptr<Interactable> element);
};

#endif // MENUS_H_INCLUDED
