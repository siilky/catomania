
#include "stdafx.h"

#include "persistence2.h"
#include "ui_multiview.h"
#include "uimultiview.h"
#include "util.h"
#include "uigconfig.h"

extern QString g_elementExePath_;   // fixme: should not be global

class ByteConversion
{
public:
    JsonValue convert(const char v) const
    {
        return JsonValue((int)v, nullptr);
    }

    byte convertIn(const JsonValue & v) const
    {
        return (char)static_cast<int>(v);
    }
};

MultiView::MultiView( JsonValue config
                    , I18n *translator 
                    , QWidget *parent /*= 0*/)
    : QWidget(parent, Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowMinMaxButtonsHint)
    , ui_(new Ui::MultiView)
    , appConfig_(config.get(L"Application"))
    , groupConfig_(config.get(L"Groups"))
    , translator_(translator)
{
    uiGlobalOpts_ = new GConfigView(this);
    connect(uiGlobalOpts_, &QDialog::accepted, this, &MultiView::onGlobalOptionsAccepted);

    ui_->setupUi(this);
    QWidget *placeholder = new QWidget();
    ui_->splitter->addWidget(placeholder);
    ui_->splitter->setStretchFactor(0, 0);
    ui_->splitter->setStretchFactor(1, 1);

    trayIcon_.setIcon(QIcon(":/catView/c48.png"));
    trayIcon_.setToolTip("Cats");
    connect(&trayIcon_,   &QSystemTrayIcon::activated,  this, &MultiView::onTrayActivated);

    connect(ui_->btnAddCat, &QToolButton::clicked,      this, &MultiView::onAddCat);

    addCat_    = new QAction(QIcon(":/catView/cat.png"), "", this);
    addGroup_  = new QAction(QIcon(":/multiView/reseller_programm.png"), "", this);
    loadCat_   = new QAction(QIcon(":/multiView/folder.png"), "", this);
    deleteCat_ = new QAction(QIcon(":/multiView/cross.png"), "", this);
    exportCat_ = new QAction(QIcon(":/multiView/disk.png"), "", this);
    setupCat_  = new QAction(QIcon(":/catView/widgets16.png"), "", this);
    connect(addCat_,    &QAction::triggered, this, &MultiView::onAddCat);
    connect(addGroup_,  &QAction::triggered, this, &MultiView::onAddGroup);
    connect(loadCat_,   &QAction::triggered, this, &MultiView::onLoadCat);
    connect(deleteCat_, &QAction::triggered, this, &MultiView::onDeleteCat);
    connect(exportCat_, &QAction::triggered, this, &MultiView::onSaveCat);
    connect(setupCat_,  &QAction::triggered, this, &MultiView::onSetupCat);

    addBtnMenu_ = new QMenu(this);
    addBtnMenu_->addAction(addCat_);
    addBtnMenu_->addAction(addGroup_);
    addBtnMenu_->addAction(loadCat_);
    addBtnMenu_->addAction(deleteCat_);
    ui_->btnAddCat->setMenu(addBtnMenu_);

    treeMenu_ = new QMenu(this);
    treeMenu_->addAction(setupCat_);
    treeMenu_->addAction(exportCat_);
    treeMenu_->addAction(deleteCat_);
    setTexts();

    connect(&model_, &QAbstractItemModel::rowsInserted, this, &MultiView::onTreeModelRowsInserted);
    connect(&model_, &QAbstractItemModel::modelReset,   this, &MultiView::onTreeModelReset);

    ui_->treeView->setModel(&model_);
    connect(ui_->treeView->selectionModel(), &QItemSelectionModel::currentChanged, this, &MultiView::onTreeCurrentChanged);
    connect(ui_->treeView, &QWidget::customContextMenuRequested, this, &MultiView::onTreeContextMenu);

    connect(&autosaveTimer_,  &QTimer::timeout, this, &MultiView::onAutosaveTimer);
    autosaveTimer_.start(1 * 60 * 1000);

    appConfig_.get(L"MinimizeToTray", minimizeToTray_);


    // UNDO remove in next release (one after one after 31.03)
    std::vector<int> position;
    appConfig_.get(L"Position", position, json::NoConversion<int>());
    if (position.size() >= 4)
    {
        QRect scr = QApplication::desktop()->geometry();
        QPoint orgn(position[0], position[1]);
        if (scr.contains(orgn))
        {
            move(orgn);
        }
        resize(position[2], position[3]);

        appConfig_.get_obj().erase(L"Position");
    }
    else
    {
        std::vector<char> geometry;
        appConfig_.get(L"Geometry", geometry, ByteConversion());
        restoreGeometry(qUncompress(QByteArray(geometry.data(), geometry.size())));
    }

    std::vector<CatGroup*> cats;
    groupConfig_.get(L"Cats", cats, json::DirectPtrConversion<CatGroup>());

    if (cats.empty())
    {
        // add first group/cat
        cats.push_back(new CatGroup());
        cats[0]->insertChildren(0, 1);
    }
    setCats(QList<CatGroup*>::fromVector(QVector<CatGroup*>::fromStdVector(cats)));
}

