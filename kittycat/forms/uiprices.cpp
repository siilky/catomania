#include "stdafx.h"

#include "ui_prices.h"
#include "uiprices.h"
#include "version.h"
#include "util.h"
#include "PWCatsApi.h"


// class IntegerScaleDraw : public QwtScaleDraw
// {
// public:
//     virtual QwtText label(double value) const
//     {
//         if (qFuzzyCompare( value + 1.0, 1.0 ))
//             value = 0.0; 
// 
//         return spacedNumber(int(value));
//     }
// };

static void minMaxValues(const QList<QPointF> & list, qreal & minX, qreal & maxX, qreal & minY, qreal & maxY)
{
    if (list.empty())
    {
        return;
    }

    minX = maxX = list[0].x();
    minY = maxY = list[0].y();

    for (int i = 1; i < list.size(); ++i)
    {
        minX = std::min(minX, list[i].x());
        minY = std::min(minY, list[i].y());
        maxX = std::max(maxX, list[i].x());
        maxY = std::max(maxY, list[i].y());
    }
}

//

PricesView::PricesView(unsigned itemId, const QString & itemName, int serverId, QWidget *parent /*= 0*/)
    : QDialog(parent, Qt::Tool)
    , ui_(new Ui::Prices)
    , itemId_(itemId)
    , serverId_(serverId)
    , api_(new PWCatsApi(this))
{
    ui_->setupUi(this);
    setWindowTitle(tr("Prices: %1").arg(itemName));

    QFont f = font();
    f.setPixelSize(8);

    //ui_->chart->setBackgroundBrush(QBrush(Qt::transparent));
    ui_->chart->setRenderHint(QPainter::Antialiasing);
    auto chart = ui_->chart->chart();
    chart->legend()->hide();
    chart->setMargins(QMargins(0, 0, 0, 0));
    chart->setBackgroundVisible(false);
    chart->setPlotAreaBackgroundVisible(false);

    auto axisX = new QValueAxis(chart);
    axisX->setLabelsVisible(false);
    axisX->setMinorTickCount(0);
    axisX->setTickCount(0);
    axisX->setTitleVisible(false);
    axisX->setGridLineVisible(false);
    axisX->setLineVisible(false);
    chart->addAxis(axisX, Qt::AlignBottom);

    auto axisY = new QValueAxis(chart);
    QString s = axisY->labelFormat();
    axisY->setLabelFormat("%.0f");
    axisY->setLabelsFont(f);
    axisY->setMinorTickCount(1);
    axisY->setTickCount(7);
    axisY->setTitleVisible(false);
    axisY->setLinePen(QPen(Qt::black, .5));
    chart->addAxis(axisY, Qt::AlignLeft);

    ui_->tableWidget->setColumnWidth(TableName, 90);
    ui_->tableWidget->setColumnWidth(TableSellColumn, 80);
    ui_->tableWidget->setColumnWidth(TableCountColumn, 45);
    ui_->tableWidget->setColumnWidth(TableBuyColumn, 80);
}

void PricesView::showEvent(QShowEvent * event)
{
    if (event->spontaneous())
    {
        QDialog::showEvent(event);
        return;
    }

    if (serverId_ < 0)
    {
        ui_->lbStatus->setText(tr("Unfortunately not supported for this server."));
        return;
    }

    request_ = new PWCatsPriceHistoryRequest(serverId_, itemId_);
    connect(request_, &PWCatsRequest::finished, this, &PricesView::onRequestFinished);

    api_->get(request_);

    ui_->stackedWidget->setCurrentIndex(PageLoading);
    ui_->lbStatus->setText(tr("Loading data ..."));
    
    QDialog::showEvent(event);
}

