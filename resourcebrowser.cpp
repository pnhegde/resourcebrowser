#include "resourcebrowser.h"
//#include "browserview.h"

//KDE Includes
#include <KXmlGuiWindow>
#include <KLocale>
#include <KIcon>
#include <KStatusBar>
#include <KAction>

#include <QListView>
#include <QDockWidget>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDebug>
#include <QButtonGroup>
#include <QToolButton>


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
#include <Nepomuk/Query/AndTerm>



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

    QHBoxLayout* searchFilterLayout = new QHBoxLayout(m_mainWidget);
    QToolButton* resourceNameButton = new QToolButton(m_mainWidget);
    resourceNameButton->setText(i18n("Name"));

    QToolButton* resourceContentButton = new QToolButton(m_mainWidget);
    resourceContentButton->setText(i18n("Content"));

    QToolButton* resourceTypeButton = new QToolButton(m_mainWidget);
    resourceTypeButton->setText(i18n("Type"));

    searchFilterLayout->addWidget(resourceNameButton);
    searchFilterLayout->addWidget(resourceContentButton);
    searchFilterLayout->addWidget(resourceTypeButton);
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
    m_linkedResourceViewModel = new Nepomuk::Utils::SimpleResourceModel(this);
    m_linkedResourceView->setModel(m_linkedResourceViewModel);
}

void resourceBrowser::populateDefaultResources()
{
        m_resourceViewModel->clear();
//        Nepomuk::Query::ComparisonTerm term
//             = Nepomuk::Vocabulary::NIE::lastModified() > Nepomuk::Query::LiteralTerm( QDateTime::currentDateTime().addDays(-7) );

        Nepomuk::Query::Term term =  Nepomuk::Query::ResourceTypeTerm( Nepomuk::Vocabulary::PIMO::Project() );

        m_currentQuery.setTerm(term);
        m_currentQuery = m_currentQuery || Nepomuk::Query::ResourceTypeTerm(Nepomuk::Vocabulary::PIMO::Person() );
        m_currentQuery.setLimit( 25 );
        QList<Nepomuk::Query::Result> results = Nepomuk::Query::QueryServiceClient::syncQuery( m_currentQuery );
        QList<Nepomuk::Resource> resources;
        Q_FOREACH( const Nepomuk::Query::Result& result,results) {
            resources.append( result.resource() );
        }
        m_resourceViewModel->setResources( resources );

}
void resourceBrowser::slotLinkedResources()
{
    m_linkedResourceViewModel->clear();
    m_recommendationViewModel->clear();
    Nepomuk::Resource resource = m_resourceViewModel->resourceForIndex(m_resourceView->selectionModel()->currentIndex() );
    QList<Nepomuk::Resource> relatedResourceList = resource.isRelatedOf();
    relatedResourceList.append(resource.isRelateds());
    if(! relatedResourceList.isEmpty() ) {
        m_linkedResourceViewModel->setResources(relatedResourceList);
    }
    if(!resource.label().isEmpty()) {
        m_recommendationViewModel->setResources(contentResourceSearch(resource.label()));
    }
}
void resourceBrowser::slotTriggerSearch( const QString str)
{
    if(!str.isEmpty()) {
           m_resourceViewModel->setResources(contentResourceSearch(str));

       } else {
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
    m_resourceViewModel->setResources( resources );

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
     return resource;

 }




#include "resourcebrowser.moc"
