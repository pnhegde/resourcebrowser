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

//Local includes
#include "linkresourcedialog.h"

//Qt includes
#include <QLabel>
#include <QVBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QCheckBox>
#include <QListWidgetItem>

//KDE includes
#include <KLocale>
#include <KIcon>
#include <KEditListBox>

/*//Nepomuk Includes
#include <Nepomuk/Query/Term>
#include <Nepomuk/Query/Result>
#include <Nepomuk/Query/ResourceTypeTerm>
#include <Nepomuk/Query/ComparisonTerm>
#include <Nepomuk/Query/LiteralTerm>
#include <Nepomuk/Query/QueryServiceClient>
#include <Nepomuk/Vocabulary/PIMO>
#include <Nepomuk/Vocabulary/NCO>
#include <Nepomuk/Vocabulary/NFO>
#include <Soprano/Vocabulary/NAO>
#include <Nepomuk/Vocabulary/NIE>
#include <Nepomuk/Query/QueryParser>
#include <Nepomuk/Variant>
#include <Nepomuk/Tag>
#include<Nepomuk/Utils/FacetWidget>
#include <Soprano/QueryResultIterator>
#include <Soprano/Model>
#include <Nepomuk/Query/AndTerm>
#include <Nepomuk/File>
#include <Nepomuk/Utils/SimpleResourceModel>*/


LinkResourceDialog::LinkResourceDialog(Nepomuk::Resource resource,QWidget* parent):KDialog(parent)
{
    setWindowTitle(i18n("Resource Link Dialog"));
    setWindowIcon(KIcon("nepomuk"));
    setButtonText(Ok,i18n("&Link"));
    m_mainResource = resource;
    setUpGui();
}


void LinkResourceDialog::setUpGui()
{
    QVBoxLayout* vbLayout = new QVBoxLayout(mainWidget());
    QLabel *resourceName = new QLabel(mainWidget());
    resourceName->setText(m_mainResource.genericLabel()+" : "+m_mainResource.className());
    m_resourceSearch = new KLineEdit(mainWidget());
    m_resourceSearch->setPlaceholderText(i18n("Search for resources"));
    vbLayout->addWidget(resourceName);
    vbLayout->addWidget(m_resourceSearch);
    QListWidget* resourceList = new QListWidget(mainWidget());
    vbLayout->addWidget(resourceList);
    resourceList->setViewMode(resourceList->IconMode);

    Q_FOREACH(Nepomuk::Resource resource, getLinkedResources()) {
        QListWidgetItem* item = new QListWidgetItem(resource.genericLabel(),resourceList);
        item->setCheckState(Qt::Checked);
        item->setIcon(KIcon("nepomuk"));
    }
}


QList<Nepomuk::Resource> LinkResourceDialog::getLinkedResources()
{
    return (m_mainResource.isRelatedOf());
}

LinkResourceDialog::~LinkResourceDialog()
{
}

#include "linkresourcedialog.moc"
