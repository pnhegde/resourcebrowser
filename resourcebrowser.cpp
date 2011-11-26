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
#include "resourcebrowser.h"
#include "linkresourcedialog.h"

//KDE Includes
#include <KXmlGuiWindow>
#include <KLocale>
#include <KIcon>
#include <KStatusBar>
#include <KAction>
#include <KRun>
#include <KDialog>

//Qt includes
#include <QMessageBox>
#include <QListView>
#include <QDockWidget>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDebug>
#include <QButtonGroup>
#include <QToolButton>
#include <QMenu>
#include <QLabel>

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
#include <Nepomuk/Query/AndTerm>
#include <Nepomuk/Vocabulary/NIE>
#include <Nepomuk/Query/QueryParser>
#include <Nepomuk/Variant>
#include <Nepomuk/Tag>
#include <Nepomuk/Utils/FacetWidget>

//Soprano includes
#include <Soprano/QueryResultIterator>
#include <Soprano/Model>
#include <Soprano/Vocabulary/NAO>


resourceBrowser::resourceBrowser() :
    KXmlGuiWindow()
{
    setWindowTitle(i18n("Resource Browser"));
    setWindowIcon(KIcon("nepomuk"));

    buildCentralUI();
    setupDockWidgets();
    setupActions();
    setupModels();
    populateDefaultResources();
    setupGUI();
}


resourceBrowser::~resourceBrowser()
{
}


void resourceBrowser::setupDockWidgets()
{
    QDockWidget* dock = new QDockWidget(i18n("Recommendation"),this);
    dock->setAllowedAreas(Qt::RightDockWidgetArea);
    m_recommendationView = new QListView(dock);

    dock->setWidget(m_recommendationView);
    addDockWidget(Qt::RightDockWidgetArea,dock);

    //TODO::if locked -> do this
    //dock->setFeatures(QDockWidget::NoDockWidgetFeatures);

    dock = new QDockWidget(i18n("Linked Resources"),this);
    dock->setAllowedAreas(Qt::RightDockWidgetArea);
    m_linkedResourceView = new QListView(dock);
    dock->setWidget(m_linkedResourceView);
    addDockWidget(Qt::RightDockWidgetArea,dock);

    dock = new QDockWidget("",this);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea);
    QWidget* buttonWidget = new QWidget(dock);
    QVBoxLayout* buttonLayout = new QVBoxLayout(buttonWidget);
    m_manualLinkResourceButton = new QPushButton(buttonWidget);
    m_manualLinkResourceButton->setIcon(KIcon("insert-link"));
    m_manualLinkResourceButton->setText(i18n("Link resources manually"));
    m_manualLinkResourceButton->setEnabled(false);
    m_manualLinkResourceButton->setFlat(true);
    connect(m_manualLinkResourceButton,SIGNAL(clicked()),this,SLOT(slotManualLinkResources()));
    m_removeDuplicateButton = new QPushButton(buttonWidget);
    m_removeDuplicateButton->setIcon(KIcon("archive-remove"));
    m_removeDuplicateButton->setText(i18n("Remove Duplicates"));
    m_removeDuplicateButton->setFlat(true);
    buttonLayout->addWidget(m_manualLinkResourceButton);
    buttonLayout->addWidget(m_removeDuplicateButton);
    dock->setWidget(buttonWidget);
    dock->setFeatures(QDockWidget::NoDockWidgetFeatures);
    addDockWidget(Qt::LeftDockWidgetArea,dock);

    dock = new QDockWidget(i18n("Resource Search "),this);
    Nepomuk::Utils::FacetWidget *searchWidget = new Nepomuk::Utils::FacetWidget(dock);
    searchWidget->addFacet(Nepomuk::Utils::Facet::createDateFacet(searchWidget));
    searchWidget->addFacet(Nepomuk::Utils::Facet::createTypeFacet(searchWidget));
    searchWidget->addFacet(Nepomuk::Utils::Facet::createRatingFacet(searchWidget));
    connect(searchWidget,SIGNAL(queryTermChanged(Nepomuk::Query::Term)),this,SLOT(slotFilterApplied(Nepomuk::Query::Term)));
    dock->setWidget(searchWidget);
    addDockWidget(Qt::LeftDockWidgetArea,dock);
    dock->setFeatures(QDockWidget::NoDockWidgetFeatures);
}