MultiView::~MultiView()
{
    saveTreeState();

    appConfig_.set(L"Geometry", toVector(qCompress(saveGeometry(), 9)), ByteConversion());
    std::vector<int> s = ui_->splitter->sizes().toVector().toStdVector();
    appConfig_.set(L"Splitter", s, json::NoConversion<int>());

    // release last cat widget from splitter
    while (ui_->splitter->count() > 1)
    {
        ui_->splitter->widget(1)->setParent(NULL);
    }

    groupConfig_.set(L"Cats", model_.cats().toVector().toStdVector(), json::DirectPtrConversion<CatGroup>());
}

void MultiView::setCats(const QList<CatGroup*> & cats)
{
    model_.setCats(cats);
    restoreTreeState();

    if (model_.rowCount() > 0)
    {
        QModelIndex idx = model_.index(0, 0, model_.index(0, 0));   // get first child item
        ui_->treeView->selectionModel()->setCurrentIndex(idx, QItemSelectionModel::SelectCurrent);
    }

    // UNDO remove in next release (31.03)
    std::vector<int> position;
    appConfig_.get(L"Position", position, json::NoConversion<int>());
    if (position.size() >= 6)
    {
        QList<int> sizes;
        sizes.append(QVector<int>::fromStdVector(std::vector<int>(position.begin() + 4, position.end())).toList());
        ui_->splitter->setSizes(sizes);
    }
    else
    {
        std::vector<int> s;
        appConfig_.get(L"Splitter", s, json::NoConversion<int>());
        if (!s.empty())
        {
            ui_->splitter->setSizes(QList<int>::fromVector(QVector<int>::fromStdVector(s)));
        }
        else
        {
            // tree is shown by default
            ui_->splitter->setSizes(QList<int>() << 110 << 110 + width());
        }
    }
}

//

void MultiView::changeEvent(QEvent *event)
{
    QEvent::Type type = event->type();
    switch (type)
    {
        case QEvent::LanguageChange:
            ui_->retranslateUi(this);
            setTexts();
            break;

        case QEvent::WindowStateChange:
        {
            if (isMinimized() && minimizeToTray_)
            {
                QTimer::singleShot(0, this, SLOT(hide()));
                QTimer::singleShot(0, &trayIcon_, SLOT(show()));
            }
            else if (trayIcon_.isVisible())
            {
                // kind of misplaced state
                showNormal();
                trayIcon_.hide();
            }
            break;
        }
    }

    QWidget::changeEvent(event);
}

void MultiView::displayGlobalOptions()
{
    GlobalOptions opts;
    opts.minimizeToTray = minimizeToTray_;
    opts.languages = translator_->getLanguages(&opts.currentLanguage);

    uiGlobalOpts_->setOptions(opts);
    uiGlobalOpts_->show();
}

void MultiView::onTrayActivated()
{
    show();         // makes restore animation running
    showNormal();
    activateWindow();
    trayIcon_.hide();
}

void MultiView::onAddGroup()
{
    QModelIndex selection = ui_->treeView->selectionModel()->currentIndex();

    int row = -1; // note +1 below
    QModelIndex group = selection;
    if (selection.isValid())
    {
        // walk up on tree to root level
        do {
            row = group.row();
            group = group.parent();
        }
        while (group.isValid());
    }
    // else no selection -add to root

    // add group
    model_.insertRow(row + 1, group);
    group = model_.index(row + 1, 0, group);
    // add cat
    model_.insertRow(0, group);

    QModelIndex cat = model_.index(0, 0, group);
    ui_->treeView->selectionModel()->setCurrentIndex( cat
                                                    , QItemSelectionModel::ClearAndSelect);

    // TBD add subgroups
}

