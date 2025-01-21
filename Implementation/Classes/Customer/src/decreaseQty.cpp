#include "ProductWindow.h"

namespace prodwindow {

bool ProductWindow::decreaseQty(ulong productID, long decQty) {
    if (this->containsProduct(productID)) {
        this->products[productID] -= decQty;
        return true;
    }
    return false;
}

}  // namespace prodwindow
