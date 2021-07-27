#include "string"
using namespace std;
//来源：https://llvm.org/docs/tutorial/MyFirstLanguageFrontend/LangImpl01.html
//这个词法解析发现如果不认识这个字符串会返回 0-255，否则会正确返回 tokens
enum Token
{
    tok_eof = -1,
    //comands 关键字
    tok_def = -2,
    tok_extern = -3,
    //primary 基础元素
    tok_identifier = -4,
    tok_number = -5,
};
static string IdentifierStr; //如果是 tok_identifier 就会赋值
static double NumVal;        //如果是 tok_number 就会赋值

//gettok - 从标准输入中返回下一个token
static int gettok()
{
    static int LastChar = ' ';

    //跳过任何空白字符
    while (isspace(LastChar))
        LastChar = getchar();

    //identifier: [a-zA-Z][a-zA-Z0-9]*
    if (isalpha(LastChar))
    {
        IdentifierStr = LastChar;
        while (isalnum(LastChar = getchar()))
            IdentifierStr += LastChar;

        if (IdentifierStr == "def")
            return tok_def;
        if (IdentifierStr == "extern")
            return tok_extern;
        return tok_identifier;
    }

    //Number: [0-9.]+
    if (isdigit(LastChar) || LastChar == '.')
    {
        string NumStr;
        do
        {
            NumStr += LastChar;
            LastChar = getchar();
        } while (isdigit(LastChar) || LastChar == '.');
        //未处理异常输入 1.23.45.67
        NumVal = strtod(NumStr.c_str(), 0);
        return tok_number;
    }

    //单行注释
    if (LastChar == '#')
    {
        do
            LastChar = getchar();
        while (LastChar != EOF && LastChar != '\n' && LastChar != '\r');
        if (LastChar != EOF)
            return gettok();
    }

    //检查文件尾，不要吞掉EOF
    if (LastChar == EOF)
        return tok_eof;

    //否则，就会返回字符本身的 ascii 值
    int ThisChar = LastChar;
    LastChar = getchar();
    return ThisChar;
}