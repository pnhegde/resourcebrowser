#include "removeduplicates.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QUrl>
#include <QDebug>
#include <QListWidgetItem>
#include <QMessageBox>

#include <KLocale>
#include <KIcon>
#include <KUrl>

#include <Nepomuk/Vocabulary/NFO>
#include <Nepomuk/Vocabulary/NIE>
#include <Nepomuk/Vocabulary/NEXIF>
#include <Nepomuk/Resource>
#include <Nepomuk/ResourceManager>
#include <Nepomuk/File>

#include <Soprano/QueryResultIterator>
#include <Soprano/Query/QueryLanguage>
#include <Soprano/Model>

RemoveDuplicates::RemoveDuplicates():KDialog()
{
    setWindowTitle(i18n("Remove Duplicates"));
    setWindowIcon(KIcon("nepomuk"));
    setButtons(User1 | Cancel);
    setButtonIcon(User1,KIcon("edit-delete"));
    setButtonText(User1,i18n("&Delete"));
    setMinimumSize(500,500);
    connect(this,SIGNAL(user1Clicked()),this,SLOT(slotDeleteDuplicates()));
    setUpGui();
    loadDuplicates();
}

RemoveDuplicates::~RemoveDuplicates()
{

}

void RemoveDuplicates::setUpGui()
{
    QVBoxLayout* vbLayout = new QVBoxLayout(mainWidget());
    m_resourceList = new QListWidget(mainWidget());
    vbLayout->addWidget(m_resourceList);
}

void RemoveDuplicates::loadDuplicates()
{
    Soprano::Model* model = Nepomuk::ResourceManager::instance()->mainModel();
    QString query
       = QString( "select distinct ?u1 where { "
                 "?r1 a %1 . ?r2 a %1. ?r1 %2 ?h. ?r2 %2 ?h. "
                 "?r1 %3 ?u1. ?r2 %3 ?u2. filter(?r1!=?r2) . }order by ?h limit 50")
         .arg( Soprano::Node::resourceToN3(Nepomuk::Vocabulary::NFO::FileDataObject()))
         .arg( Soprano::Node::resourceToN3(Nepomuk::Vocabulary::NFO::hasHash()))
         .arg( Soprano::Node::resourceToN3(Nepomuk::Vocabulary::NIE::url()));

    Soprano::QueryResultIterator it
       = model->executeQuery( query,
                              Soprano::Query::QueryLanguageSparql );
    Nepomuk::File tempRsc;
    while( it.next() ) {
        tempRsc = it.binding("u1").uri() ;
        QString usagecount = QString::number(tempRsc.usageCount());
        QListWidgetItem* item = new QListWidgetItem(tempRsc.genericLabel() + ":: Usage Count:" + usagecount,m_resourceList);
        item->setCheckState(Qt::Unchecked);
        item->setToolTip(tempRsc.url().path());
        qDebug()<<tempRsc.url().path();
    }

}

void RemoveDuplicates::slotDeleteDuplicates()
{
    QMessageBox msgBox;
    msgBox.setText("Alert");
    msgBox.setInformativeText("Do you really want to delete these duplicates?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No );
    msgBox.setDefaultButton(QMessageBox::No);
    msgBox.setIcon(QMessageBox::Warning);
    int ret = msgBox.exec();
    if(ret == QMessageBox::Yes) {
        for(int i=0;i<m_resourceList->count();i++) {
            QListWidgetItem *item = m_resourceList->item(i);
            if(item->checkState() == Qt::Checked) {
                Nepomuk::File resource(KUrl(item->toolTip()));
                QString fileTobeDeleted = "rm "+resource.url().path();
                system(fileTobeDeleted.toLocal8Bit().data());
            }
        }
        loadDuplicates();
        close();
    }
}
