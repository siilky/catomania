#ifndef UIHISTORY_H
#define UIHISTORY_H

#include <QDialog>

class HistoryDb;
class CatCtl;

namespace Ui {class History;}

class HistoryView : public QDialog
{
	Q_OBJECT

public:
	HistoryView(QSharedPointer<CatCtl> ctl, QWidget * parent = 0);
	~HistoryView();

private slots:
    void on_btnOpIn_toggled(bool checked);
    void on_btnOpOut_toggled(bool checked);
    void updateEvents();
    void on_btnReset_clicked();

private:
    void fillItems();
    void fillEvents(quint32 itemId, bool showin, bool showOut);

    enum ItemsColumns
    {
        Name = 0,
        Stock,
        StockValue,
        Profit,

        NumItemsColumns
    };
    const static int itemsWidths[ItemsColumns::NumItemsColumns];

    enum EventsColumns
    {
        Operation = 0,
        Date,
        Count,
        Price,

        NumEventsColumns
    };
    const static int eventsWidths[EventsColumns::NumEventsColumns];

	Ui::History *ui;

    QSharedPointer<CatCtl> ctl_;
};

#endif