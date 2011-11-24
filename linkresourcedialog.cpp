#include "linkresourcedialog.h"

#include <QLabel>
#include <QVBoxLayout>

#include <KLocale>
#include <KIcon>
#include <Nepomuk/File>

LinkResourceDialog::LinkResourceDialog(Nepomuk::Resource resource,QWidget* parent):KDialog(parent)
{
    setWindowTitle(i18n("Resource Link Dialog"));
    setWindowIcon(KIcon("nepomuk"));
    setFixedSize(300,200);

    QVBoxLayout* vbLayout = new QVBoxLayout(this);
    QLabel *resourceName = new QLabel(this);
    resourceName->setText(resource.genericLabel()+" : "+resource.className());
    m_resourceSearch = new KLineEdit(this);
    m_resourceSearch->setPlaceholderText(i18n("Search for resources"));
    vbLayout->addWidget(resourceName);
    vbLayout->addWidget(m_resourceSearch);


}

LinkResourceDialog::~LinkResourceDialog()
{

}
