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

#include "data_node.h"

#include "QCoreApplication"
#include "QTextStream"
#include "QDir"
#include "QDebug"
#include "QProcess"

#include "source/data/base/frg.h"
#include "source/graphics/nodes/graphics_node.h"
#include "source/data/rslwriter.h"
#include "source/data/base/dnspace.h"
#include "source/graphics/base/vnspace.h"
#include "source/data/base/project.h"
#include "source/graphics/shaderpreview.h"
#include "source/graphics/sourcedock.h"

unsigned short DNode::count = 1;
QHash<unsigned short, DNode*>LoadNodeIDMapper::loadIDMapper;
QHash<DNode*, DNode*> CopyNodeMapper::nodeMap;

unsigned short LoadNodeIDMapper::getID(DNode *node)
{
    return loadIDMapper.key(node);
}

void LoadNodeIDMapper::setID(DNode *node, unsigned short ID)    
{
    loadIDMapper.insert(ID, node);
}

DNode * LoadNodeIDMapper::getNode(unsigned short ID)    
{
    return loadIDMapper.value(ID);
}

void LoadNodeIDMapper::clear()    
{
    loadIDMapper.clear();
}

void CopyNodeMapper::setNodePair(DNode *original, DNode *copy)    
{
   nodeMap.insert(original, copy); 
}

DNode * CopyNodeMapper::getCopy(DNode *original)    
{
    if(!nodeMap.contains(original)) return 0;
    return nodeMap.value(original);
}

DNode::DNode(QString name)
    : space(0), varcnt(0), ID(count++), nodeVis(0), nodeName(name), varsocket(0), lastsocket(0), blockCBregister(false)
{};

DNode::DNode(const DNode* node)
    : space(0), varcnt(0), ID(count++), nodeVis(0),
    nodeName(node->getNodeName()), NodeType(node->getNodeType()), blockCBregister(false)
{
    blockCB();
    foreach(DinSocket *socket, node->getInSockets())
        new DinSocket(socket, this);
    foreach(DoutSocket *socket, node->getOutSockets())
        new DoutSocket(socket, this);
    varsocket = CopySocketMapper::getCopy(node->getVarSocket());
    lastsocket = CopySocketMapper::getCopy(node->getLastSocket());

    CopyNodeMapper::setNodePair(const_cast<DNode*>(node), this);
    unblockCB();
}

DSocket* DNode::getSocketByIDName(QString idname)    
{
    foreach(DinSocket *socket, getInSockets())
        if(socket->getIDName() == idname)
            return socket;
    foreach(DoutSocket *socket, getOutSockets())
        if(socket->getIDName() == idname)
            return socket;
    return 0;
}

void DNode::blockCB()    
{
    addSocketCallbacks.setBlock(true);
}

void DNode::unblockCB()    
{
    addSocketCallbacks.setBlock(false);
}

void DNode::blockRegCB()    
{
    blockCBregister = true; 
}

void DNode::unblockRegCB()    
{
    blockCBregister = false;
}

DNode* DNode::copy(const DNode *original)    
{
    DNode *newNode;
    switch(original->getNodeType())
    {
    case CONTAINER:
        newNode = new ContainerNode(original->getDerivedConst<ContainerNode>());
        break;
    case FUNCTION:
        newNode = new FunctionNode(original->getDerivedConst<FunctionNode>());
        break;
    case MULTIPLY:
    case DIVIDE:
    case ADD:
    case SUBTRACT:
    case DOTPRODUCT:
        newNode = new MathNode(original->getDerivedConst<MathNode>());
        break;
    case GREATERTHAN:
    case SMALLERTHAN:
    case EQUAL:
    case AND:
    case OR:
    case NOT:
        newNode = new ConditionNode(original->getDerivedConst<ConditionNode>());
        break;
    case CONDITIONCONTAINER:
        newNode = new ConditionContainerNode(original->getDerivedConst<ConditionContainerNode>());
        break;
    case FOR:
        newNode = new ForNode(original->getDerivedConst<ForNode>());
        break;
    case WHILE: 
        newNode = new WhileNode(original->getDerivedConst<WhileNode>());
        break;
    case GATHER:
        newNode = new GatherNode(original->getDerivedConst<GatherNode>());
        break;
    case ILLUMINANCE:
        newNode = new IlluminanceNode(original->getDerivedConst<IlluminanceNode>());
        break;
    case ILLUMINATE:
        newNode = new IlluminateNode(original->getDerivedConst<IlluminateNode>());
        break;
    case SOLAR:
        newNode = new SolarNode(original->getDerivedConst<SolarNode>());
        break;
    case SURFACEINPUT:
    case DISPLACEMENTINPUT:
    case VOLUMEINPUT:
    case LIGHTINPUT:
    case ILLUMINANCEINPUT:
    case SOLARINPUT:
    case ILLUMINATEINPUT:
        newNode = new InputNode(original->getDerivedConst<InputNode>());
        break;
    case SURFACEOUTPUT:
    case DISPLACEMENTOUTPUT:
    case VOLUMEOUTPUT:
    case LIGHTOUTPUT:
        newNode = new OutputNode(original->getDerivedConst<OutputNode>());
        break;
    case INSOCKETS:
    case OUTSOCKETS:
        newNode = new SocketNode(original->getDerivedConst<SocketNode>());
        break;
    case LOOPINSOCKETS:
    case LOOPOUTSOCKETS:
        newNode = new LoopSocketNode(original->getDerivedConst<LoopSocketNode>());
        break;
    case COLORNODE:
        newNode = new ColorValueNode(original->getDerivedConst<ColorValueNode>());
        break;
    case FLOATNODE:
        newNode = new FloatValueNode(original->getDerivedConst<FloatValueNode>());
        break;
    case STRINGNODE:
        newNode = new StringValueNode(original->getDerivedConst<StringValueNode>());
        break;
    case VECTORNODE:
        newNode = new VectorValueNode(original->getDerivedConst<VectorValueNode>());
        break;
    case PREVIEW:
        newNode = new DShaderPreview(original->getDerivedConst<DShaderPreview>());
        break;
    case GETARRAY:
        newNode = new GetArrayNode(original->getDerivedConst<GetArrayNode>());
        break;
    case SETARRAY:
        newNode = new SetArrayNode(original->getDerivedConst<SetArrayNode>());
        break;
    case VARNAME:
        newNode = new VarNameNode(original->getDerivedConst<VarNameNode>());
        break;
    }
    FRG::CurrentProject->setNodePosition(newNode, FRG::CurrentProject->getNodePosition(original));
    return newNode;
}

QList<DNode*> DNode::copy(QList<DNode*>nodes)    
{
   QList<DNode*>nodeCopies;
   foreach(DNode *node, nodes)
        nodeCopies.append(copy(node));
   CopySocketMapper::remap();
   return nodeCopies;
}

DNode::~DNode()
{
    foreach(DinSocket *socket, getInSockets())
        delete socket;
    foreach(DoutSocket *socket, getOutSockets())
        delete socket;

    if(nodeVis) deleteNodeVis();
    FRG::CurrentProject->clearNodePosition(this);
}

NodeList DNode::getAllInNodes(NodeList nodes)
{
    foreach(DinSocket *socket, getInSockets())
    {
        DoutSocket *cntdSocket = socket->getCntdFunctionalSocket();

        //try to not forget special containers and their default inputs
        DoutSocket *wSocket = socket->getCntdWorkSocket();
        if(wSocket != cntdSocket && wSocket->getNode()->isContainer())
            nodes.append(wSocket->getNode()->getAllInNodes(nodes));

        if(cntdSocket)
        {
            DNode *cntdNode = cntdSocket->getNode();
            if(nodes.contains(cntdNode)) return nodes;
            nodes.append(cntdNode);

            nodes.append(cntdNode->getAllInNodes(nodes));
        }
    }
    return nodes;
}

