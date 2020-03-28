#ifndef ERROR_H
#define ERROR_H


class Error
{
public:
    Error(int page,int col);
    inline static void addError(){errorTotal+=10;}
private:
    int page;
    int col;

    static int errorTotal;
};

#endif // ERROR_H
