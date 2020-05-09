#ifndef uiabout_h
#define uiabout_h

#include <QDialog>
#include <QScopedPointer>

#include "catcontainer.h"
#include "i18n.h"
#include "uiconfig.h"

namespace Ui
{
    class MultiView;
}

class CatCtl;
class GConfigView;

class MultiView : public QWidget
{
    Q_OBJECT

public:
    MultiView(JsonValue config, I18n *translator, QWidget *parent = 0);
    ~MultiView();


protected:
    virtual void changeEvent(QEvent *event);

public slots:
    void displayGlobalOptions();

private slots:
    void onTrayActivated();
    void on_btnGlobalSettings_clicked();
    void on_btnLeft_clicked();

    void onAddCat();
    void onAddGroup();
    void onLoadCat();
    void onSaveCat();
    void onDeleteCat();
    void onSetupCat();
    void onTreeCurrentChanged(const QModelIndex & current, const QModelIndex & previous);
    void onTreeContextMenu(const QPoint & pos);
    void onTreeModelRowsInserted(const QModelIndex & parent, int start, int end);
    void onTreeModelReset();
    void onGlobalOptionsAccepted();
    void onCatUpdate();

    void onAutosaveTimer();

private:
    void setCats(const QList<CatGroup*> & cats);
    void setTexts();
    void saveTreeState();
    void restoreTreeState();
    void connectCat(CatView *widget);
    void connectCat(const QModelIndex & index);
    void connectCatRecursive(const QModelIndex & index);

    QScopedPointer<Ui::MultiView>   ui_;

    GConfigView        *uiGlobalOpts_;
    QSystemTrayIcon     trayIcon_;
    QTimer              autosaveTimer_;

    QMenu          *addBtnMenu_;
    QAction        *addGroup_;
    QAction        *addCat_;
    QAction        *loadCat_;
    QMenu          *treeMenu_;
    QAction        *setupCat_;
    QAction        *deleteCat_;
    QAction        *exportCat_;

    I18n           *translator_;
    JsonValue       appConfig_;
    JsonValue       groupConfig_;
    CatTreeModel    model_;
    bool            minimizeToTray_;
};

#endif