NodeList DNode::getAllInNodesConst(NodeList nodes) const
{
    foreach(DinSocket *socket, getInSockets())
    {
        DoutSocket *cntdSocket = socket->getCntdFunctionalSocket();
        if(cntdSocket)
        {
            DNode *cntdNode = cntdSocket->getNode();
            if(nodes.contains(cntdNode)) return nodes;
            nodes.append(cntdNode);

            nodes.append(cntdNode->getAllInNodes(nodes));
        }
    }
    return nodes;
}

VNode* DNode::createNodeVis()
{
    nodeVis = new VNode(this);
    return nodeVis;
}

void DNode::deleteNodeVis()    
{
    foreach(DinSocket *socket, getInSockets())
        socket->setSocketVis(0);

    foreach(DoutSocket *socket, getOutSockets())
        socket->setSocketVis(0);

    FRG::Space->removeItem(nodeVis);
    delete nodeVis;
    nodeVis = 0;
}

QPointF DNode::getPos() const
{
    return FRG::CurrentProject->getNodePosition(this);
}

bool DNode::isContainer() const
{
    if(NodeType == CONTAINER
            ||NodeType == CONDITIONCONTAINER
            ||NodeType == FOR
            ||NodeType == WHILE
            ||NodeType == GATHER
            ||NodeType == ILLUMINANCE
            ||NodeType == ILLUMINATE
            ||NodeType == SOLAR)
        return true;
    else return false;
}

QDataStream &operator <<(QDataStream &stream, DNode  *node)
{
    stream<<node->getNodeName()<<node->getID()<<node->getNodeType();
    stream<<FRG::CurrentProject->getNodePosition(node);
    stream<<(qint16)node->getInSockets().size()<<(qint16)node->getOutSockets().size();

    foreach(DinSocket *socket, node->getInSockets())
        stream<<(DSocket*)socket;
    foreach(DoutSocket *socket, node->getOutSockets())
        stream<<(DSocket*)socket;

    if(node->getNodeType() == FUNCTION)
    {
        FunctionNode *fnode = (FunctionNode*) node;
        stream<<fnode->getFunctionName();
    }

    if(node->isContainer())
    {
        ContainerNode *cnode =(ContainerNode*) node;
        stream<<cnode->getInputs()->getID()<<cnode->getOutputs()->getID();
        stream<<cnode->getSocketMapSize();
        foreach(DSocket *socket, cnode->getMappedSocketsOnContainer())
            stream<<socket->getID()<<cnode->getSocketInContainer(socket)->getID();
        stream<<cnode->getContainerData();
    }

    if(     node->getNodeType() == LOOPINSOCKETS
          ||node->getNodeType() == LOOPOUTSOCKETS)
    {

        LoopSocketNode *lsnode = (LoopSocketNode*)node;
        stream<<lsnode->getLoopedSocketsCount();
        foreach(DSocket *socket, lsnode->getLoopedSockets())
        {
            stream<<socket->getID()<<lsnode->getPartnerSocket(socket)->getID();
        }

    }

    if(node->getNodeType() == COLORNODE)
    {
        ColorValueNode *colornode = (ColorValueNode*)node;
        stream<<colornode->getValue();
    }

    if(node->getNodeType() == FLOATNODE)
    {
        FloatValueNode *floatnode = (FloatValueNode*)node;
        stream<<floatnode->getValue();
    }

    if(node->getNodeType() == STRINGNODE)
    {
        StringValueNode *stringnode = (StringValueNode*)node;
        stream<<stringnode->getValue();
    }

    if(node->getNodeType() == VECTORNODE)
    {
        VectorValueNode *vectornode = (VectorValueNode*)node;
        stream<<vectornode->getValue().x<<vectornode->getValue().y<<vectornode->getValue().z;
    }

    if(node->getNodeType() == PREVIEW)
    {
        PreviewScene scene;
        DShaderPreview *prev = node->getDerived<DShaderPreview>();
        stream<<scene.image<<scene.material<<scene.scene<<scene.dir;
        prev->setPrevScene(scene);
    }

    return stream;
}

DNode *DNode::newNode(QString name, NType t, int insize, int outsize)
{
    DNode *node;
    switch(t)
    {
        case CONTAINER:
            node = new ContainerNode(name, true);
            break;
        case FOR:
            node = new ForNode(true);
            break;
        case WHILE:
            node = new WhileNode(true);
            break;
        case GATHER:
            node = new GatherNode(true);
            break;
        case SOLAR:
            node = new SolarNode(true);
            break;
        case ILLUMINATE:
            node = new IlluminateNode(true);
            break;
        case ILLUMINANCE:
            node = new IlluminanceNode(true);
            break;
        case CONDITIONCONTAINER:
            node = new ConditionContainerNode(true);
            break;
        case FUNCTION:
            node = new FunctionNode();
            break;
        case MULTIPLY:
        case DIVIDE:
        case ADD:
        case SUBTRACT:
        case DOTPRODUCT:
            node = new MathNode(t, true);
            break;
        case GREATERTHAN:
        case SMALLERTHAN:
        case EQUAL:
        case AND:
        case OR:
        case NOT:
            node = new ConditionNode(t, true);
            break;
        case COLORNODE:
            node = new ColorValueNode(name, true);
            break;
        case FLOATNODE:
            node = new FloatValueNode(name, true);
            break;
        case STRINGNODE:
            node = new StringValueNode(name, true);
            break;
        case VECTORNODE:
            node = new VectorValueNode(name, true);
            break;
        case INSOCKETS:
        case OUTSOCKETS:
            node = new SocketNode(insize == 0 ? OUT : IN, 0, true);
            break;
        case LOOPINSOCKETS:
        case LOOPOUTSOCKETS:
            node = new LoopSocketNode(insize == 0 ? OUT : IN, 0, true);
            break;
        case SURFACEINPUT:
        case DISPLACEMENTINPUT:
        case VOLUMEINPUT:
        case LIGHTINPUT:
            node = new InputNode();
            break;
        case SURFACEOUTPUT:
        case DISPLACEMENTOUTPUT:
        case VOLUMEOUTPUT:
        case LIGHTOUTPUT:
            node = new OutputNode();
            break;
        case GETARRAY:
            node = new GetArrayNode(true);
            break;
        case SETARRAY:
            node = new SetArrayNode(true);
            break;
        case VARNAME:
            node = new VarNameNode(true);
            break;
        case PREVIEW:
            node = new DShaderPreview(true);
            break;
        default:
            node = new DNode();
            break;
    }

    node->setNodeName(name);
    node->setNodeType(t);
    return node;
}

