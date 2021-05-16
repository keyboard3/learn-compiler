import { defineKeywords, K3_EOF, K3_Token, K3_TokenType as Token, CharStream } from "./models";

/**
 * 词法分析器
 */
export function tokenize(code: string): K3_Token[] {
  let tokens: K3_Token[] = [];
  const cs = new CharStream(code);
  let ch;
  do {
    clearEmpty();
    matchNameAndKeyword();
    matchNumber();
    matchString();
    matchOther();
  } while (ch);
  return tokens;

  function skipLine() {
    while ((ch = cs.getChar()) != K3_EOF && ch != '\n') {
      cs.unGetChar(ch);
      return;
    }
  }

  function matchNameAndKeyword() {
    if (isAlpha(ch) || ch === '_' || ch === '$') {
      do {
        ch = cs.appendToTokenBuf(ch);
      } while (isAlpha(ch) || ch === '_' || ch === '$')
      if (defineKeywords[cs.tokenBuf]) {
        captureToken(defineKeywords[cs.tokenBuf]);
      } else captureToken(Token.NAME);
    }
  }

  function clearEmpty() {
    if (!ch) ch = cs.getChar();
    while (isSpace(ch)) {
      if (ch == '\n') {
        //sourceNote
      }
      ch = cs.getChar();
    }
    if (ch == K3_EOF) captureToken(Token.EOF);
  }

  function matchNumber() {
    if (isDigit(ch) || (ch == '.' && isDigit(cs.peekChar()))) {
      let base = 10;
      if (ch == '0') {
        ch = cs.appendToTokenBuf(ch);
        if (ch.toLocaleLowerCase() === 'x') {
          ch = cs.appendToTokenBuf(ch);
          base = 16;
        } else if (isDigit(ch) && ch < '8') {
          base = 8;
        }
      }
      while (isDigit(ch)) {
        //todo
        ch = cs.appendToTokenBuf(ch);
      }
      if (base == 10 && (ch === '.' || ch?.toLocaleLowerCase() === 'e')) {
        if (ch === '.') {
          do {
            ch = cs.appendToTokenBuf(ch);
          } while (isDigit(ch));
        }
        if (ch?.toLocaleLowerCase() === 'e') {
          ch = cs.appendToTokenBuf(ch);
          if (ch === '+' || ch === '-') {
            ch = cs.appendToTokenBuf(ch);
          }
          if (!isDigit(ch)) {
            //todo 上报异常 "missing exponent
          } else {
            do {
              ch = cs.appendToTokenBuf(ch);
            } while (isDigit(ch))
          }
        }
      }
      captureToken(Token.NUMBER);
    }
  }

  function matchString() {
    if (ch == '"' || ch == '\'') {
      const qc = ch;
      ch = cs.getChar();
      while (ch != qc) {
        if (ch == '\n' || ch == K3_EOF) {
          cs.unGetChar(ch);
          //todo 上报语法错误 未终止的字符串字面量
          break;
        }
        if (ch === "\\") {
          switch (ch = cs.getChar()) {
            case 'b':
              ch = '\b';
              break;
            case 'f':
              ch = '\f';
              break;
            case 'n':
              ch = '\n';
              break;
            case 'r':
              ch = '\r';
              break;
            case 't':
              ch = '\t';
              break;
            case 'v':
              ch = '\v';
              break;
            default:
              if (isDigit(ch) && ch < '8') {
                //todo \7
              } else if (ch === 'u') {
                //todo \uxxx
              } else if (ch == 'x') {
                //todo \x
              }
              break;
          }
        }
        ch = cs.appendToTokenBuf(ch);
      }
      ch = cs.getChar();
      captureToken(Token.STRING);
    }
  }

  function matchOther() {
    if (!ch) return;
    let tt;
    switch (ch) {
      case '\n':
        tt = Token.EOL;
        break;
      case ';':
        tt = Token.SEMI;
        break;
      case '[':
        tt = Token.LB;
        break;
      case ']':
        tt = Token.RB;
        break;
      case '{':
        tt = Token.LC;
        break;
      case '}':
        tt = Token.RC;
        break;
      case '(':
        tt = Token.LP;
        break;
      case ')':
        tt = Token.RP;
        break;
      case ',':
        tt = Token.COMMA;
        break;
      case '?':
        tt = Token.HOOK;
        break;
      case ':':
        tt = Token.COLON;
        break;
      case '.':
        tt = Token.DOT;
        break;
      case '|':
        if (matchChar('|')) tt = Token.OR;
        else if (matchChar('=')) tt = Token.ASSIGN;
        else tt = Token.BITOR;
        break;
      case '^':
        if (matchChar('=')) tt = Token.ASSIGN;
        else tt = Token.BITXOR;
        break;
      case '&':
        if (matchChar('&')) tt = Token.AND;
        else if (matchChar('=')) tt = Token.ASSIGN;
        else tt = Token.BITAND;
        break;
      case '=':
        if (matchChar('=')) tt = Token.EQOP;
        else tt = Token.ASSIGN;
        break;
      case '!':
        if (matchChar('=')) tt = Token.EQOP;
        else tt = Token.UNARYOP;
        break;
      case '<':
        /* XXX treat HTML begin-comment as comment-till-end-of-line */
        if (matchChar('!')) {
          if (matchChar('-')) {
            if (matchChar('-')) skipLine()
            unGetChar('-');
          }
          unGetChar('!');
        }
        if (matchChar('<')) {
          tt = matchChar('=') ? Token.ASSIGN : Token.SHOP;
        } else tt = Token.RELOP;
        break;
      case '>':
        if (matchChar('>')) {
          tt = matchChar('=') ? Token.ASSIGN : Token.SHOP;
        } else tt = Token.RELOP;
        break;
      case '*':
        tt = matchChar('=') ? Token.ASSIGN : Token.MULOP;
        break;
      case '/':
        if (matchChar('/')) {
          skipLine();
          return;
        }
        if (matchChar('*')) {
          while ((tt = cs.getChar()) != K3_EOF && !(ch as any == '*' && matchChar('/'))) {
            if (ch == '/' && matchChar('*')) {
              if (matchChar('/')) return;
            }
          }
          if (ch == K3_EOF) return;
        }
        tt = matchChar('=') ? Token.ASSIGN : Token.MULOP;
        break;
      case '%':
        tt = matchChar('=') ? Token.ASSIGN : Token.MULOP;
        break;
      case '~':
        tt = Token.UNARYOP;
        break;
      case '+':
      case '-':
        if (matchChar('=')) tt = Token.ASSIGN;
        else if (matchChar('-')) tt = Token.INCOP;
        else if (ch == '-') tt = Token.MINUS;
        else tt = Token.PLUS;
        break;
      default:
        return;
    }
    if (tt) {
      ch = cs.appendToTokenBuf(ch);
      captureToken(tt);
    }

    function matchChar(char) {
      let res = cs.matchChar(char);
      if (res) {
        ch = ch + char;
      }
      return res;
    }

    function unGetChar(char) {
      cs.unGetChar(char);
      ch = (ch as string).slice(0, ch.length - char.length);
    }
  }

  function captureToken(type: Token) {
    cs.token = {
      ...cs.token,
      type: type,
      text: cs.tokenBuf
    }
    cs.tokenBuf = "";
    tokens.push({ ...cs.token });
    cs.token = new K3_Token();
  }
}

function isSpace(ch: string) {
  if (!ch) return false;
  return /\s/.test(ch);
}

function isAlpha(ch: string) {
  if (!ch) return false;
  return /[a-zA-Z]/.test(ch);
}

function isDigit(ch: string) {
  if (!ch) return false;
  return /[0-9]/.test(ch);
}
