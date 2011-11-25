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

#ifndef RESOURCEBROWSER_H
#define RESOURCEBROWSER_H

//Qt includes
#include <QListView>
#include <QPoint>
#include <QPushButton>
#include <QToolButton>

//KDE includes
#include <KXmlGuiWindow>
#include <KAction>
#include <KLineEdit>

//Nepomuk includes
#include <Nepomuk/Utils/SimpleResourceModel>
#include <Nepomuk/Query/Term>
#include <Nepomuk/Query/Query>

class resourceBrowser : public KXmlGuiWindow
{
    Q_OBJECT
public:
    resourceBrowser();
    virtual ~resourceBrowser();

private:
    void buildCentralUI();
    void setupDockWidgets();
    void setupActions();
    void setupModels();
    void populateDefaultResources();
    QList<Nepomuk::Resource> contentResourceSearch( const QString str );
    QList<Nepomuk::Resource> nameResourceSearch( const QString str );
    QList<Nepomuk::Resource> typeResourceSearch( const QString str );

private slots:
    void slotTriggerSearch( QString );
    void slotLinkedResources();
    void slotFilterApplied( Nepomuk::Query::Term );
    void slotOpenResource( QModelIndex );
    void slotOpenRecommendedResource( QModelIndex );
    void slotOpenLinkedResource( QModelIndex );
    void slotShowResourceContextMenu( const QPoint& );
    void slotManualLinkResources();

private:

    QWidget* m_mainWidget;
    QListView* m_resourceView;
    QListView* m_recommendationView;
    QListView* m_linkedResourceView;
    QPushButton* m_manualLinkResourceButton;
    QPushButton* m_removeDuplicateButton;
    QToolButton* m_resourceNameButton;
    QToolButton* m_resourceContentButton;
    QToolButton* m_resourceTypeButton;

    KLineEdit* m_searchBox;
    KAction* m_openResourceAction;

    Nepomuk::Utils::SimpleResourceModel* m_resourceViewModel;
    Nepomuk::Utils::SimpleResourceModel* m_recommendationViewModel;
    Nepomuk::Utils::SimpleResourceModel* m_linkedResourceViewModel;
    Nepomuk::Query::Query m_currentQuery;

};

#endif // RESOURCEBROWSER_H
