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

int findSubQuad(Rectangle rec,olc::vf2d center)
{
    olc::vi2d quadrent = rec.ufo(center);
    int targetQuad = quadrent.x * 3 + quadrent.y;
    switch (targetQuad){
        case 4:
            return 0;
        case 2:
            return 1;
        case -2:
            return 2;
        case -4:
            return 3;
        default:
            return -1;
    }
}
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
    entStored.insert(std::make_pair(newEnt->getUID(),newEnt));
    newEnt->setTreeLocation(this);//,entStored.begin());

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
    for(auto& [key,ent] : entStored){////auto it = entStored.begin(); it!= entStored.end(); it++){
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
                                    {return ( ( (Npc*)(f.get()) )->getHP() < ( (Npc*)(s.get()) )->getHP() ); };
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
    for(auto& [key,ent] : entStored){
        if ( ent->whoAreYou() != Entity::NPC ){
            continue; /// Not who we are looking for
        }
        if(( ent->location() - targetLoc).mag2() > range ){
            continue; /// ent is out of range
        }
        if(returns.size() > numTarg && targType( ent,returns.back()) ){
            continue; /// target is not better than current options
        } /// else
        /// add ent, sort into correct location and remove last one if list was full
        returns.push_front(ent);
        returns.sort(targType);
        if(returns.size() > numTarg){
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
void QuadTree::validateEnt(QuadTree*& treeNode, int entID){
    /// First record a stable copy of the iterator because once the entity is moved the entIT will already be updated on the return trip

    std::map<int,std::shared_ptr<Entity>>::iterator entLoc = entStored.find(entID);
    if(entLoc == entStored.end())
        return; // no such entity here

    if(depth != 0 && !quadArea.contains(entLoc->second->getBoxCollider())){

        parentNode->upEscalator(treeNode,entLoc->second);
        entStored.erase(entID);

        return;
    } // else not going up, check down
    for(int i = 0; i < 4; i++){
        if(depth < depthLimit && childArea[i].contains(entLoc->second->getBoxCollider())){
            if(!quads[i]){
                quads[i] = new QuadTree(childArea[i],depth + 1,this);
            }
            quads[i]->downEscalator(treeNode,entLoc->second);
            entStored.erase(entID);
            return;
        }
    } // else: Validation successful, nothing to do
}

/// Passes units up the tree recursively during the validation process.
/// Updates treeNode once correct node is found.
void QuadTree::upEscalator(QuadTree*& treeNode, std::shared_ptr<Entity>& entIT){
    if(depth != 0 && !quadArea.contains(entIT->getBoxCollider())){
        parentNode->upEscalator(treeNode,entIT);
        return;
    } //else we check sub quads
    for(int i = 0; i < 4; i++){
        if(depth < depthLimit && childArea[i].contains(entIT->getBoxCollider())){
            if(!quads[i]){
                quads[i] = new QuadTree(childArea[i],depth + 1,this);
            }
            quads[i]->downEscalator(treeNode,entIT);
            return;
        }
    } // else : we know it is not going up, but does not fit in a sub quad thus belongs here
    entStored.insert(std::make_pair(entIT->getUID(),std::move(entIT) ));
    treeNode = this;
}

/// passes units down the tree recursively during validation.
/// Updates treeNode
void QuadTree::downEscalator(QuadTree*& treeNode, std::shared_ptr<Entity>& entIT){
    for(int i = 0; i < 4; i++){
        if( depth < depthLimit && childArea[i].contains(entIT->getBoxCollider())){
            if(!quads[i]){
                quads[i] = new QuadTree(childArea[i],depth + 1,this);
            }
            quads[i]->downEscalator(treeNode,entIT);
            return;
        }
    } // else : we know its not going up, but does not fit in a sub quad
    entStored.insert(std::make_pair(entIT->getUID(),std::move(entIT) ));
    treeNode = this;
}

/// Removal function for expired entities.
void QuadTree::removeMe(QuadTree*& treeNode, int entID){
    std::map<int,std::shared_ptr<Entity>>::iterator entLoc = entStored.find(entID);
    if(entLoc == entStored.end())
        return; // no such entity here
    entLoc->second->setTreeLocation(nullptr);
    entStored.erase(entID);
}

/// Debug function to draw all nodes to the specified area.
void QuadTree::drawTree(Rectangle area, olc::Pixel item,olc::Pixel noItem ){
    if(quadArea.overlaps(area)){
        if(entStored.size() > 0){
            srpg::viewer->DrawRect(quadArea.tl,quadArea.sides,item);
        } else {
            srpg::viewer->DrawRect(quadArea.tl,quadArea.sides,noItem);
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
    bool isClean = (entStored.size() == 0);
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
        coreFrame.emplace(frameCounter++,Event(defaultName,frameCounter));
    }

    void Profiler::frameMark()
    { /// Called to mark when a new frame cycle is started.
        coreFrame.emplace(frameCounter++, Event(defaultName,frameCounter));

        /// set the stop time of last counter to the start time we just created.
        /// because an event is added in contruction and we just added a second we can safely access the second member of the list.
        std::next(coreFrame.rbegin())->second.stopT = coreFrame.rbegin()->second.startT;
        std::next(coreFrame.rbegin())->second.openCount--;

        /// remove events keeping track of only the last 60.
        while(coreFrame.size() > 61){
            coreFrame.erase(coreFrame.begin());
        }

        /// remove map events that nolonger have a corosponding coreFrame event
        for(auto& [key,eventList] : events ){
            while(eventList.size() > 0 && eventList.back().frameNum < coreFrame.begin()->first)
                eventList.pop_back();
        }
    }

    /// Create an event and add it at first time in list
    void Profiler::start(std::string timerID)
    {
        /// if this list is empty or if the last timer has been closed properly we create new timer.
        /// this is to handle calls in recursive functions and treat them as a single event until fully closed.
        if(events[timerID].size() == 0 || events[timerID].front().openCount == 0){
            events[timerID].push_front(Event(timerID,coreFrame.rbegin()->second.frameNum));
            return;
        }
        /// otherwise increment open Counters on this timmer.
        events[timerID].front().openCount++ ;

    }

    /// Update the end time of the front item.
    std::chrono::_V2::steady_clock::duration Profiler::stop(std::string timerID)
    {
        /// if the event list is empty, or the prior timer is completed we do not want to access or overwright stopT so return 0.
        if(events[timerID].size() == 0 || events[timerID].front().openCount == 0){
            return std::chrono::_V2::steady_clock::duration {0};
        }
        /// record the current stop time, decriment open counter, return the passed time.
        events[timerID].front().stopT = std::chrono::_V2::steady_clock::now();
        events[timerID].front().openCount-- ;
        return events[timerID].front().passedTime();
    }

    void Profiler::drawDebug(olc::PixelGameEngine* game)
    {
        start("debug"); /// valuable to know how much time is lost to createing this display.

        /// Go through default event to gather total tracked history data
        float totalTimeTracked = std::chrono::duration<float>(std::next(coreFrame.rbegin())->second.stopT - coreFrame.begin()->second.startT ).count();

        if(coreFrame.size() <= 1)
            return; /// theres no closed frame events, nothing to do.

        int numFramesShown = 5;
        float timePerFrame = 1.0f/60.0f;  /// 60 fps
        int lastFrameNum = std::next(coreFrame.rbegin())->first;


        game->SetDrawTarget(nullptr); /// Draw to default layer above all others
        int lineHeight = 10;

        int wide = game->ScreenWidth();
        int height = game->ScreenHeight();

        float frameScale = (wide/numFramesShown) / timePerFrame;
        float displayStart = (wide/numFramesShown);


        int drawHeight = height - lineHeight;

        for(auto& [key,eventList] : events ){
            if(eventList.size() == 0){
                continue; /// no need to operate on empty lists
            }
            /// Check if a timer is being called an excessive number of times (more than 50 per frame).
            /// It will still be calculated for times over the last frame and adjusted, but draw is skipped
            bool skipExcessiveDraws = false;
            if(eventList.size() >= 50 * events[defaultName].size() )
                skipExcessiveDraws = true;

            /// iterate through this event drawing the time frames it occupied
            float eventTimeRun = 0;
            int eventsTracked = 0;
            for(Event& timer : eventList){
                if(timer.openCount > 0 || timer.frameNum > lastFrameNum)
                    continue; /// this timer event or cycle is not yet closed or has aged out and will be skipped.

                float length = std::chrono::duration<float>(timer.passedTime()).count();

                /// Accumulate our runtime data
                eventTimeRun += length;
                eventsTracked ++;

                if(skipExcessiveDraws){ /// skip drawing phase for excessive data
                    continue;
                }

                bool lastFrame = timer.frameNum == lastFrameNum; // ? colour = olc::GREEN : olc::DARK_YELLOW;

                float start = std::chrono::duration<float>(timer.startT - coreFrame.find(timer.frameNum)->second.startT ).count();

                int frameStart = (1 + (lastFrameNum - timer.frameNum)) * displayStart;
                int rectStart = start * frameScale + frameStart;
                int rectLong = length * frameScale;
                if (lastFrame)
                    game->FillRect(rectStart,drawHeight,rectLong,lineHeight,olc::GREEN);
                if (rectStart < wide)
                    game->DrawRect(rectStart,drawHeight,rectLong,lineHeight,olc::DARK_YELLOW);
            }
            float aveEvents = (float)eventsTracked / ((float)coreFrame.size() - 1);
            float percentOfFrame = (eventTimeRun / totalTimeTracked) * 100;
            /// Write our analytics about the bar on screen
            game->DrawString(lineHeight,drawHeight+1,key + " "+ std::to_string(aveEvents).substr(0,5),olc::YELLOW);

            std::string percentage = std::to_string(percentOfFrame);
            percentage = percentage.substr(0,percentage.find(".")+4);
            game->DrawString(displayStart - (percentage.size() * 8),drawHeight+1,percentage,olc::YELLOW);

            drawHeight -= lineHeight;
        }
        float percentOfFrame = totalTimeTracked / (coreFrame.size() - 1) / timePerFrame * 100;
        game->DrawString(lineHeight,drawHeight+1,"coreFrame "+ std::to_string(coreFrame.size() - 1).substr(0,5),olc::YELLOW);

        std::string percentage = std::to_string(percentOfFrame);
        percentage = percentage.substr(0,percentage.find(".")+4);
        game->DrawString(displayStart - (percentage.size() * 8),drawHeight+1,percentage,olc::YELLOW);

        for(int i = displayStart; i < wide;i += displayStart){
            game->DrawLine(i,drawHeight,i,height,olc::BLUE);
        }
    stop("debug");
    }

