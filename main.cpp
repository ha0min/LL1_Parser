//
// Created by Max Cheng on 2020/10/10.
//

#include <iostream>
#include <vector>
#include <algorithm>
#include <cstring>
#include <string>
#include <iomanip>
#include <functional>

#define tableWidth 20//打印宽度
#define maxNum 50
#define bufSize 300
char sentence[bufSize];
using namespace std;

/*--------------------
 * ------变量声明-------
 * ------------------*/

string grammar[maxNum];//产生式存放数组，不含公共左因子、不含左递归
char startChar;//开始符号, 默认第一个文法的第一个字母为开始符号
int numOfProd;//产生式个数
int numOfNonTer;//非终结符的数量
int numOfTerm;//终结符的数量
vector<char> nonTerminator;//所有的非终结符
vector<char> terminator;//所有的终结符
vector<char> firstSetOfProd[maxNum];//每一个产生式的 first 集合，注意 '$' '@'
vector<char> firstSetOfNonTer[maxNum];//每一个非终结符的 first 集合，注意 '$' '@'
vector<char> followSet[maxNum];//每一个非终结符的 followSet 集合
int analyseTable[maxNum][maxNum];//预测分析表，存放产生式数组下标，没有的地方设置为 -1


// 保存需将产生式左边非终结符（索引为第一个 int ）的 Follow 集
// 添加至索引为第二个 int 的非终结符的 Follow 集情况
vector<pair<int, int> > toAddLeftPartFollow;

int preSizeOfFollowSet[maxNum];//之前求解的 FollowSet 大小


/*--------------------
 * ------函数声明-------
 * ------------------*/

//是否为非终结符
bool isNonTerminator(char var);

//是否为终结符
bool isTerminator(char var);

//检测一个字符是否为空字
bool isEmptySymbol(char var);

//查询 字符 x 在某个 vector<char> 中的下标，没有则返回 -1
int indexOf(const vector<char>& v, char x);

// 查询 x 在 nonTerSet 中下标
int indexOfNonTerSet(char x);

// 查询 x 在 TerSet 中下标
int indexOfTerSet(char x);

// 向 v 中添加元素，如果已经存在则不添加
void addUniqueItem(vector<char>& v, char c);

// 读取文法
void inputGrammar();

// 提取文法中的非终结符和终结符
void analyseGrammar();

// 单个非终结符的 first 集
void getFirstSetOfSingleNonTer(char nonTer);

// 求所有非终结符的 First 集
void getFirstSetOfNonTer();

// 求所有产生式右部的 First 集
void getFirstSetOfProd();

// 根据产生式和式中对应位置求指定元素的 First 集
vector<char> returnFirstSetOfIdentifier(int i, int pos);

// 求单个元素 nonTer 的 Follow 集 followSetOfNonTer
void getSingleFollow(char nonTer, vector<char> &followSetOfNonTer);

// FollowSet 大小是否变化
bool isSameSize();

// 将 b 中元素全部添加到 a 中
void unionSetTo(vector<char>& b, vector<char>& a);

//获取全部的 Follow 集
void getFollow();

//生成预测分析表
void generateTable();

//打印当前状态
void printCurrentAnalyse(const char* s1, const char* s2, const string& s3);

// 分析函数, 如果成功分析，输出 分析动作
void analyze(const char* sentence, int len);

//打印所有终结符
void printAllTerminator();

//打印所有非终结符
void printAllNonTerminator();

//打印 FirstSetOfNonTer
void printFirstSetOfNonTer();

//打印 FirstOfProd
void printFirstSetOfProd();

//打印 FollowSet
void printFollowSet();

//打印预测分析表
void printAnalyzeTable();

