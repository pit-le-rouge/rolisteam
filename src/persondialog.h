/*************************************************************************
 *     Copyright (C) 2011 by Joseph Boudou                               *
 *      Copyright (C) 2014 by Renaud Guezennec                            *
 *     http://www.rolisteam.org/                                         *
 *                                                                       *
 *   Rolisteam is free software; you can redistribute it and/or modify   *
 *   it under the terms of the GNU General Public License as published   *
 *   by the Free Software Foundation; either version 2 of the License,   *
 *   or (at your option) any later version.                              *
 *                                                                       *
 *   This program is distributed in the hope that it will be useful,     *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of      *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the       *
 *   GNU General Public License for more details.                        *
 *                                                                       *
 *   You should have received a copy of the GNU General Public License   *
 *   along with this program; if not, write to the                       *
 *   Free Software Foundation, Inc.,                                     *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.           *
 *************************************************************************/


#ifndef PERSONDIALOG_H
#define PERSONDIALOG_H

#include <QDialog>
#include <QLineEdit>

#include "widgets/colorbutton.h"
/**
 * @brief The PersonDialog class
 */
class PersonDialog
    : public QDialog
{
    Q_OBJECT

public:
    PersonDialog(QWidget * parent = NULL);
    ~PersonDialog();

    QString getName() const;
    QColor  getColor() const;

public slots :
    int edit(QString title, QString name, QColor color);

protected:
    void setVisible(bool visible);

private:
    QLineEdit   * m_name_w;
    ColorButton * m_color_w;

    void setUI();
};

#endif
