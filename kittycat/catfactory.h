#ifndef catfactory_h_
#define catfactory_h_

class CatView;

class CatFactory
{
public:
    CatFactory();

    CatView * create();

private:
};

#endif