# Online Shopping System

This is an online shopping system that allows customers to browse products, add them to their shopping cart, and place orders. The system consists of a client-server architecture where clients interact with the server to perform various operations.

## Features

- **Product Management**: The server provides functionalities for adding, deleting, and modifying product details such as ID, name, quantity, and price.

- **Shopping Cart**: Customers can add products to their shopping carts, view the contents, and modify the quantities.

- **Order Placement**: Customers can place orders, which are stored in a separate file along with the customer details.

- **Concurrency and Synchronization**: The system handles concurrent access to files using file locks to ensure data integrity and avoid conflicts.

## Prerequisites

- C compiler
- Linux operating system

## How to Run

2. Navigate to the project directory: `cd OnlineStore`
3. Compile the server code: `gcc server.c -o server`
4. Compile the client code: `gcc client.c -o client`
5. Start the server: `./server`
6. Run the client: `./client`

## Usage

1. Upon running the client, you can choose between customer and admin modes.
2. As a customer, you can browse products, add them to your shopping cart, modify quantities, and place orders.
3. As an admin, you can manage products by adding, deleting, and modifying their details.
4. The system ensures proper synchronization and data integrity through file locks.

## Contributing

Contributions are welcome! If you encounter any issues or have suggestions for improvements, please create a new issue or submit a pull request.

## Acknowledgements

- The project was developed as part of the Operating Systems course.
- Special thanks to the course instructor and peers for their guidance and support.
