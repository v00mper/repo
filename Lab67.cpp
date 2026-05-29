#include <iostream>
#include <string>
#include <vector>
#include <cassert>
#include <cmath>

using namespace std;

struct Transformer;
struct Number;
struct BinaryOperation;
struct FunctionCall;
struct Variable;
struct AquariumObject;
struct Barbus;
struct Carp;
struct Catfish;
struct BigSnail;
struct SmallSnail;
struct Waterweed;
struct Fish;
struct sinal;
struct Water;

struct Expression {
    virtual ~Expression() {}
    virtual double evaluate() const = 0;
    virtual Expression* transform(Transformer* tr) const = 0;
};

struct Transformer {
    virtual ~Transformer() {}
    virtual Expression* transformNumber(Number const*) = 0;
    virtual Expression* transformBinaryOperation(BinaryOperation const*) = 0;
    virtual Expression* transformFunctionCall(FunctionCall const*) = 0;
    virtual Expression* transformVariable(Variable const*) = 0;
};

struct Number : Expression {
    Number(double value) : value_(value) {}
    double value() const { return value_; }
    double evaluate() const override { return value_; }
    Expression* transform(Transformer* tr) const override {
        return tr->transformNumber(this);
    }
private:
    double value_;
};

struct BinaryOperation : Expression {
    enum { PLUS = '+', MINUS = '-', DIV = '/', MUL = '*' };
    BinaryOperation(Expression const* left, int op, Expression const* right)
        : left_(left), right_(right), op_(op) {
        assert(left_ && right_);
    }
    ~BinaryOperation() {
        delete left_;
        delete right_;
    }
    double evaluate() const override {
        double l = left_->evaluate();
        double r = right_->evaluate();
        switch (op_) {
        case PLUS: return l + r;
        case MINUS: return l - r;
        case DIV: return l / r;
        case MUL: return l * r;
        default: return 0.0;
        }
    }
    Expression* transform(Transformer* tr) const override {
        return tr->transformBinaryOperation(this);
    }
    Expression const* left() const { return left_; }
    Expression const* right() const { return right_; }
    int operation() const { return op_; }
private:
    Expression const* left_;
    Expression const* right_;
    int op_;
};

struct FunctionCall : Expression {
    FunctionCall(string const& name, Expression const* arg)
        : name_(name), arg_(arg) {
        assert(arg_);
        assert(name_ == "sqrt" || name_ == "abs");
    }
    ~FunctionCall() { delete arg_; }
    double evaluate() const override {
        double a = arg_->evaluate();
        if (name_ == "sqrt") return sqrt(a);
        else return fabs(a);
    }
    Expression* transform(Transformer* tr) const override {
        return tr->transformFunctionCall(this);
    }
    string const& name() const { return name_; }
    Expression const* arg() const { return arg_; }
private:
    string const name_;
    Expression const* arg_;
};

struct Variable : Expression {
    Variable(string const& name) : name_(name) {}
    string const& name() const { return name_; }
    double evaluate() const override { return 0.0; }
    Expression* transform(Transformer* tr) const override {
        return tr->transformVariable(this);
    }
private:
    string const name_;
};

struct CopySyntaxTree : Transformer {
    Expression* transformNumber(Number const* number) override {
        return new Number(number->value());
    }
    Expression* transformBinaryOperation(BinaryOperation const* binop) override {
        Expression* newLeft = binop->left()->transform(this);
        Expression* newRight = binop->right()->transform(this);
        return new BinaryOperation(newLeft, binop->operation(), newRight);
    }
    Expression* transformFunctionCall(FunctionCall const* fcall) override {
        Expression* newArg = fcall->arg()->transform(this);
        return new FunctionCall(fcall->name(), newArg);
    }
    Expression* transformVariable(Variable const* var) override {
        return new Variable(var->name());
    }
};
struct FoldConstants : Transformer {
    Expression* transformNumber(Number const* number) override {
        return new Number(number->value());
    }
    Expression* transformBinaryOperation(BinaryOperation const* binop) override {
        Expression* left = binop->left()->transform(this);
        Expression* right = binop->right()->transform(this);
        Number* leftNum = dynamic_cast<Number*>(left);
        Number* rightNum = dynamic_cast<Number*>(right);
        if (leftNum && rightNum) {
            double res;
            switch (binop->operation()) {
            case BinaryOperation::PLUS: res = leftNum->value() + rightNum->value(); break;
            case BinaryOperation::MINUS: res = leftNum->value() - rightNum->value(); break;
            case BinaryOperation::MUL: res = leftNum->value() * rightNum->value(); break;
            case BinaryOperation::DIV: res = leftNum->value() / rightNum->value(); break;
            default: res = 0.0;
            }
            delete left;
            delete right;
            return new Number(res);
        }
        else {
            return new BinaryOperation(left, binop->operation(), right);
        }
    }
    Expression* transformFunctionCall(FunctionCall const* fcall) override {
        Expression* arg = fcall->arg()->transform(this);
        Number* argNum = dynamic_cast<Number*>(arg);
        if (argNum) {
            double val;
            if (fcall->name() == "sqrt") val = sqrt(argNum->value());
            else val = fabs(argNum->value());
            delete arg;
            return new Number(val);
        }
        else {
            return new FunctionCall(fcall->name(), arg);
        }
    }
    Expression* transformVariable(Variable const* var) override {
        return new Variable(var->name());
    }
};

