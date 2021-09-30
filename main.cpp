#include <iostream>
#include <string>
#include <cctype>
#include <stdexcept>
#include <utility>
#include "forward_list.hpp"
#include "dynamic_array.hpp"
#include "stack.hpp"


std::string editStr(const std::string& str)
{
    // remove spaces
    std::string result;
    for (size_t i = 0; i < str.size(); i++)
        if (str[i] != ' ')
            result = result + str[i];

    // move uppercase letters to lowercase letters
    for (size_t i = 0; i < result.size(); i++)
        result[i] = std::tolower(result[i]);

    return result;
}

class StrHandler
{
public:
    virtual ~StrHandler(){}
    virtual bool isMine(const std::string& str) = 0;
};
class OperationHandler: public StrHandler
{
public:
    OperationHandler(std::string&& str): _standart(std::move(str))
    {}
    ~OperationHandler() {}
    bool isMine(const std::string& str)
    {
        return _standart.find(str) != std::string::npos;
    }
private:
    std::string _standart;
};
class NumberHandler: public StrHandler
{
public:
    ~NumberHandler() {}
    bool isMine(const std::string& str)
    {
        for (size_t i = 0; i < str.size(); i++)
            if (!(str[i] >= '0' && str[i] <= '9'))
                return false;

        return true;
    }
};

class SyntaxAnalyzer
{
    typedef std::unique_ptr<StrHandler> h;
public:
    SyntaxAnalyzer()
    {
        _handlers.push_front(h(new OperationHandler("+")));
        _handlers.push_front(h(new OperationHandler("-")));
        _handlers.push_front(h(new OperationHandler("*")));
        _handlers.push_front(h(new OperationHandler("/")));
        _handlers.push_front(h(new OperationHandler("^")));
        _handlers.push_front(h(new OperationHandler("sin")));
        _handlers.push_front(h(new OperationHandler("cos")));
        _handlers.push_front(h(new OperationHandler("(")));
        _handlers.push_front(h(new OperationHandler(")")));
        _handlers.push_front(h(new NumberHandler()));
    }
    size_t getNumOfFit(const std::string& str)
    {
        size_t counter = 0;
        for (auto i = _handlers.begin(); i != _handlers.end(); i++)
            if ( (*i)->isMine(str) )
                counter++;

        return counter;
    }
private:
    ForwardList<h> _handlers;
};

/* priorities
    0 - number
    1 - )
    2 - sin, cos
    3 - +, -
    4 - *, /
    5 - ^
    6 - (
*/
enum Priorities
{
    CL_BR = -1,
    OP_BR = -2,
    NUMBER = 0,
    UNAR_OP = 1,
    PL_MIN = 2,
    MUL_DIV = 3,
    DEG = 4
};

class ExprPart
{
public:
    virtual ~ExprPart() {}
    virtual std::string performance() = 0;
    virtual Priorities getPriority() = 0;
    virtual bool isLeftAssot() = 0;
};
class Operation: public ExprPart
{
public:
    ~Operation() {}
    Operation(std::string&& per, Priorities prior, bool isLeftAssot): 
            _per(per), _prior(prior), _isLeftAssot(isLeftAssot) {}
    Operation(const Operation&) = default;
    virtual std::string performance() {return _per; }
    virtual Priorities getPriority() { return _prior; }
    virtual bool isLeftAssot() { return _isLeftAssot; }
private:
    std::string _per;
    Priorities _prior;
    bool _isLeftAssot;
};
class Number: public ExprPart
{
public:
    Number(std::string&& per): _per(per) {}
    ~Number() {}
    virtual std::string performance() {return _per; }
    virtual Priorities getPriority() { return NUMBER; }
    virtual bool isLeftAssot() { throw std::runtime_error("unexpected error"); }
private:
    std::string _per;
};

class AllParts
{
    typedef std::unique_ptr<Operation> p;
public:
    AllParts()
    {
        _parts.push_front(p(new Operation("+", PL_MIN, true)));
        _parts.push_front(p(new Operation("-", PL_MIN, true)));
        _parts.push_front(p(new Operation("*", MUL_DIV, true)));
        _parts.push_front(p(new Operation("/", MUL_DIV, true)));
        _parts.push_front(p(new Operation("^", DEG, false)));
        _parts.push_front(p(new Operation("sin", UNAR_OP, false)));
        _parts.push_front(p(new Operation("cos", UNAR_OP, false)));
        _parts.push_front(p(new Operation("(", OP_BR, true)));
        _parts.push_front(p(new Operation(")", CL_BR, true)));
    }