QDataStream &operator >>(QDataStream &stream, DNode  **node)
{
    QString name;
    unsigned short ID;
    qint16 insocketsize, outsocketsize;
    int nodetype;
    QPointF nodepos;
    stream>>name>>ID>>nodetype>>nodepos;
    stream>>insocketsize>>outsocketsize;

    DNode *newnode;
    newnode = DNode::newNode(name, (NType)nodetype, insocketsize, outsocketsize);
    LoadNodeIDMapper::setID(newnode, ID);
    FRG::CurrentProject->setNodePosition(newnode, nodepos);
    *node = newnode;

    DSocket *socket;
    newnode->blockCB();
    newnode->blockRegCB();
    for(int i=0; i<insocketsize; i++)
    {
        socket = new DinSocket("", VARIABLE, newnode);
        stream>>&socket;
    }
    for(int j=0; j<outsocketsize; j++)
    {
        socket = new DoutSocket("", VARIABLE, newnode);
        stream>>&socket;
    }
    newnode->unblockCB();
    newnode->unblockRegCB();

    if(newnode->getNodeType() == FUNCTION)
    {
        FunctionNode *fnode = newnode->getDerived<FunctionNode>();
        QString fname;
        stream>>fname;
        fnode->setFunctionName(fname);
    }

    unsigned short inSocketID, outSocketID, keyID, valueID;
    int smapsize;
    if(newnode->isContainer())
    {
        ContainerNode *contnode = newnode->getDerived<ContainerNode>();
        stream>>inSocketID>>outSocketID;
        stream>>smapsize;
        QPair<unsigned short, unsigned short>cont_socket_map_ID_mapper[smapsize];
        for(int i = 0; i < smapsize; i++)
        {
            stream>>keyID>>valueID;
            cont_socket_map_ID_mapper[i].first = keyID;
            cont_socket_map_ID_mapper[i].second = valueID;
        }
        ContainerSpace *space = 0;
        stream>>&space;
        contnode->setContainerData(space);
        SocketNode *innode = LoadNodeIDMapper::getNode(inSocketID)->getDerived<SocketNode>();
        contnode->setInputs(innode);
        SocketNode *outnode = LoadNodeIDMapper::getNode(outSocketID)->getDerived<SocketNode>();
        contnode->setOutputs(outnode);
        for(int j = 0; j < smapsize; j++)
        {
            keyID = cont_socket_map_ID_mapper[j].first;
            valueID = cont_socket_map_ID_mapper[j].second;
            contnode->mapOnToIn(LoadSocketIDMapper::getSocket(keyID), LoadSocketIDMapper::getSocket(valueID));
        }
        if(LoopNode::isLoopNode(newnode))
        {
            LoopSocketNode *loutnode = outnode->getDerived<LoopSocketNode>();
            LoopSocketNode *linnode = innode->getDerived<LoopSocketNode>();
            loutnode->setPartner(linnode);
        }
    }

    if(newnode->getNodeType() == COLORNODE)
    {
        ColorValueNode *colornode = newnode->getDerived<ColorValueNode>();
        QColor color;
        stream>>color;
        colornode->setValue(color);
    }

    if(newnode->getNodeType() == FLOATNODE)
    {
        FloatValueNode *floatnode = newnode->getDerived<FloatValueNode>();
        float fval;
        stream>>fval;
        floatnode->setValue(fval);
    }

    if(newnode->getNodeType() == STRINGNODE)
    {
        StringValueNode *stringnode = newnode->getDerived<StringValueNode>();
        QString string;
        stream>>string;
        stringnode->setValue(string);
    }

    if(newnode->getNodeType() == VECTORNODE)
    {
        VectorValueNode *vectornode = newnode->getDerived<VectorValueNode>();
        Vector vec(vectornode->getValue());
        stream>>vec.x>>vec.y>>vec.z;
    }


    if(newnode->getNodeType() == LOOPINSOCKETS
            ||newnode->getNodeType() == LOOPOUTSOCKETS)
    {
        LoopSocketNode *lsnode = newnode->getDerived<LoopSocketNode>();
        int partnerSockets, socketID, partnerID;
        stream>>partnerSockets;
        for(int i = 0; i < partnerSockets; i++)
        {
            stream>>socketID>>partnerID;
            DSocket *socket = LoadSocketIDMapper::getSocket(socketID);
            DSocket *partner = LoadSocketIDMapper::getSocket(partnerID);
            lsnode->mapPartner(socket, partner);
        }
    }

    if(newnode->getNodeType() == PREVIEW)
    {
        DShaderPreview *prev = newnode->getDerived<DShaderPreview>();
        PreviewScene scene = prev->getPrevScene();
        stream>>scene.image>>scene.material>>scene.scene>>scene.dir;
    }
    if(MathNode::isMathNode(newnode)
        ||newnode->getNodeType() == GETARRAY
        ||newnode->getNodeType() == SETARRAY
        ||newnode->getNodeType() == VARNAME) {
        DinSocket *in = newnode->getInSockets().first();
        DoutSocket *out = newnode->getOutSockets().first();
        in->addTypeCB(new ScpTypeCB(in, out));
    }


    return stream;
}

bool DNode::operator==(const DNode &node)const
{
    if(getNodeType() != node.getNodeType())
        return false;

    if(nodeName != node.nodeName)
        return false;

    if(getInSockets().size() != node.getInSockets().size()
            ||getOutSockets().size() != node.getOutSockets().size())
        return false;

    for(int i=0; i<getInSockets().size(); i++)
        if(*getInSockets().at(i) != *node.getInSockets().at(i))
            return false;

    for(int i=0; i<getOutSockets().size(); i++)
        if(*getOutSockets().at(i) != *node.getOutSockets().at(i))
            return false;
    return true;
}

bool DNode::operator!=(const DNode &node)const
{
    return (!(*this == node));
}

void DNode::setNodeType(NType t)
{
    NodeType = t;
}

void DNode::setNodeName(QString name)
{
    nodeName = name;
}

void DNode::addSocket(DSocket *socket)
{
    setSocketIDName(socket);
    if(socket->getDir()==IN) inSockets.add(socket);
    else 
		outSockets.add(socket);

    if(socket->getType() == VARIABLE
        &&!blockCBregister)
        socket->addRmLinkCB(new SsetToVarCB(socket));
    addSocketCallbacks();
}

void DNode::setSocketIDName(DSocket *socket)    
{
    QStringList socketNames = getSocketNames();
    for(int i = 1; socketNames.contains(socket->getIDName()); i++)
        socket->setIDName(socket->getName() + QString::number(i));
}

QStringList DNode::getSocketNames()    
{
    QStringList names;
    foreach(DinSocket *socket, getInSockets())
        names.append(socket->getIDName());
    foreach(DoutSocket *socket, getOutSockets())
        names.append(socket->getIDName());

    return names;
}

void DNode::regAddSocketCB(Callback *cb)
{
    addSocketCallbacks.add(cb);
}

void DNode::remAddSocketCB(Callback *cb)
{
    addSocketCallbacks.remove(cb);
}

void DNode::removeSocket(DSocket *socket)
{
    if(!socket)return;
    if(socket->getDir() == IN)
        inSockets.rm(socket);
    else
        outSockets.rm(socket);
}

void DNode::dec_var_socket(DSocket *socket)
{
    removeSocket(socket);
    varcnt -= 1;
}

void DNode::inc_var_socket()
{
    lastsocket = varsocket;
    if(lastsocket->getDir() == IN)
        varsocket = new DinSocket("Add Socket", VARIABLE, this);
    else
        varsocket = new DoutSocket("Add Socket", VARIABLE, this);
    varsocket->setVariable(true);
    varcnt +=1;
}

void DNode::clearSocketLinks()
{
    foreach(DinSocket *socket, getInSockets())
       socket->clearLink();
}

bool DNode::isGhost()
{
    return ghost;
}

QString DNode::getNodeName() const
{
    return nodeName;
}

unsigned short DNode::getID() const
{
    return ID;
}

void DNode::setID(unsigned short value)
{
    ID = value;
}

VNode* DNode::getNodeVis() const
{
    return nodeVis;
}

void DNode::setNodeVis(VNode* value)
{
    nodeVis = value;
}

DoutSocketList DNode::getOutSockets() const
{
    return outSockets.returnAsOutSocketList();
}

DSocketList *DNode::getOutSocketLlist() const
{
    return &outSockets;
}

