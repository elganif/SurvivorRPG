#include "olcPixelGameEngine.h"
#include "olcPGEX_TransformedView.h"
#include "srpg_data.h"
#include "Rectangle.h"
#include "Entities.h"


///class QuadTree
int QuadTree::depthLimit = 6; /// initialize static depth limit.

/// Internally used contsructor that uses a passed rectangle instead of corner coordinates.
QuadTree::QuadTree(Rectangle newArea, int newDepth,QuadTree* parent)
    :quadArea(newArea),depth(newDepth),parentNode(parent)
{}

/// General constructor for external calls
QuadTree::QuadTree(olc::vf2d newtl, olc::vf2d newbr)
    :quadArea({newtl,newbr}),depth(0),parentNode(nullptr)
{}

/// Deconstruct, cleaning up lists and subquads.
QuadTree::~QuadTree(){
    // to ensure shared pointers and memory are cleaned up clear the list and delete each quad
    entStored.clear();
    for(int i = 0; i < 4; i++){
        delete quads[i];
    }
};

/// Standard item insertion. calls method on inserted item to give it node and list info for validation later.
/// (I would like to find a simple way to accomplish the same effect that would allow a template design instead of specialized.)
void QuadTree::insertItem(const std::shared_ptr<Entity>& newEnt){
     // Check if it belongs lower in the tree
     for(int i = 0;i < 4;i++){
        if(depth < depthLimit && childArea[i].contains(newEnt->getBoxCollider() )){
            if(!quads[i]){
                quads[i] = new QuadTree(childArea[i],depth+1,this);
            }
            quads[i]->insertItem(newEnt);
            return;
        }
    } // else we store it in this element
    entStored.push_front(newEnt);
    newEnt->setTreeLocation(this,entStored.begin());

    return;
}

/// Checks an area for any items in the tree that overlap.
void QuadTree::getOverlapItems(Rectangle area, std::list<std::shared_ptr<Entity>>& returns){
    // collect overlaps from children
    for(int i = 0;i < 4;i++){
        if(quads[i] && childArea[i].overlaps(area)){
             quads[i]->getOverlapItems(area,returns);
        }
    }
    // add any overlaped items from this layer
    for(std::shared_ptr<Entity>& ent : entStored){////auto it = entStored.begin(); it!= entStored.end(); it++){
        if(area.overlaps(ent->getBoxCollider())){
            returns.push_back(ent);
        }
    }
    return;
}

/// Starting at targLoc finds up to numTarg targets within range using specified function. returns number of targets found
int QuadTree::getFoes(olc::vf2d targetLoc, float range, int numTarg, std::list<std::shared_ptr<Entity>>& returns, TARG targType){

    /// First take targType and exchange it for the function used to determine which entities are desired
    std::function<float(const std::shared_ptr<Entity> f, const std::shared_ptr<Entity> s)> targMethod;

    switch (targType){
        case CLOSE:
            targMethod = [targetLoc](const std::shared_ptr<Entity> f, const std::shared_ptr<Entity> s)
                                    {return (f->location() - targetLoc).mag2() < (s->location() - targetLoc).mag2(); };
        break;
        case WEAK:
            targMethod = [](const std::shared_ptr<Entity> f, const std::shared_ptr<Entity> s)
                                    {return (( (Npc*)(f.get()) )->getHP() < ( (Npc*)(s.get()) )->getHP()); };
        break;
        case STRONG:
            targMethod = [](const std::shared_ptr<Entity> f, const std::shared_ptr<Entity> s)
                                    {return (( (Npc*)(f.get()) )->getHP() > ( (Npc*)(s.get()) )->getHP()); };
        break;
    }

    /// square up range now as other distance calculations will also be squared
    range = range * range;
    /// call and return recursive search method
    getFoes(targetLoc,range,numTarg,returns,targMethod);
    return returns.size();
}

