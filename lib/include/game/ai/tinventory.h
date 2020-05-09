#ifndef tinventory_h
#define tinventory_h


typedef std::function<DWORD (unsigned & slot, InventoryType & type)>  GetUsableItemFn;

class TUseItem : public ITask
{
public:
    TUseItem(std::shared_ptr<Game> game, GetUsableItemFn getUsableItem);;

protected:
    virtual State onStarted();
    virtual void onStopped();

private:
    void onPlayerUseItem(const serverdata::FragmentGiPlayerUseItem *f);

    std::shared_ptr<Game> game_;
    GetUsableItemFn         getUsableItem_;
    Connection::Cookie      cPlayerUseItem_;

    unsigned                slot;
    InventoryType           inventoryType;
    DWORD                   itemId;
};

#endif
