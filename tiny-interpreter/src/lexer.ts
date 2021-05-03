import { defineKeywords, K3_EOF, K3_Token, K3_TokenType as Token, OP_TYPE as OP, CharStream } from "./models";

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
      // cs.unGetChar(ch);
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
        // captureToken(Token.EOF);
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
      // cs.unGetChar(ch);
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
            case 'b': ch = '\b'; break;
            case 'f': ch = '\f'; break;
            case 'n': ch = '\n'; break;
            case 'r': ch = '\r'; break;
            case 't': ch = '\t'; break;
            case 'v': ch = '\v'; break;
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
          // cs.appendToTokenBuf(ch);
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
      case '\n': tt = Token.EOL; break;
      case ';': tt = Token.SEMI; break;
      case '[': tt = Token.LB; break;
      case ']': tt = Token.RB; break;
      case '{': tt = Token.LC; break;
      case '}': tt = Token.RC; break;
      case '(': tt = Token.LP; break;
      case ')': tt = Token.RP; break;
      case ',': tt = Token.COMMA; break;
      case '?': tt = Token.HOOK; break;
      case ':': tt = Token.COLON; break;
      case '.': tt = Token.DOT; break;
      case '|':
        if (cs.matchChar('|')) tt = Token.OR;
        else if (cs.matchChar('=')) {
          cs.token.atom.op = OP.BITOR;
          tt = Token.ASSIGN;
        } else {
          tt = Token.BITOR;
        }
        break;
      case '^':
        if (cs.matchChar('=')) {
          cs.token.atom.op = OP.BITXOR;
          tt = Token.ASSIGN;
        } else {
          tt = Token.BITXOR;
        }
        break;
      case '&':
        if (cs.matchChar('&')) tt = Token.AND;
        else if (cs.matchChar('=')) {
          cs.token.atom.op = OP.BITAND;
          tt = Token.ASSIGN;
        } else {
          tt = Token.BITAND;
        }
        break;
      case '=':
        if (cs.matchChar(ch)) {
          cs.token.atom.op = OP.EQ;
          tt = Token.EQOP;
        } else {
          cs.token.atom.op = OP.NOP;
          tt = Token.ASSIGN;
        }
        break;
      case '!':
        if (cs.matchChar('=')) {
          cs.token.atom.op = OP.NE;
          tt = Token.EQOP;
        } else {
          cs.token.atom.op = OP.NOT;
          tt = Token.UNARYOP;
        }
        break;
      case '<':
        /* XXX treat HTML begin-comment as comment-till-end-of-line */
        if (cs.matchChar('!')) {
          if (cs.matchChar('-')) {
            if (cs.matchChar('-')) skipLine()
            cs.unGetChar('-');
          }
          cs.unGetChar('!');
        }
        if (cs.matchChar(ch)) {
          cs.token.atom.op = OP.LSH;
          tt = cs.matchChar('=') ? Token.ASSIGN : Token.SHOP;
        } else {
          cs.token.atom.op = cs.matchChar('=') ? OP.LE : OP.LT;
          tt = Token.RELOP;
        }
        break;
      case '>':
        if (cs.matchChar(ch)) {
          cs.token.atom.op = cs.matchChar(ch) ? OP.URSH : OP.RSH;
          tt = cs.matchChar('=') ? Token.ASSIGN : Token.SHOP;
        } else {
          cs.token.atom.op = cs.matchChar('=') ? OP.GE : OP.GT;
          tt = Token.RELOP;
        }
        break;
      case '*':
        cs.token.atom.op = OP.MUL;
        tt = cs.matchChar('=') ? Token.ASSIGN : Token.MULOP;
        break;
      case '/':
        if (cs.matchChar('/')) {
          skipLine();
          return;
        }
        if (cs.matchChar('*')) {
          while ((tt = cs.getChar()) != K3_EOF && !(ch as any == '*' && cs.matchChar('/'))) {
            if (ch == '/' && cs.matchChar('*')) {
              if (cs.matchChar('/')) return;
              // ReporcsyntaxError(mc, cs, "nested comment");
            }
          }
          if (ch == K3_EOF) return;
          // ReporcsyntaxError(mc, cs, "unterminated comment");
        }
        cs.token.atom.op = OP.DIV;
        tt = cs.matchChar('=') ? Token.ASSIGN : Token.MULOP;
        break;
      case '%':
        cs.token.atom.op = OP.MOD;
        tt = cs.matchChar('=') ? Token.ASSIGN : Token.MULOP;
        break;
      case '~':
        cs.token.atom.op = OP.BITNOT;
        tt = Token.UNARYOP;
        break;
      case '+':
      case '-':
        if (cs.matchChar('=')) {
          cs.token.atom.op = (ch == '+') ? OP.ADD : OP.SUB;
          tt = Token.ASSIGN;
        } else if (cs.matchChar(ch)) {
          cs.token.atom.op = (ch == '+') ? OP.INC : OP.DEC;
          tt = Token.INCOP;
        } else if (ch == '-') {
          cs.token.atom.op = OP.NEG;
          tt = Token.MINUS;
        } else {
          tt = Token.PLUS;
        }
        break;

      default:
        // ReporcsyntaxError(mc, cs, "illegal character");
        return;
    }
    if (tt) {
      ch = cs.appendToTokenBuf(ch);
      captureToken(tt);
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