#include "main.h"

#define DEGRADE_PERC 0.005


/**
 * Generates new customers.
 * The customers generated have an order limit equals to orderLimit.
 *
 * The customer population is initially equal to initCustomerBase.
 * This is a helper function
 *
 * @param customerPoolSize pointer to the customer pool size that is updated
 *                         by this function.
 * @param canGrow          pointer to a boolean sentinel which indicates
 *                         if generateCustomers can effectively generate
 *                         customers, it's up to the 
 * @return total customers generated.
 */
ulong generateCustomers(custPoolSizeT customerPoolSize, bool* canGrow,
                       ulong maxCustomerQty, ulong initCustomerBase,
                       rng_type randGen, ulong* customerGrowthBase) {


    ulong customerIncr = initCustomerBase;

    std::uniform_int_distribution<rng_type::result_type> randBase(
        (ulong)0, maxCustomerQty - initCustomerBase);

    /* CUSTOMER GENERATION */
    // the growth rate decreases by 0.5% 
    // (geometric sequence customerIncr x (1 - 0.005)^k)
    if (*canGrow && (customerPoolSize < maxCustomerQty)) {
        if (customerPoolSize + customerIncr > maxCustomerQty)
            customerIncr = maxCustomerQty - customerPoolSize;
        else 
            customerIncr = 
                std::max((ulong)1, (ulong)(customerIncr * (1 - DEGRADE_PERC)));

        std::cout << "Customer: customer pool is growing for " << customerIncr 
             << "..." << std::endl;

        return customerIncr;

    } else if (*canGrow && customerPoolSize >= maxCustomerQty) {
        std::cout << "Customer: customer pool growth is suspended..." << std::endl;
        /* customer pool growth is suspended so calculate a new
         * customerGrowthBase at which to continue growing when the
         * custPoolSize reduces to the customerGrowthBase just calculated 
         */
        *canGrow = false;
        *customerGrowthBase = randBase(randGen);

    } else if (!(*canGrow) && 
               customerPoolSize <= *customerGrowthBase) {
        std::cout << "Customer: customer pool can grow again..." << std::endl;
        *canGrow = true;
    }

    return 0;
}