int main() {
    cout << "请输入文法，每行一个，在行首按 Ctrl+Z 结束。第一个文法的首字符将被视为开始符号" << endl;
    inputGrammar();
    analyseGrammar();
    getFirstSetOfNonTer();
    getFirstSetOfProd();
    getFollow();
    generateTable();
    printAllTerminator();
    printAllNonTerminator();
    printFirstSetOfNonTer();
    printFirstSetOfProd();
    printFollowSet();
    printAnalyzeTable();
    cout << "---句子测试---" << endl;
    while (true) {
        cout << "请输入一个句子：（中间不含空格，回车结束, 退出请输入 quit）" << endl;
        cin >> sentence;
        if (strcmp("quit", sentence) == 0)
            break;
        analyze(sentence, -1);
    }

}
/*--------------------
 * ------函数实现------ -
 * ------------------*/


bool isNonTerminator(char var) {
    return indexOf(nonTerminator,var) != -1 ? true : false;
}

bool isTerminator(char var) {
    return indexOf(terminator,var) != -1 ? true : false;
}

bool isEmptySymbol(char var) {
    if(var == '@'){
        return true;
    }
    else{
        return false;
    }
}

int indexOf(const vector<char> &v, char x) {
    vector<char>::const_iterator it = find(v.begin(), v.end(), x);
    return it == v.end() ? -1 : it - v.begin();
}

int indexOfNonTerSet(char x) {
    return indexOf(nonTerminator, x);
}

int indexOfTerSet(char x) {
    return indexOf(terminator, x);
}

void addUniqueItem(vector<char> &v, char c) {
    if (indexOf(v, c) == -1)
        v.push_back(c);
}

void inputGrammar() {
    numOfProd = 0;
    string str;
    while (cin >> str){
        grammar[numOfProd] = str;
        numOfProd++;
    }

    cin.clear();
}

void analyseGrammar() {
    bool pushed[256] = {0};//是否已经分析过

    //产生式左部的处理，全是非终结符
    for (int prod = 0; prod < numOfProd; ++prod) {
        char leftPart = grammar[prod][0];

        if (!pushed[(int)leftPart]) {
            nonTerminator.push_back(leftPart);
            pushed[(int)leftPart] = true;
        }
    }

    //产生式右部的处理，不是非终结符的都是终结符
    for (int prod = 0; prod < numOfProd; prod++) {
        int n = grammar[prod].length();

        for (int symbol = 3; symbol < n; symbol++) {
            char rightPart = grammar[prod][symbol];

            if (!pushed[(int)rightPart]) {
                terminator.push_back(rightPart);
                pushed[(int)rightPart] = true;
            }
        }
    }

    terminator.push_back('$');

    numOfNonTer = nonTerminator.size();
    numOfTerm = terminator.size();

    startChar = grammar[0][0];
}

void getFirstSetOfSingleNonTer(char nonTer) {

    int indexOfNonTer = indexOfNonTerSet(nonTer);

    //遍历产生式
    for (int prod = 0; prod < numOfProd; prod++) {
        if (grammar[prod][0] != nonTer) {
            continue;
        }

        // 是否继续向后检查
        bool keepScan = true;

        //identifier 表示产生式后一个字符
        for (char identifier : grammar[prod].substr(3)) {

            if (!keepScan) {
                break;
            }

            //如果是终结符，直接将 identifier 添加进 First 集后终止
            if (isTerminator(identifier)) {
                addUniqueItem(firstSetOfNonTer[indexOfNonTer], identifier);
                keepScan = false;
            }
                //如是非终结符，将该非终结符 identifier 的 First 集添加
            else {
                keepScan = false;

                // 添加之前，递归求解一下 identifier 的 First 集
                getFirstSetOfSingleNonTer(identifier);
                for (char b : firstSetOfNonTer[indexOfNonTerSet(identifier)]) {
                    if (b == '@')
                        keepScan = true;
                    else
                        addUniqueItem(firstSetOfNonTer[indexOfNonTer], b);
                }
            }
        }

        // 如果最后 keepScan 仍为 true, 则说明 右部可空
        if (keepScan) {
            addUniqueItem(firstSetOfNonTer[indexOfNonTer], '@');
        }
    }
}

