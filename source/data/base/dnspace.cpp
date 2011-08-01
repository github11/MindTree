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

#include "dnspace.h"

#include "QDebug"
#include "source/data/base/project.h"
#include "source/data/base/frg.h"

DNSpace::DNSpace()
    : spaceVis(0)
{
    FRG::CurrentProject->registerSpace(this);
}

DNSpace::DNSpace(DNSpace* space)
    : name(space->getName())
{
    foreach(DNode *node, space->getNodes())
    {
        DNode *copy = DNode::copy(node);
        addNode(copy);
        FRG::CurrentProject->setNodePosition(copy, node->getPos());
    }
    foreach(DNode *node, getNodes())
        foreach(DinSocket *socket, node->getInSockets())
            socket->setCntdSocket(static_cast<DoutSocket*>(CopySocketMapper::getCopy(socket->getCntdSocket())));
}

DNSpace::~DNSpace()
{
    FRG::CurrentProject->unregisterSpace(this);
    clearLinksCache();
    foreach(DNode *node, getNodes())
        removeNode(node);
    nodes.clear();
}


QDataStream & operator<<(QDataStream &stream, DNSpace *space)
{
    stream<<space->getName();
    stream<<(qint16)space->getNodeCnt();
    foreach(DNode *node, space->getNodes())
        stream<<node;
}

QDataStream & operator>>(QDataStream &stream, DNSpace **space)
{
    DNSpace *newspace = *space;
    qint16 nodecnt;
    QString name;
    stream>>name;
    newspace->setName(name);
    stream>>nodecnt;
    DNode *node = 0;
    for(int i = 0; i<nodecnt; i++)
    {
        stream>>&node;
        newspace->addNode(node);
    }

	//set pointer to connected sockets from temp. IDs
	foreach(DSocket *socket, LoadSocketIDMapper::getAllSockets())
	{
		if(socket->getDir() == IN)
		{
			DinSocket *inSocket = (DinSocket*)socket;
			if(inSocket->getTempCntdID() > 0)
				inSocket->setCntdSocket((DoutSocket*)LoadSocketIDMapper::getSocket(inSocket->getTempCntdID()));
		}
	}
}

bool DNSpace::operator==(DNSpace &space)
{
    if(name != space.name
            ||getNodeCnt() != space.getNodeCnt())
        return false;
    foreach(DNode* node1, getNodes())
    {
        foreach(DNode *node2, space.getNodes())
        {
            if(*node1 == *node2)
                continue;
            else
                return false;
        }
    }
    return true;
}

bool DNSpace::operator!=(DNSpace &space)
{
    return(!(*this == space));
}

void DNSpace::cacheLinks()
{
    clearLinksCache();
    foreach(DNode *node, getNodes())
        foreach(DinSocket *socket, node->getInSockets())
            if(!isSocketLinkCached(socket)
                && socket->getCntdSocket())cacheNewLink(socket);
}

void DNSpace::clearLinksCache()    
{
    while(!cachedLinks.isEmpty())
        delete cachedLinks.takeLast();
}

void DNSpace::deleteCachedLink(DNodeLink *dnlink)    
{
    cachedLinks.removeAll(dnlink);
    delete dnlink;
}

void DNSpace::visCachedLinks()    
{
    foreach(DNodeLink *dnlink, cachedLinks)
        if(!dnlink->vis)dnlink->vis = new VNodeLink(dnlink);
}

QList<DNodeLink*> DNSpace::getCachedLinks()    
{
    return cachedLinks;
}

QList<VNodeLink*> DNSpace::getCachedLinksVis()    
{
    QList<VNodeLink*> linkVis;
    visCachedLinks();
    if(!cachedLinks.isEmpty())
        foreach(DNodeLink *dnlink, cachedLinks)
            linkVis.append(dnlink->vis);
    return linkVis;
}

void DNSpace::cacheNewLink(DinSocket *socket)    
{
    cachedLinks.append(new DNodeLink(socket, socket->getCntdSocket()));
}

bool DNSpace::isSocketLinkCached(DSocket *socket)    
{
    if(socket->getDir() == IN)
    {
        foreach(DNodeLink *dnlink, cachedLinks)
            if(dnlink->in == socket) return true;
    }
    else
    {
        foreach(DNodeLink *dnlink, cachedLinks)
            if(dnlink->out == socket) return true;
    }
    return false;
}

QList<DinSocket*> DNSpace::getConnected(DoutSocket* out)    
{
    cacheLinks();
    QList<DinSocket*> ins;
    foreach(DNodeLink *dnlink, cachedLinks)
        if(dnlink->out == out)
            ins.append(dnlink->in);
    return ins;
}

QList<DNodeLink*> DNSpace::getOutLinksToNode(DNode *node)    
{
    cacheLinks();
    QList<DNodeLink*> nodeLinks;
    foreach(DNodeLink *dnlink, cachedLinks)
        if(dnlink->out->getNode() == node)
            nodeLinks.append(dnlink);
    return nodeLinks;
}

int DNSpace::getLinkCount(DoutSocket* socket)    
{
    int i = 0;
    foreach(DNodeLink *dnlink, cachedLinks)
        if(dnlink->out == socket) ++i;
    return i;
}

void DNSpace::addNode(DNode *node)
{
    node->setSpace(this);
    if(!nodes.contains(node))nodes.append(node);
}

void DNSpace::removeNode(DNode *node)
{
    unregisterNode(node);
    delete node;
}

void DNSpace::unregisterNode(DNode *node)    
{
    nodes.removeAll(node);
    if(this == FRG::SpaceDataInFocus)
        node->deleteNodeVis();
}

VNSpace *DNSpace::getSpaceVis()
{
    return spaceVis;
}

void DNSpace::setSpaceVis(VNSpace *spaceVis)
{
    this->spaceVis = spaceVis;
}

qint16 DNSpace::getNodeCnt()
{
    return (qint16)nodes.size();
}

NodeList DNSpace::getNodes()
{
    return nodes;
}

QString DNSpace::getName()
{
		return name;
}

void DNSpace::setName(QString value)
{
		name = value;
}

ContainerSpace::ContainerSpace()
    : DNSpace()
{
}

ContainerSpace::ContainerSpace(ContainerSpace* space)
    : DNSpace(space)
{
}

ContainerNode* ContainerSpace::getContainer()
{
    return node;
}

void ContainerSpace::setContainer(ContainerNode* value)
{
    node = value;
}