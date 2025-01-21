#ifndef PRODUCT_WINDOW_H
#define PRODUCT_WINDOW_H

#include <algorithm>
#include <iostream>
#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>

namespace prodwindow {

typedef unsigned long ulong;

class ProductWindow {
  private:
    // we want to limit the number of this class objects to 256;
    // this object can costs a lot of memory.
    unsigned char id;
    // Invariant: products.size() == productKeys.size();
    std::unordered_map<ulong, long> products;
    std::vector<ulong> productKeys;

  public:
    ProductWindow(unsigned char id) { this->id = id; }

    void insertProduct(ulong productID, long quantity);

    bool containsProduct(ulong productID) {
        return this->products.contains(productID) &&
            std::find(
                productKeys.begin(),
                productKeys.end(),
                productID) != productKeys.end();
    }

    unsigned char getID() { return this->id; }

    /* Gets quantity of 'ProductID' if exists, else throws std::out_of_range */
    long getProductQuantity(ulong productID) {
        return this->products.at(productID);
    }

    /* Gets k-th product ID if exists, else throws std::out_of_range.
     * Useful for random selection of products. */
    ulong getKthProductID(unsigned int k) {
        return this->productKeys.at(k);
    }

    ulong getNumProducts() { return this->productKeys.size(); }

    void removeProduct(ulong productID);

    bool increaseQty(ulong productID, long incQty);
    bool decreaseQty(ulong productID, long decQty);

    bool isValid() { return this->products.size() == this->productKeys.size(); }

    void printProductWindow() {
        std::cout << "----- Product Window -----" << std::endl;
        std::cout << "prodID: quantity" << std::endl;
        if (getNumProducts() > 0) {
            for (auto p : this->products) {
                std::cout << p.first << ": " << p.second << std::endl;
            }
        } else {
            std::cout << "Empty..." << std::endl;
        }
        std::cout << "--------------------------" << std::endl;
    }
};

}  // namespace prodwindow

#endif  // PRODUCT_WINDOW_H
