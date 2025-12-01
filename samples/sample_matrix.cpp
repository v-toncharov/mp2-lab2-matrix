#include <iostream>
#include "owned_mat.hxx"
using std::cout, std::endl;

int main(int, char**) {
    owned_col_mat<int> a(5, 5), b(5, 5), c(5, 5);
    for (size_t i = 0; i < 5; i++) for (size_t j = i; j < 5; j++) {
        a[i][j] =  (i * 10) + j;
        b[i][j] = ((i * 10) + j) * 100;
    }

    c = a + b;
    cout << "a:" << endl << a << endl;
    cout << "b:" << endl << b << endl;
    cout << "c = a + b:" << endl << c << endl;
}
//---------------------------------------------------------------------------
