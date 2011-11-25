#include "linkresourcedialog.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QListView>

#include <KLocale>
#include <KIcon>
#include <KEditListBox>
#include <Nepomuk/File>
#include <QPushButton>

LinkResourceDialog::LinkResourceDialog(Nepomuk::Resource resource,QWidget* parent):KDialog(parent)
{
    setWindowTitle(i18n("Resource Link Dialog"));
    setWindowIcon(KIcon("nepomuk"));
    setButtonText(Ok,i18n("&Done"));

    QVBoxLayout* vbLayout = new QVBoxLayout(mainWidget());
    QLabel *resourceName = new QLabel(mainWidget());
    resourceName->setText(resource.genericLabel()+" : "+resource.className());
    m_resourceSearch = new KLineEdit(mainWidget());
    m_resourceSearch->setPlaceholderText(i18n("Search for resources"));
    vbLayout->addWidget(resourceName);
    vbLayout->addWidget(m_resourceSearch);
    QListView* resourceLinkView = new QListView(mainWidget());
    vbLayout->addWidget(resourceLinkView);
    QPushButton* unlinkButton = new QPushButton(resourceLinkView->viewport());
    unlinkButton->setIcon(KIcon("edit-delete"));


}

LinkResourceDialog::~LinkResourceDialog()
{

}
