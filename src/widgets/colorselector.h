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

#ifndef SELECTEUR_COULEUR_H
#define SELECTEUR_COULEUR_H

#include <QWidget>
#include <QFrame>
#include <QColor>
#include <QLabel>


#include "preferences/preferencesmanager.h"
#include "types.h"

class QHBoxLayout;
class QGridLayout;
class QVBoxLayout;
/**
 * @brief The ColorWidget class is small widget to display color and select it. It is used a lot of timer
 * by ColorSelector.
 */
class ColorWidget : public QWidget
{
    Q_OBJECT
public:
    /**
     * @brief ColorWidget
     * @param parent
     */
    ColorWidget(QWidget* parent = NULL);
    /**
     * @brief setColor
     */
    void setColor(QColor);
    /**
     * @brief getColor
     * @return
     */
    QColor getColor();


signals:
    void clicked(QColor);

protected:
    /**
     * @brief mousePressEvent
     * @param event
     */
    void mousePressEvent(QMouseEvent* event);
    /**
     * @brief paintEvent
     * @param event
     */
    void paintEvent(QPaintEvent* event);


private:
    QColor m_color;
};

/**
 * @brief The ColorSelector class is displayed in Toolsbar for color selection.
 */
class ColorSelector : public QWidget
{
    Q_OBJECT
public:
    /**
     * @brief ColorSelector
     * @param parent
     */
    ColorSelector(QWidget *parent = 0);
    /**
      *
      */
    ~ColorSelector();
    /**
     * @brief majCouleursPersonnelles
     */
    void updatePersonalColor();
    /**
     * @brief autoriserOuInterdireCouleurs
     */
    void checkPermissionColor();
    /**
     * @brief donnerCouleurPersonnelle
     * @param numero
     * @return
     */
    QColor getPersonColor(int numero);
    /**
     * @brief setBackgroundColorToWidget
     * @param wid
     * @param color
     */
    void setBackgroundColorToWidget(QWidget* wid,QColor color);
    /**
     * @brief eventFilter
     * @return
     */
    virtual bool eventFilter(QObject *, QEvent *);
public slots:
    /**
     * @brief changeCurrentColor
     * @param couleur
     */
    void changeCurrentColor(QColor couleur);
private:
    QLabel* m_currentColor;
    QLabel* m_eraseColor;
    QLabel* m_maskColor;
    QLabel* m_unveilColor;
    QList<ColorWidget*> m_predefinedColor;
    QList<ColorWidget*> m_personalColor;
    QWidget *m_separator1;
    QWidget *m_separator2;
    QPixmap *m_pixelErase;
    QPixmap *m_maskPixel;
    QPixmap *m_unveilPixel;
    QHBoxLayout *m_specialColor;
    QGridLayout *m_characterGrid;
    QVBoxLayout *m_layoutSelector;
    QGridLayout *m_predefinedGrid;
    bool m_pressedButton;
    PreferencesManager* m_preferences;

};

#endif
