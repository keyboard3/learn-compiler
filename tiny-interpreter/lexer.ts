import { defineKeywords, EOF, Token, TokenType as TT, CharStream } from "./models";

/**
 * 词法分析器
 */
export function tokenize(code: string): Token[] {
  let tokens: Token[] = [];
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
    while ((ch = cs.getChar()) != EOF && ch != '\n') {
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
      } else captureToken(TT.NAME);
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
    if (ch == EOF) captureToken(TT.EOF);
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
      captureToken(TT.NUMBER);
    }
  }

  function matchString() {
    if (ch == '"' || ch == '\'') {
      const qc = ch;
      ch = cs.getChar();
      while (ch != qc) {
        if (ch == '\n' || ch == EOF) {
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
      captureToken(TT.STRING);
    }
  }

  function matchOther() {
    if (!ch) return;
    let tt;
    switch (ch) {
      case '\n':
        tt = TT.EOL;
        break;
      case ';':
        tt = TT.SEMI;
        break;
      case '[':
        tt = TT.LB;
        break;
      case ']':
        tt = TT.RB;
        break;
      case '{':
        tt = TT.LC;
        break;
      case '}':
        tt = TT.RC;
        break;
      case '(':
        tt = TT.LP;
        break;
      case ')':
        tt = TT.RP;
        break;
      case ',':
        tt = TT.COMMA;
        break;
      case '?':
        tt = TT.HOOK;
        break;
      case ':':
        tt = TT.COLON;
        break;
      case '.':
        tt = TT.DOT;
        break;
      case '|':
        if (matchChar('|')) tt = TT.OR;
        else if (matchChar('=')) tt = TT.ASSIGN;
        else tt = TT.BITOR;
        break;
      case '^':
        if (matchChar('=')) tt = TT.ASSIGN;
        else tt = TT.BITXOR;
        break;
      case '&':
        if (matchChar('&')) tt = TT.AND;
        else if (matchChar('=')) tt = TT.ASSIGN;
        else tt = TT.BITAND;
        break;
      case '=':
        if (matchChar('=')) tt = TT.EQOP;
        else tt = TT.ASSIGN;
        break;
      case '!':
        if (matchChar('=')) tt = TT.EQOP;
        else tt = TT.UNARYOP;
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
          tt = matchChar('=') ? TT.ASSIGN : TT.SHOP;
        } else tt = TT.RELOP;
        break;
      case '>':
        if (matchChar('>')) {
          tt = matchChar('=') ? TT.ASSIGN : TT.SHOP;
        } else tt = TT.RELOP;
        break;
      case '*':
        tt = matchChar('=') ? TT.ASSIGN : TT.MULOP;
        break;
      case '/':
        if (matchChar('/')) {
          skipLine();
          return;
        }
        if (matchChar('*')) {
          while ((tt = cs.getChar()) != EOF && !(ch as any == '*' && matchChar('/'))) {
            if (ch == '/' && matchChar('*')) {
              if (matchChar('/')) return;
            }
          }
          if (ch == EOF) return;
        }
        tt = matchChar('=') ? TT.ASSIGN : TT.MULOP;
        break;
      case '%':
        tt = matchChar('=') ? TT.ASSIGN : TT.MULOP;
        break;
      case '~':
        tt = TT.UNARYOP;
        break;
      case '+':
      case '-':
        if (matchChar('=')) tt = TT.ASSIGN;
        else if (matchChar('-')) tt = TT.INCOP;
        else if (ch == '-') tt = TT.MINUS;
        else tt = TT.PLUS;
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

  function captureToken(type: TT) {
    cs.token = {
      ...cs.token,
      type: type,
      text: cs.tokenBuf
    }
    cs.tokenBuf = "";
    tokens.push({ ...cs.token });
    cs.token = new Token();
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
