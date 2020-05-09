#ifndef tristate_h_
#define tristate_h_


template<typename Type>
class Tristate
{
public:
    Tristate()
        : value_(Type())
        , isTristate_(true)
    {}

    Tristate(const Type & v)
        : value_(v)
        , isTristate_(false)
    {}

    Tristate(const Tristate & r)
        : value_(r.value_)
        , isTristate_(r.isTristate_)
    {}

    Tristate & operator=(const Tristate & r)
    {
        value_ = r.value_;
        isTristate_ = r.isTristate_;
        return *this;
    }

    Tristate & operator=(const Type & r)
    {
        value_ = r;
        isTristate_ = false;
        return *this;
    }

    operator Type() const
    {
        return value();
    }

    Type value() const
    {
        return isTristate_ ? Type() : value_;
    }

    Type valueOpt(const Type & ifTristate) const
    {
        return isTristate_ ? ifTristate : value_;
    }

    void setValueOpt(const Type & value, const Type & tristatedValue)
    {
        isTristate_ = (value == tristatedValue);
        value_ = value;
    }

    Tristate & operator<<(const Tristate & r)
    {
        if (value_ != r.value_)
        {
            isTristate_ = true;
        }
        return *this;
    }

    const Tristate & operator>>(Type & r) const
    {
        if ( ! isTristate_)
        {
            r = value_;
        }
        return *this;
    }

    const Tristate & operator>>(Tristate<Type> & r) const
    {
        if ( ! isTristate_)
        {
            r = value_;
        }
        return *this;
    }

    bool operator==(const Type & r)
    {
        return ! isTristate_ && value_ == r;
    }

    bool isTristate() const
    {
        return isTristate_;
    }

    void setTristate(bool isTristate = true)
    {
        isTristate_ = isTristate;
    }

private:
    Type    value_;
    bool    isTristate_;
};



#endif