void resourceBrowser::buildCentralUI()
{
    m_mainWidget = new QWidget(this);
    QVBoxLayout *gLayout = new QVBoxLayout(m_mainWidget);
    m_mainWidget->setLayout(gLayout);
    m_searchBox = new KLineEdit(m_mainWidget);
    m_searchBox->setClearButtonShown(true);
    m_searchBox->setPlaceholderText(i18n("Search for resources"));
    connect(m_searchBox,SIGNAL(textChanged(QString)),this,SLOT(slotTriggerSearch(QString)));
    m_resourceView = new QListView(m_mainWidget);
    gLayout->addWidget(m_searchBox);
    m_resourceView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_resourceView,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(slotShowResourceContextMenu(QPoint&)));
    connect(m_resourceView,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(slotOpenResource(QModelIndex)));
    QHBoxLayout* searchFilterLayout = new QHBoxLayout(m_mainWidget);
    m_resourceNameButton = new QToolButton(m_mainWidget);
    m_resourceNameButton->setText(i18n("Name"));
    m_resourceNameButton->setCheckable(true);
    m_resourceContentButton = new QToolButton(m_mainWidget);
    m_resourceContentButton->setText(i18n("Content"));
    m_resourceContentButton->setCheckable(true);
    m_resourceContentButton->setChecked(true);

    m_resourceTypeButton = new QToolButton(m_mainWidget);
    m_resourceTypeButton->setCheckable(true);
    m_resourceTypeButton->setText(i18n("Type"));

    searchFilterLayout->addWidget(m_resourceNameButton);
    searchFilterLayout->addWidget(m_resourceContentButton);
    searchFilterLayout->addWidget(m_resourceTypeButton);
    searchFilterLayout->setAlignment(Qt::AlignLeft);

    gLayout->addLayout(searchFilterLayout);
    gLayout->addWidget(m_resourceView);

    setCentralWidget(m_mainWidget);
}


void resourceBrowser::setupActions()
{
//    KAction* showm_m_searchBoxAction = new KAction(this);
////    showm_m_searchBoxAction->setShortcut(Qt::CTRL + Qt::Key_F);
//    connect(show,SIGNAL(triggered()),m_searchBox,SLOT(slotTriggerSearch()));
}


void resourceBrowser::setupModels()
{
    m_resourceViewModel = new Nepomuk::Utils::SimpleResourceModel(this);
    m_resourceView->setModel(m_resourceViewModel);
    connect(m_resourceView->selectionModel(),SIGNAL(selectionChanged(QItemSelection,QItemSelection)),this,SLOT(slotLinkedResources()));
    m_recommendationViewModel = new Nepomuk::Utils::SimpleResourceModel(this);
    m_recommendationView->setModel(m_recommendationViewModel);
    connect(m_recommendationView,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(slotOpenRecommendedResource(QModelIndex)));
    m_linkedResourceViewModel = new Nepomuk::Utils::SimpleResourceModel(this);
    m_linkedResourceView->setModel(m_linkedResourceViewModel);
    connect(m_linkedResourceView,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(slotOpenLinkedResource(QModelIndex)));
}


void resourceBrowser::slotOpenResource(QModelIndex selectedResource)
{/*
    KUrl url = (m_linkedResourceViewModel->resourceForIndex(m_linkedResourceView->selectionModel()->currentIndex())).property(Nepomuk::Vocabulary::NIE::url()).toString();
    if(url.isEmpty()) {
       url= (m_recommendationViewModel->resourceForIndex(m_recommendationView->selectionModel()->currentIndex())).property(Nepomuk::Vocabulary::NIE::url()).toString();
    }
    if(url.isEmpty()) {
        url= (m_resourceViewModel->resourceForIndex(m_resourceView->selectionModel()->currentIndex())).property(Nepomuk::Vocabulary::NIE::url()).toString();
    }*/
    m_resourceView->selectionModel()->setCurrentIndex(selectedResource,QItemSelectionModel::NoUpdate);
    Nepomuk::Resource currentResource = (m_resourceViewModel->resourceForIndex(
                                             m_resourceView->selectionModel()->currentIndex()));
    KUrl url = currentResource.property(Nepomuk::Vocabulary::NIE::url()).toString();
    qDebug()<<url;
    if(!url.isEmpty()) {
        new KRun(url,this);
        currentResource.increaseUsageCount();
    }
    //krun(url);
}


void resourceBrowser::slotOpenRecommendedResource(QModelIndex selectedResource)
{
    m_recommendationView->selectionModel()->setCurrentIndex(selectedResource,QItemSelectionModel::NoUpdate);
    Nepomuk::Resource currentResource = (m_recommendationViewModel->resourceForIndex(
                                             m_recommendationView->selectionModel()->currentIndex()));
    KUrl url = currentResource.property(Nepomuk::Vocabulary::NIE::url()).toString();
    qDebug()<<url<<"usage count="<<currentResource.usageCount();
    if(!url.isEmpty()) {
        new KRun(url,this);
        currentResource.increaseUsageCount();
    }
}


