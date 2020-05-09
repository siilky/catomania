#ifndef uilicense_h
#define uilicense_h

#include <QDialog>
#include <QNetworkReply>
#include <QPointer>

namespace Ui
{
    class Prices;
}
class PWCatsApi;
class PWCatsPriceHistoryRequest;

class PricesView
    : public QDialog
{
    Q_OBJECT
public:
    PricesView(unsigned itemId, const QString & itemName, int serverId, QWidget *parent = 0);

protected:
    virtual void showEvent(QShowEvent * event);

private slots:
    void onRequestFinished();
    void onError();

private:
    enum
    {
        PageLoading = 0,
        PageData,
    };

    enum
    {
        TableName = 0,
        TableSellColumn,
        TableCountColumn,
        TableBuyColumn,
    };

    QScopedPointer<Ui::Prices> ui_;

    unsigned    itemId_;
    int         serverId_;

    PWCatsApi   *api_;
    QPointer<PWCatsPriceHistoryRequest> request_;
};

#endif
