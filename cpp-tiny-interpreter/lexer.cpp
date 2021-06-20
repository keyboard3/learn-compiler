
#include "models.h"
#define ASSET_CHAR(cs) \
  if (cs.empty())      \
    return;
#define GET_CHAR(cs) \
  if (cs.empty())    \
    chr = NULL;      \
  else               \
    chr = cs.front();
#define RETURN_TOKEN(tb, tp)           \
  if (tb == nullptr || tb->type != tp) \
    return;                            \
  tokens.push_back(tb);                \
  tb = nullptr;                        \
  return;

list<Token *> tokenize(string code)
{
  list<char> cs;
  list<Token *> tokens;
  Token *tokenBuffer = nullptr;
  for (auto chr : code)
    cs.push_back(chr);

  auto skipline = [&]()
  {
    ASSET_CHAR(cs);
    char chr = cs.front();
    while (chr == EOF || chr == '\n')
    {
      cs.pop_front();
      GET_CHAR(cs);
    }
  };
  auto clearEmpty = [&]()
  {
    ASSET_CHAR(cs);
    char chr = cs.front();
    while (isspace(chr))
    {
      cs.pop_front();
      GET_CHAR(cs);
    };
  };
  auto matchNameAndKeyword = [&]()
  {
    ASSET_CHAR(cs);
    char chr = cs.front();
    while (isalpha(chr) || chr == '_' || chr == '$')
    {
      cs.pop_front();
      if (tokenBuffer == nullptr)
        tokenBuffer = new Token(TokenType::NAME, "");
      tokenBuffer->text += chr;
      GET_CHAR(cs);
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
      cs.pop_front();
      tokenBuffer = new Token(TokenType::NUMBER, to_string(chr));
      if (cs.empty())
        RETURN_TOKEN(tokenBuffer, TokenType::NUMBER);
      //8进制
      GET_CHAR(cs);
      if (chr == 'x')
      {
        tokenBuffer->text += chr;
        base = 16;
        GET_CHAR(cs);
      }
      while (isdigit(chr))
        GET_CHAR(cs);
      if (base == 10 && (chr == '.' || tolower(chr) == 'e'))
      {
        if (chr == '.')
          do
          {
            tokenBuffer->text += chr;
            GET_CHAR(cs);
          } while (isdigit(chr));
        //读取科学技术法
        if (tolower(chr) == 'e')
        {
          tokenBuffer->text += chr;
          GET_CHAR(cs);
          if (chr == '+' || chr == '-')
          {
            tokenBuffer->text += chr;
            GET_CHAR(cs);
          }
          if (!isdigit(chr))
          {
            //TODO 上报科学计数法表示有异常
          }
          else
            do
            {
              tokenBuffer->text += chr;
              GET_CHAR(cs);
            } while (isdigit(chr));
        }
      }
    }
    RETURN_TOKEN(tokenBuffer, TokenType::NUMBER);
  };
  auto matchString = [&]()
  {
    ASSET_CHAR(cs);
    char chr;
    GET_CHAR(cs);
    if (chr != '"' && chr != '\'')
      return;
    char prevChr = chr;
    GET_CHAR(cs);
    tokenBuffer = new Token(TokenType::STRING, "");
    while (prevChr != chr)
    {
      if (chr == '\n' || chr == EOF)
      {
        //TODO 上报语法错误 未终止字符串常
      };
      if (chr == '\\') //转义字符
      {
        GET_CHAR(cs);
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
      GET_CHAR(cs);
    }
    RETURN_TOKEN(tokenBuffer, TokenType::STRING);
  };
  auto matchOther = [&]()
  {
    ASSET_CHAR(cs);
    char chr;
    GET_CHAR(cs);
    TokenType tt;
    switch (chr)
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
      GET_CHAR(cs);
      if (chr == '|')
        tt = TokenType::OR;
      else if (chr == '=')
        tt = TokenType::ASSIGN;
      else
      {
        tt = TokenType::BITOR;
        cs.push_front(chr);
      }
      break;
    case '^':
      GET_CHAR(cs);
      if (chr == '=')
        tt = TokenType::ASSIGN;
      else
      {
        tt = TokenType::BITXOR;
        cs.push_front(chr);
      }
      break;
    case '&':
      GET_CHAR(cs);
      if (chr == '&')
        tt = TokenType::AND;
      else if (chr == '=')
        tt = TokenType::ASSIGN;
      else
      {
        tt = TokenType::BITAND;
        cs.push_front(chr);
      }
      break;
    case '=':
      GET_CHAR(cs);
      if (chr == '=')
        tt = TokenType::EQOP;
      else
      {
        tt = TokenType::ASSIGN;
        cs.push_front(chr);
      }
      break;
    case '!':
      GET_CHAR(cs);
      if (chr == '=')
        tt = TokenType::EQOP;
      else
      {
        tt = TokenType::UNARYOP;
        cs.push_front(chr);
      }
      break;
    case '<':
      GET_CHAR(cs);
      char preChr = chr;
      if (chr == '<')
      {
        GET_CHAR(cs);
        tt = chr == '=' ? TokenType::ASSIGN : TokenType::SHOP;
      }
      else
      {
        tt = TokenType::RELOP;
        cs.push_front(preChr);
      }
      break;
    case '>':
      GET_CHAR(cs);
      char preChr = chr;
      if (chr == '>')
      {
        GET_CHAR(cs);
        tt = chr == '=' ? TokenType::ASSIGN : TokenType::SHOP;
      }
      else
      {
        tt = TokenType::RELOP;
        cs.push_front(preChr);
      }
      break;
    case '*':
      GET_CHAR(cs);
      tt = chr == '=' ? TokenType::ASSIGN : TokenType::MULOP;
      break;
    case '/':
      GET_CHAR(cs);
      if (chr == '/') //单行注释
      {
        skipline();
        tokenBuffer = nullptr;
        return;
      }
      GET_CHAR(cs);
      //多行注释
      if (chr == '*')
      {
        GET_CHAR(cs);
        while (chr != EOF)
        {
          if (chr == '*')
          {
            GET_CHAR(cs);
            if (chr == '/')
              return;
          }
          else
            GET_CHAR(cs);
        }
        if (chr == EOF)
          return;
      }
      tt = chr == '=' ? TokenType::ASSIGN : TokenType::MULOP;
      break;
    case '%':
      GET_CHAR(cs);
      tt = chr == '=' ? TokenType::ASSIGN : TokenType::MULOP;
      break;
    case '~':
      tt = TokenType::UNARYOP;
      break;
    case '+':
    case '-':
      if (chr == '-')
        tt = TokenType::MINUS;
      else if (chr == '+')
        tt = TokenType::PLUS;
      GET_CHAR(cs);
      if (chr == '=')
        tt = TokenType::ASSIGN;
      else if (chr == '-')
        tt = TokenType::INCOP;
      else if (tt == TokenType::MINUS || tt == TokenType::PLUS)
      {
        cs.push_front(chr);
      }
      break;
    default:
      return;
    }
  };

  char chr = NULL;
  GET_CHAR(cs);
  while (chr != NULL)
  {
    clearEmpty();
    matchNameAndKeyword();
    matchNumber();
    matchString();
    matchOther();
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