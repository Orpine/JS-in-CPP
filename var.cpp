#include "Var.h"

VarLink::VarLink(Var *var, const std::string &name) {
    this->name = name;
    this->prevSibling = nullptr;
    this->nextSibling = nullptr;
    this->var = var->ref();
    this->owned = false;
}

VarLink::VarLink(const VarLink &link) {
    this->name = link.name;
    this->prevSibling = nullptr;
    this->nextSibling = nullptr;
    this->var = link.var->ref();
    this->owned = false;
}

VarLink::~VarLink() {
    var->unref();
}

void VarLink::replaceWith(Var *var) {
    Var *temp = this->var;
    this->var = var->ref();
    temp->unref();
}

void VarLink::replaceWith(shared_ptr<VarLink> varLink) {
    if (varLink)
        replaceWith(varLink->var);
    else
        replaceWith(new Var());
}


void Var::init() {
    refNum = 0;
    firstChild = nullptr;
    lastChild = nullptr;
    type = 0;
    intData = 0;
    doubleData = 0;
    stringData = "";
}

Var *Var::ref() {
    refNum++;
    return this;
}

void Var::unref() {
    if (--refNum == 0)
        delete this;
}

Var::Var() {
    init();
    type = VAR_UNDEFINED;
}

Var::Var(const std::string &varData, int varType) {
    init();
    type = varType;
    switch (type) {
        case VAR_DOUBLE:
            doubleData = atof(varData.c_str());
            break;
        case VAR_INTEGER:
            intData = atoi(varData.c_str());
            break;
        case VAR_BOOLEAN:
            if (varData == "false")
                intData = 0;
            else
                intData = 1;
            break;
        default:
            stringData = varData;
    }
}

Var::Var(bool varData) {
    init();
    setBool(varData);
}

Var::Var(int varData) {
    init();
    setInt(varData);
}

Var::Var(double varData) {
    init();
    setDouble(varData);
}




Var::~Var() {
    removeAllChildren();
}


int Var::getInt() {
    if (isInt() || isBoolean()) return intData;
    if (isDouble()) return (int) doubleData;
    return 0;
}

double Var::getDouble() {
    if (isInt()) return (double) intData;
    if (isDouble()) return doubleData;
    return 0;
}

bool Var::getBool() {
    return (intData != 0);
}

std::string Var::getString() {
    std::stringstream ss;
    std::string ret;
    if (isInt()) {
        ss << intData;
        ss >> ret;
        return ret;
    }
    if (isDouble()) {
        ss << doubleData;
        ss >> ret;
        return ret;
    }
    if (isBoolean()) {
        if (intData != 0)
            return "true";
        else
            return "false";
    }
    if (isNull()) {
        return "null";
    }
    if (isUndefined()) {
        return "undefined";
    }
    return stringData;
}


void Var::setInt(int _int) {
    type = VAR_INTEGER;
    intData = _int;
    doubleData = 0;
    stringData = "";
}

void Var::setDouble(double _double) {
    type = VAR_DOUBLE;
    intData = 0;
    doubleData = _double;
    stringData = "";
}

void Var::setBool(bool _bool) {
    type = VAR_BOOLEAN;
    intData = _bool;
    doubleData = 0;
    stringData = "";
}

void Var::setString(const std::string &_string) {
    type = VAR_STRING;
    intData = 0;
    doubleData = 0;
    stringData = _string;
}

void Var::setUndefined() {
    type = VAR_UNDEFINED;
    intData = 0;
    doubleData = 0;
    stringData = "";
    removeAllChildren();
}

