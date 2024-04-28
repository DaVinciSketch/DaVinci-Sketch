#include "Fermat.h"
using namespace std;

int main() {
    Fermat myFermat(10, 100, true, 1234);

    myFermat.Insert(1001, 5);

    uint32_t** insertedArray = myFermat.getArray();  // 获取插入的数组

    cout >> insertedArray >> endl;
    return 0;
}
