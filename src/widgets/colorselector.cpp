/***************************************************************************
 *	Copyright (C) 2007 by Romain Campioni   			   *
 *	Copyright (C) 2009 by Renaud Guezennec                             *
 *   http://renaudguezennec.homelinux.org/accueil,3.html                   *
 *                                                                         *
 *   rolisteam is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <QVBoxLayout>
#include <QMouseEvent>
#include <QColorDialog>
#include <QPainter>
#include <QDebug>

#include "widgets/colorselector.h"
#include "variablesGlobales.h"

///////////////////////////////////
// ColorSelector
///////////////////////////////////
ColorWidget::ColorWidget(QWidget* parent)
    : QWidget(parent)
{

}

void ColorWidget::setColor(QColor color)
{
    m_color = color;
}

QColor ColorWidget::getColor()
{
    return m_color;
}

void ColorWidget::mousePressEvent(QMouseEvent* event)
{
    if(event->button()==Qt::LeftButton)
    {
        emit clicked(m_color);
        event->accept();
    }
    QWidget::mousePressEvent(event);
}
void ColorWidget::paintEvent(QPaintEvent* event)
{
     QPainter painter(this);
     painter.fillRect(rect(),m_color);
}



///////////////////////////////////
// ColorSelector
///////////////////////////////////
SelectedColor G_couleurCourante;

ColorSelector::ColorSelector(QWidget *parent)
	: QWidget(parent)
{
    m_pressedButton = false;

    m_preferences =  PreferencesManager::getInstance();

    int rouge[48] = {255,255,128,0,128,0,255,255, 255,255,128,0,0,0,128,255, 128,255,0,0,0,128,128,255, 128,255,0,0,0,0,128,128, 64,128,0,0,0,0,64,64, 0,128,128,128,64,192,64,255};
    int vert[48] = {128,255,255,255,255,128,128,128, 0,255,255,255,255,128,128,0, 64,128,255,128,64,128,0,0, 0,128,128,128,0,0,0,0, 0,64,64,64,0,0,0,0, 0,128,128,128,128,192,64,255};
    int bleu[48] = {128,128,128,128,255,255,192,255, 0,0,0,64,255,192,192,255, 64,64,0,128,128,255,64,128, 0,0,0,64,255,160,128,255, 0,0,0,64,128,64,64,128, 0,0,64,128,128,192,64,255};


    // Creation du layout principale
    m_layoutSelector = new QVBoxLayout(this);
    m_layoutSelector->setMargin(2);
    m_layoutSelector->setSpacing(1);

    G_couleurCourante.type = ColorType;
    G_couleurCourante.color = QColor(rouge[40],vert[40],bleu[40]);

    m_currentColor = new QLabel(this);
    m_currentColor->setFrameStyle(QFrame::Panel | QFrame::Raised);
    m_currentColor->setLineWidth(1);
    m_currentColor->setMidLineWidth(1);

    m_currentColor->setFixedSize(45,40);
    setBackgroundColorToWidget(m_currentColor, G_couleurCourante.color);
    m_currentColor->setToolTip(tr("Predefine color 1"));
    m_currentColor->setAutoFillBackground(true);
    m_currentColor->setScaledContents(true);

    // Ajout de la couleur actuelle au layout principal
    m_layoutSelector->addWidget(m_currentColor);
    m_layoutSelector->setAlignment(m_currentColor, Qt::AlignHCenter | Qt::AlignTop);

    // Création du layout de la grille de couleurs predefinies
    m_predefinedGrid = new QGridLayout();
    m_predefinedGrid->setSpacing(1);
    m_predefinedGrid->setMargin(1);
    // Ajout de la grille de couleurs predefinies dans le layout principal
    m_layoutSelector->addLayout(m_predefinedGrid);

    // Creation des widgets pour les couleurs predefinies
    int i=0, x=0, y=0;
    for (; i<48; i++)
    {
        QColor couleur(rouge[i],vert[i],bleu[i]);

        m_predefinedColor.append(new ColorWidget(this));
        m_predefinedColor[i]->setColor(couleur);
        m_predefinedColor[i]->setAutoFillBackground(true);
        m_predefinedColor[i]->setFixedHeight(5);
        m_predefinedColor[i]->setToolTip(tr("Predefine color %1 ").arg(i+1));
        connect(m_predefinedColor[i],SIGNAL(clicked(QColor)),this, SLOT(changeCurrentColor(QColor)));

        QColorDialog::setStandardColor(x*6+y, couleur.rgb());
        m_predefinedGrid->addWidget(m_predefinedColor[i], y, x);

        x++;
        y = x>=8?y+1:y;
        x = x>=8?0:x;
    }

    // Ajout d'un separateur entre les couleurs predefinies et les couleurs personnelles
    m_separator1 = new QWidget(this);
    m_separator1->setMaximumHeight(2);
    m_layoutSelector->addWidget(m_separator1);

    // Création du layout de la grille de couleurs personnelles
    m_characterGrid = new QGridLayout();
    m_characterGrid->setSpacing(1);
    m_characterGrid->setMargin(1);
    // Ajout de la grille de couleurs personnelles dans le layout principal
    m_layoutSelector->addLayout(m_characterGrid);

    // Creation des widgets pour les couleurs personnelles
    for (i=0, x=0, y=7; i<16; i++)
    {
        // Creation d'un widget de couleur blanche
        m_personalColor.append(new ColorWidget(this));
        connect(m_personalColor[i],SIGNAL(clicked(QColor)),this, SLOT(changeCurrentColor(QColor)));
        m_personalColor[i]->setAutoFillBackground(true);
        m_personalColor[i]->setFixedHeight(5);
        m_personalColor[i]->setToolTip(tr("Custom Color %1 ").arg(i+1));

        // Mise a jour des couleurs personnelles de QColorDialog
        QColorDialog::setCustomColor(i, m_preferences->value(QString("customcolors%1").arg(i),QColor(Qt::white)).value<QColor>().rgb());

        // Ajout du widget au layout
        m_characterGrid->addWidget(m_personalColor[i], y, x);

        x++;
        y = x>=8?y+1:y;
        x = x>=8?0:x;
    }
    updatePersonalColor();

    // Ajout d'un separateur entre les couleurs personnelles et les couleurs speciales
    m_separator2 = new QWidget(this);
    m_separator2->setMaximumHeight(3);
    m_layoutSelector->addWidget(m_separator2);

    // Création du layout des couleurs speciales
    m_specialColor = new QHBoxLayout();
    m_specialColor->setSpacing(1);
    m_specialColor->setMargin(0);
    // Ajout du layout des couleurs specuales dans le layout principal
    m_layoutSelector->addLayout(m_specialColor);

    // Ajout de la couleur speciale qui efface
    m_eraseColor = new QLabel(this);
    m_eraseColor->setFrameStyle(QFrame::Box | QFrame::Raised);
    m_eraseColor->setLineWidth(0);
    m_eraseColor->setMidLineWidth(1);
    m_eraseColor->setFixedHeight(15);
    m_pixelErase = new QPixmap(":/resources/icons/erase.png");
    m_eraseColor->setPixmap(*m_pixelErase);
    setBackgroundColorToWidget(m_eraseColor,QColor(Qt::white));
    m_eraseColor->setScaledContents(true);
    m_eraseColor->setToolTip(tr("Erase"));
    m_eraseColor->setAutoFillBackground(true);
    m_specialColor->addWidget(m_eraseColor);

    // Ajout de la couleur speciale qui masque
    m_maskColor = new QLabel(this);
    m_maskColor->setFrameStyle(QFrame::Box | QFrame::Raised);
    m_maskColor->setLineWidth(0);
    m_maskColor->setMidLineWidth(1);
    m_maskColor->setFixedHeight(15);
    setBackgroundColorToWidget(m_maskColor,QColor(Qt::white));
    m_maskPixel = new QPixmap(":/resources/icons/hide.png");
    m_maskColor->setPixmap(*m_maskPixel);
    m_maskColor->setScaledContents(true);
    m_maskColor->setAutoFillBackground(true);
    m_specialColor->addWidget(m_maskColor);

    // Ajout de la couleur speciale qui demasque
    m_unveilColor = new QLabel(this);
    m_unveilColor->setFrameStyle(QFrame::Box | QFrame::Raised);
    m_unveilColor->setLineWidth(0);
    m_unveilColor->setMidLineWidth(1);
    m_unveilColor->setFixedHeight(15);
    setBackgroundColorToWidget(m_unveilColor,QColor(Qt::white));
    m_unveilPixel = new QPixmap(":/resources/icons/showMap.png");
    m_unveilColor->setPixmap(*m_unveilPixel);
    m_unveilColor->setScaledContents(true);
    m_unveilColor->setAutoFillBackground(true);
    m_specialColor->addWidget(m_unveilColor);

    // Taille de la palette
    setFixedHeight(126);

    m_maskColor->installEventFilter(this);
    m_unveilColor->installEventFilter(this);
    m_eraseColor->installEventFilter(this);
    m_currentColor->installEventFilter(this);

}
ColorSelector::~ColorSelector()
{
    delete m_currentColor;
    delete m_eraseColor;
    delete m_maskColor;
    delete m_unveilColor;
    delete m_separator1;
    delete m_separator2;
    delete m_pixelErase;
    delete m_maskPixel;
    delete m_unveilPixel;
    delete m_specialColor;
    delete m_characterGrid;
    delete m_layoutSelector;
}

void ColorSelector::checkPermissionColor()
{
	// L'utilisateur est un joueur
	if (m_preferences->value("isPlayer",false).toBool())
	{
        PreferencesManager::getInstance()->registerValue("Fog_color",QColor(0,0,0),false);
        m_maskColor->setToolTip(tr("Hide (GM only)"));
        m_unveilColor->setToolTip(tr("Unveil (GM only)"));
        m_maskColor->setEnabled(false);
        m_unveilColor->setEnabled(false);
	}
    else//GM
	{
        PreferencesManager::getInstance()->registerValue("Fog_color",QColor(50,50,50),false);
        m_maskColor->setToolTip(tr("Hide"));
        m_unveilColor->setToolTip(tr("Unveil"));
	}
}
bool ColorSelector::eventFilter(QObject* obj, QEvent* event)
{
    if(event->type() == QEvent::MouseButtonPress)
    {
        if(obj==m_maskColor)
        {
            if (m_preferences->value("isPlayer",false).toBool())
                return false;

            setBackgroundColorToWidget(m_currentColor,QColor(Qt::white));
            m_currentColor->setPixmap(*m_maskPixel);

            G_couleurCourante.type = Veil;
            m_currentColor->setToolTip(m_maskColor->toolTip());
        }
        else if(obj==m_unveilColor)
        {
            if (m_preferences->value("isPlayer",false).toBool())
                return false;

            setBackgroundColorToWidget(m_currentColor,QColor(Qt::white));
            m_currentColor->setPixmap(*m_unveilPixel);
            G_couleurCourante.type = Unveil;
            m_currentColor->setToolTip(m_unveilColor->toolTip());
        }
        else if(obj==m_eraseColor)
        {
            setBackgroundColorToWidget(m_currentColor,QColor(Qt::white));
            m_currentColor->setPixmap(*m_pixelErase);
            G_couleurCourante.type = Erase;
            m_currentColor->setToolTip(m_eraseColor->toolTip());
        }
    }
    else if(event->type() == QEvent::MouseButtonDblClick)
    {
        if(obj == m_currentColor)
        {
            QColor couleur = QColorDialog::getColor(G_couleurCourante.color);

            if (couleur.isValid())
            {
                m_currentColor->clear();
                setBackgroundColorToWidget(m_currentColor,couleur);
                G_couleurCourante.type = ColorType;
                G_couleurCourante.color = couleur;
                m_currentColor->setToolTip(tr("Red: %1, Green: %2, Blue: %3").arg(couleur.red()).arg(couleur.green()).arg(couleur.blue()));
            }
        }
    }
    return QWidget::eventFilter(obj,event);
}
void ColorSelector::changeCurrentColor(QColor color)
{
    m_currentColor->clear();
    setBackgroundColorToWidget(m_currentColor,color);
    m_currentColor->setToolTip(tr("Red: %1, Green: %2, Blue: %3").arg(color.red()).arg(color.green()).arg(color.blue()));
	G_couleurCourante.type = ColorType;
    G_couleurCourante.color = color;
}

void ColorSelector::updatePersonalColor()
{
	for (int i=0, j=0; i<16; i++)
	{
        setBackgroundColorToWidget(m_personalColor[i],QColor(QColorDialog::customColor(j)));
		j+=2;
		j = j<=15?j:1;
	}
}

QColor ColorSelector::getPersonColor(int numero)
{
	int numCouleur;

	if (numero%2)
		numCouleur = (numero-1)/2+8;
	else
		numCouleur = numero/2;

    return (m_personalColor[numCouleur]->palette()).color(QPalette::Window);
}

void ColorSelector::setBackgroundColorToWidget(QWidget* wid,QColor color)
{
    if(color.isValid())
    {
        wid->setStyleSheet(QString("background: rgb(%1,%2,%3)").arg(color.red()).arg(color.green()).arg(color.blue()));
    }
    else
    {
        wid->setStyleSheet("");
    }
}

