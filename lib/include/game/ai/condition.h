#ifndef condition_h
#define condition_h

// Usage sample:
//
// <> cond = GameCond<c1> && GameCond<c2>
//

class Game;


class Condition
{
public:
    virtual ~Condition()
    {}

    // return condition result
    virtual bool operator ()() const = 0;
};

class ConditionAnd : public Condition
{
    // conditions is true if both condition are true
public:
    ConditionAnd(const std::shared_ptr<Condition> & r, const std::shared_ptr<Condition> & l)
        : r_(r), l_(l)
    {}

    virtual bool operator ()() const
    {
        if (r_ && l_)
        {
            return (*r_)() && (*l_)();
        }
        else
        {
            assert(0);
            return false;
        }
    }

private:
    std::shared_ptr<Condition>    r_, l_;
};


class ConditionOr : public Condition
{
public:
    ConditionOr(std::shared_ptr<Condition> r, std::shared_ptr<Condition> l)
        : r_(r), l_(l)
    {}

    virtual bool operator ()() const
    {
        if (r_ && l_)
        {
            return (*r_)() || (*l_)();
        }
        else
        {
            assert(0);
            return false;
        }
    }

private:
    std::shared_ptr<Condition>    r_, l_;
};


class ConditiionNot : public Condition
{
public:
    ConditiionNot(std::shared_ptr<Condition> inner)
        : inner_(inner)
    {}

    // return true if condition is 'true'
    virtual bool operator ()() const
    {
        return !(*inner_)();
    }

private:
    std::shared_ptr<Condition> inner_;
};

template< class Cond >
class GameCond : public Condition
               , private Cond
{
public:
    GameCond(std::shared_ptr<Game> game)
        : game_(game)/*, lateSwitch_(false)*/
    {}
//     GameCond(std::shared_ptr<Game> game, bool lateSwitch)
//         : game_(game), lateSwitch_(lateSwitch)
//     {}
// 
    virtual bool operator ()() const
    {
        bool result = Cond::operator()(game_);
//         if (lateSwitch_ && result)
//         {
//             lateSwitch_ = false;
//             return false;
//         }
        return result;
    }

private:
      std::shared_ptr<Game>   game_;
//      mutable bool              lateSwitch_;
};


class FuncCond : public Condition
{
public:
    typedef std::function<bool ()>    conditionFn;

    FuncCond(conditionFn fn)
        : fn_(fn)
    {}

    virtual bool operator ()() const
    {
        return fn_();
    }

private:
    conditionFn     fn_;
};


class BoolCond : public Condition
{
public:
    BoolCond(const bool & value)
        : value_(value)
    {}

    virtual bool operator ()() const
    {
        return value_;
    }

private:
    BoolCond operator=(const BoolCond &);

    const bool & value_;
};


class Trigger
{
private:
    Trigger(const Trigger&) = delete;
    Trigger& operator=(const Trigger&) = delete;

public:
    Trigger()
        : value_(false)
    {}

    void set()
    {
        value_ = true;
    }

    void reset()
    {
        value_ = false;
    }

    bool checked()
    {
        return value_;
    }

    operator std::shared_ptr<Condition>()
    {
        return std::shared_ptr<Condition>( new FuncCond(std::bind(&Trigger::triggered, this)));
    }

    std::shared_ptr<Condition> check()
    {
        return std::shared_ptr<Condition>( new FuncCond(std::bind(&Trigger::checked, this)));
    }

private:
    bool triggered()
    {
        if (value_)
        {
            value_ = false;
            return true;
        }
        return false;
    }

    bool    value_;
};
//

inline std::shared_ptr<Condition> FCondition(std::function<bool ()> conditionFn)
{
    return std::shared_ptr<Condition>(new FuncCond(conditionFn));
}

template< class Cond >
inline std::shared_ptr<Condition> GCondition(std::shared_ptr<Game> game)
{
    return std::shared_ptr<Condition>(new GameCond<Cond>(game));
}

inline std::shared_ptr<Condition> BCondition(const bool & value)
{
    return std::shared_ptr<Condition>(new BoolCond(value));
}

inline std::shared_ptr<Condition> operator &&(std::shared_ptr<Condition> r, std::shared_ptr<Condition> l)
{
    return std::shared_ptr<Condition>(new ConditionAnd(r, l));
}

inline std::shared_ptr<Condition> operator ||(std::shared_ptr<Condition> r, std::shared_ptr<Condition> l)
{
    return std::shared_ptr<Condition>(new ConditionOr(r, l));
}

inline std::shared_ptr<Condition> not(std::shared_ptr<Condition> v)
{
    return std::shared_ptr<Condition>(new ConditiionNot(v));
}

// ***

struct GotServerErrCond
{
    bool operator ()(std::shared_ptr<Game> game) const;
};

struct GotGameErrCond
{
    bool operator ()(std::shared_ptr<Game> game) const;
};


#endif
