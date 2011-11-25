/*
  ***************************************************************************
  *   Copyright (C) 2011 by Phaneendra Hegde <phaneendra.hegde@gmail.com>   *
  *                                                                         *
  *   This program is free software; you can redistribute it and/or modify  *
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
  *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
  ***************************************************************************
*/


#ifndef LINKRESOURCEDIALOG_H
#define LINKRESOURCEDIALOG_H

//KDE includes
#include <KDialog>
#include <KLineEdit>

//Nepomuk includes
#include <Nepomuk/Resource>

class LinkResourceDialog : public KDialog
{
    Q_OBJECT
public:
    LinkResourceDialog(Nepomuk::Resource resource,QWidget* parent=0);
    virtual ~LinkResourceDialog();
private:
    KLineEdit* m_resourceSearch;
};

#endif // LINKRESOURCEDIALOG_H
