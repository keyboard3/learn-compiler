import { DfaState, Token, TokenType } from "./models";

/**
 * 简单的词法分析器
 */
function initToken(ch: string) {
  let tokenType: TokenType = null;
  let tokenText = "";
  let newState: DfaState = DfaState.Initial;
  if (ch == "i") {
    newState = DfaState.int1;
    tokenType = TokenType.Int;
    tokenText = ch;
  } else if (isAlpha(ch)) {              //第一个字符是字母
    newState = DfaState.Id; //进入Id状态
    tokenType = TokenType.Identifier;
    tokenText = ch;
  } else if (isDigit(ch)) {       //第一个字符是数字
    newState = DfaState.IntLiteral;
    tokenType = TokenType.IntLiteral;
    tokenText = ch;
  } else if (ch == ";") {
    newState = DfaState.SemiColon;
    tokenType = TokenType.SemiColon;
    tokenText = ch;
  } else if (ch == '>') {         //第一个字符是>
    newState = DfaState.GT;
    tokenType = TokenType.GT;
    tokenText = ch;
  } else if (ch == "=") {
    newState = DfaState.Assignment;
    tokenType = TokenType.Assignment;
    tokenText = ch;
  } else if (ch == "+") {
    newState = DfaState.Plus;
    tokenType = TokenType.Plus;
    tokenText = ch;
  } else if (ch == "-") {
    newState = DfaState.Minus;
    tokenType = TokenType.Minus;
    tokenText = ch;
  } else if (ch == "*") {
    newState = DfaState.Star;
    tokenType = TokenType.Star;
    tokenText = ch;
  } else if (ch == "/") {
    newState = DfaState.Slash;
    tokenType = TokenType.Slash;
    tokenText = ch;
  } else if (ch == "(") {
    newState = DfaState.LeftParen;
    tokenType = TokenType.LeftParen;
    tokenText = ch;
  } else if (ch == ")") {
    newState = DfaState.RightParen;
    tokenType = TokenType.RightParen;
    tokenText = ch;
  }
  return {
    newState,
    token: { type: tokenType, text: tokenText }
  };
}

export function tokenize(code: string):Token[] {
  let tokens: Token[] = [];
  let tempToken: Token = null;
  let state: DfaState = DfaState.Initial;
  const transferState = (ch) => {
    tokens.push(tempToken);
    const result = initToken(ch); //退出当前状态，并保存Token
    tempToken = result.token;
    state = result.newState;
  }
  const forwardState = (ch: string) => tempToken.text += ch;
  const idStateTransfer = (ch) => {
    if (isAlpha(ch) || isDigit(ch)) forwardState(ch)
    else transferState(ch);
  }

  for (let i = 0; i < code.length; i++) {
    let ch = code[i];
    switch (state) {
      case DfaState.Initial:
        const result = initToken(ch);          //重新确定后续状态
        tempToken = result.token;
        state = result.newState;
        break;
      case DfaState.Id:
        idStateTransfer(ch);
        break;
      case DfaState.int1:
        if (ch == "n") {
          tempToken.type = TokenType.Int;  //转换成GE
          state = DfaState.int2;
          forwardState(ch);
        } else idStateTransfer(ch);
        break;
      case DfaState.int2:
        if (ch == "t") {
          tempToken.type = TokenType.Int;  //转换成GE
          state = DfaState.int3;
          forwardState(ch);
        } else idStateTransfer(ch);
        break;
      case DfaState.GT:
        if (ch == '=') {
          tempToken.type = TokenType.GE;  //转换成GE
          state = DfaState.GE;
          forwardState(ch);
        } else transferState(ch)
        break;
      case DfaState.IntLiteral:
        if (isDigit(ch)) forwardState(ch)
        else transferState(ch)
        break;
      case DfaState.Assignment:
        if (ch == "=") {
          tempToken.type = TokenType.Equals;  //转换成Equals
          state = DfaState.Equals;
          forwardState(ch);
        } else transferState(ch);
        break;
      case DfaState.SemiColon:
      case DfaState.LeftParen:
      case DfaState.RightParen:
      case DfaState.Equals:
      case DfaState.int3:
      case DfaState.GE:
      case DfaState.Plus:
      case DfaState.Minus:
      case DfaState.Star:
      case DfaState.Slash:
        transferState(ch);
    }
  }
  if (tempToken.text) tokens.push(tempToken); //将最后一个未结束状态的token加入
  return tokens;
}

function isAlpha(ch: string) {
  return /[a-zA-Z]/.test(ch);
}

function isDigit(ch: string) {
  return /[0-9]/.test(ch);
}
