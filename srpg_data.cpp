#include "olcPixelGameEngine.h"
#include "olcPGEX_TransformedView.h"
#include "Rectangle.h"
#include "Entities.h"
#include "srpg_data.h"


//class QuadTree
int QuadTree::depthLimit = 4;
QuadTree::QuadTree(olc::vf2d newtl, olc::vf2d newbr, int newdepth)
    :quadArea({newtl,newbr}),depth(newdepth),parentNode(nullptr)
{}

QuadTree::QuadTree(Rectangle newArea, int newdepth,QuadTree* parent)
    :quadArea(newArea),depth(newdepth),parentNode(parent)
{}
QuadTree::~QuadTree(){
    // to ensure shared pointers and memory are cleaned up properly delete each sub quad and then clear the local list
    for(int i = 0; i < 4; i++){
        delete quads[i];
    }
    while (entStored.begin() != entStored.end()){
        entStored.erase(entStored.begin());
    }
};

void QuadTree::insertItem(const std::shared_ptr<Entity>& newEnt){
     // Check if it belongs lower in the tree
     for(int i = 0;i < 4;i++){
        if(depth < depthLimit && childArea[i].contains(newEnt->getBoxCollider())){
            if(!quads[i]){
                quads[i] = new QuadTree(childArea[i],depth+1,this);
            }
            quads[i]->insertItem(newEnt);
            return;
        }
    } // else we store it in this element
    entStored.push_back(newEnt);

    return;
}

void QuadTree::getOverlapItems(Rectangle area, std::list<std::shared_ptr<Entity>>& returns){
    // collect overlaps from children
    for(int i = 0;i < 4;i++){
        if(childArea[i].overlaps(area) && quads[i]){
             quads[i]->getOverlapItems(area,returns);
        }
    }
    // add any overlaped items from this layer
    for(auto it = entStored.begin(); it!= entStored.end(); it++){
        if(area.overlaps((*it)->getBoxCollider())){
            returns.push_back((*it));
        }
    }
    return;
}


void QuadTree::validateLocations(){
    std::list<std::shared_ptr<Entity>> riders;
    for(auto it = entStored.begin(); it != entStored.end(); ){ //no incrementor, all paths will increment
        if(!(*it)->isAlive()){
            it->reset();
            entStored.erase(it++);
            continue;
        }
        if (depth != 0 && !quadArea.contains((*it)->getBoxCollider())){
            // not at root and item does not fit. Send it up
            riders.push_back((*it));
            it->reset();
            entStored.erase(it++);
            continue;
        }
        int targetQuad = -1;
        if(depth < depthLimit){
            for(int i = 0; i < 4; i++)
                if(childArea[i].contains((*it)->getBoxCollider()))
                    targetQuad = i;
        }
        if(targetQuad == -1){ // negitive 1 means item should remain at this level, move on
            it++;
            continue;
        } //else targetQuad contains the quad index to send this item
        if(!quads[targetQuad]){ //depth already checked
            quads[targetQuad] = new QuadTree(childArea[targetQuad],depth+1,this);
        }
        quads[targetQuad]->insertItem((*it));
        it->reset();
        entStored.erase(it++);
    }
    for(int i = 0; i < 4; i++){
        if(quads[i])
            quads[i]->validateLocations();
    }
    if(riders.size() > 0)
        parentNode->upElevator(riders);

    if(depth == 0){ // Only once root is fully finished and all items are in proper place
        prune();
    }
}
void QuadTree::upElevator(std::list<std::shared_ptr<Entity>> &riders){
    for(auto it = riders.begin();it != riders.end(); ){
        if(depth == 0 || quadArea.contains((*it)->getBoxCollider())){
            insertItem((*it));
            it->reset();
            riders.erase(it++);
            continue;
        } //else
        it++;
    }
    if(riders.size() > 0)
        parentNode->upElevator(riders);
}
void QuadTree::prune(){
    for(int i = 0; i < 4; i++){
        if(quads[i]){
            if(quads[i]->size() == 0){
                delete quads[i];
                quads[i] = nullptr;
            } else {
                quads[i]->prune();
            }
        }
    }
}


int QuadTree::size()
{
    int thisCount = entStored.size();
    for(int i = 0;i < 4;i++){
        thisCount += quads[i] ? quads[i]->size() : 0;
    }

    return thisCount;
}
int QuadTree::activity(){
    int numQuads = 1;
    for(int i = 0; i < 4; i++){
        numQuads += (quads[i])?quads[i]->activity() : 0;
    }
    return numQuads;
}

int QuadTree::curDepth(){
    int depthCharge = depth;
    for (int i = 0; i < 4; i++){
        if(quads[i]){
            if(quads[i]->curDepth() > depthCharge)
                depthCharge = quads[i]->curDepth();
        }
    }
    return depthCharge;
}

void QuadTree::drawTree(olc::Pixel item,olc::Pixel noItem ){
    for(int i = 0;i < 4; i++)
        if(quads[i])
            quads[i]->drawTree(item,noItem);
    if(entStored.size() > 0){
        srpg_data::viewer->DrawRect(quadArea.tl,quadArea.sides,item);
    } else {
        srpg_data::viewer->DrawRect(quadArea.tl,quadArea.sides,noItem);
    }
}