void MultiView::onLoadCat()
{
    QScopedPointer<QFileDialog> fd(new QFileDialog( NULL
                                                  , tr("Load cat(s) from configuration file")
                                                  , QString()
                                                  , tr("Cat configuration (*.jccfg);;Previous version configuration (*.cfg)")));
    fd->setFileMode(QFileDialog::ExistingFiles);
    if (fd->exec())
    {
        QStringList files = fd->selectedFiles();
        if (files.empty())
        {
            return;
        }

        QModelIndex selection = ui_->treeView->selectionModel()->currentIndex();
        int row = -1; // note +1 below
        QModelIndex group = selection;
        if (selection.isValid())
        {
            // walk up on tree to group level
            while (group.parent().isValid())
            {
                row = group.row();
                group = group.parent();
            }
            row++;
        }
        else
        {
            // load to last group
            group = model_.index(model_.rowCount(), 0);
            row = model_.rowCount(group);
        }

        for (int i = 0; i < files.count(); i++)
        {
            model_.loadItem(files[i], row, group);
        }
    }
}

void MultiView::onAddCat()
{
    QModelIndex selection = ui_->treeView->selectionModel()->currentIndex();

    int row = -1; // note +1 below
    QModelIndex group = selection;
    if (selection.isValid())
    {
        // walk up on tree to group level
        while (group.parent().isValid())
        {
            row = group.row();
            group = group.parent();
        }

        model_.insertRow(row + 1, group);
        QModelIndex cat = model_.index(row + 1, 0, group);

        ui_->treeView->selectionModel()->setCurrentIndex(cat, QItemSelectionModel::ClearAndSelect);
    }
    else
    {
        onAddGroup();
    }
}

void MultiView::onSaveCat()
{
    QModelIndex index = ui_->treeView->selectionModel()->currentIndex();
    if ( ! index.isValid())
    {
        return;
    }

    QScopedPointer<QFileDialog> fd(new QFileDialog( NULL
                                                  , tr("Save cat configuration to file")
                                                  , QString()
                                                  , tr("Cat configuration (*.jccfg)")));
    fd->setAcceptMode(QFileDialog::AcceptSave);
    if (fd->exec())
    {
        QStringList files = fd->selectedFiles();
        if ( ! files.isEmpty()
            && ! files[0].isEmpty())
        {
            QString filename = files[0];
            // add an extension if its missing
            if ( ! filename.contains('.'))
            {
                filename.append(".jccfg");
            }
            model_.saveItem(filename, index);
        }
    }
}

void MultiView::onDeleteCat()
{
    QModelIndex index = ui_->treeView->selectionModel()->currentIndex();
    if (index.isValid())
    {
        model_.removeRow(index.row(), index.parent());
    }
}

void MultiView::onSetupCat()
{
    QModelIndex index = ui_->treeView->selectionModel()->currentIndex();
    if (index.isValid())
    {
        QVariant d = model_.data(index, CatTreeModel::CatItemRole);
        if (d.isNull())
        {
            return;
        }
        TreeItem *item = (TreeItem *)d.value<void *>();
        if (item == NULL)
        {
            return;
        }

        QList< QSharedPointer<CatCtl> > catList = item->catList();
        if (catList.isEmpty())
        {
            return;
        }

        ConfigView *configView = new ConfigView(catList, 0, this);
        configView->setAttribute(Qt::WA_DeleteOnClose, true);

        connect( configView,    &ConfigView::globalConfig
               , this,          &MultiView::displayGlobalOptions);
        configView->show();
    }
}

void MultiView::on_btnGlobalSettings_clicked()
{
    displayGlobalOptions();
}

void MultiView::on_btnLeft_clicked()
{
    ui_->splitter->setSizes(QList<int>() << 0 << width());
}

void MultiView::onTreeCurrentChanged(const QModelIndex & current, const QModelIndex & /*previous*/)
{
    QVariant d = model_.data(current, CatTreeModel::CatItemRole);
    if (d.isNull())
    {
        return;
    }
    TreeItem *item = (TreeItem *)d.value<void *>();
    if (item == NULL)
    {
        return;
    }

    QSharedPointer<CatView> pcv = item->catView();
    if (pcv)
    {
        // reparent so widget will be removed from splitter
        while (ui_->splitter->count() > 1)
        {
            ui_->splitter->widget(1)->setParent(0);
        }

        QWidget *widget = pcv.data();
        ui_->splitter->addWidget(widget);
        ui_->splitter->setStretchFactor(1, 1);
    }
}