class PointDecart {
public:
    virtual ~PointDecart() {}
    virtual double getX() const = 0;
    virtual double getY() const = 0;
    virtual void setX(double x) = 0;
    virtual void setY(double y) = 0;
    virtual void print() const = 0;
};

class PointPolar {
public:
    double r_;      // радиус
    double theta_;  // угол 

    PointPolar(double r = 0.0, double theta = 0.0) : r_(r), theta_(theta) {}

    double getR() const { return r_; }
    double getTheta() const { return theta_; }
    void setR(double r) { r_ = r; }
    void setTheta(double theta) { theta_ = theta; }

    void print() const {
        cout << "Полярные координаты: r = " << r_ << ", theta = " << theta_ << " рад" << endl;
    }
};

class PointPolarToDecartAdapter : public PointDecart, private PointPolar {
public:

    PointPolarToDecartAdapter(double r = 0.0, double theta = 0.0) : PointPolar(r, theta) {}

    PointPolarToDecartAdapter(const PointPolar& polar) : PointPolar(polar) {}

    double getX() const override {
        return getR() * cos(getTheta());
    }

    double getY() const override {
        return getR() * sin(getTheta());
    }

    void setX(double x) override {
        double y = getY();
        double r = sqrt(x * x + y * y);
        double theta = atan2(y, x);
        const_cast<PointPolarToDecartAdapter*>(this)->setR(r);
        const_cast<PointPolarToDecartAdapter*>(this)->setTheta(theta);
    }

    void setY(double y) override {
        double x = getX();
        double r = sqrt(x * x + y * y);
        double theta = atan2(y, x);
        const_cast<PointPolarToDecartAdapter*>(this)->setR(r);
        const_cast<PointPolarToDecartAdapter*>(this)->setTheta(theta);
    }

    void print() const override {
        cout << "Декартовы координаты: x = " << getX() << ", y = " << getY() << endl;
    }

    void printPolar() const {
        PointPolar::print();
    }
};

const double pi = 3.14;

int main() {
    setlocale(LC_ALL, "Russian");
    cout << "Задание 1:\n";
    Number* n32 = new Number(32.0);
    Number* n16 = new Number(16.0);
    BinaryOperation* minus = new BinaryOperation(n32, BinaryOperation::MINUS, n16);
    FunctionCall* callSqrt = new FunctionCall("sqrt", minus);
    Variable* var = new Variable("var");
    BinaryOperation* mult = new BinaryOperation(var, BinaryOperation::MUL, callSqrt);
    FunctionCall* original = new FunctionCall("abs", mult);

    CopySyntaxTree cst;
    Expression* copy = original->transform(&cst);

    cout << "Оригинал: " << original->evaluate() << endl;
    cout << "Копия: " << copy->evaluate() << endl;

    cout << "\nЗадание 2:\n";
    FoldConstants fc;
    Expression* folded = original->transform(&fc);

    cout << "до: " << original->evaluate() << endl;
    cout << "после:   " << folded->evaluate() << endl;

    cout << "\nЗадание 3:\n";
    PointPolarToDecartAdapter p1(5.0, pi / 3);  // r=5, угол=60
    cout << "Адаптер создан с полярными координатами (r=5, theta=60):" << endl;
    p1.printPolar();
    p1.print();
    cout << endl;

    cout << "Используем адаптер как PointDecart:" << endl;
    PointDecart* ptr = &p1;
    cout << "x = " << ptr->getX() << ", y = " << ptr->getY() << endl;
    ptr->print();
    cout << endl;

    cout << "Меняем x на 3.0, y на 4.0 (декартовы координаты):" << endl;
    ptr->setX(3.0);
    ptr->setY(4.0);
    ptr->print();
    p1.printPolar();
    cout << endl;

    cout << "Создаем адаптер PointPolar (r=10, theta=90):" << endl;
    PointPolar polar(10.0, pi / 2);
    PointPolarToDecartAdapter p2(polar);
    p2.print();
    p2.printPolar();

    delete original;
    delete copy;
    delete folded;

    return 0;
}