/// Recursive search for NPCs meeting targeting requirements.
void QuadTree::getFoes(olc::vf2d targetLoc, float range, int numTarg,std::list<std::shared_ptr<Entity>>& returns,
                        std::function<bool(const std::shared_ptr<Entity> f, const std::shared_ptr<Entity> s)> targType)
{

    /// Start by traveling to the bottom of the tree. Items will be checked on the return.
    for(int i = 0;i < 4;i++){
        if(childArea[i].contains(targetLoc) && quads[i]){
             quads[i]->getFoes(targetLoc,range,numTarg,returns,targType);
        }
    }

    /// Check entities in the current quad
    for(auto ent = entStored.begin(); ent != entStored.end(); ent++){
        if ( (*ent)->whoAreYou() != Entity::NPC ){
            continue; /// Not who we are looking for
        }
        if(( (*ent)->location() - targetLoc).mag2() > range ){
            continue; /// ent is out of range
        }
        if(returns.size() >= numTarg && targType( (*ent),returns.back())){
            continue; /// target is not better than current options
        } /// else
        /// add ent, sort into correct location and remove last one if list was full
        returns.push_front(*ent);
        returns.sort(targType);
        if(returns.size() >= numTarg){
            returns.pop_back();
        }
    }

    /// check sub-quads that do not contain the point are in range and search them as needed
    for(int i = 0; i < 4; i++){
        if(!childArea[i].contains(targetLoc) && quads[i]){
            olc::vf2d closestPoint = targetLoc.clamp(childArea[i].tl,childArea[i].tl + childArea[i].sides);
            float qDist = (closestPoint - targetLoc).mag2();
            if(qDist < range){
                quads[i]->getFoes(targetLoc,range,numTarg,returns,targType);
            }
        }
    }
}

/// Position validator that is called directly by entities each time one moves.
/// Treenode and entIT Values are to be updated by elevator functions once entity
/// is placed in new node.
void QuadTree::validateEnt(QuadTree*& treeNode, std::list<std::shared_ptr<Entity>>::iterator& entIT){
    /// First record a stable copy of the iterator because once the entity is moved the entIT will already be updated on the return trip

    std::list<std::shared_ptr<Entity>>::iterator myEntIT = entIT;

    if(depth != 0 && !quadArea.contains((*entIT)->getBoxCollider())){

        parentNode->upEscalator(treeNode,entIT);
        entStored.erase(myEntIT);
        srpg_data::timers->stop("validation");
        return;
    } // else not going up, check down
    for(int i = 0; i < 4; i++){
        if(depth < depthLimit && childArea[i].contains((*entIT)->getBoxCollider())){
            if(!quads[i]){
                quads[i] = new QuadTree(childArea[i],depth + 1,this);
            }
            quads[i]->downEscalator(treeNode,entIT);
            entStored.erase(myEntIT);
            return;
        }
    } // else: Validation successful, return.
    return;
}

/// Passes units up the tree recursively during the validation process.
/// Updates treeNode and entIT once correct node is found.
void QuadTree::upEscalator(QuadTree*& treeNode, std::list<std::shared_ptr<Entity>>::iterator& entIT){
    if(depth != 0 && !quadArea.contains((*entIT)->getBoxCollider())){
        parentNode->upEscalator(treeNode,entIT);
        return;
    } //else we check sub quads
    for(int i = 0; i < 4; i++){
        if(depth < depthLimit && childArea[i].contains((*entIT)->getBoxCollider())){
            if(!quads[i]){
                quads[i] = new QuadTree(childArea[i],depth + 1,this);
            }
            quads[i]->downEscalator(treeNode,entIT);
            return;
        }
    } // else : we know it is not going up, but does not fit in a sub quad
    entStored.push_front(std::move(*entIT));
    treeNode = this;
    entIT = entStored.begin();
    return;
}

/// passes units down the tree recursively during validation.
/// Updates treeNode and entIT once correct node is found
void QuadTree::downEscalator(QuadTree*& treeNode, std::list<std::shared_ptr<Entity>>::iterator& entIT){
    for(int i = 0; i < 4; i++){
        if( depth < depthLimit && childArea[i].contains((*entIT)->getBoxCollider())){
            if(!quads[i]){
                quads[i] = new QuadTree(childArea[i],depth + 1,this);
            }
            quads[i]->downEscalator(treeNode,entIT);
            return;
        }
    } // else : we know its not going up, but does not fit in a sub quad
    entStored.push_front(std::move(*entIT));
    treeNode = this;
    entIT = entStored.begin();
    return;
}

