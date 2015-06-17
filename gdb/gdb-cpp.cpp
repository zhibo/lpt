#include <iostream>

using namespace std;


class Food {
    public:
        static int ref;

        Food() {
            cout << "[" << ++ref << "]" << "I am Food." << endl;
        }

        ~Food() {
            cout << "[" << ref << "]" << "Food bye." << endl;
        }

        virtual void aboutMe() {
            cout << "Yes, I am Food." << endl;
        }
};

class Meat: virtual public Food{
    public:
        Meat(){
            cout << "I am Meat." << endl;
        }

        ~Meat() {
            cout << "Meat bye." << endl;
        }

        void aboutMe() {
            cout << "Yes, I am Meat." << endl;
        }
};

class Rice: virtual public Food{
    public:
        Rice(){
            cout << "I am Rice." << endl;
        }

        ~Rice() {
            cout << "Rice bye." << endl;
        }

        void aboutMe(){
            cout << "Yes, I am Rice." << endl;
        }
};

class Lunch: public Rice, public Meat{
    public:
        Lunch(){
            cout << "I am Lunch." << endl;
        }

        ~Lunch() {
            cout << "Lunch bye." << endl;
        }

        void aboutMe(){
            cout << "Yes, I am Lunch." << endl;
        }
};

int Food::ref = 0;

int main()
{
    int i = 0, j = 2;
    i = 2;
    i += j;

    Lunch l;
    l.aboutMe();
    return 0;
}
