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
        // Ctor/dtor. It receives the list of filenames to display
        DlgErrorList(QString message, QStringList filenames, QWidget *parent = nullptr);
        ~DlgErrorList();

    private:
        // UI
        Ui::DlgErrorList *ui;
};


#endif // DLGERRORLIST_HPP