void getFirstSetOfProd() {
    for (int prod = 0; prod < numOfProd; prod++) {

        bool keepScan = true; // 是否继续向后检查

        // 产生式右部的 First 集
        // identifier 表示产生式后一个字符
        for (char identifier : grammar[prod].substr(3)) {
            if (!keepScan)
                break;

            //如果是终结符，直接将 identifier 添加进 First 集后终止
            if (isTerminator(identifier)) {
                addUniqueItem(firstSetOfProd[prod], identifier);
                keepScan = false;
            }

                //后面是非终结符，将非终结符 identifier 的 First 集添加
            else {
                keepScan = false;
                for (char b : firstSetOfNonTer[indexOfNonTerSet(identifier)]) {
                    if (b == '@')
                        keepScan = true;
                    else
                        addUniqueItem(firstSetOfProd[prod], b);
                }
            }
        }
        // 如果最后 keepScan 仍为 true, 则说明 产生式 右部可空
        if(keepScan)
            addUniqueItem(firstSetOfProd[prod], '@');
    }
}

void getFirstSetOfNonTer() {
    for_each(nonTerminator.begin(), nonTerminator.end(), getFirstSetOfSingleNonTer);
    /*for (auto i : nonTerminator)
    {
        getFirstSetOfSingleNonTer(i);
    }*/
}

vector<char> returnFirstSetOfIdentifier(int i, int pos) {
    // 结果串
    vector<char> res;
    // 是否继续向后检查
    bool keepScan = true;

    for (char c : grammar[i].substr(pos)) {
        if (!keepScan)
            break;

        if (isTerminator(c)) {
            addUniqueItem(res, c);
            keepScan = false;
        }

        else {
            keepScan = false;
            for (char b : firstSetOfNonTer[indexOfNonTerSet(c)]) {
                if (b == '@')
                    keepScan = true;
                else
                    addUniqueItem(res, b);
            }
        }
    }

    // 如果最后 keepScan 仍为 true, 则说明 产生式右部可空
    if (keepScan)
        addUniqueItem(res, '@');

    return res;
}

void getSingleFollow(char nonTer, vector<char> &followSetOfNonTer) {
    // 非终结符 nonTer 在非终结符集中的位置
    int indexNonTer = indexOfNonTerSet(nonTer);

    // 遍历所有规则
    for (int prod = 0; prod < numOfProd; prod++) {
        //当前规则
        string& curProd = grammar[prod];
        //当前规则长度
        int lenOfCurProd = curProd.length();

        //产生式左部非终结符在非终结符集的位置
        int indexLeft = indexOfNonTerSet(curProd[0]);

        //单规则求 Follow 开始
        //pos 为产生式指定元素右侧标识符的位置
        for (int pos = 3; pos < lenOfCurProd; pos++) {
            // 移动到 nonTer 的位置
            if (curProd[pos] != nonTer)
                continue;

            bool haveEmpty = false;

            // 获取后一个符号的 First 集
            auto firstSetOfNextID = returnFirstSetOfIdentifier(prod, pos + 1);


            // 遍历 First 集所有元素
            for (auto d : firstSetOfNextID) {
                //isEmptySymbol，存在为空情况
                if (d == '@')
                    haveEmpty = true;
                else
                    //将 first 集，除空以外元素，添加至目标 Follow 集中
                    addUniqueItem(followSetOfNonTer, d);
            }



            //若Ch 右部第一个非终结符存在空，且不为产生式左部
            if (haveEmpty && indexLeft != indexNonTer)
                //将产生式左部的Follow 集添加至目标 Follow 集
                toAddLeftPartFollow.push_back(make_pair(indexLeft, indexNonTer));
        }
    }
}

bool isSameSize() {
    for (int i = 0;i < numOfNonTer;i++) {
        if (preSizeOfFollowSet[i] != followSet[i].size())
            return false;
    }
    return true;
}

void unionSetTo(vector<char> &b, vector<char> &a) {
    for (char var : b) {
        addUniqueItem(a, var);
    }
}

