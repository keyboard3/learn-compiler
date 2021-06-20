#include "models.h"
#include "iostream"
struct TokenState
{
  DfaState newState;
  Token token;
};
TokenState initToken(char ch);
list<Token> tokenize(string code)
{
  list<Token> tokens;
  Token tempToken;
  DfaState state = DfaState::Initial;
  auto transferState = [&](char ch)
  {
    tokens.push_back(tempToken);
    auto result = initToken(ch);
    tempToken = result.token;
    state = result.newState;
  };
  auto forwardState = [&](char ch)
  { tempToken.text += ch; };
  auto idStateTransfer = [&](char ch)
  {
    if (isalpha(ch) || isdigit(ch))
      forwardState(ch);
    else
      transferState(ch);
  };

  for (int i = 0; i < code.length(); i++)
  {
    char ch = code[i];
    switch (state)
    {
    case DfaState::Initial:
    {
      TokenState result = initToken(ch); //重新确定后续状态
      tempToken = result.token;
      state = result.newState;
      break;
    }
    case DfaState::Id:
      idStateTransfer(ch);
      break;
    case DfaState::int1:
      if (ch == 'n')
      {
        tempToken.type = TokenType::Int; //转换成GE
        state = DfaState::int2;
        forwardState(ch);
      }
      else
        idStateTransfer(ch);
      break;
    case DfaState::int2:
      if (ch == 't')
      {
        tempToken.type = TokenType::Int; //转换成GE
        state = DfaState::int3;
        forwardState(ch);
      }
      else
        idStateTransfer(ch);
      break;
    case DfaState::GT:
      if (ch == '=')
      {
        tempToken.type = TokenType::GE; //转换成GE
        state = DfaState::GE;
        forwardState(ch);
      }
      else
        transferState(ch);
      break;
    case DfaState::IntLiteral:
      if (isdigit(ch))
        forwardState(ch);
      else
        transferState(ch);
      break;
    case DfaState::Assignment:
      if (ch == '=')
      {
        tempToken.type = TokenType::Equals; //转换成Equals
        state = DfaState::Equals;
        forwardState(ch);
      }
      else
        transferState(ch);
      break;
    case DfaState::SemiColon:
    case DfaState::LeftParen:
    case DfaState::RightParen:
    case DfaState::Equals:
    case DfaState::int3:
    case DfaState::GE:
    case DfaState::Plus:
    case DfaState::Minus:
    case DfaState::Star:
    case DfaState::Slash:
      transferState(ch);
    }
  }
  if (tempToken.text.length() > 0)
    tokens.push_back(tempToken); //将最后一个未结束状态的token加入
  return tokens;
}

TokenState initToken(char ch)
{
  TokenType tokenType;
  string tokenText;
  DfaState newState = DfaState::Initial;
  if (ch == 'i')
  {
    newState = DfaState::int1;
    tokenType = TokenType::Int;
    tokenText += ch;
  }
  else if (isalpha(ch))
  {                          //第一个字符是字母
    newState = DfaState::Id; //进入Id状态
    tokenType = TokenType::Identifier;
    tokenText += ch;
  }
  else if (isdigit(ch))
  { //第一个字符是数字
    newState = DfaState::IntLiteral;
    tokenType = TokenType::IntLiteral;
    tokenText += ch;
  }
  else if (ch == ';')
  {
    newState = DfaState::SemiColon;
    tokenType = TokenType::SemiColon;
    tokenText += ch;
  }
  else if (ch == '>')
  { //第一个字符是>
    newState = DfaState::GT;
    tokenType = TokenType::GT;
    tokenText += ch;
  }
  else if (ch == '=')
  {
    newState = DfaState::Assignment;
    tokenType = TokenType::Assignment;
    tokenText += ch;
  }
  else if (ch == '+')
  {
    newState = DfaState::Plus;
    tokenType = TokenType::Plus;
    tokenText += ch;
  }
  else if (ch == '-')
  {
    newState = DfaState::Minus;
    tokenType = TokenType::Minus;
    tokenText += ch;
  }
  else if (ch == '*')
  {
    newState = DfaState::Star;
    tokenType = TokenType::Star;
    tokenText += ch;
  }
  else if (ch == '/')
  {
    newState = DfaState::Slash;
    tokenType = TokenType::Slash;
    tokenText += ch;
  }
  else if (ch == '(')
  {
    newState = DfaState::LeftParen;
    tokenType = TokenType::LeftParen;
    tokenText += ch;
  }
  else if (ch == ')')
  {
    newState = DfaState::RightParen;
    tokenType = TokenType::RightParen;
    tokenText += ch;
  }
  TokenState tokenState;
  tokenState.newState = newState;
  tokenState.token.type = tokenType;
  tokenState.token.text = tokenText;
  return tokenState;
}
bool isalpha(char ch)
{
  return ((ch >= 'a') && (ch <= 'z')) || ((ch >= 'A') && (ch <= 'Z')) || ch == '_';
}
bool isdigit(char ch)
{
  return (ch >= '0') && (ch <= '9');
}