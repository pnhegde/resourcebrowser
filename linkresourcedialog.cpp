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

//Nepomuk Includes
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
#include <Nepomuk/Query/OrTerm>
#include <Nepomuk/File>
#include <Nepomuk/Utils/SimpleResourceModel>


LinkResourceDialog::LinkResourceDialog(Nepomuk::Resource resource,QWidget* parent):KDialog(parent)
{
    setWindowTitle(i18n("Resource Link Dialog"));
    setWindowIcon(KIcon("nepomuk"));
    setButtonText(Ok,i18n("&Ok"));
    m_mainResource = resource;
    connect(this,SIGNAL(okClicked()),this,SLOT(slotLinkResources()));
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
    connect(m_resourceSearch,SIGNAL(textChanged(QString)),this,SLOT(slotTriggerSearch(QString)));

    m_resourceList = new QListWidget(mainWidget());
    vbLayout->addWidget(m_resourceList);
    //m_resourceList->setViewMode(m_resourceList->IconMode);

    Q_FOREACH(Nepomuk::Resource resource, getLinkedResources()) {
        QListWidgetItem* item = new QListWidgetItem(resource.genericLabel(),m_resourceList);
        item->setCheckState(Qt::Checked);
        item->setToolTip(resource.uri());
        if(!resource.genericIcon().isEmpty()) {
            item->setIcon(KIcon(resource.genericIcon()));
        }
        else {
            item->setIcon(KIcon("nepomuk"));
        }
    }
}


LinkResourceDialog::~LinkResourceDialog()
{
}


QList<Nepomuk::Resource> LinkResourceDialog::getLinkedResources()
{
    return (m_mainResource.isRelateds());
}


void LinkResourceDialog::slotTriggerSearch(QString str)
{
    m_resourceList->clear();
    QString regex = QRegExp::escape(str);
           regex.replace("\\*", QLatin1String(".*"));
           regex.replace("\\?", QLatin1String("."));
           regex.replace("\\", "\\\\");
    Nepomuk::Query::ComparisonTerm linkTerm(
                       Nepomuk::Vocabulary::NFO::fileName(),
                       Nepomuk::Query::LiteralTerm(regex),
                       Nepomuk::Query::ComparisonTerm::Regexp);
    Nepomuk::Query::ComparisonTerm term(Soprano::Vocabulary::NAO::prefLabel(),Nepomuk::Query::LiteralTerm(str));
    //Nepomuk::Query::Query test(temp);
    Nepomuk::Query::OrTerm queryTerm( linkTerm,term );
    Nepomuk::Query::Query query(queryTerm);
    query.setLimit(20);

    QList<Nepomuk::Query::Result>results = Nepomuk::Query::QueryServiceClient::syncQuery( query );
    //Nepomuk::Query::QueryServiceClient::syncQuery( test );
    QList<Nepomuk::Resource> resource;
    Q_FOREACH( const Nepomuk::Query::Result& result, results ) {
        resource.append( result.resource() );
    }

    Q_FOREACH(Nepomuk::Resource rsc, resource) {
        QListWidgetItem* item = new QListWidgetItem(rsc.genericLabel(),m_resourceList);
        item->setToolTip(rsc.uri());
        if(m_mainResource.isRelatedOf().contains(rsc)) {
            item->setCheckState(Qt::Checked);
        }
        else {
            item->setCheckState(Qt::Unchecked);
        }
        if(rsc.genericIcon().isEmpty()) {
            if(rsc.className().compare("Folder") == 0) {
                item->setIcon(KIcon("folder-blue"));
            }
            else if(rsc.className().compare("Photo") == 0) {
                item->setIcon(KIcon("image-x-generic"));
                rsc.addSymbol("image-x-generic");
            }
            else if(rsc.className().compare("Document") == 0) {
                item->setIcon(KIcon("libreoffice34-oasis-master-document"));
                rsc.addSymbol("libreoffice34-oasis-master-document");
            }
            else if(rsc.className().compare("MusicPiece") == 0) {
                item->setIcon(KIcon("audio-ac3"));
                rsc.addSymbol("audio-ac3");
            }
            else if(rsc.className().compare("InformationElement") == 0) {
                item->setIcon(KIcon("video-x-generic"));
                rsc.addSymbol("video-x-generic");
            }
            else if(rsc.className().compare("TextDocument") == 0) {
                item->setIcon(KIcon("text-x-generic"));
                rsc.addSymbol("text-x-generic");
            }
            else if(rsc.className().compare("PaginatedTextDocument") == 0) {
                item->setIcon(KIcon("application-pdf"));
                rsc.addSymbol("application-pdf");
            }
            else if(rsc.className().compare("Archive") == 0) {
                item->setIcon(KIcon("application-x-archive"));
                rsc.addSymbol("application-x-archive");
            }
            else if(rsc.className().compare("Person") == 0) {
                item->setIcon(KIcon("x-office-contact"));
                rsc.addSymbol("x-office-contact");
            }
            else {
                item->setIcon(KIcon("nepomuk"));
            }
        }
        else {
            item->setIcon(KIcon(rsc.genericIcon()));
        }
     }
}

void LinkResourceDialog::slotLinkResources()
{
    for(int i=0;i<m_resourceList->count();i++) {
        QListWidgetItem *item = m_resourceList->item(i);
        if(item->checkState() == Qt::Checked) {
            QString resourceUri = item->toolTip();
            Nepomuk::Resource resource = (QUrl)resourceUri;
            m_mainResource.addIsRelated( resource );
        }
    }
}

#include "linkresourcedialog.moc"
