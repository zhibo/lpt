#include <iostream>
using namespace std;

class A {
    public:
        virtual void func() = 0;

};

class B: public A {
    public:
        virtual void func() {
            cout << "I am B." << endl;
        }
};

class C: public B {
    public:
        void func() {
            cout << "I am C." << endl;
        }
};

int main()
{
    static const volatile int d = 10;
    class B b;
    class C c;
    class A *a = &b;
    a->func();
    a = &c;
    a->func();
    cout << d << endl;
    return 0;
}