    std::unique_ptr<ExprPart> getPart(std::string&& str)
    {
        for (auto i = _parts.begin(); i != _parts.end(); i++)
            if ( (*i)->performance() == str )
                return p(new Operation(*(*i)));
        
        if (str[0] >= '0' && str[0] <= '9')
            return std::unique_ptr<Number>(new Number(std::move(str)));

        throw std::runtime_error("unexpected error");
    }
private:
    ForwardList<p> _parts;
};

std::pair<std::string, std::string> getBackNotation(const std::string& str)
{
    SyntaxAnalyzer syntAn;
    AllParts allP;
    std::string tokens;
    std::string backNotation;
    Stack<std::unique_ptr<ExprPart>> stack;
    for (size_t i = 0; i < str.size(); i++)
    {
        for (size_t len = 1; (i + len) <= str.size()+1; len++)
        {
            std::string subStr = str.substr(i, len);
            if ((i + len) == str.size() - 1 && syntAn.getNumOfFit(subStr) != 1)
                throw std::runtime_error("undefined operation");

            if (syntAn.getNumOfFit(subStr) == 1)
            {
                // operation os defined
                // go to the end of word
                for (; (i + len) <= str.size(); len++)
                    if (syntAn.getNumOfFit(str.substr(i, len)) == 0)
                        break;

                subStr = str.substr(i, len-1);

                if (subStr.size() == 0)
                    throw std::runtime_error("unsopported operation");

                // realize what is the part
                auto part = allP.getPart(std::move(subStr));
                tokens += part->performance() + ", ";

                // now we go to the algorithm of sorting station
                // put number into resulted expression
                if (part->getPriority() == NUMBER)
                    backNotation += part->performance() + " ";
                else
                {
                    // it is operation
                    if (stack.empty())
                    {
                        if (part->performance() == ")")
                            throw std::runtime_error("brackets problem");
                        // first operation of subexpression
                        stack.push(std::move(part));
                    }
                    else if (part->performance() == "(")
                        stack.push(std::move(part));
                    else if (part->performance() == ")")
                    {
                        for (; !stack.empty() && stack.top()->performance() != "(";)
                        {
                            if (stack.size() == 1 && stack.top()->performance() != "(")
                                // last operation in stack is not "("
                                throw std::runtime_error("brackets problem");   

                            backNotation += stack.top()->performance() + " ";
                            stack.pop();
                        }
                        // remove "("
                        stack.pop();
                    }
                    else
                    {
                        // it is operation
                        if (stack.top()->getPriority() >= part->getPriority())
                        {
                            for (; !stack.empty() && stack.top()->getPriority() > part->getPriority();)
                            {
                                backNotation += stack.top()->performance() + " ";
                                stack.pop();
                            }
                        }
                        if (part->isLeftAssot())
                        {
                            for (; !stack.empty() && stack.top()->getPriority() == part->getPriority();)
                            {
                                backNotation += stack.top()->performance() + " ";
                                stack.pop();
                            }
                        }
                        // push this operation into stack
                        stack.push(std::move(part));
                    }
                }                
                i += len - 2;
                break;
            }
        }
    }
    // move remaining operations from stack to resulted expression
    for (; !stack.empty();)
    {
        if (stack.top()->performance() == "(")
            throw std::runtime_error("brackets problem");
        
        backNotation += stack.top()->performance() + " ";
        stack.pop();
    }

    tokens = tokens.substr(0, tokens.size()-2);
    return {tokens, backNotation};
}

int main()
{
    std::cout << "Enter expression with +, -, *, /, ^, sin, cos, (, ), 0, 1, 2, 3, 4, 5, 6, 7, 8, 9\n>";
    std::string inputExpr;
    std::getline(std::cin, inputExpr);
    try
    {
        auto editedExpr = editStr(inputExpr);
        auto [tokens, backNotation] = getBackNotation(editedExpr);
        std::cout << "Tokens: " << tokens << "\n" << "Result: " << backNotation << "\n";
    }
    catch(const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << '\n';
    }
}