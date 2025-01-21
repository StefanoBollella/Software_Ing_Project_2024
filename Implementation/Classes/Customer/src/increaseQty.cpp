#include "ProductWindow.h"

namespace prodwindow {

bool ProductWindow::increaseQty(ulong productID, long incQty) {
    if (this->containsProduct(productID)) {
        this->products[productID] += incQty;
        return true;
    }
    return false;
}

}  // namespace prodwindow