Var *Var::mathOp(Var *b, TOKEN_TYPES op) {
    Var *a = this;
    if (op == TK_TYPEEQUAL || op == TK_N_TYPEEQUAL) {
        bool eql = (a->type == b->type);
        if (eql) {
            Var *temp = a->mathOp(b, TK_EQUAL);
            if (!temp->getBool()) eql = false;
            delete temp;
        }

        if (op == TK_TYPEEQUAL)
            return new Var(eql);
        else
            return new Var(!eql);
    }


    if ((a->isUndefined() || a->isNull()) && (b->isUndefined() || b->isNull())) {
        if (op == TK_EQUAL)return new Var("true", VAR_BOOLEAN);
        if (op == TK_N_EQUAL)return new Var("false", VAR_BOOLEAN);
        return new Var();
    }
    else if (a->isBoolean() && b->isBoolean()) {
        if (op == TK_AND_AND) {
            return new Var(a->getBool() && b->getBool());
        } else if (op == TK_OR_OR) {
            return new Var(a->getBool() || b->getBool());
        } else {
            assert(0);
        }
    }
    else if ((a->isNumber() || a->isUndefined()) && (b->isNumber() || b->isUndefined())) {
        if (!a->isDouble() && !b->isDouble()) {
            int aa = a->getInt();
            int bb = b->getInt();
            switch (op) {
                case TK_PLUS:
                    return new Var(aa + bb);
                case TK_MINUS:
                    return new Var(aa - bb);
                case TK_MULTIPLY:
                    return new Var(aa * bb);
                case TK_DIVIDE:
                    return new Var(aa / bb);
                case TK_BITWISE_AND:
                    return new Var(aa & bb);
                case TK_BITWISE_OR:
                    return new Var(aa | bb);
                case TK_BITWISE_XOR:
                    return new Var(aa ^ bb);
                case TK_MOD:
                    return new Var(aa % bb);
                case TK_EQUAL:
                    return new Var(aa == bb);
                case TK_N_EQUAL:
                    return new Var(aa != bb);
                case TK_LESS:
                    return new Var(aa < bb);
                case TK_GREATER:
                    return new Var(aa > bb);
                case TK_L_EQUAL:
                    return new Var(aa <= bb);
                case TK_G_EQUAL:
                    return new Var(aa >= bb);
                default:;//fault TODO
            }
        }
        else {
            double aa = a->getDouble();
            double bb = b->getDouble();
            switch (op) {
                case TK_PLUS:
                    return new Var(aa + bb);
                case TK_MINUS:
                    return new Var(aa - bb);
                case TK_MULTIPLY:
                    return new Var(aa * bb);
                case TK_DIVIDE:
                    return new Var(aa / bb);
                case TK_EQUAL:
                    return new Var(aa == bb);
                case TK_N_EQUAL:
                    return new Var(aa != bb);
                case TK_LESS:
                    return new Var(aa < bb);
                case TK_GREATER:
                    return new Var(aa > bb);
                case TK_L_EQUAL:
                    return new Var(aa <= bb);
                case TK_G_EQUAL:
                    return new Var(aa >= bb);
                default:;//fault TODO
            }
        }
    }
    else if (a->isArray() || a->isObject()) {
        switch (op) {
            case TK_EQUAL:
                return new Var(a == b);
            case TK_N_EQUAL:
                return new Var(a != b);
            default:;//fault TODO
        }
    }
    else {
        string aa = a->getString();
        string bb = b->getString();
        switch (op) {
            case TK_PLUS:
                return new Var(aa + bb, VAR_STRING);
            case TK_EQUAL:
                return new Var(aa == bb);
            case TK_N_EQUAL:
                return new Var(aa != bb);
            case TK_LESS:
                return new Var(aa < bb);
            case TK_GREATER:
                return new Var(aa > bb);
            case TK_L_EQUAL:
                return new Var(aa <= bb);
            case TK_G_EQUAL:
                return new Var(aa >= bb);
            default:;//fault TODO
        }
    }
    assert(0);
    return 0;
}

std::shared_ptr<VarLink> Var::findChild(const std::string &childName) {
    auto v = firstChild;
    while (v) {
        if (v->name == childName)
            return v;
        v = v->nextSibling;
    }
    return nullptr;
}

std::shared_ptr<VarLink> Var::findChildOrCreate(const std::string &childName, int childType) {
    auto v = findChild(childName);
    if (v)
        return v;
    else
        return addChild(childName, new Var("", childType));
}

std::shared_ptr<VarLink> Var::addChild(const std::string &childName, Var *child) {
    if (isUndefined())
        type = VAR_OBJECT;
    if (!child)
        child = new Var();

    std::shared_ptr<VarLink> link(new VarLink(child, childName));
    link->owned = true;
    if (lastChild) {
        lastChild->nextSibling = link;
        link->prevSibling = lastChild;
        lastChild = link;
    }
    else {
        firstChild = link;
        lastChild = link;
    }
    return link;

}

std::shared_ptr<VarLink> Var::addUniqueChild(const std::string &childName, Var *child) {
    if (!child)
        child = new Var();

    auto link = findChild(childName);
    if (!link) {
        link = addChild(childName, child);
    }
    else {
        link->replaceWith(child);
    }
    return link;
}

void Var::removeChild(Var *child) {
    auto link = firstChild;
    while (link) {
        if (link->var == child)
            break;
        link = link->nextSibling;
    }
    removeLink(link);
}

void Var::removeLink(std::shared_ptr<VarLink> link) {
    if (!link) return;
    if (link->nextSibling)
        link->nextSibling->prevSibling = link->prevSibling;
    if (link->prevSibling)
        link->prevSibling->nextSibling = link->nextSibling;
    if (lastChild == link)
        lastChild = link->prevSibling;
    if (firstChild == link)
        firstChild = link->nextSibling;
    link = nullptr;
}

void Var::removeAllChildren() {
    auto link = firstChild;
    while (link) {
        auto temp = link;
        link = link->nextSibling;
        temp.reset();
    }
    firstChild = nullptr;
    lastChild = nullptr;
}


//
int Var::getArrayLength() {
    int max = -1;
    if (!isArray()) {
        return 0;
    }
    auto link = firstChild;
    while (link) {
        int idx = atoi(link->name.c_str());
        if (idx > max)
            max = idx;
        link = link->nextSibling;
    }
    return max + 1;

}

Var *Var::copyThis() {
    Var *ret = new Var();

    ret->copy(this);

    auto link = firstChild;
    while (link) {
        Var *copiedVar = link->var->copyThis();

        ret->addChild(link->name, copiedVar);
        link = link->nextSibling;
    }
    return ret;
}

void Var::copy(Var *var) {
    this->stringData = var->stringData;
    this->intData = var->intData;
    this->doubleData = var->doubleData;
    this->type = var->type;

}
//int main(){
//    Var *a=new Var(1);
//    Var *b=new Var(2);
//    Var *c=new Var(1.0);
//    std::cout<<a->mathOp(b,TK_GREATER)->getBool()<<endl;
//    std::cout<<a->mathOp(c,TK_TYPEEQUAL)->getBool()<<endl;
//    std::cout<<a->mathOp(c,TK_EQUAL)->getBool()<<endl;
//    std::cout<<a->equals(c)<<endl;
//    Var* array=new Var("",VAR_ARRAY);
//    for(int i=0;i<10;i++){
//        array->setAtIndex(i,new Var(i));
//    }
//    std::cout<<array->getArrayLength();
//
//}