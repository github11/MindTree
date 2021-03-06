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

#include "newnodeeditor.h"

#include "QLayout"
#include "QtGui"

#include "source/data/nodes/data_node_socket.h"
#include	"source/graphics/base/vnspace.h"
#include "source/data/base/frg.h"

SlotTypeEditor::SlotTypeEditor()
{
    insertItem(0, "Color");
    insertItem(1, "Float");
    insertItem(2, "String");
    insertItem(3, "Point");
    insertItem(4, "Normal");
    insertItem(5, "Vector");
    insertItem(6, "Variable");

    setItemData(0,  COLOR);
    setItemData(1, FLOAT);
    setItemData(2, STRING);
    setItemData(3, POINT);
    setItemData(4, NORMAL);
    setItemData(5, VECTOR);
    setItemData(6, VARIABLE);
}

RemoveButton::RemoveButton(int index)
{
    setIndex(index);
    setText("X");
    connect(this, SIGNAL(clicked()), this, SLOT(emitClicked()));
}

RemoveButton::~RemoveButton()
{

}

void RemoveButton::emitClicked()
{
    emit clicked(index);
}

void RemoveButton::setIndex(int index)
{
    this->index = index;
}

SocketEditor::SocketEditor()
{
    QStringList inputheader;
    inputheader<<"Name"<<"Type"<<"Token"<<"Dynamic"<<"Remove";
    setColumnCount(6);
    setHeaderLabels(inputheader);
    setAlternatingRowColors(true);
    header()->resizeSection(0, 90);
    header()->resizeSection(1, 70);
    header()->resizeSection(2, 40);
    header()->resizeSection(3, 40);
    header()->resizeSection(5, 40);
}

SocketEditor::~SocketEditor()
{

}

void SocketEditor::addSocket()
{
    int index = topLevelItemCount();
    QTreeWidgetItem *newitem = new QTreeWidgetItem();
    newitem->setFlags(Qt::ItemIsEditable | Qt::ItemIsSelectable |
                      Qt::ItemIsEnabled);
    addTopLevelItem(newitem);
    setItemWidget(newitem, 1, new SlotTypeEditor);
    setItemWidget(newitem, 2, new QCheckBox());
    setItemWidget(newitem, 3, new QCheckBox());
    RemoveButton *rembutton = new RemoveButton(index);
    connect(rembutton, SIGNAL(clicked(int)), this, SLOT(removeSocket(int)));
    setItemWidget(newitem, 4,rembutton);
}

void SocketEditor::removeSocket(int index)
{
    QTreeWidgetItem *item = takeTopLevelItem(index);
    delete item;
    refreshIndices();
}

void SocketEditor::refreshIndices()
{
    QTreeWidgetItem *item = topLevelItem(0);
    for(int i = 0; item; i++)
    {
        RemoveButton *button = (RemoveButton*)itemWidget(item, 4);
        button->setIndex(i);
        item = itemBelow(item);
    }
}

NewNodeEditor::NewNodeEditor()
{
    resize(700, 400);

    setUpdatesEnabled(true);
    createLayout();
}

NewNodeEditor::~NewNodeEditor()
{
    delete inputsockets;
    delete outputsockets;
    delete func_name;
    delete node_name;
}

void NewNodeEditor::createLayout()
{
    grid = new QGridLayout;

    QLabel *name_label = new QLabel("Node Name:");
    node_name = new QLineEdit;
    node_name->setText("New Node");
    grid->addWidget(name_label, 0, 0);
    grid->addWidget(node_name, 0, 1);

    QLabel *func_label = new QLabel("Function Name:");
    func_name = new QLineEdit;
    grid->addWidget(func_label, 1, 0);
    grid->addWidget(func_name, 1, 1);

    QPushButton *add_in_button = new QPushButton("add Input");
    grid->addWidget(add_in_button, 3, 0);
    QPushButton *add_out_button = new QPushButton("add Output");
    grid->addWidget(add_out_button, 3, 1);

    inputsockets = new SocketEditor;
    grid->addWidget(inputsockets, 4, 0);

    outputsockets = new SocketEditor;
    grid->addWidget(outputsockets, 4, 1);

    QPushButton *add_to_lib = new QPushButton("Add to Library");
    grid->addWidget(add_to_lib, 5, 0, 1, 2);

    setLayout(grid);

    connect(add_in_button, SIGNAL(clicked()), inputsockets, SLOT(addSocket()));
    connect(add_out_button, SIGNAL(clicked()), outputsockets, SLOT(addSocket()));
    connect(add_to_lib, SIGNAL(clicked()), this, SLOT(addtoLib()));
}

void NewNodeEditor::update()
{
    QWidget::update();
}

void NewNodeEditor::addtoLib()
{
    QDir::setCurrent(QCoreApplication::applicationDirPath());
    QString filename;
    filename.append("nodes/");
    filename.append(node_name->text());
    filename.append(".node");
    QFile file(filename);
    file.open(QIODevice::WriteOnly);
    QDataStream out(&file);

    qint16 outs, ins;
    outs = outputsockets->topLevelItemCount();
    ins = inputsockets->topLevelItemCount();

    out<<FRG_NODE_HEADER;
    out<<node_name->text()<<(unsigned short)0<<NType(FUNCTION)<<QPointF(0, 0);
    out<<ins<<outs;
    QTreeWidgetItem *item = inputsockets->topLevelItem(0);
    while(item)
    {
        QComboBox *sockettype = dynamic_cast<QComboBox*>(inputsockets->itemWidget(item, 1));
        QCheckBox *isToken = dynamic_cast<QCheckBox*>(inputsockets->itemWidget(item, 2));
        QCheckBox *isVar = dynamic_cast<QCheckBox*>(inputsockets->itemWidget(item, 3));
        QString socketname(item->text(0));
        socket_type stype = (socket_type)sockettype->itemData(sockettype->currentIndex()).toInt();

        out<<(unsigned short)0<<isVar->isChecked();
        out<<socketname<<stype<<isToken->isChecked()<<(unsigned short)0;
        item = inputsockets->itemBelow(item);
    }
    item = outputsockets->topLevelItem(0);
    while(item)
    {
        QComboBox *sockettype = dynamic_cast<QComboBox*>(outputsockets->itemWidget(item, 1));
        QCheckBox *isToken = dynamic_cast<QCheckBox*>(outputsockets->itemWidget(item, 2));
        QCheckBox *isVar = dynamic_cast<QCheckBox*>(outputsockets->itemWidget(item, 3));
        QString socketname(item->text(0));
        socket_type stype = (socket_type)sockettype->itemData(sockettype->currentIndex()).toInt();

        out<<(unsigned short)0<<isVar->isChecked();
        out<<socketname<<stype;
        item = outputsockets->itemBelow(item);
    }

    out<<func_name->text();
    file.close();
    
    FRG::lib->update();
}
