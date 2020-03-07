#ifndef DLGERRORLIST_HPP
#define DLGERRORLIST_HPP

#include <QDialog>
#include <QStringList>

namespace Ui {
    class DlgErrorList;
}

//
//  DlgErrorList
//
// This class is a QDialog showing the list of files that couldn't be dropped or resized
//

class DlgErrorList : public QDialog
{
    Q_OBJECT

public:
    static void openDlgErrorList(QString message, QStringList filenames, QWidget* parent);

private:
    DlgErrorList(QString message, QStringList filenames, QWidget* parent);
    ~DlgErrorList();
    Ui::DlgErrorList* ui;
};


#endif // DLGERRORLIST_HPP