void getFollow() {

    //开始符号的 Follow 添加'$'符
    followSet[indexOfNonTerSet(startChar)].push_back('$');

    //重置向量
    toAddLeftPartFollow.clear();

    //遍历所有非终结符的 Follow 集
    for (int i = 0;i < numOfNonTer;i++) {
        getSingleFollow(nonTerminator[i], followSet[i]);
    }

    do {
        for (int i = 0;i < numOfNonTer;i++) {
            preSizeOfFollowSet[i] = followSet[i].size();
        }

        for (auto p : toAddLeftPartFollow) {
            unionSetTo(followSet[p.first], followSet[p.second]);
        }
    } while (!isSameSize()); //只要不一致，就重复
}

void generateTable() {

    //初始化所有为-1
    memset(analyseTable, -1, sizeof analyseTable);

    for (int prod = 0; prod < numOfProd; prod++) {
        // 标记 firstP[prod] 中是否含 '@'
        bool hasEmpty = false;

        // 第 prod 个产生式的左部非终结符在 nonT 中下标
        int leftPart = indexOfNonTerSet(grammar[prod][0]);

        for (char idInFirstSet : firstSetOfProd[prod]) {
            // 分析表中不用添加 @ 这一列，因为用不到
            if (idInFirstSet == '@') {
                hasEmpty = true;
            }
            else {
                int term = indexOfTerSet(idInFirstSet);
                // 将产生式在 grammar 中的下标填入分析表中
                analyseTable[leftPart][term] = prod;
            }
        }

        // 如果有 “空”，那么还要 加入 followSet[leftPart] 中的每个终结符
        if (hasEmpty) {
            // 注：follow 中不可能含有 '@'
            for (char idInFirstSet : followSet[leftPart]) {
                int term = indexOfTerSet(idInFirstSet);
                analyseTable[leftPart][term] = prod; // 将产生式 在 grammar 中的下标 填入分析表中
            }
        }
    }
}

void analyze(const char *sentence, int len) {
    char analyzeSentence[300];
    strcpy(analyzeSentence, sentence);

    if (len == -1)
        len = strlen(analyzeSentence);

    // 在分析串末尾加入 '$'
    analyzeSentence[len++] = '$';
    analyzeSentence[len] = 0;

    char identifierStack[200]; //数组模拟栈
    int stackPos = 0;

    // 向栈中加入'$'和开始符号
    identifierStack[stackPos++] = '$';
    identifierStack[stackPos++] = startChar;

    cout << setw(tableWidth) << "栈" << setw(tableWidth) << "输入" << setw(tableWidth) << "动作" << endl;

    //打印分析栈、剩余分析串、使用的产生式
    identifierStack[stackPos] = 0;
    printCurrentAnalyse(identifierStack, analyzeSentence, string("初始状态"));

    //开始分析
    for (int sentencePos = 0; sentencePos < len;) {
        // 退栈
        char top = identifierStack[--stackPos];
        identifierStack[stackPos] = 0;

        // 栈顶元素为空时
        if (top == '$') {
            //  分析串也空，分析结束
            if (analyzeSentence[sentencePos] == '$') {
                printCurrentAnalyse("$", "$", string("接受"));
                cout << "分析成功！" << endl;
                break;
            } else {
                // 分析串仍有字符，分析失败
                printCurrentAnalyse(identifierStack, analyzeSentence + sentencePos, string("错误！"));
                cerr << "分析栈栈顶为'$'，但是输入尚未结束。" << endl;
                return;
            }
        }

        // 栈存在元素且为终结符
        if (isTerminator(top)) {
            // 栈顶元素和字符串指针匹配
            if (top == analyzeSentence[sentencePos]) {
                sentencePos++; // 当前字符匹配成功，指针后移
                string res("匹配 ");
                //打印
                printCurrentAnalyse(identifierStack, analyzeSentence + sentencePos, res + top);
            } else {
                // 匹配失败
                printCurrentAnalyse(identifierStack, analyzeSentence + sentencePos, string("错误！"));
                cerr << "分析栈栈顶为终结符 '" << top << "'， 但是输入符号为 '" << analyzeSentence[sentencePos] << "'" << endl;
                return;
            }
        }
            // 栈存在元素且为非终结符
        else if (isNonTerminator(top)) {

            // 预测分析表中对应终结符的产生式
            int production = analyseTable[indexOfNonTerSet(top)][indexOfTerSet(analyzeSentence[sentencePos])];

            // 不为-1，则存在对应解法
            if (production != -1) {
                // 右边是空串，则不操作，将非终结符出栈
                if (grammar[production][3] != '@') {
                    // 否则将产生式中右部内容倒序入栈
                    for (int i = grammar[production].size() - 1; i >= 3; i--) {
                        identifierStack[stackPos++] = grammar[production][i];
                    }
                    identifierStack[stackPos] = 0;
                }
                // 栈顶元素替换成功
                printCurrentAnalyse(identifierStack, analyzeSentence + sentencePos, grammar[production]);
            } else {
                // 出错，无对应解法
                printCurrentAnalyse(identifierStack, analyzeSentence + sentencePos, string("错误！"));
                cerr << "分析表 table[" << top << "][" << analyzeSentence[sentencePos] << "] 项为空。" << endl;
                return;
            }
        } else {
            cerr << "解析错误！" << endl;
            return;
        }
    }
}

