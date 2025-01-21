#ifndef supplier_h
#define supplier_h

#include<iostream>
#include<cstdio>
#include<cstdlib> 
#include<stdexcept> 
#include<bitset>  
#include<cassert>
#include<cmath>
#include<vector>
#include<limits>
#include<unordered_map>
#include<algorithm>


 enum state {
             starting,           
             generate_product,
             waiting,         
             update     
 };


class Supplier{

  private : 
  	
   unsigned long supplier_id;                   
   state current_state;   
      
   std::unordered_map<unsigned long, unsigned int> products; 
   unsigned int max_id_products;
   unsigned long last_product_id;
   bool has_created_product;
   
   double initial_waiting_threshold; 
   int waiting_iterations; 
   double reduction_factor;
   int waiting_iterations_max;
        
  public :
  
    Supplier(unsigned long supplier_id, 
            const double initial_waiting_threshold, 
            const double reduction_factor,
            const unsigned int max_id_products);

   unsigned long get_id();
 
   unsigned long get_last_product_id();
   unsigned int get_num_generated_products() const;
   unsigned int get_product_quantity(unsigned long product_id);
   state get_current_state();
   int get_waiting_iterations();
  
   void new_product(unsigned long product_id,unsigned int initial_quantity);
   void add_quantity(unsigned long product_id, unsigned int quantity);

   void set_current_state(state new_state);
   void increase_waiting_iterations();
   double update_waiting_threshold();

};
#endif

