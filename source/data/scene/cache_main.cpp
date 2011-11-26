/*
    FRG Shader Editor, a Node-based Renderman Shading Language Editor
    Copyright (C) 2011  Sascha Fricke

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "cache_main.h"

QHash<AbstractDataCache*, const DoutSocket*>CacheControl::caches;
QHash<const LoopNode*, LoopCache*> LoopCacheControl::loops;

bool CacheControl::isCached(const DoutSocket *socket)    
{
    return caches.values().contains(socket);
}

void CacheControl::addCache(const DoutSocket *socket, AbstractDataCache *cache)    
{
    caches.insert(cache, socket); 
}

void CacheControl::removeCache(AbstractDataCache *cache)    
{
    caches.remove(cache);
}

LoopCache::LoopCache()
    : loopentry(0), stepValue(0)
{
}

LoopCache::~LoopCache()
{
    free();
}

void LoopCache::free()    
{
    while(!cachedData.isEmpty())
        foreach(const DoutSocket *socket, cachedData.keys())
            delete cachedData.take(socket);
}

void LoopCache::setStep(int step)    
{
    stepValue = step;
}

int LoopCache::getStep()
{
    return stepValue;
}


void LoopCache::addData(AbstractDataCache *cache)    
{
    if(cachedData.isEmpty())
        loopentry = cache->getStart();
    if(cachedData.contains(cache->getStart()))
        delete cachedData.take(cache->getStart());

    cachedData.insert(cache->getStart(), cache);
}

AbstractDataCache* LoopCache::getCache(const DoutSocket *socket)    
{
    const DoutSocket *s;
    if(!socket)
        s = loopentry;
    else
        s = socket;

    if(!cachedData.contains(s))
        return 0;
    return cachedData.value(s);
}

const LoopNode* LoopCache::getNode()
{
    return loopentry->getNode()->getDerivedConst<LoopSocketNode>()->getContainer()->getDerivedConst<LoopNode>();
}

LoopCache* LoopCacheControl::loop(const LoopNode *node)
{
    LoopCache *lc;
    if(loops.contains(node))
        lc = loops.value(node);
    else {
        lc = new LoopCache();
        loops.insert(node, lc);
    }
    return lc;
}

void LoopCacheControl::del(const LoopNode *node)    
{
    delete loops.take(node);
}

AbstractDataCache::AbstractDataCache(const DinSocket *socket)
    : start(socket->getCntdWorkSocket()), inSocket(socket), dataOwner(true)
{
    //CacheControl::addCache(socket, this);
}

AbstractDataCache::AbstractDataCache(const AbstractDataCache &cache)
{
    cache.setOwner(false);
}

AbstractDataCache::~AbstractDataCache()
{
}

void AbstractDataCache::setOwner(bool owner)const
{
    dataOwner = owner;
}

void AbstractDataCache::cacheSocket(DoutSocket *socket)    
{
    cachedSockets.add(socket);
}

AbstractDataCache* AbstractDataCache::getDerived()    
{
    return this; 
}

const DoutSocket* AbstractDataCache::getStart()    
{
    return start;
}

void AbstractDataCache::cacheInputs()    
{
    if(!start)
        return;
    switch(start->getNode()->getNodeType())
    {
    case CONTAINER:
        break;
    case FUNCTION:
        break;
    case CONDITIONCONTAINER:
        break;
    case FOR:
        forloop();
        break;
    case WHILE:
        break;
    case COLORNODE:
        break;
    case STRINGNODE:
        break;
    case FLOATNODE:
        floatValue();
        break;
    case INTNODE:
        intValue();
        break;
    case VECTORNODE:
        vectorValue();
        break;
    case FLOATTOVECTOR:
        floattovector();
        break;
    case INSOCKETS:
        break;
    case GETARRAY:
        break;
    case SETARRAY:
        setArray();
        break;
    case COMPOSEARRAY:
        composeArray();
        break;
    case OBJECTNODE:
        composeObject();
        break;
    case POLYGONNODE:
        composePolygon();
        break;
    case ADD:
        add();
        break;
    case SUBTRACT:
        subtract();
        break;
    case MULTIPLY:
        multiply();
        break;
    case DIVIDE:
        divide();
        break;
    case DOTPRODUCT:
    case GREATERTHAN:
    case SMALLERTHAN:
    case EQUAL:
    case AND:
    case OR:
    case NOT:
    case VARNAME:
        break;
    case LOOPINSOCKETS:
        getLoopedCache();
    default:
        break;
            
    }
}

void AbstractDataCache::setArray()    
{
}

void AbstractDataCache::composeArray()    
{
}

void AbstractDataCache::composePolygon()    
{
    
}

void AbstractDataCache::composeObject()    
{
}

void AbstractDataCache::vectorValue()    
{
}

void AbstractDataCache::floattovector()    
{
}

void AbstractDataCache::intValue()    
{
}

void AbstractDataCache::floatValue()    
{
}

void AbstractDataCache::forloop()    
{
}

void AbstractDataCache::getLoopedCache()    
{
}

void AbstractDataCache::add()    
{
}

void AbstractDataCache::subtract()    
{
}

void AbstractDataCache::multiply()    
{
}

void AbstractDataCache::divide()    
{
}
