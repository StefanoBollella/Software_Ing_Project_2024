INSERT INTO operation_type(name) VALUES -- for customer
    ('order'), ('cancel-order'),
    ('get-products'), ('customer-reg');

INSERT INTO operation_type(name) VALUES -- for carrier
    ('lose-order'), ('get-order'), ('deliver-order');

INSERT INTO operation_type(name) VALUES -- for supplier
    ('save-product'), ('product-quantity-update');

INSERT INTO operation_type(name) VALUES -- for monitors
    ('db-disk-usage-check'), ('carrier-activity'), ('supplier-activity'), ('ping-servers');