void DNode::setOutSockets(DoutSocketList value)
{
}

DinSocketList DNode::getInSockets() const
{
    return inSockets.returnAsInSocketList();
}

DSocketList* DNode::getInSocketLlist()    const
{
    return &inSockets;
}

void DNode::setInSockets(DinSocketList value)
{
}

NType DNode::getNodeType() const
{
    return NodeType;
}

DSocket* DNode::getVarSocket() const
{
    return varsocket;
}

void DNode::setVarSocket(const DSocket* value)
{
    varsocket = const_cast<DSocket*>(value);
}

DSocket* DNode::getLastSocket() const
{
    return lastsocket;
}

void DNode::setLastSocket(const DSocket* value)
{
    lastsocket = const_cast<DSocket*>(value);
}

int DNode::getVarcnt() const
{
    return varcnt;
}

void DNode::setVarcnt(int value)
{
    varcnt = value;
}

DNSpace* DNode::getSpace() const
{
    return space;
}

void DNode::setSpace(DNSpace* value)
{
    space = value;
}

void DNode::setsurfaceInput(DNode *node)
{
    new DoutSocket("P", POINT, node);
    new DoutSocket("N", NORMAL, node);
    new DoutSocket("Cs", COLOR, node);
    new DoutSocket("Os", COLOR, node);
    new DoutSocket("u", FLOAT, node);
    new DoutSocket("v", FLOAT, node);
    new DoutSocket("du", FLOAT, node);
    new DoutSocket("dv", FLOAT, node);
    new DoutSocket("s", FLOAT, node);
    new DoutSocket("t", FLOAT, node);
    new DoutSocket("I", VECTOR, node);
    node->setNodeName("Surface Input");
    node->setNodeType(SURFACEINPUT);
}

void DNode::setdisplacementInput(DNode *node)
{
    new DoutSocket("P", POINT, node);
    new DoutSocket("N", NORMAL, node);
    new DoutSocket("u", FLOAT, node);
    new DoutSocket("v", FLOAT, node);
    new DoutSocket("du", FLOAT, node);
    new DoutSocket("dv", FLOAT, node);
    new DoutSocket("s", FLOAT, node);
    new DoutSocket("t", FLOAT, node);
    node->setNodeName("Displacement Input");
    node->setNodeType(DISPLACEMENTINPUT);
}

void DNode::setvolumeInput(DNode *node)
{
    new DoutSocket("P", POINT, node);
    new DoutSocket("I", VECTOR, node);
    new DoutSocket("Ci", COLOR, node);
    new DoutSocket("Oi", COLOR, node);
    new DoutSocket("Cs", COLOR, node);
    new DoutSocket("Os", COLOR, node);
    new DoutSocket("L", VECTOR, node);
    new DoutSocket("Cl", COLOR, node);
    node->setNodeName("Volume Input");
    node->setNodeType(VOLUMEINPUT);
}

void DNode::setlightInput(DNode *node)
{
    new DoutSocket("P", POINT, node);
    new DoutSocket("Ps", POINT, node);
    new DoutSocket("L", VECTOR, node);
    node->setNodeName("Light Input");
    node->setNodeType(LIGHTINPUT);
}

void DNode::setsurfaceOutput(DNode *node)
{
    new DinSocket("Ci", COLOR, node);
    new DinSocket("Oi", COLOR, node);
    node->setNodeName("Surface Output");
    node->setNodeType(SURFACEOUTPUT);
    node->setDynamicSocketsNode(IN);
}

void DNode::setdisplacementOutput(DNode *node)
{
    new DinSocket("P", POINT, node);
    new DinSocket("N", NORMAL, node);
    node->setNodeName("Displacement Output");
    node->setNodeType(DISPLACEMENTOUTPUT);
    node->setDynamicSocketsNode(IN);
}

void DNode::setvolumeOutput(DNode *node)
{
    new DinSocket("Ci", COLOR, node);
    new DinSocket("Oi", COLOR, node);
    node->setNodeName("Volume Output");
    node->setNodeType(VOLUMEOUTPUT);
    node->setDynamicSocketsNode(IN);
}

void DNode::setlightOutput(DNode *node)
{
    new DinSocket("Cl", COLOR, node);
    node->setNodeName("Light Output");
    node->setNodeType(LIGHTOUTPUT);
    node->setDynamicSocketsNode(IN);
}

void DNode::setDynamicSocketsNode(socket_dir dir, socket_type t)
{
    DSocket *varsocket = 0;
    if(dir == IN)
        varsocket = new DinSocket("Add Socket", t, this);
    else
        varsocket = new DoutSocket("Add Socket", t, this);
    varsocket->setVariable(true);
}

bool DNode::isInput(const DNode *node)
{
    if(node->getNodeType() == SURFACEINPUT
       ||node->getNodeType() == DISPLACEMENTINPUT
       ||node->getNodeType() == VOLUMEINPUT
       ||node->getNodeType() == LIGHTINPUT)
        return true;
    else
        return false;
}

DNode *DNode::dropNode(QString filepath)
{
    QFile file(filepath);
    file.open(QIODevice::ReadOnly);
    QDataStream stream(&file);

    DNode *node = 0;
    FRG_NODE_HEADER_CHECK
    {
        stream>>&node;

        file.close();

        LoadNodeIDMapper::clear();
        LoadSocketIDMapper::remap();

        FRG::SpaceDataInFocus->addNode(node);
    }
    return node;
}

ContainerNode *DNode::buildContainerNode(QList<DNode*>nodes)
{
    int SPACING = 200;

    if(nodes.isEmpty()) return 0;
    ContainerNode *contnode = new ContainerNode("New Node", false);

    QList<DNodeLink*>ins = FRG::CurrentProject->getInLinks(FRG::Space->selectedNodes());
    QList<DNodeLink*>outs = FRG::CurrentProject->getOutLinks(FRG::Space->selectedNodes());

    FRG::CurrentProject->setNodePosition(contnode, FRG::Space->getCenter(nodes));
    FRG::Space->centerNodes(nodes);
    float minX = 0, maxX = 0;
    foreach(DNode *node, nodes)
    {
        int nodeWidth = node->getNodeVis()->getNodeWidth();
        float nodeX = FRG::CurrentProject->getNodePosition(node).x();
        float nodeMinX = nodeX - nodeWidth/2;
        float nodeMaxX = nodeX + nodeWidth/2;
        FRG::SpaceDataInFocus->unregisterNode(node);
        contnode->getContainerData()->addNode(node);
        if(minX > nodeMinX) minX = nodeMinX;
        if(maxX < nodeMaxX) maxX = nodeMaxX;
    }

    FRG::CurrentProject->setNodePosition(contnode->getInputs(), QPointF(minX - SPACING, 0));
    FRG::CurrentProject->setNodePosition(contnode->getOutputs(), QPointF(maxX + SPACING, 0));
    foreach(DNodeLink *nld, ins)
    {
        //relink the first input of the slected nodes to the container input node
        DoutSocket *entryInContainer = static_cast<DoutSocket*>(contnode->getInputs()->getVarSocket());
        nld->in->addLink(entryInContainer);

        //link the generated mapped socket of the container input to the output previously conntected
        DinSocket *containerInput = static_cast<DinSocket*>(contnode->getSocketOnContainer(entryInContainer));
        containerInput->addLink(nld->out);
    }

    foreach(DNodeLink *nld, outs)
    {
        //link the container output node to the last output of the selected nodes
        DinSocket *exitInContainer = contnode->getOutputs()->getVarSocket()->toIn();
        foreach(DinSocket *output, contnode->getOutputs()->getInSockets())
            if(output->getCntdSocket() == nld->out)
            {
                exitInContainer = output;
                break;
            }
        exitInContainer->addLink(nld->out);

        //relink the input previously connected to the generated mapped socket of the output of the container
        nld->in->addLink(contnode->getSocketOnContainer(exitInContainer)->toOut());
    }

    FRG::SpaceDataInFocus->addNode(contnode);
    return contnode;
}