void resourceBrowser::slotOpenLinkedResource(QModelIndex selectedResource)
{
    m_linkedResourceView->selectionModel()->setCurrentIndex(selectedResource,QItemSelectionModel::NoUpdate);
    Nepomuk::Resource currentResource = (m_linkedResourceViewModel->resourceForIndex(
                                              m_linkedResourceView->selectionModel()->currentIndex()));
    KUrl url = currentResource.property(Nepomuk::Vocabulary::NIE::url()).toString();
    qDebug()<<url<<"usagecount="<<currentResource.usageCount();
    if(!url.isEmpty()) {
        new KRun(url,this);
        currentResource.increaseUsageCount();
    }
}


void resourceBrowser::slotLinkedResources()
{
    m_linkedResourceViewModel->clear();
    m_recommendationViewModel->clear();
    m_manualLinkResourceButton->setEnabled(true);
    Nepomuk::Resource resource = m_resourceViewModel->resourceForIndex(m_resourceView->selectionModel()->currentIndex() );
    QList<Nepomuk::Resource> relatedResourceList = resource.isRelatedOf();
    relatedResourceList.append(resource.isRelateds());
    if(! relatedResourceList.isEmpty() ) {
        mySort(relatedResourceList);
        m_linkedResourceViewModel->setResources(relatedResourceList);
    }
    if(!resource.label().isEmpty()) {
        m_recommendationViewModel->setResources(contentResourceSearch(resource.label()));
    }
}


void resourceBrowser::slotTriggerSearch( const QString str)
{
    if(!str.isEmpty() && m_resourceContentButton->isChecked()) {
        m_resourceViewModel->setResources(contentResourceSearch(str));
    }
    else if(!str.isEmpty() && m_resourceNameButton->isChecked()) {
        m_resourceViewModel->setResources(nameResourceSearch(str));
    }
    else if(!str.isEmpty() && m_resourceTypeButton->isChecked()){
        m_resourceViewModel->setResources(typeResourceSearch(str));
    }
    else {
        populateDefaultResources();
    }
}


void resourceBrowser::slotFilterApplied(Nepomuk::Query::Term term)
{
    Nepomuk::Query::AndTerm query( m_currentQuery.term(),term );
    m_currentQuery.setTerm(query);

    m_currentQuery.setLimit( 25 );
    QList<Nepomuk::Query::Result> results = Nepomuk::Query::QueryServiceClient::syncQuery( m_currentQuery );
    QList<Nepomuk::Resource> resources;
    Q_FOREACH( const Nepomuk::Query::Result& result,results) {
        resources.append( result.resource() );
    }
    mySort(resources);
    m_resourceViewModel->setResources( resources );
}


void resourceBrowser::slotShowResourceContextMenu(const QPoint &pos)
{
    m_openResourceAction = new KAction(this);
    m_openResourceAction->setText(i18n("&Open "));
    //m_openResourceAction->setIcon(KIcon("edit-delete"));
   // connect (m_openResourceAction, SIGNAL( triggered(bool) ), this, SLOT( slotOpenResource() ) );

    QMenu myMenu;
    QPoint globalPos = m_resourceView->mapToGlobal(pos);
    myMenu.addAction(m_openResourceAction);
    myMenu.exec(globalPos);
}


void resourceBrowser::slotManualLinkResources()
{
    LinkResourceDialog manualLinkDialog(m_resourceViewModel->resourceForIndex(m_resourceView->currentIndex()));
    manualLinkDialog.exec();
}


void resourceBrowser::populateDefaultResources()
{
        m_resourceViewModel->clear();
//        Nepomuk::Query::ComparisonTerm term
//             = Nepomuk::Vocabulary::NIE::lastModified() > Nepomuk::Query::LiteralTerm( QDateTime::currentDateTime().addDays(-7) );

        Nepomuk::Query::Term term =  Nepomuk::Query::ResourceTypeTerm( Nepomuk::Vocabulary::NFO::Website() );

        m_currentQuery.setTerm(term);
        m_currentQuery = m_currentQuery || Nepomuk::Query::ResourceTypeTerm(Nepomuk::Vocabulary::PIMO::Person() );
        m_currentQuery = m_currentQuery || Nepomuk::Query::ResourceTypeTerm(Nepomuk::Vocabulary::NFO::PaginatedTextDocument() );
        m_currentQuery = m_currentQuery || Nepomuk::Query::ResourceTypeTerm(Nepomuk::Vocabulary::NFO::Presentation() );
        m_currentQuery.setLimit( 35 );
        QList<Nepomuk::Query::Result> results = Nepomuk::Query::QueryServiceClient::syncQuery( m_currentQuery );
        QList<Nepomuk::Resource> resources;
        Q_FOREACH( const Nepomuk::Query::Result& result,results) {
            resources.append( result.resource() );
        }
        mySort(resources);
        m_resourceViewModel->setResources( resources );
}

