#ifndef TABLEITEM_HPP
#define TABLEITEM_HPP


#include <QString>
#include <QTableWidgetItem>


//
//  TableItem
//
// This class is a QTableWidgetItem which centers its content
//

class TableItem:
    public QTableWidgetItem
{
    public:
        explicit TableItem(QString text);
};


#endif // TABLEITEM_HPP