void DNode::unpackContainerNode(DNode *node)
{
    ContainerNode *contnode = node->getDerived<ContainerNode>();
    QList<DNodeLink*>ins = FRG::CurrentProject->getInLinks(contnode);
    QList<DNodeLink*>outs = FRG::CurrentProject->getOutLinks(contnode);

    QList<DNodeLink> newInLinks;
    QList<DNodeLink> newOutLinks;

    foreach(DNodeLink *in, ins)
    {
        in->out->unregisterSocket(in->in);
        DoutSocket *inContainer = contnode->getSocketInContainer(in->in)->toOut();
        QList<DNodeLink> TMPnewInLinks = inContainer->getLinks();
        foreach(DNodeLink dnlink, TMPnewInLinks)
        {
            newInLinks.append(DNodeLink(dnlink.in, in->out));
            inContainer->unregisterSocket(dnlink.in, false);
        }
    }

    foreach(DNodeLink *out, outs)
    {
        out->out->unregisterSocket(out->in);
        DinSocket *inContainer = contnode->getSocketInContainer(out->out)->toIn();
        DoutSocket *newOutSocket = inContainer->getCntdSocket();
        newOutSocket->unregisterSocket(inContainer, false);
        newOutLinks.append(DNodeLink(out->in, newOutSocket));
    }

    QList<DNode*>nodes(contnode->getContainerData()->getNodes());
    QPointF contNodePos = FRG::CurrentProject->getNodePosition(contnode);
    foreach(DNode *node, nodes)
    {
        if(node == contnode->getInputs()
            ||node == contnode->getOutputs())
            continue;

        node->getSpace()->unregisterNode(node);
        FRG::SpaceDataInFocus->addNode(node);
        VNode *nodeVis = node->createNodeVis();
        FRG::Space->addItem(nodeVis);
        QPointF oldPos = FRG::CurrentProject->getNodePosition(node);
        QPointF newPos = contNodePos + oldPos;
        FRG::CurrentProject->setNodePosition(node, newPos);
        nodeVis->setPos(newPos);
        nodeVis->setSelected(true);
    }

    foreach(DNodeLink in, newInLinks)
        in.in->setCntdSocket(in.out);

    foreach(DNodeLink out, newOutLinks)
        out.in->setCntdSocket(out.out);

    FRG::SpaceDataInFocus->unregisterNode(node);
}

void DNode::addArray(DSocket *first)    
{
    arrays.append(new ArrayContainer(first));
}

FunctionNode::FunctionNode(QString name)
    : DNode(name)
{}

FunctionNode::FunctionNode(const FunctionNode* node)
    : DNode(node), function_name(node->getFunctionName())
{
}

QString FunctionNode::getFunctionName() const
{
    return function_name;
}

void FunctionNode::setFunctionName(QString value)
{
    function_name = value;
}

bool FunctionNode::operator==(const DNode &node)const
{
    if(!DNode::operator==(node)) return false;
    if(function_name != node.getDerivedConst<FunctionNode>()->function_name)
        return false;
    return true;
}

bool FunctionNode::operator!=(const DNode &node)const
{
    return (!(operator==(node)));
}

ContainerNode::ContainerNode(QString name, bool raw)
    : DNode(name), containerData(0)
{
    setNodeType(CONTAINER);
    if(!raw)
    {
        setContainerData(new ContainerSpace);
        containerData->setName(name);
        new SocketNode(IN, this);
        new SocketNode(OUT, this);
    }
}

ContainerNode::ContainerNode(const ContainerNode* node)
    : DNode(node)
{
    setNodeType(CONTAINER);
    setContainerData(new ContainerSpace(node->getContainerData()));
    foreach(DSocket *socket, node->getMappedSocketsOnContainer())
        mapOnToIn(CopySocketMapper::getCopy(socket), CopySocketMapper::getCopy(node->getSocketInContainer(socket)));

    setInputs(CopyNodeMapper::getCopy(node->getInputs()));
    setOutputs(CopyNodeMapper::getCopy(node->getOutputs()));
}

ContainerNode::~ContainerNode()
{
    if(containerData) delete containerData;
}


VNode* ContainerNode::createNodeVis()
{
    setNodeVis(new VContainerNode(this));
    //setNodeVis(new VNode(this));
    return getNodeVis();
}

void ContainerNode::addMappedSocket(DSocket *socket)
{
    DSocket *mapped_socket = 0;
    if(socket->getDir() == IN)
        mapped_socket = new DoutSocket(socket->getName(), socket->getType(), inSocketNode);
    else
        mapped_socket = new DinSocket(socket->getName(), socket->getType(), outSocketNode);
    mapOnToIn(socket, mapped_socket);     
}

void ContainerNode::addtolib()
{
    QDir::setCurrent(QCoreApplication::applicationDirPath());

    QString filename;
    filename.append("nodes/");
    filename.append(getNodeName());
    filename.append(".node");
    QFile file(filename);
    file.open(QIODevice::WriteOnly);
    QDataStream out(&file);
    out<<FRG_NODE_HEADER;
    out<<this;
}

void ContainerNode::C_addItems(QList<DNode *> nodes)
{
    foreach(DNode *node, nodes)
        containerData->addNode(node);
}

void ContainerNode::setInputs(SocketNode *inputNode)
{
    inSocketNode = inputNode;
    if(!containerData->getNodes().contains(inputNode))
        containerData->addNode(inputNode);
    inputNode->connectToContainer(this);
}

void ContainerNode::setInputs(DNode *inputNode)    
{
    setInputs(static_cast<SocketNode*>(inputNode));
}

void ContainerNode::setOutputs(SocketNode *outputNode)
{
    outSocketNode = outputNode;
    containerData->addNode(outputNode);
    outputNode->connectToContainer(this);
}

void ContainerNode::setOutputs(DNode *outputNode)    
{
    setOutputs(static_cast<SocketNode*>(outputNode));
}

SocketNode* ContainerNode::getInputs() const
{
    return inSocketNode;
}

SocketNode* ContainerNode::getOutputs() const
{
    return outSocketNode;
}

void ContainerNode::newSocket(DSocket *socket)
{
    DSocket *mapped_socket = 0;
    if(socket->getDir() == IN)
        mapped_socket = new DoutSocket(socket->toOut(), this);
    else
        mapped_socket = new DinSocket(socket->toIn(), this);
    mapOnToIn(socket, mapped_socket);
}

void ContainerNode::killSocket(DSocket *socket)
{
    DSocket *contsocket = socket_map.value(socket);
    socket_map.remove(socket);
    removeSocket(contsocket);
}

DSocket *ContainerNode::getSocketInContainer(const DSocket *socket) const
{
    return socket_map.value(const_cast<DSocket*>(socket));
}

void ContainerNode::mapOnToIn(DSocket *on, DSocket *in)    
{
    if(!on &&!in) return;
    socket_map.insert(on, in);
    in->addRenameCB(new ScpNameCB(in, on));
    in->addTypeCB(new ScpTypeCB(in, on));
}

DSocket *ContainerNode::getSocketOnContainer(const DSocket *socket) const
{
    return socket_map.key(const_cast<DSocket*>(socket));
}

QList<DSocket*> ContainerNode::getMappedSocketsOnContainer() const
{
    return socket_map.keys();
}