void printCurrentAnalyse(const char *s1, const char *s2, const string &s3) {
    cout << setw(tableWidth) << s1
         << setw(tableWidth) << s2
         << setw(tableWidth) << s3 << endl;
}

void printAllTerminator() {
    cout << "终结符数量  ：" << numOfTerm << " 分别为:{";
    for (char c : terminator) {
        cout << " " << c;
    }
    cout << " }" << endl;
}

void printAllNonTerminator() {
    cout << "非终结符数量：" << numOfNonTer << " 分别为:{";
    for (char c : nonTerminator) {
        cout << " " << c;
    }
    cout << " }" << endl;
}

void printFirstSetOfNonTer() {
    cout << "所有非终结符的 First 集：" << endl;
    for (int i = 0;i < numOfNonTer;i++) {
        cout << "FIRST(" << nonTerminator[i] << ") = {";
        for (char fi : firstSetOfNonTer[i]) {
            cout << " " << fi;
        }
        cout << " }" << endl;
    }
    cout << endl;
}

void printFirstSetOfProd() {
    cout << "所有产生式右部的 First 集：" << endl;
    for (int i = 0;i < numOfProd;i++) {
        cout << "FIRST(" << grammar[i].substr(3) << ") = {";
        for (char fi : firstSetOfProd[i]) {
            cout << " " << fi;
        }
        cout << " }" << endl;
    }
    cout << endl;
}

void printFollowSet() {
    cout << "所有非终结符的 Follow 集：" << endl;
    for (int i = 0;i < numOfNonTer;i++) {
        cout << "Follow(" << nonTerminator[i] << ") = {";
        for (char fi : followSet[i]) {
            cout << " " << fi;
        }
        cout << " }" << endl;
    }
    cout << endl;
}

void printAnalyzeTable() {
    int output_width = 10;
    cout << "预测分析表：" << endl;
    cout << setw(output_width) << " ";
    for (char char_term : terminator) {
        if (char_term == '@')
            continue;
        cout << setw(output_width) << char_term;
    }
    cout << endl;
    for (int i = 0;i < numOfNonTer;i++) {
        cout << setw(output_width) << nonTerminator[i];
        for (int j = 0;j < numOfTerm;j++) {
            if (terminator[j] == '@')
                continue;
            if(analyseTable[i][j] == -1)
                cout << setw(output_width) << " ";
            else
                cout << setw(output_width) << grammar[analyseTable[i][j]];
        }
        cout << endl;
    }
}