void PricesView::onRequestFinished()
{
    request_->deleteLater();

    if (!request_->isOk())
    {
        if (request_->networkError() != QNetworkReply::NoError)
        {
            ui_->lbStatus->setText(tr("Failed to load data.<br>Network error %1<br>%2")
                                   .arg(request_->networkError())
                                   .arg(request_->errorString()));
        }
        else
        {
            ui_->lbStatus->setText(tr("Failed to load data.<br>Error: %1").arg(request_->errorString()));
        }

        return;
    }

    // display

    QColor sellColor("blue");
    QColor buyColor("darkred");
    QColor altColor("green");

    int rows = request_->sellList.size() + request_->buyList.size()
        + request_->comSellList.size() + request_->comBuyList.size();
    ui_->tableWidget->setRowCount(rows);

    int row = 0;

    // buying
    auto ib = request_->buyList.begin();
    auto icb = request_->comBuyList.begin();
    while (ib != request_->buyList.end()
           || icb != request_->comBuyList.end())
    {
        // get nearest
        bool alternateColor = false;
        PWCatsPriceHistoryRequest::PriceElement pe;

        if (ib == request_->buyList.end())
        {
            pe = *icb;
            ++icb;
            alternateColor = true;
        }
        else if (icb == request_->comBuyList.end())
        {
            pe = *ib;
            ++ib;
        }
        else if (ib->price < icb->price)
        {
            pe = *ib;
            ++ib;
        }
        else
        {
            pe = *icb;
            ++icb;
            alternateColor = true;
        }

        QTableWidgetItem *nameItem = new QTableWidgetItem(pe.name);
        if (alternateColor)
        {
            nameItem->setTextColor(altColor);
        }
        nameItem->setTextAlignment(Qt::AlignLeft);

        QTableWidgetItem *priceItem = new QTableWidgetItem(spacedNumber(pe.price));
        priceItem->setTextColor(buyColor);
        priceItem->setTextAlignment(Qt::AlignRight);

        QTableWidgetItem *countItem = new QTableWidgetItem(spacedNumber(pe.count));
        if (alternateColor)
        {
            countItem->setTextColor(altColor);
        }
        countItem->setTextAlignment(Qt::AlignRight);

        ui_->tableWidget->setItem(row, TableName, nameItem);
        ui_->tableWidget->setItem(row, TableCountColumn, countItem);
        ui_->tableWidget->setItem(row, TableBuyColumn, priceItem);
        row++;
    }
    // Selling
    auto is = request_->sellList.begin();
    auto ics = request_->comSellList.begin();
    while (is != request_->sellList.end()
           || ics != request_->comSellList.end())
    {
        // get nearest
        bool alternateColor = false;
        PWCatsPriceHistoryRequest::PriceElement pe;

        if (is == request_->sellList.end())
        {
            pe = *ics;
            ++ics;
            alternateColor = true;
        }
        else if (ics == request_->comSellList.end())
        {
            pe = *is;
            ++is;
        }
        else if (is->price < ics->price)
        {
            pe = *is;
            ++is;
        }
        else
        {
            pe = *ics;
            ++ics;
            alternateColor = true;
        }

        QTableWidgetItem *nameItem = new QTableWidgetItem(pe.name);
        if (alternateColor)
        {
            nameItem->setTextColor(altColor);
        }
        nameItem->setTextAlignment(Qt::AlignLeft);

        QTableWidgetItem *priceItem = new QTableWidgetItem(spacedNumber(pe.price));
        priceItem->setTextColor(sellColor);
        priceItem->setTextAlignment(Qt::AlignRight);

        QTableWidgetItem *countItem = new QTableWidgetItem(spacedNumber(pe.count));
        if (alternateColor)
        {
            countItem->setTextColor(altColor);
        }
        countItem->setTextAlignment(Qt::AlignRight);

        ui_->tableWidget->setItem(row, TableName, nameItem);
        ui_->tableWidget->setItem(row, TableCountColumn, countItem);
        ui_->tableWidget->setItem(row, TableSellColumn, priceItem);
        row++;
    }
//     ui_->tableWidget->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
//     ui_->tableWidget->resizeColumnsToContents();
    ui_->tableWidget->setFixedWidth(ui_->tableWidget->horizontalHeader()->length() + 25);

    // graph


    QList<QPointF> sellPoints;
    QList<QPointF> buyPoints;

    for (size_t i = 0; i < request_->history.size(); i++)
    {
//         QDateTime date = QDateTime::fromTime_t(catSellHistory[i].time);
//         qreal x = QwtDate::toDouble(date); //qwt needs a double to work
        auto sv = request_->history[i].sell;
        if (sv != 0)
        {
            sellPoints.append(QPointF(request_->history[i].time/*x*/, sv));
        }

        auto bv = request_->history[i].buy;
        if (bv != 0)
        {
            buyPoints.append(QPointF(request_->history[i].time/*x*/, bv));
        }
    }

    qreal minX, minY, maxX, maxY;
    if (!sellPoints.isEmpty())
    {
        minMaxValues(sellPoints, minX, maxX, minY, maxY);

        qreal minX1, minY1, maxX1, maxY1;
        if (!buyPoints.isEmpty())
        {
            minMaxValues(buyPoints, minX1, maxX1, minY1, maxY1);
            minX = std::min(minX, minX1);
            minY = std::min(minY, minY1);
            maxX = std::max(maxX, maxX1);
            maxY = std::max(maxY, maxY1);
        }
    }
    else if (!buyPoints.isEmpty())
    {
        minMaxValues(buyPoints, minX, maxX, minY, maxY);
    }
    else
    {
        minX = minY = maxX = maxY = 0;
    }

    // applyNiceNumbers is buggy when min = max so adding difference of 100
    auto xDiff = std::max(100., maxX - minX);
    minX -= xDiff * 0.05;
    maxX += xDiff * 0.05;
    auto yDiff = std::max(100., maxY - minY);
    minY -= yDiff * 0.05;
    maxY += yDiff * 0.05;
    minY = std::max(0., minY);
//     auto roundExp = std::max(0., std::floor(log10(yDiff)) - 1);
//     minY = std::max(0., std::floor(minY / std::pow(10, roundExp)) * std::pow(10, roundExp));
//     maxY = std::ceil(maxY / std::pow(10, roundExp)) * std::pow(10, roundExp);

    auto chart = ui_->chart->chart();

    auto axisX = chart->axisX();
    auto axisY = chart->axisY();

    QLineSeries *buySeries = new QLineSeries(chart);
    buySeries->append(buyPoints);
    buySeries->setPointsVisible(true);
    buySeries->setColor(buyColor);
    buySeries->setPen(QPen(buyColor, 2.0));
    chart->addSeries(buySeries);
    buySeries->attachAxis(axisX);
    buySeries->attachAxis(axisY);

    QLineSeries *sellSeries = new QLineSeries(chart);
    sellSeries->append(sellPoints);
    sellSeries->setPointsVisible(true);
    sellSeries->setColor(sellColor);
    sellSeries->setPen(QPen(sellColor, 2.0));
    chart->addSeries(sellSeries);
    // attach should go after series are added to chart
    sellSeries->attachAxis(axisX);
    sellSeries->attachAxis(axisY);

//chart->createDefaultAxes();

    axisX->setRange(minX, maxX);
    axisY->setRange(minY, maxY);

    qobject_cast<QValueAxis*>(axisY)->applyNiceNumbers();

    ui_->stackedWidget->setCurrentIndex(PageData);
}

void PricesView::onError()
{
    request_->deleteLater();
}
