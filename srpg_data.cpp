#include "olcPixelGameEngine.h"
#include "olcPGEX_TransformedView.h"
#include "Rectangle.h"
#include "Entities.h"
#include "srpg_data.h"

const bool srpg_data::debugTools = false; /// Set false to skip debug

///class QuadTree
int QuadTree::depthLimit = 6; /// static depth limit.

/// Internally used contsructor that uses a passed rectangle instead of corner coordinates.
QuadTree::QuadTree(Rectangle newArea, int newDepth,QuadTree* parent)
    :quadArea(newArea),depth(newDepth),parentNode(parent)
{}

/// Genereal constructor for external calls
QuadTree::QuadTree(olc::vf2d newtl, olc::vf2d newbr, int newdepth)
    :quadArea({newtl,newbr}),depth(newdepth),parentNode(nullptr)
{}

/// Deconstruct, cleaning up lists and subquads.
QuadTree::~QuadTree(){
    // to ensure shared pointers and memory are cleaned up properly delete each sub quad and then clear the local list
    for(int i = 0; i < 4; i++){
        delete quads[i];
    }
    while (entStored.size() > 0 ){
        entStored.erase(entStored.begin());
    }
};

/// Standard item insertion. calls method on inserted item to give it node and list info for validation later.
/// (I would like to find a simple way to accomplish the same effect that would allow a template design instead of specialized.)
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
    entStored.push_front(newEnt);
    newEnt->setTreeLocation(this,entStored.begin());

    return;
}

/// Checks an area for any items in the tree that overlap however slightly.
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


/// Position validation that is called directly by entities each time one moves.
/// Treenode and entIT Values are to be updated be elevator functions once entity
/// is placed in new node.
void QuadTree::validateEnt(QuadTree*& treeNode, std::list<std::shared_ptr<Entity>>::iterator& entIT){
    /// First record a stable copy of the iterator because once the entity is moved the entIT will already be updated on the return trip
    std::list<std::shared_ptr<Entity>>::iterator myEntIT = entIT;

    if(depth != 0 && !quadArea.contains((*entIT)->getBoxCollider())){

        parentNode->upEscalator(treeNode,entIT);
        entStored.erase(myEntIT);
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
    } // else: not down either so nothing to do.
    return;
}
/// Passes units up the tree during the validation process. Updates treeNode and entIT
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
/// passes units down the tree during validation. Updates treeNode and entIT
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

/// Cleans up the tree by recursively removing any Entities that are nolonger needed and removing any empty quads
void QuadTree::clean(){
//    for(auto ent = entStored.begin(); ent != entStored.end(); ){
//        if(!(*ent)->isValid()){
//            //entStored.erase(ent++);
//        } else {
//            ent++;
//        }
//    }
    for(int i = 0; i < 4; i++){
        if(quads[i]){
            quads[i]->clean();
            if(quads[i]->size() == 0){
                delete quads[i];
                quads[i] = nullptr;
            }
        }
    }

}

void QuadTree::removeMe(QuadTree*& treeNode, std::list<std::shared_ptr<Entity>>::iterator& entIT){
    if(entIT != entStored.end()){
        entStored.erase(entIT);
        entIT = entStored.end();
        treeNode = nullptr;
    }
    return;
}

/// NOTE: I would like to find a good way to do these functions without full tree traversal
/// Count of all items in this node and each sub node.
int QuadTree::size()
{   return 0;
    int thisCount = entStored.size();
    for(int i = 0;i < 4;i++){
        thisCount += quads[i] ? quads[i]->size() : 0;
    }

    return thisCount;
}

/// Counts total number of nodes in the tree.
int QuadTree::activity(){return 0;
    int numQuads = 1;
    for(int i = 0; i < 4; i++){
        numQuads += quads[i] ? quads[i]->activity() : 0;
    }
    return numQuads;
}

/// Checks for the deepest node
int QuadTree::curDepth(){return 0;
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

/// Debug function to draw all nodes in an area to the screen.
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





