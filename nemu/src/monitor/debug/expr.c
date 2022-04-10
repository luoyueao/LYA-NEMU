#include "nemu.h"


#include <sys/types.h>
#include <regex.h>

enum {//表示token类型
  TK_NOTYPE = 256, TK_EQ, TK_TEN, TK_SIXTEEN, TK_REG, TK_UNEQ, TK_AND, TK_OR, TK_DEREF, TK_NEGATIVE

};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  //定义优先级

  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  {"==", TK_EQ},        // equal
  {"0[xX][0-9a-fA-F]+", TK_SIXTEEN},            // 十六进制数
  {"[0-9]+", TK_TEN},   // 十进制数
  {"\\$e[a-d]x|\\$e[bsi]p|\\$e[sd]i", TK_REG},  // 9个寄存器
  {"\\(", '('},         // 左括号
  {"\\)", ')'},         // 右括号
  {"\\-", '-'},         // 减
  {"\\*", '*'},         // 乘
  {"\\/", '/'},         // 除
  {"!=", TK_UNEQ},      // 不等于
  {"&&", TK_AND},       // 和
  {"\\|\\|", TK_OR},    // 或
  {"!", '!'}            // 非
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];


void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    //依次尝试每个规则
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

//        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
//            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;

        //识别出token

        if(rules[i].token_type != TK_NOTYPE) {
          tokens[nr_token].type = rules[i].token_type;  //先将识别的token的类型存入tokens数组(除了TK_NOTYPE类型)

          if(rules[i].token_type == TK_TEN || rules[i].token_type == TK_SIXTEEN || rules[i].token_type == TK_REG) {
            //对一些需要特定处理的token，将具体内容存入takens的str成员变量中
            strncpy(tokens[nr_token].str, substr_start, substr_len);
          }

          nr_token++; //已经被识别出的token数目+1
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

bool flag_parentheses;  //标志表达式括号匹配是否合法

bool check_parentheses(int p, int q) {
  bool flag = false;    //flag标志位，表示在统计过程中，居于首位的'('是否与非最后的')'匹配
  flag_parentheses = true;
  int left = 0, right = 0;
  if(tokens[p].type == '(')
    flag = true;        //若首位是'('，flag置true
  for( ; p <= q; p++) {
    if(tokens[p].type == '(')
      left++;           //左括号数加1
    else if(tokens[p].type == ')')
      right++;          //右括号数加1
    if(right > left) {
      flag_parentheses = false;
      return false;     //右括号数大于左括号数，则非法
    }
    else if(right == left && p != q)
      flag = false;     //未到结尾，左右括号数全匹配，flag置false
  }
  if(left == right) {   //若左右括号数相等，则合法
    if(flag)
      return true;      //表达式被括号包围
    else
      return false;     //表达式未被括号包围
  }
  else {                //若左右括号数不相等，则非法
    flag_parentheses = false;
    return false;
  }
}

uint32_t find_dominated_op(int p, int q) {
  int op = -1, parentheses = 0;   //op表示中心操作符的下标，parentheses表示未匹配的括号数
  for( ; p <= q; p++) {
    if(tokens[p].type == '(') {   //遇到'('，parentheses加1
      parentheses++;
    }
    else if(tokens[p].type == ')') {//遇到')'，parentheses减1
      parentheses--;
    }
    else if(!parentheses) {       //若该位置未被括号包围，则继续判断，否则跳过
      if(tokens[p].type == TK_TEN || tokens[p].type == TK_SIXTEEN || tokens[p].type == TK_REG) {
        continue;
      }
      else {
        if(op == -1) {            //若该位不是数值并且op还未标识，则直接让op指向该位置
          op = p;
          continue;
        }
      }
      if(tokens[p].type == TK_OR) //若该位运算符是"||"，则优先级一定小于等于op
        op = p;
      else if(tokens[p].type == TK_AND) {   //若该位运算符是"&&"
        if(tokens[op].type != TK_OR)        //若op不是"||"，则op指向该位置
          op = p;
      }
      else if(tokens[p].type == TK_EQ || tokens[p].type == TK_UNEQ) { //若该位运算符是"=="或"!="
        if(tokens[op].type != TK_OR && tokens[op].type != TK_AND)     //若op不是"||"和"&&"，则op指向该位置
          op = p;
      }
      else if(tokens[p].type == '+' || tokens[p].type == '-') {   //若该位运算符是'+'或'-'
        if(tokens[op].type != TK_OR && tokens[op].type != TK_AND && tokens[op].type != TK_EQ && tokens[op].type != TK_UNEQ)
          op = p;                                                 //若op不是"||"和"&&"和"=="和"!="，则op指向该位置
      }
     else if(tokens[p].type == '*' || tokens[p].type == '/') {   //若该位运算符是'*'或'/'
        if(tokens[op].type == '*' || tokens[op].type == '/' || tokens[op].type == '!' || tokens[op].type == TK_DEREF || tokens[op].type == TK_NEGATIVE)
          op = p;                                                 //若op是'*'或'/'或'!'或解引用或负号，则op指向该位置
    }
  }
 }
 return op;
}

uint32_t eval(int p, int q) {
    if (p > q) {                              //头在尾后，表达式有误，返回false
      printf("Bad expression!\n");
      return false;
    }
    else if (p == q) {                        //p、q指向同一个token，返回其值
      int value;
      if(tokens[p].type == TK_TEN) {          //若token是十进制数
        sscanf(tokens[p].str, "%d", &value);
      }
      else if(tokens[p].type == TK_SIXTEEN) { //若token是十六进制数
        sscanf(tokens[p].str, "%x", &value);
      }
      else if(tokens[p].type == TK_REG) {     //若token是寄存器
        char str[4] = {};
        strncpy(str, tokens[p].str + 1, 3);   //取出寄存器名
        if(!strcmp(str, "eip")) {             //判断寄存器是否为"eip"
          value = cpu.eip;
        }
        else {
          for(int i = 0; i < 8; i++)          //判断寄存器是否为"eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi"
            if(!strcmp(str, regsl[i])) {
              value = cpu.gpr[i]._32;
              break;
            }
        }
      }
      return value;                           //返回值
    }
    else if (check_parentheses(p, q) == true) {//若表达式被括号包围，则去括号
        return eval(p + 1, q - 1);
    }
    else {                                    //p<q，则拆分表达式
        if(!flag_parentheses) {
          printf("Mismatched parentheses!\n");
          return false;
        }
        int op, val1 = 0, val2 = 0;
        op = find_dominated_op(p, q);         //找到中心操作符，分割表达式
       	if(tokens[op].type != TK_DEREF && tokens[op].type != TK_NEGATIVE && tokens[op].type != '!')
          val1 = eval(p, op - 1);             //当中心操作符不是一元运算符时
        val2 = eval(op + 1, q);
        switch (tokens[op].type) {            //根据中心操作符种类，运算对分割的两部分运算
            case '+': return val1 + val2;
            case '-': return val1 - val2;
            case '*': return val1 * val2;
            case '/': return val1 / val2;
            case TK_NEGATIVE: return -val2;
            case TK_DEREF: return vaddr_read(val2, 4);
	    case '!': return !val2;
	    case TK_AND: return val1 & val2;
            case TK_OR: return val1 | val2;
            case TK_EQ: return val1 == val2;
            case TK_UNEQ: return val1 != val2;
            default: assert(0);
        }
    }
}

uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  //计算表达式的值
  for(int i = 0; i < nr_token; i++) {
    if (tokens[i].type == '*' && (i == 0 || (tokens[i - 1].type != TK_TEN && tokens[i - 1].type != TK_SIXTEEN && tokens[i - 1].type != TK_REG && tokens[i - 1].type != ')'))) {  //'*'前不是操作数和右括号，则'*'是解引用
        tokens[i].type = TK_DEREF;
    }
    if (tokens[i].type == '-' && (i == 0 || (tokens[i - 1].type != TK_TEN && tokens[i - 1].type != TK_SIXTEEN && tokens[i - 1].type != TK_REG && tokens[i - 1].type != ')'))) {  //'-'前不是操作数和右括号，则'-'是负号
        tokens[i].type = TK_NEGATIVE;
    }
  }
  
  return eval(0, nr_token - 1);
}
