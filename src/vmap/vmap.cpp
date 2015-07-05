#include <QGraphicsSceneMouseEvent>
#include <QDebug>
#include <QGraphicsProxyWidget>
#include <QLineEdit>
#include <QPainter>
#include <QPixmap>


#include "vmap.h"
#include "items/rectitem.h"
#include "items/ellipsitem.h"
#include "items/pathitem.h"
#include "items/lineitem.h"
#include "items/textitem.h"
#include "items/characteritem.h"


#include "network/networkmessagewriter.h"


VMap::VMap(QObject * parent)
    : QGraphicsScene(parent)
{
    m_currentItem = NULL;
    m_id = QUuid::createUuid().toString();
    m_itemList=new  QList<VisualItem*>;
    setItemIndexMethod(QGraphicsScene::NoIndex);
}


VMap::VMap(int width,int height,QString& title,QColor& bgColor,QObject * parent)
    : QGraphicsScene(0,0,width,height,parent)
{
    m_title = title;
    m_bgColor = bgColor;
    setBackgroundBrush(m_bgColor);
    m_id = QUuid::createUuid().toString();
    m_currentItem = NULL;
    m_itemList=new  QList<VisualItem*>;
    setItemIndexMethod(QGraphicsScene::NoIndex);
    
}

void VMap::setWidth(int width)
{
    m_width = width;
    setSceneRect();
}

void VMap::setHeight(int height)
{
    m_height = height;
    setSceneRect();
}

void VMap::setTitle(QString title)
{
    m_title = title;
}

void VMap::setBackGroundColor(QColor& bgcolor)
{
    m_bgColor = bgcolor;
    generateBackground();
    
}
void VMap::generateBackground()
{
    if(m_gridPattern.isNull())
    {
        setBackgroundBrush(m_bgColor);
    }
    else
    {
        QPixmap p(m_gridPattern);
        p.fill(m_bgColor);
        QPainter painter(&p);
        painter.drawPixmap(0,0,m_gridPattern);
        painter.end();
        setBackgroundBrush(p);
    }
}

void VMap::setSceneRect()
{
    QGraphicsScene::setSceneRect(0,0,m_width,m_height);
}
int VMap::mapWidth() const
{
    return m_width;
}
int VMap::mapHeight() const
{
    return m_height;
}
const QString& VMap::mapTitle() const
{
    return m_title;
}
const QColor& VMap::mapColor() const
{
    return m_bgColor;
}
int VMap::getNpcSize() const
{
    return m_npcSize;
}

void VMap::setCurrentTool(VToolsBar::SelectableTool selectedtool)
{
    m_selectedtool = selectedtool;
    m_currentItem = NULL;
}
void VMap::mouseMoveEvent ( QGraphicsSceneMouseEvent * mouseEvent )
{
    if(m_currentItem!=NULL)
    {
        /*  if(m_selectedtool!=ToolsBar::LINE)
    {*/

        m_end = mouseEvent->scenePos ();
        m_currentItem->setNewEnd( m_end);
        update();
        // }
    }
    if(m_selectedtool==VToolsBar::HANDLER)
        QGraphicsScene::mouseMoveEvent(mouseEvent);
}
void VMap::addItem()
{
    //QGraphicsItem* item = NULL;
    
    switch(m_selectedtool)
    {
    case VToolsBar::PEN:
        m_currentItem=new PathItem(m_first,m_itemColor,m_penSize);
        break;
    case VToolsBar::LINE:
        m_currentItem= new LineItem(m_first,m_itemColor,m_penSize);
        break;
    case VToolsBar::EMPTYRECT:
        m_currentItem = new RectItem(m_first,m_end,false,m_itemColor);
        break;
    case VToolsBar::FILLRECT:
        m_currentItem = new RectItem(m_first,m_end,true,m_itemColor);
        break;
    case VToolsBar::EMPTYELLIPSE:
        m_currentItem = new EllipsItem(m_first,false,m_itemColor);
        break;
    case VToolsBar::FILLEDELLIPSE:
        m_currentItem = new EllipsItem(m_first,true,m_itemColor);
        break;
    case VToolsBar::TEXT:
    {
        QLineEdit* tempedit = new QLineEdit();
        TextItem* temptext = new TextItem(m_first,tempedit,m_itemColor);
        m_currentItem = temptext;
        QGraphicsProxyWidget * tmp = QGraphicsScene::addWidget(tempedit);
        tmp->setPos(m_first.x(),m_first.y()-tempedit->height());
        tempedit->setEnabled(true);
        tempedit->setFocus();
        connect(tempedit,SIGNAL(editingFinished()),temptext,SLOT(editingFinished()));
        connect(tempedit,SIGNAL(editingFinished()),this,SLOT(update()));
    }
        break;
    case VToolsBar::HANDLER:

        break;
    case VToolsBar::ADDNPC:

        break;
    case VToolsBar::DELNPC:

        break;
    case VToolsBar::MOVECHARACTER:

        break;
    case VToolsBar::STATECHARACTER:

        break;
    }
    if(m_currentItem!=NULL)
    {
        QGraphicsScene::addItem(m_currentItem);
        m_itemList->append(m_currentItem);

        NetworkMessageWriter msg(NetMsg::VMapCategory,NetMsg::addItem);
        msg.string8(m_id);
        msg.uint8(m_currentItem->type());
        m_currentItem->fillMessage(&msg);
        msg.sendAll();
    }
}
void VMap::setPenSize(int p)
{
    m_penSize =p;
}

