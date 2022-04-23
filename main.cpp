#include <iostream>
#include <vector>
#include <cmath>
using namespace std;

const char let = 'L';
const char const_code = 'C';
const char print = ';';
const char number = '8';
const char name = 'a';
const char declKey = '#';
const char exitcode = 'q';

const char assigncode = 'A';
const char help = 'H';

const string help_str = "h";
const string assign_str = "A";
const string quit = "q";
const string const_str = "const";

void error(string s){
    throw runtime_error(s);
}

class Token {
public:
    char kind;
    double value;
    string name;
    bool isConst;
    Token(char ch) :kind(ch), value(0) { }
    Token(char ch, double val) :kind(ch), value(val) { }
    Token(char ch, string name) :kind(ch), name(name){};
};

class Token_stream {
    bool full;
    Token buffer;
public:
    Token_stream() :full(0), buffer(0) { }

    Token get();
    void unget(Token t) { buffer = t; full = true; }

    void ignore(char);
};


Token Token_stream::get()
{
    if (full) { full = false; return buffer; }
    char ch;
    cin >> ch;
    switch (ch) {
    case '(':
    case ')':
    case '+':
    case '-':
    case '*':
    case '/':
    case '%':
    case ';':
    case '=':
        return Token(ch);
    case '.':
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    {    cin.putback(ch);
    double val;
    cin >> val;
        
    //if (val != int(val)) error("yeni kural. bundan sonra sadece int kullancaksınız. nokta"); //allowing only integers.
        
    return Token(number, val);
    }
    case declKey:
            return Token(let);
    default:
        if (isalpha(ch)) {
            string s;
            s += ch;
            while (cin.get(ch) && (isalpha(ch) || isdigit(ch) || ch == '_')) s += ch;
            cin.putback(ch);
            if (s == quit) return Token(exitcode);
            if(s == const_str) return Token(const_code);
            if(s == help_str) return Token(help);
            if(s == assign_str) return Token(assigncode);
            
            return Token(name, s);
        }
            cout<<"bad token: "<<ch<<endl;
        error("Bad token");
            return Token('0');
    }
}

void Token_stream::ignore(char c)
{
    if (full && c == buffer.kind) {
        full = false;
        return;
    }
    full = false;

    char ch;
    while (cin >> ch)
        if (ch == c) return;
}

struct Variable {
    string name;
    double value;
    bool isConst;
    Variable(string n, double v,bool b) :name(n), value(v),isConst(b) { }
};


class Symbol_table{
public:
    double get(string s);
    void set(string s, double d);
    bool is_declared(string s);
    void decleare(string name, double val, bool isCons);
private:
    vector<Variable> var_table;
};

double Symbol_table::get(string s){
    for (int i = 0; i < var_table.size() ; ++i) {
        if (var_table[i].name == s) {
            return var_table[i].value;
        }
    }
    error("cannot find variable");
    return -1;
}
bool Symbol_table::is_declared(string s){
    for (int i = 0; i < var_table.size(); ++i) {
        if(var_table[i].name == s) return true;
    }
    return false;
}

void Symbol_table::set(string s, double d){
    for (int i = 0; i < var_table.size(); ++i) {
        if(var_table[i].name == s){
            if(var_table[i].isConst == true) error("cannot assign to the const variables");
            var_table[i].value = d;
            return;
        }
    }
}

void Symbol_table::decleare(string name, double val, bool isConst){
    var_table.push_back(Variable(name,val,isConst));
}

Symbol_table table;

Token_stream ts;

void help_func(){
    cout<<"-exit- for quit. -;- for see the calculate the expression."<<endl;
    cout<<"-#- for declare variable.  -A- for assigment. -const- for declare constant variables."<<endl;
}

double expression();

double primary()
{
    Token t = ts.get();
    switch (t.kind) {
    case '(':
    {
    double d = expression();
    t = ts.get();
    if (t.kind != ')') error("'(' expected");
        return d;
    }
    case '-':
        return -primary();
    case '+':
        return primary();
    case number:
        return t.value;
    case name:
        return table.get(t.name);
    default:
        cout<<" t kind = "<<t.kind<<endl;
        error("primary expected");
            return -1;
    }
}

double term()
{
    double left = primary();
    while (true) {
        Token t = ts.get();
        switch (t.kind) {
        case '*':
            left *= primary();
            break;
        case '/':
        {    double d = primary();
        if (d == 0) error("divide by zero");
        left /= d;
        break;
        }
        default:
            ts.unget(t);
            return left;
        }
    }
}

double expression()
{
    double left = term();
    while (true) {
        Token t = ts.get();
        switch (t.kind) {
        case '+':
            left += term();
            break;
        case '-':
            left -= term();
            break;
        default:
            ts.unget(t);
            return left;
        }
    }
}

double declaration(bool isC)
{
    Token t = ts.get();
    if (t.kind != name) error("name expected in declaration");
    string name = t.name;
    if (table.is_declared(name)) error(" declared twice");
    
    Token t2 = ts.get();
    if (t2.kind != '=') error("= missing in declaration of ");
    double d = expression();
    
    if(!isC) table.decleare(name,d,false);
    else table.decleare(name, d, true);
    
    return d;
}

double assigment(){
    Token t = ts.get();
    if(t.kind != name || !table.is_declared(t.name)) error("assigment error");
    Token t2 = ts.get();
    if(t2.kind != '=') error("assigment error");
    double d = expression();
    table.set(t.name, d);
    return d;
}

double statement()
{
    Token t = ts.get();
    switch (t.kind) {
    case let:
        return declaration(false);
    case const_code:
        return declaration(true);
    case assigncode:
        return assigment();
    case help:
        help_func();
        return 0;
    default:
        ts.unget(t);
        return expression();
    }
}

void clean_up_mess()
{
    ts.ignore(print);
}

const string prompt = "> ";
const string result = "= ";

void calculate()
{
    cout<<"press h for help"<<endl;
    while (true) try {
        cout << prompt;
        Token t = ts.get();
        while (t.kind == print) t = ts.get();
        if (t.kind == exitcode) return;
        ts.unget(t);
        cout << result << statement() << endl;
    }
    catch (runtime_error& e) {
        cerr << e.what() << endl;
        clean_up_mess();
    }
}

int main()
try {
    table.decleare("pi", 3.145143, true);
    table.decleare("e", 2.34354, true);
    cout<<"type: "<<endl;
    calculate();
    return 0;
}
catch (exception& e) {
    cerr << "exception: " << e.what() << endl;
    char c;
    while (cin >> c && c != ';');
    return 1;
}
catch (...) {
    cerr << "exception\n";
    char c;
    while (cin >> c && c != ';');
    return 2;
}
