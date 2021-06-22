
#include "models.h"
#include "iostream"
#define ASSET_CHAR(cs) \
  if (cs.empty())      \
    return;

#define GET_CHAR(cs) \
  if (cs.empty())    \
    chr = EOF;       \
  else               \
    chr = cs.front();
#define GET_CHAR_POP(cs) \
  {                      \
    cs.pop_front();      \
    GET_CHAR(cs)         \
  }

#define RETURN_TOKEN(tb, tp)                     \
  {                                              \
    if (tb->type != tp)                          \
      return;                                    \
    tokens.push_back(new Token(tb->type, tb->text)); \
    resetBuffer();                               \
    return;                                      \
  }

list<Token *> tokenize(string code)
{
  list<char> cs;
  list<Token *> tokens;
  Token *tokenBuffer = new Token(TokenType::DEFAULT, "");
  for (auto chr : code)
    cs.push_back(chr);

  auto resetBuffer = [&]()
  {
    tokenBuffer->text = "";
    tokenBuffer->type = TokenType::DEFAULT;
  };
  auto skipline = [&]()
  {
    ASSET_CHAR(cs);
    char chr = cs.front();
    while (chr == EOF || chr == '\n')
      GET_CHAR_POP(cs);
  };
  auto clearEmpty = [&]()
  {
    ASSET_CHAR(cs);
    char chr = cs.front();
    while (isspace(chr))
      GET_CHAR_POP(cs);
  };
  auto matchNameAndKeyword = [&]()
  {
    ASSET_CHAR(cs);
    char chr = cs.front();
    while (isalpha(chr) || chr == '_' || chr == '$')
    {
      if (tokenBuffer->type == TokenType::DEFAULT)
        tokenBuffer = new Token(TokenType::NAME, "");
      tokenBuffer->text += chr;
      GET_CHAR_POP(cs);
    }
    if (defineKeywords.find(tokenBuffer->text) != defineKeywords.end())
    {
      tokenBuffer->type = defineKeywords[tokenBuffer->text];
      RETURN_TOKEN(tokenBuffer, defineKeywords[tokenBuffer->text]);
      return;
    }
    RETURN_TOKEN(tokenBuffer, TokenType::NAME);
  };
  auto matchNumber = [&]()
  {
    ASSET_CHAR(cs);
    char chr = cs.front();

    if (isdigit(chr))
    {
      int base = 10;
      tokenBuffer = new Token(TokenType::NUMBER, chr);
      if (cs.empty())
        RETURN_TOKEN(tokenBuffer, TokenType::NUMBER);
      //8进制
      GET_CHAR_POP(cs);
      if (chr == 'x')
      {
        tokenBuffer->text += chr;
        base = 16;
        GET_CHAR_POP(cs);
      }
      while (isdigit(chr))
      {
        tokenBuffer->text += chr;
        GET_CHAR_POP(cs);
      }
      if (base == 10 && (chr == '.' || tolower(chr) == 'e'))
      {
        if (chr == '.')
          do
          {
            tokenBuffer->text += chr;
            GET_CHAR_POP(cs);
          } while (isdigit(chr));
        //读取科学技术法
        if (tolower(chr) == 'e')
        {
          tokenBuffer->text += chr;
          GET_CHAR_POP(cs);
          if (chr == '+' || chr == '-')
          {
            tokenBuffer->text += chr;
            GET_CHAR_POP(cs);
          }
          if (!isdigit(chr))
          {
            //TODO 上报科学计数法表示有异常
          }
          else
            do
            {
              tokenBuffer->text += chr;
              GET_CHAR_POP(cs);
            } while (isdigit(chr));
        }
      }
    }
    RETURN_TOKEN(tokenBuffer, TokenType::NUMBER);
  };
  auto matchString = [&]()
  {
    ASSET_CHAR(cs);
    char chr = cs.front();
    if (chr != '"' && chr != '\'')
      return;
    char prevChr = chr;
    GET_CHAR_POP(cs);
    tokenBuffer = new Token(TokenType::STRING, "");
    while (prevChr != chr)
    {
      if (chr == '\n' || chr == EOF)
      {
        //TODO 上报语法错误 未终止字符串常
      };
      if (chr == '\\') //转义字符
      {
        GET_CHAR_POP(cs);
        switch (chr)
        {
        case 'b':
          chr = '\b';
          break;
        case 'f':
          chr = '\f';
          break;
        case 'n':
          chr = '\n';
          break;
        case 'r':
          chr = '\r';
          break;
        case 't':
          chr = '\t';
          break;
        case 'v':
          chr = '\v';
          break;
        default: //字符串中数字的多进制转义
          // if (isdigit(chr) && chr < '8') //todo \7
          // else if (chr == 'u') //todo \uxxx
          // if (chr == 'x') //todo \x
          break;
        }
      }
      tokenBuffer->text += chr;
      GET_CHAR_POP(cs);
    }
    RETURN_TOKEN(tokenBuffer, TokenType::STRING);
  };
  auto matchOther = [&]()
  {
    ASSET_CHAR(cs);
    string preChr(1, cs.front());
    char chr;
    TokenType tt = TokenType::DEFULT;
    switch (preChr[0])
    {
    case '\n':
      tt = TokenType::EOL;
      break;
    case ';':
      tt = TokenType::SEMI;
      break;
    case '[':
      tt = TokenType::LB;
      break;
    case ']':
      tt = TokenType::RB;
      break;
    case '{':
      tt = TokenType::LC;
      break;
    case '}':
      tt = TokenType::RC;
      break;
    case '(':
      tt = TokenType::LP;
      break;
    case ')':
      tt = TokenType::RP;
      break;
    case ',':
      tt = TokenType::COMMA;
      break;
    case '?':
      tt = TokenType::HOOK;
      break;
    case ':':
      tt = TokenType::COLON;
      break;
    case '.':
      tt = TokenType::DOT;
      break;
    case '|':
      GET_CHAR_POP(cs);
      preChr += chr;
      if (chr == '|')
        tt = TokenType::OR;
      else if (chr == '=')
        tt = TokenType::ASSIGN;
      else
        tt = TokenType::BITOR, preChr.pop_back(), cs.push_front(chr);
      break;
    case '^':
      GET_CHAR_POP(cs);
      preChr += chr;
      if (chr == '=')
        tt = TokenType::ASSIGN;
      else
        tt = TokenType::BITXOR, preChr.pop_back(), cs.push_front(chr);
      break;
    case '&':
      GET_CHAR_POP(cs);
      preChr += chr;
      if (chr == '&')
        tt = TokenType::AND;
      else if (chr == '=')
        tt = TokenType::ASSIGN;
      else
        tt = TokenType::BITAND, preChr.pop_back(), cs.push_front(chr);
      break;
    case '=':
      GET_CHAR_POP(cs);
      preChr += chr;
      if (chr == '=')
        tt = TokenType::EQOP;
      else
        tt = TokenType::ASSIGN, preChr.pop_back(), cs.push_front(chr);
      break;
    case '!':
      GET_CHAR_POP(cs);
      preChr += chr;
      if (chr == '=')
        tt = TokenType::EQOP;
      else
        tt = TokenType::UNARYOP, preChr.pop_back(), cs.push_front(chr);
      break;
    case '<':
      GET_CHAR_POP(cs);
      preChr += chr;
      if (chr == '<')
      {
        GET_CHAR_POP(cs);
        preChr += chr;
        tt = chr == '=' ? TokenType::ASSIGN : TokenType::SHOP;
        if (chr != '=')
          preChr.pop_back(), cs.push_front(chr);
      }
      else
        tt = TokenType::RELOP, preChr.pop_back(), cs.push_front(chr);
      break;
    case '>':
      GET_CHAR_POP(cs);
      preChr += chr;
      if (chr == '>')
      {
        GET_CHAR_POP(cs);
        preChr += chr;
        tt = chr == '=' ? TokenType::ASSIGN : TokenType::SHOP;
        if (chr != '=')
          preChr.pop_back(), cs.push_front(chr);
      }
      else
        tt = TokenType::RELOP, preChr.pop_back();
      break;
    case '*':
      GET_CHAR_POP(cs);
      preChr += chr;
      if (chr != '=')
        preChr.pop_back(), cs.push_front(chr);

      tt = chr == '=' ? TokenType::ASSIGN : TokenType::MULOP;
      break;
    case '/':
      GET_CHAR_POP(cs);
      if (chr == '/') //单行注释
      {
        skipline();
        resetBuffer();
        return;
      }
      GET_CHAR(cs);
      //多行注释
      if (chr == '*')
      {
        GET_CHAR_POP(cs);
        while (chr != EOF)
        {
          if (chr == '*')
          {
            GET_CHAR_POP(cs);
            if (chr == '/')
              return;
          }
          else
            GET_CHAR_POP(cs);
        }
        if (chr == EOF)
          return;
      }
      preChr += chr;
      if (chr != '=')
        preChr.pop_back(), cs.push_front(chr);

      tt = chr == '=' ? TokenType::ASSIGN : TokenType::MULOP;
      break;
    case '%':
      GET_CHAR_POP(cs);
      preChr += chr;
      if (chr != '=')
        preChr.pop_back(), cs.push_front(chr);
      tt = chr == '=' ? TokenType::ASSIGN : TokenType::MULOP;
      break;
    case '~':
      tt = TokenType::UNARYOP;
      break;
    case '+':
    case '-':
      if (preChr == "-")
        tt = TokenType::MINUS;
      else if (preChr == "+")
        tt = TokenType::PLUS;
      GET_CHAR_POP(cs);
      preChr += chr;
      if (chr == '=')
        tt = TokenType::ASSIGN;
      else if (chr == '-')
        tt = TokenType::INCOP;
      else
        preChr.pop_back(), cs.push_front(chr);
      break;
    default:
      return;
    }
    if (tt != TokenType::DEFAULT)
    {
      cs.pop_front();
      tokenBuffer = new Token(tt, preChr);
      RETURN_TOKEN(tokenBuffer, tt);
    }
  };

  char chr = EOF;
  GET_CHAR(cs);
  while (!cs.empty())
  {
    clearEmpty();
    matchNameAndKeyword();
    matchNumber();
    matchString();
    matchOther();
    GET_CHAR(cs);
  }
  return tokens;
}

bool isWhitespace(char ch)
{
  return (ch == ' ') || (ch == '\t') || (ch == '\n') || (ch == '\r');
}
bool isalpha(char ch)
{
  return ((ch >= 'a') && (ch <= 'z')) || ((ch >= 'A') && (ch <= 'Z')) || ch == '_';
}
bool isdigit(char ch)
{
  return (ch >= '0') && (ch <= '9');
}