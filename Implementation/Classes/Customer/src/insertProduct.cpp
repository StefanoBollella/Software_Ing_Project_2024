#include "ProductWindow.h"

namespace prodwindow {

/** 
 * Updates/inserts a new product into the
 * ProductWindow with it's related quantity. 
 */
void ProductWindow::insertProduct(unsigned long productID,
                                  long quantity) {
    if (!this->containsProduct(productID)) {
        this->productKeys.push_back(productID);
    }
    this->products[productID] = quantity;
}

}  // namespace prodwindow