void resourceBrowser::mySort(QList<Nepomuk::Resource> &resources)
{
    for (int i=0; i<resources.size()-1; i++) {
        for (int j=0; j<resources.size()-1; j++) {
            if (resources.at(j).usageCount() < resources.at(j+1).usageCount()) {
                Nepomuk::Resource temp = resources.at(j);
                resources.replace(j,resources.at(j+1));
                resources.replace(j+1,temp);
            }
        }
    }
}

QList<Nepomuk::Resource> resourceBrowser::contentResourceSearch(const QString str)
 {
     Nepomuk::Query::ComparisonTerm linkTerm(Nepomuk::Vocabulary::NIE::plainTextContent(), Nepomuk::Query::LiteralTerm(str));

    //linkTerm.setVariableName(QLatin1String("text"));
     //Nepomuk::Query::Query query(linkTerm);
       m_currentQuery.setTerm(linkTerm);

     //query.addRequestProperty(Nepomuk::Query::Query::RequestProperty(Nepomuk::Vocabulary::NIE::lastModified()));

//     if(limit != 0) {
//         query.setLimit(limit);
//     }
     m_currentQuery.setLimit(25);
     QList<Nepomuk::Query::Result>results = Nepomuk::Query::QueryServiceClient::syncQuery( m_currentQuery );
     QList<Nepomuk::Resource> resource;
     Q_FOREACH( const Nepomuk::Query::Result& result, results ) {
         resource.append( result.resource() );
     }
     mySort(resource);
     return resource;
 }


QList<Nepomuk::Resource> resourceBrowser::nameResourceSearch(const QString str)
{
    Nepomuk::Query::ComparisonTerm linkTerm( Nepomuk::Vocabulary::NFO::fileName(), Nepomuk::Query::LiteralTerm(str));

    m_currentQuery.setTerm(linkTerm);
    m_currentQuery.setLimit(50);
    QList<Nepomuk::Query::Result>results = Nepomuk::Query::QueryServiceClient::syncQuery( m_currentQuery );
    QList<Nepomuk::Resource> resource;
    Q_FOREACH( const Nepomuk::Query::Result& result, results ) {
        resource.append( result.resource() );
    }
    mySort(resource);
    return resource;
}


QList<Nepomuk::Resource> resourceBrowser:: typeResourceSearch(const QString str)
{
    Nepomuk::Query::Term linkTerm;
    if(str.contains("music") || str.contains("songs") || str.contains("audio")) {
        qDebug()<<"Found";
        linkTerm =  Nepomuk::Query::ResourceTypeTerm( Nepomuk::Vocabulary::NFO::Audio() );
    }
    else if(str.contains("photo") || str.contains("picture") || str.contains("image")) {
        qDebug()<<"Found";
        linkTerm =  Nepomuk::Query::ResourceTypeTerm( Nepomuk::Vocabulary::NFO::Image() );
    }
    else if(str.contains("archive") || str.contains("compressed") ) {
        qDebug()<<"Found";
        linkTerm =  Nepomuk::Query::ResourceTypeTerm( Nepomuk::Vocabulary::NFO::Archive() );
    }
    else if(str.contains("pdf")) {
        qDebug()<<"Found";
        linkTerm =  Nepomuk::Query::ResourceTypeTerm( Nepomuk::Vocabulary::NFO::PaginatedTextDocument() );
    }
    else if(str.contains("ppt") || str.contains("presentation")) {
        qDebug()<<"Found";
        linkTerm =  Nepomuk::Query::ResourceTypeTerm( Nepomuk::Vocabulary::NFO::Presentation() );
    }
    else if(str.contains("text") || str.contains("txt") || str.contains("document") || str.contains("doc")  ) {
        qDebug()<<"Found";
        linkTerm =  Nepomuk::Query::ResourceTypeTerm( Nepomuk::Vocabulary::NFO::Document() );
    }

    m_currentQuery.setTerm(linkTerm);
    m_currentQuery.setLimit(25);
    QList<Nepomuk::Query::Result>results = Nepomuk::Query::QueryServiceClient::syncQuery( m_currentQuery );
    QList<Nepomuk::Resource> resource;
    Q_FOREACH( const Nepomuk::Query::Result& result, results ) {
        resource.append( result.resource() );
    }
    mySort(resource);
    return resource;
}


#include "resourcebrowser.moc"
