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

#include "graphics_node_socket.h"

#include "QMenu"
#include "QInputDialog"
#include "QPainter"
#include "QGraphicsSceneMouseEvent"

#include "source/data/nodes/data_node_socket.h"
#include "source/data/base/frg.h"
#include "source/data/nodes/data_node.h"
#include "graphics_node.h"
#include "source/graphics/base/vnspace.h"
#include "source/data/base/project.h"

VNSocket::VNSocket(DNSocket *data)
{
    this->data = data;
    setAcceptDrops(true);
    setAcceptHoverEvents(true);
    setZValue(zValue()+0.1);
    setFlag(ItemIsSelectable, false);
//    setGraphicsEffect(new QGraphicsDropShadowEffect);
    createContextMenu();
}

VNSocket::~VNSocket()
{
    disconnect();
}

void VNSocket::createContextMenu()
{
    contextMenu = new QMenu;
    QAction *changeNameAction = contextMenu->addAction("Rename Socket");
    QAction *changeTypeAction = contextMenu->addAction("change Type");

    connect(changeNameAction, SIGNAL(triggered()), this, SLOT(changeName()));
    connect(changeTypeAction, SIGNAL(triggered()), this, SLOT(changeType()));
}

void VNSocket::changeName()
{
    bool ok;
    QString newname;
    newname = QInputDialog::getText(0, "Change Socket Name", "New Name", QLineEdit::Normal, "", &ok);
    if(ok)
    {
        data->name = newname;
        data->node->setSocketVarName(data);
    }
}

void VNSocket::changeType()
{
    socket_type newtype;
    QStringList typelist;
    bool ok;
    typelist<<"Normal" << "Vector"<<"Float"<<"Color"<< "Point"<<"String"<<"Variable";
    QString newtypestr(QInputDialog::getItem(0, "Change Socket Type", "New Type", typelist, 0, false, &ok));

    if (newtypestr == "Normal")
        newtype = NORMAL;
    else if (newtypestr == "Vector")
        newtype = VECTOR;
    else if (newtypestr == "Float")
        newtype  = FLOAT;
    else if (newtypestr == "Color")
        newtype = COLOR;
    else if (newtypestr == "Point")
        newtype = POINT;
    else if (newtypestr == "String")
        newtype = STRING;
    else if (newtypestr == "Variable")
        newtype = VARIABLE;

    if(ok)
        data->type = newtype;
}

QRectF VNSocket::boundingRect() const
{
    return QRectF(-15, 15, -15, 15);
};

void VNSocket::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
    QColor color;
    switch(data->type)
    {
    case STRING:
        if (isUnderMouse())
            color = QColor(255, 100, 100);
        else
            color = QColor(160, 60, 60);
        break;
    case POINT:
        if (isUnderMouse())
            color = QColor(0, 255, 255);
        else
            color = QColor(90, 140, 210, 100);
        break;
    case NORMAL:
        if (isUnderMouse())
            color = QColor(0, 255, 255);
        else
            color = QColor(90, 140, 210, 100);
        break;
    case FLOAT:
        if (isUnderMouse())
            color = QColor(0, 255, 0);
        else
            color = QColor(90, 200, 90, 100);
        break;
    case VECTOR:
        if (isUnderMouse())
            color = QColor(0, 255, 255);
        else
            color = QColor(90, 140, 210, 100);
        break;
    case COLOR:
        if (isUnderMouse())
            color = QColor(255, 255, 0);
        else
            color = QColor(210, 210, 80, 100);
        break;
    case CONDITION:
        if (isUnderMouse())
            color = QColor(255, 0, 0);
        else
            color = QColor(170, 50, 50, 100);
        break;
    case VARIABLE:
        if(isUnderMouse())
            color = QColor(255, 0, 255);
        else
            color = QColor(120, 70, 120, 100);
    };
    painter->setBrush(QBrush(color, Qt::SolidPattern));
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(-15, 15, -15, 15, 2.5, 2.5);
};

void VNSocket::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    update();
};

void VNSocket::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    update();
};

void VNSocket::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if(event->button() != Qt::LeftButton)
        return;

	if(FRG::space->isLinkNodeMode())
    {
        FRG::space->enterlinkNodeMode(this);
    }
    else
    {
        FRG::space->leavelinkNodeMode(event->scenePos());
    }
}

void VNSocket::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    contextMenu->exec(event->screenPos());
}