/// Removal function for expired entities.
void QuadTree::removeMe(QuadTree*& treeNode, std::list<std::shared_ptr<Entity>>::iterator& entIT){
    if(entIT != entStored.end()){
        entStored.erase(entIT);
        entIT = entStored.end();
        treeNode = nullptr;
    }
    return;
}

/// Debug function to draw all nodes to the screen, or to the specified area.
void QuadTree::drawTree(Rectangle area, olc::Pixel item,olc::Pixel noItem ){
    if(quadArea.overlaps(area)){
        if(entStored.size() > 0){
            srpg_data::viewer->DrawRect(quadArea.tl,quadArea.sides,item);
        } else {
            srpg_data::viewer->DrawRect(quadArea.tl,quadArea.sides,noItem);
        }

        for(int i = 0;i < 4; i++){
            if(quads[i] && childArea[i].overlaps(area)){
                quads[i]->drawTree(area, item,noItem);
            }
        }
    }
}

/// cleans up the tree clearing any empty nodes.
/// returns bool indicating if it is clean and can be deleted or if there is any object under that needs to be preserved.
bool QuadTree::clean()
{
    bool isClean = entStored.size() == 0;
    for(int i = 0; i < 4; i++){
        if(quads[i]){
            if(quads[i]->clean()){
                delete quads[i];
                quads[i] = nullptr;
                continue;
            } /// else sub quad contains entities.
            isClean = false;
        }
    }
    return isClean;
}

/// Count of all items in this node and each sub node.
/// starts with the count of this node, then adds the size of each subquad.
int QuadTree::size()
{
    int thisCount = entStored.size();
    for(int i = 0;i < 4;i++){
        thisCount += quads[i] ? quads[i]->size() : 0;
    }
    return thisCount;
}

/// Counts total number of nodes in the tree. For debugging or performance checking.
int QuadTree::activity()
{
    int numQuads = 1;
    for(int i = 0; i < 4; i++){
        numQuads += quads[i] ? quads[i]->activity() : 0;
    }
    return numQuads;
}

/// Checks for the deepest node. for Debugging or performance checking.
int QuadTree::curDepth()
{
    int depthCharge = depth;
    for (int i = 0; i < 4; i++){
        if(quads[i]){
            int depthcheck = quads[i]->curDepth();
            if(depthcheck > depthCharge)
                depthCharge = depthcheck;
        }
    }
    return depthCharge;
}




