#include "ProductWindow.h"
#include <cassert>

namespace prodwindow {

void ProductWindow::removeProduct(ulong productID) {
    this->products.erase(productID);

    int totalKeys = this->productKeys.size();
    for (int i = 0; i < totalKeys; ++i) {
        if (this->productKeys.at(i) == productID) {
            this->productKeys.erase(this->productKeys.begin() + i);
            assert(this->products.size() == this->productKeys.size());
            break;
        }
    }
    assert(this->isValid());
}

}  // namespace prodwindow
