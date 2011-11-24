#ifndef LINKRESOURCEDIALOG_H
#define LINKRESOURCEDIALOG_H

#include <KDialog>
#include <KLineEdit>

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
