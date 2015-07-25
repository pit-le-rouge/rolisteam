/***************************************************************************
    *	Copyright (C) 2009 by Renaud Guezennec                                 *
    *   http://renaudguezennec.homelinux.org/accueil,3.html                    *
    *                                                                          *
    *   rolisteam is free software; you can redistribute it and/or modify      *
    *   it under the terms of the GNU General Public License as published by   *
    *   the Free Software Foundation; either version 2 of the License, or      *
    *   (at your option) any later version.                                    *
    *                                                                          *
    *   This program is distributed in the hope that it will be useful,        *
    *   but WITHOUT ANY WARRANTY; without even the implied warranty of         *
    *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
    *   GNU General Public License for more details.                           *
    *                                                                          *
    *   You should have received a copy of the GNU General Public License      *
    *   along with this program; if not, write to the                          *
    *   Free Software Foundation, Inc.,                                        *
    *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.              *
    ***************************************************************************/


#include <QScrollArea>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QUuid>

#include "vmapframe.h"
//#include "map/mapwizzard.h"
#include "map/newemptymapdialog.h"

VMapFrame::VMapFrame()
    : MediaContainer()
{
    setObjectName("VMapFrame");
    m_vmap = new VMap();
}

VMapFrame::VMapFrame(CleverURI* uri,VMap *map)
    : MediaContainer(),m_vmap(map)
{
    setObjectName("VMapFrame");
    m_uri =uri;
    
    createAction();
    updateMap();  
}


VMapFrame::~VMapFrame()
{
    //delete m_widgetLayout;
    delete m_maskPixmap;
    
}


void VMapFrame::closeEvent(QCloseEvent *event)
{
    
    hide();
	event->accept();
}
void  VMapFrame::createAction()
{
    m_graphicView = new RGraphicsView(m_vmap);
}
void VMapFrame::updateMap()
{
	setTitle(m_vmap->mapTitle());
    m_graphicView->setGeometry(0,0,m_vmap->mapWidth(),m_vmap->mapHeight());
    setGeometry(m_graphicView->geometry());
	setWidget(m_graphicView);
    setWindowIcon(QIcon(":/resources/icons/map.png"));
    m_maskPixmap = new QPixmap(m_graphicView->size());
    
    
}

VMap * VMapFrame::map()
{
    return m_vmap;
}
int VMapFrame::editingMode()
{
    return m_currentEditingMode;
}

void VMapFrame::startMoving(QPoint position)
{

}


void VMapFrame::Moving(QPoint position)
{

}
void VMapFrame::currentCursorChanged(QCursor* cursor)
{
    m_currentCursor = cursor;
    m_graphicView->setCursor(*cursor);
}
void VMapFrame::currentToolChanged(VToolsBar::SelectableTool selectedtool)
{

    m_currentTool = selectedtool;
    if(m_vmap != NULL)
    {
        m_vmap->setCurrentTool(selectedtool);
    }
    
}
void VMapFrame::mousePressEvent(QMouseEvent* event)
{
    MediaContainer::mousePressEvent(event);
}
void VMapFrame::currentPenSizeChanged(int ps)
{
    if(m_vmap != NULL)
        m_vmap->setPenSize(ps);
}
void VMapFrame::currentNPCSizeChanged(int ps)
{
    if(m_vmap != NULL)
        m_vmap->setNPCSize(ps);
}

void VMapFrame::currentColorChanged(QColor& penColor)
{
    m_penColor = penColor;
    
    if(m_vmap !=NULL)
    {
        m_vmap->setCurrentChosenColor(m_penColor);
    }
}
void VMapFrame::setEditingMode(int mode)
{
    m_currentEditingMode = mode;
}
bool VMapFrame::defineMenu(QMenu* /*menu*/)
{
    return false;
}
void VMapFrame::saveFile(const QString & filepath)
{
    if(!filepath.isEmpty())
    {
        QFile output(filepath);
        if (!output.open(QIODevice::WriteOnly))
            return;
        QDataStream out(&output);
        m_vmap->saveFile(out);

    }
    
}

void VMapFrame::openFile(const QString& filepath)
{
    if(!filepath.isEmpty())
    {
        QFile input(filepath);
        if (!input.open(QIODevice::ReadOnly))
            return;
        QDataStream in(&input);
        m_vmap->openFile(in);
        createAction();

        updateMap();
    }
}
void VMapFrame::keyPressEvent ( QKeyEvent * event )
{
    switch (event->key())
    {
    case Qt::Key_Delete:
        /// @todo remove selected item
        break;
    default:
        MediaContainer::keyPressEvent(event);
    }
}
void VMapFrame::setCleverURI(CleverURI* uri)
{
    m_uri=uri;
}

bool VMapFrame::hasDockWidget() const
{
    return false;
}
QDockWidget* VMapFrame::getDockWidget()
{
    return NULL;
    //return m_toolsbar;
}
bool VMapFrame::readFileFromUri()
{
    return false;
}
void VMapFrame::processAddItemMessage(NetworkMessageReader* msg)
{
    if(NULL!=m_vmap)
    {
        m_vmap->processAddItemMessage(msg);
    }
}
void VMapFrame::processMoveItemMessage(NetworkMessageReader* msg)
{
    if(NULL!=m_vmap)
    {
        m_vmap->processMoveItemMessage(msg);
    }
}

QString VMapFrame::getMapId()
{
    if(NULL!=m_vmap)
    {
        return m_vmap->getId();
    }
    return QString();
}