/// class Profiler {

    Profiler::Profiler()
    {
    }

    void Profiler::frameMark()
    { /// Called to mark when a new frame cycle is started.
        stop(defaultName);
        events[defaultName].push_front(Event(defaultName,frameCounter++));
        /// remove events over 1 second old to prevent excessive memory build up.
        std::chrono::_V2::steady_clock::time_point now = std::chrono::_V2::steady_clock::now();
        for(auto& [key,eventList] : events ){
            eventList.remove_if( [now](auto &value)
                                    { if( value.stopT.time_since_epoch().count() == 0) {return false;}
                                    return std::chrono::duration<float>(now - value.startT).count() > 1.0f ;} );
        }
    }

    /// Create an event and add it at first time in list
    void Profiler::start(std::string timerID)
    {
        /// if the list is empty or if the last timer has been closed properly we create new timer.
        /// this is to handle calls in recursive functions and treat them as a single event until fully closed.
        if(events[timerID].size() == 0 || events[timerID].front().openCount == 0){
            events[timerID].push_front(Event(timerID,events[defaultName].front().frameNum));
        } else {
            /// increment open Counters on this label.
            events[timerID].front().openCount++ ;
        }
    }

    /// Update the end time of the front item.
    float Profiler::stop(std::string timerID)
    {
        /// if the event list is empty, or the prior timer is completed we do not want to access or overright stopT so skip and return 0.
        if(events[timerID].size() == 0 || events[timerID].front().openCount == 0){
            return 0;
        }
        /// record the current stop time, decriment open counter, return the passed time.
        events[timerID].front().stopT = std::chrono::_V2::steady_clock::now();
        events[timerID].front().openCount-- ;
        return events[timerID].front().passedTime();
    }

    void Profiler::drawDebug(olc::PixelGameEngine* game)
    {
        start("debug"); /// valuable to know how much time is lost to createing this display.


        /// the first closed default event is used to determine graph scaleing
        std::chrono::_V2::steady_clock::time_point frameBegin;
        std::chrono::_V2::steady_clock::time_point frameEnd;
        int lastFrameNum;
        bool frameFound = false;

        for(Event& checkFrame : events[defaultName]){
            if(!frameFound && checkFrame.stopT.time_since_epoch().count() != 0){
                frameBegin = checkFrame.startT;
                frameEnd = checkFrame.stopT;
                lastFrameNum = checkFrame.frameNum;
                frameFound = true;
                break; // Closed frame found no need to continue for loop.
            }
        }
        if(!frameFound){
            return; // If no closed frame was found We dont have assigned values. Expected to happen on first frame call.
        }

        game->SetDrawTarget(nullptr); /// Draw to default layer above all others
        float numFramesShown = 5;
        int lineHeight = 10;

        int wide = game->ScreenWidth();
        int height = game->ScreenHeight();

        float frameScale = (wide/numFramesShown) / (std::chrono::duration<float>(frameEnd - frameBegin).count());
        float displayStart = (wide/numFramesShown);

        float totTimeUsed = std::chrono::duration<float>(frameEnd - frameBegin).count();

        int drawHeight = height - lineHeight;

        for(auto& [key,eventList] : events ){
            if(eventList.size() == 0){
                continue; /// no need to operate on empty lists
            }
            /// Check if a timer is being called an excessive number of times. It will still be calculated for times, but on 1 frame will be drawn.
            bool skipExcessiveDraws = false;
            if(eventList.size() >= 100 + events[defaultName].size())
                skipExcessiveDraws = true;

            /// iterate through this event drawing the time frames it occupied
            float totalTimeRun = 0;
            int eventPerFrame = 0;
            for(Event& timer : eventList){
                if(timer.frameNum > lastFrameNum || timer.stopT.time_since_epoch().count() == 0)
                    continue; // this cycle or timer event is not yet closed and will be skipped.
                if(skipExcessiveDraws && timer.frameNum < lastFrameNum)
                    break;// if a timer has excessive events once we finish the timed frame we skip the remainder of the list.

                float ending = std::chrono::duration<float>(frameEnd - timer.stopT).count();
                float length = timer.passedTime();
                olc::Pixel colour = olc::DARK_YELLOW;

                /// Accumulate some data based on the last rendered frame.
                if(timer.frameNum == lastFrameNum){
                    totalTimeRun += length;
                    eventPerFrame ++;
                    colour = olc::GREEN;
                }
                int rectStart = ending*frameScale+displayStart;
                /// Only need to draw if it will appear on screen.
                if (rectStart < wide)
                    game->DrawRect(rectStart,drawHeight,length*frameScale,lineHeight,colour);

            }

            float percentOfFrame = totalTimeRun / (1.0f/60.0f) * 100;
            /// Write our analytics about the bar on screen
            game->DrawString(lineHeight,drawHeight+1,key +" "+ std::to_string(eventPerFrame),olc::YELLOW);

            std::string percentage = std::to_string(percentOfFrame);
            percentage = percentage.substr(0,percentage.find(".")+4);
            game->DrawString(displayStart - (percentage.size() * 8),drawHeight+1,percentage,olc::YELLOW);
            drawHeight -= lineHeight;
        }
        for(int i = displayStart; i < wide;i += displayStart){
            game->DrawLine(i,drawHeight,i,height,olc::BLUE);
        }
    stop("debug");
    }