void VMap::setNPCSize(int p)
{
    m_npcSize =p;
}
void VMap::mousePressEvent ( QGraphicsSceneMouseEvent * mouseEvent )
{
    
    if(m_selectedtool==VToolsBar::HANDLER)
    {
        //m_currentItem = dynamic_cast<VisualItem*>(itemAt(mouseEvent->scenePos()));
        QGraphicsScene::mousePressEvent(mouseEvent);
    }
    else if(mouseEvent->button() == Qt::LeftButton)
    {
        m_first = mouseEvent->scenePos();
        m_end = m_first;
        addItem();
    }

}

void VMap::mouseReleaseEvent ( QGraphicsSceneMouseEvent * mouseEvent )
{
    Q_UNUSED(mouseEvent);
    m_currentItem = NULL;
    if(m_selectedtool==VToolsBar::HANDLER)
        QGraphicsScene::mouseReleaseEvent(mouseEvent);
}
void VMap::setCurrentChosenColor(QColor& p)
{
    m_itemColor = p;
    
    
}
QString VMap::getId() const
{
    return m_id;
}
void VMap::setId(QString id)
{
    m_id = id;
}

QDataStream& operator<<(QDataStream& out, const VMap& con)
{
    out << con.m_width;
    out << con.m_height;
    out << con.m_title;
    out << con.m_bgColor;
    
    out << con.m_itemList->size();
    for(int i = 0; i< con.m_itemList->size();i++)
    {
        VisualItem* item = con.m_itemList->at(i);
        out << *item ;
    }
    return out;
}

QDataStream& operator>>(QDataStream& is,VMap& con)
{
    is >>(con.m_width);
    is >>(con.m_height);
    is >>(con.m_title);
    is >>(con.m_bgColor);
    
    int size;
    is >> size;
    
    
    
    return is;
}
void VMap::saveFile(QDataStream& out)
{
    if(m_itemList->isEmpty())
        return;
    
    out << m_width;
    out<< m_height;
    out<< m_title;
    out<< m_bgColor;
    out << m_itemList->size();
    qDebug()<< "m_itemList size" << m_itemList->size() <<  m_bgColor << m_width << m_height << m_title ;
    
    foreach(VisualItem* tmp, *m_itemList)
    {
        out << tmp->getType() << *tmp;
    }
}

void VMap::openFile(QDataStream& in)
{
    if(m_itemList!=NULL)
    {
        in >> m_width;
        in >> m_height;
        in>> m_title;
        in>> m_bgColor;

        int numberOfItem;
        in >> numberOfItem;
        qDebug()<< "m_itemList size" << numberOfItem <<  m_bgColor << m_width << m_height << m_title ;
        for(int i =0 ; i<numberOfItem;i++)
        {
            VisualItem* item;
            item=NULL;
            VisualItem::ItemType type;
            int tmptype;
            in >> tmptype;
            type=(VisualItem::ItemType)tmptype;

            switch(type)
            {
            case VisualItem::TEXT:
                item=new TextItem();

                break;
            case VisualItem::CHARACTER:
                /// @TODO: Reimplement that feature
                // item=new CharacterItem();

                break;
            case VisualItem::LINE:
                item=new LineItem();

                break;
            case VisualItem::RECT:
                item=new RectItem();
                break;
            case VisualItem::ELLISPE:
                item=new EllipsItem();

                break;
            case VisualItem::PATH:
                item=new PathItem();

                break;

            }
            in >> *item;
            QGraphicsScene::addItem(item);
            m_itemList->append(item);
        }
        qDebug()<< m_itemList->size();
    }
}

void VMap::addCharacter(const Character* p, QPointF pos)
{
    CharacterItem* item= new CharacterItem(p,pos);
    QGraphicsScene::addItem(item);
}

void VMap::setPatternSize(int p)
{
    m_sizePattern = p;
}

void VMap::setPattern(QPixmap p)
{
    m_gridPattern = p;
}

void VMap::setScale(int p)
{
    m_patternScale = p;
}

void VMap::setScaleUnit(int p)
{
    switch(p)
    {
    case PX:
    case FEET:
    case CM:
    case INCH:
    case M:
        m_patternUnit = (VMap::SCALE_UNIT)p;
        break;
    default:
        m_patternUnit = PX;
        break;
    }
    
    
}
void VMap::processAddItemMessage(NetworkMessageReader* msg)
{
    VisualItem* item=NULL;
    VisualItem::Type type = msg->uint8();
    switch(type)
    {
    case VisualItem::TEXT:
        item=new TextItem();
        break;
    case VisualItem::CHARACTER:
        /// @TODO: Reimplement that feature
        item=new CharacterItem();
        break;
    case VisualItem::LINE:
        item=new LineItem();
        break;
    case VisualItem::RECT:
        item=new RectItem();
        break;
    case VisualItem::ELLISPE:
        item=new EllipsItem();
        break;
    case VisualItem::PATH:
        item=new PathItem();
        break;

    }
    if(NULL!=item)
    {
        item->readItem(msg);
        QGraphicsScene::addItem(item);
        m_itemList->append(item);
    }
}