void MultiView::onTreeContextMenu(const QPoint & pos)
{
    QModelIndex index = ui_->treeView->indexAt(pos);
    if (index.isValid())
    {
        exportCat_->setEnabled(model_.canBeSaved(index));

        treeMenu_->exec(ui_->treeView->mapToGlobal(pos));
    }
}

void MultiView::onTreeModelRowsInserted(const QModelIndex & parent, int start, int end)
{
    for (int index = start; index <= end; index++)
    {
        connectCat(model_.index(index, 0, parent));
    }
}

void MultiView::onTreeModelReset()
{
    int rowCount = model_.rowCount();
    for (int i = 0; i < rowCount; i++)
    {
        connectCatRecursive(model_.index(i, 0));
    }
}

void MultiView::onGlobalOptionsAccepted()
{
    GlobalOptions opts = uiGlobalOpts_->getOptions();

    translator_->setLanguage(translator_->getLanguages()[opts.currentLanguage]);

    minimizeToTray_ = opts.minimizeToTray;
    appConfig_.set(L"MinimizeToTray", minimizeToTray_);
}

void MultiView::onCatUpdate()
{
    QObject *s = sender();
    CatView *w = qobject_cast<CatView *>(s);
    if (w != NULL)
    {
        model_.itemUpdate(w);
    }
}

void MultiView::onAutosaveTimer()
{
    groupConfig_.set(L"Cats", model_.cats().toVector().toStdVector(), json::DirectPtrConversion<CatGroup>());
    groupConfig_.save();
}

//

void MultiView::setTexts()
{
    addCat_->setText(tr("Add Cat"));
    addGroup_->setText(tr("Add Group"));
    loadCat_->setText(tr("Load Cat(s)"));
    exportCat_->setText(tr("Save Cat"));
    deleteCat_->setText(tr("Delete cat/group"));
    setupCat_->setText(tr("Setup"));
}

void MultiView::saveTreeState()
{
    std::vector<bool> states;

    size_t count = model_.rowCount();
    for (size_t i = 0; i < count; i++)
    {
        states.push_back(ui_->treeView->isExpanded(model_.index(i, 0)));
    }

    appConfig_.set(L"TreeState", states, json::NoConversion<bool>());
}

void MultiView::restoreTreeState()
{
    size_t count = model_.rowCount();
    if (count <= 0)
    {
        return;
    }

    std::vector<bool> states;
    appConfig_.get(L"TreeState", states, json::NoConversion<bool>());

    if (states.empty())
    {
        // expand first item by default
        states.push_back(true);
    }

    for (size_t i = 0; i < qMin(count, states.size()); i++)
    {
        ui_->treeView->setExpanded(model_.index(i, 0), states[i]);
    }
}

void MultiView::connectCat(CatView *widget)
{
    if (widget == NULL)
    {
        return;
    }

    widget->setClientExePath(g_elementExePath_);

    connect(widget, &CatView::globalSettings, this, &MultiView::displayGlobalOptions);
    connect(widget, &CatView::accountUpdate, this, &MultiView::onCatUpdate);
    connect( widget, &CatView::stateUpdated, this, &MultiView::onCatUpdate);
}

void MultiView::connectCat(const QModelIndex & index)
{
    QVariant d = model_.data(index, CatTreeModel::CatItemRole);
    if (d.isNull())
    {
        return;
    }

    TreeItem *item = (TreeItem *)d.value<void *>();
    if (item == NULL)
    {
        return;
    }

    CatContainer *cc = dynamic_cast<CatContainer*>(item);
    if (cc == NULL)
    {
        return;
    }

    connectCat(cc->catView().data());
}

void MultiView::connectCatRecursive(const QModelIndex & index)
{
    QVariant d = model_.data(index, CatTreeModel::CatItemRole);
    if (d.isNull())
    {
        return;
    }

    TreeItem *item = (TreeItem *)d.value<void *>();
    if (item == NULL)
    {
        return;
    }

    CatGroup *cg = dynamic_cast<CatGroup *>(item);
    if (cg != NULL)
    {
        for (int i = 0; i < item->rowCount(); i++)
        {
            connectCatRecursive(model_.index(i, 0, index));
        }
    }
    else
    {
        connectCat(index);
    }
}
