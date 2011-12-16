#ifndef REMOVEDUPLICATES_H
#define REMOVEDUPLICATES_H

#include <KDialog>
#include <QListWidget>

class RemoveDuplicates: public KDialog
{
    Q_OBJECT
public:
    RemoveDuplicates();
    virtual ~RemoveDuplicates();
private:
    void setUpGui();
    void loadDuplicates();
private slots:
    void slotDeleteDuplicates();
private:
     QListWidget* m_resourceList;

};

#endif // REMOVEDUPLICATES_H
