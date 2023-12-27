#ifndef MENUS_H_INCLUDED
#define MENUS_H_INCLUDED
struct Rectangle; // from Rectangle.h

class UIElement{

    protected:
    olc::PixelGameEngine* srpg;
    olc::vi2d sides;
    std::shared_ptr<olc::Sprite> surface = nullptr;

    public:
    UIElement(olc::PixelGameEngine* game, olc::vf2d area):srpg(game),sides(area){
        surface = std::make_shared<olc::Sprite>(sides.x,sides.y);
    }

    olc::vi2d getSize(){return sides;}

    virtual ~UIElement(){};

    virtual olc::vi2d render(olc::vi2d tl,srpg_data::controls& inputs)=0;
};

class TitlePlate : public UIElement {
    std::string name;
    int magnitude;
    public:
    TitlePlate(olc::PixelGameEngine* game,std::string name, olc::vf2d padding,int fontSize);
    ~TitlePlate(){};

    olc::vi2d render(olc::vi2d tl, srpg_data::controls& inputs);

};

class Button : public UIElement {
    std::string name;
    std::function<void()> execute;

    public:
    Button(olc::PixelGameEngine* game,std::string name, olc::vf2d area,std::function<void()> task);
    ~Button(){};

    olc::vi2d render(olc::vi2d tl,srpg_data::controls& inputs);

};

class MenuContainer : public UIElement {
protected:
    std::vector<std::unique_ptr<UIElement>> components;

    int borderZone = 0;
public:
    MenuContainer(olc::PixelGameEngine* game, olc::vf2d area);
    ~MenuContainer(){};
    void prepareRender();
    void drawNewBackground();
    olc::vi2d render(olc::vi2d tl,srpg_data::controls& inputs);
    void addItem(std::unique_ptr<UIElement> element);

};

class Menu : public MenuContainer {
    olc::vi2d center;


    public:

    Menu(olc::PixelGameEngine* game, olc::vi2d centerP, olc::vi2d area = {100,100});
    ~Menu(){};

    void render(srpg_data::controls& inputs);

    olc::vi2d render(olc::vi2d tl,srpg_data::controls& inputs);

};


class hud : public MenuContainer{
 private:
 ///inherited variables for refrence
//    olc::PixelGameEngine* srpg;
//    olc::vi2d sides;
//    std::shared_ptr<olc::Sprite> surface = nullptr;
//    std::vector<std::unique_ptr<UIElement>> components;
    olc::vi2d tl;
public:
    hud(olc::PixelGameEngine* game,olc::vi2d topL ,olc::vi2d area);

    ~hud();

    void render(srpg_data::controls& inputs);
};

class hudDisplay : public UIElement {
public:
    olc::vi2d tl;
    hudDisplay(olc::PixelGameEngine* game,olc::vf2d area, olc::vf2d topL ) : UIElement(game,area),tl(topL){}
    virtual ~hudDisplay() = default;
    virtual void update() = 0;
    virtual void render() = 0;
};

class valueBar : public hudDisplay{
public:
    valueBar(olc::PixelGameEngine* game,olc::vi2d topL ,olc::vi2d area);
    ~valueBar() = default;

    void update();
    void render();

};
#endif // MENUS_H_INCLUDED
