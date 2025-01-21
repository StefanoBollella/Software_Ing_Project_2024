#include "supplier.h"
/**
 * @brief Constructor for the Supplier class.
 *
 * Initializes a Supplier object with the specified ID, initial waiting threshold,
 * reduction factor, and maximum number of products.
 *
 * @param supplier_id The unique ID of the supplier.
 * @param initial_waiting_threshold.
 * @param reduction_factor The factor by which the waiting threshold decreases.
 * @param max_id_products The maximum number of products the supplier can create.
 */
Supplier::Supplier(unsigned long supplier_id, 
                   const double initial_waiting_threshold,
                   const double reduction_factor,
                   const unsigned int max_id_products){
   
   this->supplier_id = supplier_id; 
   
   this->current_state = starting;    
   this->initial_waiting_threshold = initial_waiting_threshold;
   this->reduction_factor = reduction_factor;

   this->waiting_iterations = 0;
  
   this->max_id_products = max_id_products; 
  
   this->last_product_id = 0;
   this->has_created_product = false;
  
   this->waiting_iterations_max = std::ceil(this->initial_waiting_threshold / this->reduction_factor);
}

/**
 * @brief Retrieves the ID of the supplier.
 *
 * @return The unique ID of the supplier.
 */
unsigned long Supplier::get_id(){ 
    return this->supplier_id;
}

/**
 * @brief Retrieves the ID of the last created product.
 *
 * @throws std::logic_error If no products have been created yet.
 * @return The ID of the last created product.
 */
unsigned long Supplier::get_last_product_id(){
   
   if(!this->has_created_product){
   
     throw std::logic_error("No products created yet");
   }
   return this->last_product_id;
}

/**
 * @brief Retrieves the total number of products generated by the supplier.
 *
 * @return The number of products created.
 */
unsigned int Supplier::get_num_generated_products() const {
    return this->products.size();
}

/**
 * @brief Retrieves the quantity of a specific product.
 *
 * @param product_id The ID of the product.
 * @throws std::runtime_error If the product with the given ID does not exist.
 * @return The quantity of the specified product.
 */
unsigned int Supplier::get_product_quantity(unsigned long product_id){
       
    auto it = this->products.find(product_id);
    
    if(it == products.end()){
       
      throw std::runtime_error("Product with the specific ID does not exist.");
    }
       
    return it->second; 

}

/**
 * @brief Retrieves the current state of the supplier.
 *
 * @return The current state of the supplier.
 */
state Supplier::get_current_state(){
     return this->current_state; 
}

/**
 * @brief Retrieves the number of iterations the supplier has spent in the waiting state.
 *
 * @return The number of waiting iterations.
 */
int Supplier::get_waiting_iterations(){
     return this->waiting_iterations;
}

/**
 * @brief Sets the current state of the supplier.
 *
 * @param new_state The new state to be assigned.
 * @throws std::invalid_argument If the new state is invalid.
 */
void Supplier::set_current_state(state new_state){
    if(new_state != starting && new_state != generate_product && new_state != waiting && new_state != update){
        throw std::invalid_argument("Invalid current state");
    }
  this->current_state = new_state;
}

/**
 * @brief Creates a new product for the supplier.
 *
 * If the maximum number of products is exceeded, an exception is thrown.
 *
 * @param product_id The unique ID of the new product.
 * @param initial_quantity The initial quantity of the new product.
 * @throws std::out_of_range If the maximum number of products is exceeded.
 */
void Supplier::new_product(unsigned long product_id, unsigned int initial_quantity){
 
    if(this->products.size() >= (this->max_id_products )){
        throw std::out_of_range("Cannot generate more products, reached maximum ID");
    }
   if(!this->has_created_product){
       
       this->last_product_id = product_id;
       this->products[product_id] = initial_quantity; 
       this->has_created_product = true; 
   }
   else{
        this->last_product_id = product_id;
        this->products[product_id] = initial_quantity;         
  }
}

/**
 * @brief Adds a specified quantity to an existing product.
 *
 * @param product_id The ID of the product to update.
 * @param quantity The quantity to add to the product.
 * @throws std::logic_error If the product ID does not exist.
 */
void Supplier::add_quantity(unsigned long product_id, unsigned int quantity){

    auto it = this->products.find(product_id);
    
    if(it == products.end()){
      
       throw std::logic_error("Product has not been generated.");
    }
    
    it->second += quantity;
}

/**
 * @brief Increments the number of iterations spent in the waiting state.
 */
void Supplier::increase_waiting_iterations(){
  
   this->waiting_iterations++; 
}

/**
 * @brief Updates the waiting threshold based on the number of waiting iterations.
 *
 * If the number of iterations reaches the maximum, the threshold is reset.
 *
 * @return The updated waiting threshold.
 */
double Supplier::update_waiting_threshold(){
   
   if(this->waiting_iterations < this->waiting_iterations_max){  
        const double new_waiting_threshold = this->initial_waiting_threshold - (this->reduction_factor * this->waiting_iterations);  
        return new_waiting_threshold;
  }
  else{
        this->waiting_iterations = 0;
        return this->initial_waiting_threshold; 
  }
}     

