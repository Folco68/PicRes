#include "TableItem.hpp"


//
//  TableItem
//
// Create a QTableWidgetItem with a centered content
//

TableItem::TableItem(QString text):
    QTableWidgetItem (text, Qt::DisplayRole)
{
    // Center the item in the cell
    setTextAlignment(Qt::AlignCenter);
}