int ContainerNode::getSocketMapSize() const
{
    return socket_map.size();
}

void ContainerNode::setNodeName(QString name)
{
    DNode::setNodeName(name);
    if(containerData)containerData->setName(name);
}

ContainerSpace* ContainerNode::getContainerData() const
{
    return containerData;
}

void ContainerNode::setContainerData(ContainerSpace* value)
{
    containerData = value;
    containerData->setContainer(this);
}

bool ContainerNode::operator==(const DNode &node)const
{
    if(!DNode::operator==(node)) return false;
    if(*containerData != *node.getDerivedConst<ContainerNode>()->containerData)
        return false;
    return true;
}

bool ContainerNode::operator!=(const DNode &node)const
{
    return (!(operator==(node)));
}

ConditionContainerNode::ConditionContainerNode(bool raw)
    : ContainerNode("Condition", raw)
{
    setNodeType(CONDITIONCONTAINER);
    if(!raw)
    {
        new DinSocket("Condition", CONDITION, this);
    }
}

ConditionContainerNode::ConditionContainerNode(const ConditionContainerNode* node)
    : ContainerNode(node)
{
    setNodeType(CONDITIONCONTAINER);
}

SocketNode::SocketNode(socket_dir dir, ContainerNode *contnode, bool raw)
    : container(0)
{
    if (dir == IN)
    {
        setNodeType(INSOCKETS);
        if(!raw && contnode)setInSocketNode(contnode);
    }
    else
    {
        setNodeType(OUTSOCKETS);
        if(!raw && contnode)setOutSocketNode(contnode);
    }
}

SocketNode::SocketNode(const SocketNode* node)
    : DNode(node), container(CopyNodeMapper::getCopy(static_cast<DNode*>(node->getContainer()))->getDerived<ContainerNode>())
{
}

void SocketNode::setInSocketNode(ContainerNode *contnode)
{
    setDynamicSocketsNode(OUT);
    setNodeName("Input");
    contnode->setInputs(this);
}

void SocketNode::setOutSocketNode(ContainerNode *contnode)
{
    setDynamicSocketsNode(IN);
    setNodeName("Output");
    contnode->setOutputs(this);
}

void SocketNode::connectToContainer(ContainerNode *contnode)
{
    container = contnode;
}

ContainerNode* SocketNode::getContainer() const
{
    return container;
}

void SocketNode::inc_var_socket()
{
    DNode::inc_var_socket();
	DSocket *newsocket;
//    unsigned short arrID = container->getSocketOnContainer(getLastSocket())->getArrayID();
	if(getLastSocket()->getDir() == IN)
		newsocket = new DoutSocket(getLastSocket()->getName(), getLastSocket()->getType(), container);
	else
		newsocket = new DinSocket(getLastSocket()->getName(), getLastSocket()->getType(), container);
    container->mapOnToIn(newsocket, getLastSocket());
 //   if(arrID > 0)
 //       newsocket->setArray(arrID);
}

void SocketNode::dec_var_socket(DSocket *socket)
{
    if(container)container->removeSocket(container->getSocketOnContainer(socket));
    DNode::dec_var_socket(socket);
}

LoopSocketNode::LoopSocketNode(socket_dir dir, ContainerNode *contnode, bool raw)
    :SocketNode(dir, contnode, raw), partner(0)
{
    if (dir == IN)
    {
        setNodeType(LOOPINSOCKETS);
    }
    else
    {
        setNodeType(LOOPOUTSOCKETS);
    }

}

LoopSocketNode::LoopSocketNode(const LoopSocketNode* node)
    : SocketNode(node), partner(0)
{
    foreach(DSocket *original, node->getLoopedSockets())
        loopSocketMap.insert(CopySocketMapper::getCopy(original), CopySocketMapper::getCopy(node->getPartnerSocket(original)));        
}

void LoopSocketNode::dec_var_socket(DSocket *socket)
{
    SocketNode::dec_var_socket(socket);
    if(partner != 0)
    {
        LoopSocketNode *tmpp = partner;
        partner->setPartner(0);
        deletePartnerSocket(socket);
        partner->setPartner(tmpp);
    }
}

QList<DSocket*> LoopSocketNode::getLoopedSockets() const
{
    return loopSocketMap.keys();
}

qint16 LoopSocketNode::getLoopedSocketsCount() const
{
    return loopSocketMap.size();
}

void LoopSocketNode::createPartnerSocket(DSocket *socket)
{
	DSocket *partnerSocket;
	if(socket->getDir() == IN)
		partnerSocket = new DoutSocket(socket->getName(), socket->getType(), partner);
	else
		partnerSocket = new DinSocket(socket->getName(), socket->getType(), partner);
    loopSocketMap.insert(socket, partnerSocket);
    partner->mapPartner(partnerSocket, socket);
}

void LoopSocketNode::deletePartnerSocket(DSocket *socket)
{
    DinSocket *partnerSocket = getPartnerSocket(socket)->toIn();
    partner->removeSocket(partnerSocket);
    loopSocketMap.remove(socket);
}

void LoopSocketNode::mapPartner(DSocket* here, DSocket *partner)    
{
   loopSocketMap.insert(here, partner); 
}

void LoopSocketNode::setPartner(LoopSocketNode *p)
{
    partner = p;
}

DSocket *LoopSocketNode::getPartnerSocket(DSocket *socket) const
{
    if(loopSocketMap.keys().contains(socket))
        return loopSocketMap.value(socket);
    else return 0;
}

void LoopSocketNode::inc_var_socket()
{
    SocketNode::inc_var_socket();
    if(partner) createPartnerSocket(getLastSocket());
}

ConditionNode::ConditionNode(NType t, bool raw)
{
    setNodeType(t);

    if(!raw)
    {
        new DoutSocket("Output", CONDITION, this);
        if(getNodeType() != AND && getNodeType() != OR)
        {
            new DinSocket("Input", getNodeType() == NOT ? CONDITION : VARIABLE, this);
            if(getNodeType() != NOT)
                new DinSocket("Input", VARIABLE, this);
        }
        switch(getNodeType())
        {
        case GREATERTHAN:
            setNodeName("Greater Than");
            break;
        case SMALLERTHAN:
            setNodeName("Smaller Than");
            break;
        case EQUAL:
            setNodeName("Equal");
            break;
        case NOT:
            setNodeName("Not");
            break;
        case AND:
            setNodeName("And");
            setDynamicSocketsNode(IN, CONDITION);
            break;
        case OR:
            setNodeName("Or");
            setDynamicSocketsNode(IN, CONDITION);
            break;
        default:
            break;
        }
    }
}

ConditionNode::ConditionNode(const ConditionNode* node)
    : DNode(node)
{
}

MathNode::MathNode(NType t, bool raw)
{
    setNodeType(t);
    switch(t)
    {
    case ADD:
        setNodeName("Add");
        break;
    case SUBTRACT:
        setNodeName("Subtract");
        break;
    case MULTIPLY:
        setNodeName("Multiply");
        break;
    case DIVIDE:
        setNodeName("Divide");
        break;
    case DOTPRODUCT:
        setNodeName("Dot Product");
        break;
    default:
        break;
    }

    if(!raw)
    {
        setDynamicSocketsNode(IN);
        DoutSocket *out = new DoutSocket("Result", VARIABLE, this);
        DinSocket *in = getInSockets().first();
        in->addTypeCB(new ScpTypeCB(getVarSocket(), out));
    }
}

MathNode::MathNode(const MathNode* node)
    : DNode(node)
{
    DoutSocket *out = getOutSockets().first();
    DinSocket *in = getInSockets().first();
    in->addTypeCB(new ScpTypeCB(in, out));
}

void MathNode::dec_var_socket(DSocket *socket)
{
    DNode::dec_var_socket(socket);
    DoutSocket *outsocket = getOutSockets().first();
    if(getVarcnt() == 0)
    {
        outsocket->setType(VARIABLE);
        getVarSocket()->setType(VARIABLE);
    }
}

bool DNode::isMathNode(const DNode *node)
{
    return node->getNodeType() == ADD
       || node->getNodeType() == SUBTRACT
       || node->getNodeType() == MULTIPLY
       || node->getNodeType() == DIVIDE
       || node->getNodeType() == DOTPRODUCT;
}

bool DNode::isValueNode()    const
{
    return getNodeType() == FLOATNODE
        || getNodeType() == STRINGNODE
        || getNodeType() == COLORNODE
        || getNodeType() == VECTORNODE;
}

bool DNode::isConditionNode(const DNode *node)    
{
    return node->getNodeType() == GREATERTHAN
    || node->getNodeType() == SMALLERTHAN
    || node->getNodeType() == NOT
    || node->getNodeType() == EQUAL;
}

ValueNode::ValueNode(QString name)
    : DNode(name), shaderInput(false)
{
}

ValueNode::ValueNode(const ValueNode* node)
    : DNode(node), shaderInput(node->isShaderInput())
{
}

VNode* ValueNode::createNodeVis()
{
    setNodeVis(new VValueNode(this));
    return getNodeVis();
}

void ValueNode::setNodeName(QString name)    
{
    DNode::setNodeName(name);
    if(!getOutSockets().isEmpty())
        getOutSockets().first()->setName(name);
}

void ValueNode::setShaderInput(bool si)
{
    shaderInput = si;
}

bool ValueNode::isShaderInput() const
{
    return shaderInput;
}

ColorValueNode::ColorValueNode(QString name, bool raw)
    : ValueNode(name), colorvalue(QColor(255, 255, 255))
{
    setNodeType(COLORNODE);
    if(!raw)
    {
        new DoutSocket("Color", COLOR, this);
    }

    setNodeName("Color");
}

ColorValueNode::ColorValueNode(const ColorValueNode* node)
    : ValueNode(node), colorvalue(node->getValue())
{
}

VNode* ColorValueNode::createNodeVis()
{
    setNodeVis(new VColorValueNode(this));
    return getNodeVis();
}

void ColorValueNode::setValue(QColor newvalue)
{
    colorvalue = newvalue;
}

QColor ColorValueNode::getValue() const
{
    return colorvalue;
}

bool ColorValueNode::operator ==(const DNode &node)const
{
    if(!DNode::operator==(node)) return false;
    if(colorvalue != node.getDerivedConst<ColorValueNode>()->colorvalue)
        return false;
    return true;
}

bool ColorValueNode::operator!=(const DNode &node)const
{
    return (!(operator==(node)));
}

StringValueNode::StringValueNode(QString name, bool raw)
    : ValueNode(name)
{
    setNodeType(STRINGNODE);
    setNodeName("String");
    if(!raw)
    {
        new DoutSocket("String", STRING, this);
    }
}

StringValueNode::StringValueNode(const StringValueNode* node)
    : ValueNode(node), stringvalue(node->getValue())
{
}

VNode* StringValueNode::createNodeVis()
{
    setNodeVis(new VStringValueNode(this));
    return getNodeVis();
}

void StringValueNode::setValue(QString newstring)
{
    stringvalue = newstring;
}

QString StringValueNode::getValue() const
{
    return stringvalue;
}

bool StringValueNode::operator ==(const DNode &node)const
{
    if(!DNode::operator==(node)) return false;
    if(stringvalue != node.getDerivedConst<StringValueNode>()->stringvalue)
        return false;
    return true;
}

bool StringValueNode::operator!=(const DNode &node)const
{
    return (!(operator==(node)));
}

FloatValueNode::FloatValueNode(QString name, bool raw)
    : ValueNode(name), floatvalue(1.0)
{
    setNodeType(FLOATNODE);
    setNodeName("Float");
    if(!raw)
    {
        new DoutSocket("Float", FLOAT, this);
    }
}

FloatValueNode::FloatValueNode(const FloatValueNode* node)
    : ValueNode(node), floatvalue(node->getValue())
{
}

VNode* FloatValueNode::createNodeVis()
{
    setNodeVis(new VFloatValueNode(this));
    return getNodeVis();
}

void FloatValueNode::setValue(double newval)
{
    floatvalue = newval;
}

float FloatValueNode::getValue() const
{
    return floatvalue;
}

bool FloatValueNode::operator==(const DNode &node)const
{
    if(!DNode::operator==(node)) return false;
    if(floatvalue != node.getDerivedConst<FloatValueNode>()->floatvalue)
        return false;
    return true;
}

bool FloatValueNode::operator!=(const DNode &node)const
{
    return (!(operator==(node)));
}

VectorValueNode::VectorValueNode(QString name, bool raw)
    :ValueNode(name), vectorvalue(0, 0, 0)
{
    setNodeType(VECTORNODE);
    setNodeName("Vector");
    if(!raw)
    {
        new DoutSocket("Vector", VECTOR, this);
    }
}

VectorValueNode::VectorValueNode(const VectorValueNode* node)
    : ValueNode(node), vectorvalue(node->getValue())
{
}

VNode* VectorValueNode::createNodeVis()
{
    setNodeVis(new VVectorValueNode(this));
    return getNodeVis();
}

void VectorValueNode::setValue(Vector newvalue)
{
    vectorvalue = newvalue;
}

Vector VectorValueNode::getValue() const
{
    return vectorvalue;
}

bool VectorValueNode::operator==(const DNode &node)const
{
    if(!DNode::operator==(node)) return false;
    const VectorValueNode *vnode = node.getDerivedConst<VectorValueNode>();
    if(vectorvalue.x != vnode->getValue().x
        ||vectorvalue.y != vnode->getValue().y
        ||vectorvalue.z != vnode->getValue().z)
        return false;
    return true;
}

bool VectorValueNode::operator!=(const DNode &node)const
{
    return (!(operator==(node)));
}

LoopNode::LoopNode(QString name, bool raw)
    : ContainerNode(name, true)
{
    if(!raw)
    {
        LoopSocketNode *loutNode, *linNode;
        setContainerData(new ContainerSpace);
        getContainerData()->setName(name);

        linNode = new LoopSocketNode(IN, this);
        loutNode = new LoopSocketNode(OUT, this);
        linNode->setPartner(loutNode);
        loutNode->setPartner(linNode);
    }
}

LoopNode::LoopNode(const LoopNode* node)
    : ContainerNode(node)
{
}

bool LoopNode::isLoopNode(DNode *node)
{
    return (node->getNodeType() == FOR
            ||node->getNodeType() == WHILE
            ||node->getNodeType() == ILLUMINANCE
            ||node->getNodeType() == ILLUMINATE
            ||node->getNodeType() == SOLAR
            ||node->getNodeType() == GATHER);
}
WhileNode::WhileNode(bool raw)
    : LoopNode("While", raw)
{
    setNodeType(WHILE);
    if(!raw)
    {
        new DinSocket("Condition", CONDITION, getOutputs());
        DSocketList *inputSocketList = getInputs()->getDerived<LoopSocketNode>()->getOutSocketLlist();
        inputSocketList->move(0, inputSocketList->len() - 1);
    }
}

WhileNode::WhileNode(const WhileNode* node)
    : LoopNode(node)
{
}

ForNode::ForNode(bool raw)
    : LoopNode("For", raw)
{
    setNodeType(FOR);
    if(!raw)
    {
        addMappedSocket(new DinSocket("Start", FLOAT, this));
        addMappedSocket(new DinSocket("End", FLOAT, this));
        addMappedSocket(new DinSocket("Step", FLOAT, this));
        DSocketList *inputSocketList = getInputs()->getDerived<LoopSocketNode>()->getOutSocketLlist();
        inputSocketList->move(0, inputSocketList->len() - 1);
    }
}

ForNode::ForNode(const ForNode* node)
    : LoopNode(node)
{
}

IlluminanceNode::IlluminanceNode(bool raw)
    : LoopNode("Illuminance", raw)
{
    setNodeType(ILLUMINANCE);
    if(!raw)
    {
        addMappedSocket(new DinSocket("Category", STRING, this));
        addMappedSocket(new DinSocket("Point", POINT, this));
        addMappedSocket(new DinSocket("Direction", VECTOR, this));
        addMappedSocket(new DinSocket("Angle", FLOAT, this));
        addMappedSocket(new DinSocket("Message Passing", STRING, this));
        DSocketList *inputSocketList = getInputs()->getDerived<LoopSocketNode>()->getOutSocketLlist();
        inputSocketList->move(0, inputSocketList->len() - 1);
    }
}

IlluminanceNode::IlluminanceNode(const IlluminanceNode* node)
    : LoopNode(node)
{
}

IlluminateNode::IlluminateNode(bool raw)
    : LoopNode("Illuminate", raw)
{
    setNodeType(ILLUMINATE);
    if(!raw)
    {
        addMappedSocket(new DinSocket("Point", POINT, this));
        addMappedSocket(new DinSocket("Direction", VECTOR, this));
        addMappedSocket(new DinSocket("Angle", FLOAT, this));
        DSocketList *inputSocketList = getInputs()->getDerived<LoopSocketNode>()->getOutSocketLlist();
        inputSocketList->move(0, inputSocketList->len() - 1);
    }
}

IlluminateNode::IlluminateNode(const IlluminateNode* node)
    : LoopNode(node)
{
}

GatherNode::GatherNode(bool raw)
    : LoopNode("Gather", raw)
{
    setNodeType(GATHER);
    if(!raw)
    {
        addMappedSocket(new DinSocket("Category", STRING, this));
        addMappedSocket(new DinSocket("Point", POINT, this));
        addMappedSocket(new DinSocket("Direction", VECTOR, this));
        addMappedSocket(new DinSocket("Angle", FLOAT, this));
        addMappedSocket(new DinSocket("Message Passing", STRING, this));
        addMappedSocket(new DinSocket("Samples", FLOAT, this));
        DSocketList *inputSocketList = getInputs()->getDerived<LoopSocketNode>()->getOutSocketLlist();
        inputSocketList->move(0, inputSocketList->len() - 1);
    }
    getInSocketLlist()->move(0, getInSocketLlist()->len() - 1);
}

GatherNode::GatherNode(const GatherNode* node)
    : LoopNode(node)
{
}

SolarNode::SolarNode(bool raw)
    : LoopNode("Solar", raw)
{
    setNodeType(SOLAR);
    if(!raw)
    {
        addMappedSocket(new DinSocket("Axis", VECTOR, this));
        addMappedSocket(new DinSocket("Angle", FLOAT, this));
        DSocketList *inputSocketList = getInputs()->getDerived<LoopSocketNode>()->getOutSocketLlist();
        inputSocketList->move(0, inputSocketList->len() - 1);
    }
}

SolarNode::SolarNode(const SolarNode* node)
    : LoopNode(node)
{
}

OutputNode::OutputNode()
{
    sedit = new SourceDock(this);
}

OutputNode::~OutputNode()
{
    delete sedit;
}

OutputNode::OutputNode(const OutputNode* node)
    : DNode(node), ShaderName(node->getShaderName())
{
    sedit = new SourceDock(this);
}

SourceDock* OutputNode::getSourceEdit()    
{
    return sedit; 
}

VNode* OutputNode::createNodeVis()
{
    setNodeVis(new VOutputNode(this));
    return getNodeVis();
}

QString OutputNode::getShaderName() const
{
    return ShaderName;
}

void OutputNode::writeCode()
{
    QFile file(filename);
    file.open(QIODevice::WriteOnly);
    QTextStream stream(&file);
    ShaderWriter sw(this);
    QString code = sw.getCode();
    stream<<code;
    file.close();
}

void OutputNode::compile()    
{
    QProcess *process = new QProcess();
    QStringList arguments;
    arguments << filename;
    process->setStandardErrorFile("compiler.log");
    process->setWorkingDirectory(QFileInfo(filename).canonicalPath());
    process->start("shaderdl", arguments);
    process->waitForFinished(10000);
}

QString OutputNode::getFileName() const
{
    return filename;
}

void OutputNode::setFileName(QString name)
{
    filename = name;
}

void OutputNode::changeName(QString newname)
{
    QString shadertype, newnodename;
    switch(getNodeType())
    {
    case SURFACEOUTPUT:
        shadertype = "Surface Output";
        break;
    case DISPLACEMENTOUTPUT:
        shadertype = "Displacement Output";
        break;
    case VOLUMEOUTPUT:
        shadertype = "Volume Output";
        break;
    case LIGHTOUTPUT:
        shadertype = "Light Output";
        break;
    default:
	break;	
    }

    ShaderName = newname;
    newnodename = shadertype;
    newnodename += " (";
    newnodename += ShaderName;
    newnodename += " )";
    setNodeName(newnodename);
}

InputNode::InputNode()
{
}

InputNode::InputNode(const InputNode* node)
    : DNode(node)
{
}

GetArrayNode::GetArrayNode(bool raw)
    : DNode("GetArray")
{
    setNodeType(GETARRAY);
    if(!raw)
    {
        DinSocket *arr = new DinSocket("Array", VARIABLE, this);
        new DinSocket("Index", FLOAT, this);
        DoutSocket *val = new DoutSocket("value", VARIABLE, this);
        arr->addTypeCB(new ScpTypeCB(arr, val));
    }
}

GetArrayNode::GetArrayNode(const GetArrayNode* node)
    : DNode(node)
{
    DinSocket *arr = getInSockets().first();
    DoutSocket *val = getOutSockets().first();
    arr->addTypeCB(new ScpTypeCB(arr, val));
}

SetArrayNode::SetArrayNode(bool raw)
    : DNode("SetArray")
{
    setNodeType(SETARRAY);
    if(!raw){
        DinSocket *val = new DinSocket("value", VARIABLE, this);
        new DinSocket("Index", FLOAT, this);
        DoutSocket *arr = new DoutSocket("Array", VARIABLE, this);
        val->addTypeCB(new ScpTypeCB(val, arr));
    }
}

SetArrayNode::SetArrayNode(const SetArrayNode* node)
    : DNode(node)
{
    DinSocket *val = getInSockets().first();
    DoutSocket *arr = getOutSockets().first();
    val->addTypeCB(new ScpTypeCB(val, arr));
}

VarNameNode::VarNameNode(bool raw)
    : DNode("Var Name")
{
    setNodeType(VARNAME);
    if(!raw){
        DinSocket *in = new DinSocket("def", VARIABLE, this);
        DoutSocket *out =  new DoutSocket("variable", VARIABLE, this);
        in->addTypeCB(new ScpTypeCB(in, out));
    }
}

VarNameNode::VarNameNode(const VarNameNode* node)
    : DNode(node)
{
    DinSocket *in = getInSockets().first();
    DoutSocket *out = getOutSockets().first();
    in->addTypeCB(new ScpTypeCB(in, out));
}
