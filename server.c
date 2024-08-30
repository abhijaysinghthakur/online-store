#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include "structures.h"

void unlock(int fd, struct flock lock)
{
    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLKW, &lock);
}

void readLock(int fd, struct flock lock)
{
    lock.l_len = 0;
    lock.l_type = F_RDLCK;
    lock.l_start = 0;
    lock.l_whence = SEEK_SET;
    fcntl(fd, F_SETLKW, &lock);
}

void CartLock(int fd_cart, struct flock lock_cart, int offset, int ch)
{
    lock_cart.l_whence = SEEK_SET;
    lock_cart.l_len = sizeof(struct cart);
    lock_cart.l_start = offset;
    if (ch == 1)
    {
        // rdlck
        lock_cart.l_type = F_RDLCK;
    }
    else
    {
        // wrlck
        lock_cart.l_type = F_WRLCK;
    }
    fcntl(fd_cart, F_SETLKW, &lock_cart);
    lseek(fd_cart, offset, SEEK_SET);
}

int currentOffset(int cust_id, int fd_custs)
{
    if (cust_id < 0)
    {
        return -1;
    }
    struct flock lock_cust;
    lock_cust.l_len = 0;
    lock_cust.l_type = F_RDLCK;
    lock_cust.l_start = 0;
    lock_cust.l_whence = SEEK_SET;
    fcntl(fd_custs, F_SETLKW, &lock_cust);
    struct index id;

    while (read(fd_custs, &id, sizeof(struct index)))
    {
        if (id.id == cust_id)
        {
            unlock(fd_custs, lock_cust);
            return id.offset;
        }
    }
    unlock(fd_custs, lock_cust);
    return -1;
}

int main()
{

    // file containing all the records is called records.txt

    int fd = open("inventory.txt", O_RDWR | O_CREAT, 0777);
    int fd_cart = open("orders.txt", O_RDWR | O_CREAT, 0777);
    int fd_custs = open("customer.txt", O_RDWR | O_CREAT, 0777);
    int fd_admin = open("loggingAdmin.txt", O_RDWR | O_CREAT, 0777);
    lseek(fd_admin, 0, SEEK_END);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd == -1)
    {
        perror("Error: ");
        return -1;
    }

    struct sockaddr_in serv, cli;

    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = INADDR_ANY;
    serv.sin_port = htons(5555);

    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
    {
        perror("Error: ");
        return -1;
    }

    if (bind(sockfd, (struct sockaddr *)&serv, sizeof(serv)) == -1)
    {
        perror("Error: ");
        return -1;
    }

    if (listen(sockfd, 5) == -1)
    {
        perror("Error: ");
        return -1;
    }

    int size = sizeof(cli);
    printf("Store is Online\n");

    while (1)
    {

        int new_socketDescriptor = accept(sockfd, (struct sockaddr *)&cli, &size);
        if (new_socketDescriptor == -1)
        {
            // perror("Error: ");
            return -1;
        }

        if (!fork())
        {
            printf("Connection with client successful\n");
            close(sockfd);

            int user;
            read(new_socketDescriptor, &user, sizeof(int));

            if (user == 1)
            {

                char ch;
                while (1)
                {
                    read(new_socketDescriptor, &ch, sizeof(char));

                    lseek(fd, 0, SEEK_SET);
                    lseek(fd_cart, 0, SEEK_SET);
                    lseek(fd_custs, 0, SEEK_SET);

                    if (ch == '7')
                    {
                        close(new_socketDescriptor);
                        break;
                    }
                    else if (ch == '1')
                    {

                        struct flock lock;
                        readLock(fd, lock);

                        struct product p;
                        while (read(fd, &p, sizeof(struct product)))
                        {
                            if (p.id != -1)
                            {
                                write(new_socketDescriptor, &p, sizeof(struct product));
                            }
                        }

                        p.id = -1;
                        write(new_socketDescriptor, &p, sizeof(struct product));
                        unlock(fd, lock);
                    }

                    else if (ch == '2')
                    {
                        int cust_id = -1;
                        read(new_socketDescriptor, &cust_id, sizeof(int));

                        int offset = currentOffset(cust_id, fd_custs);
                        struct cart c;

                        if (offset == -1)
                        {

                            struct cart c;
                            c.custid = -1;
                            write(new_socketDescriptor, &c, sizeof(struct cart));
                        }
                        else
                        {
                            struct cart c;
                            struct flock lock_cart;

                            CartLock(fd_cart, lock_cart, offset, 1);
                            read(fd_cart, &c, sizeof(struct cart));
                            write(new_socketDescriptor, &c, sizeof(struct cart));
                            unlock(fd_cart, lock_cart);
                        }
                    }

                    else if (ch == '3')
                    {
                        int cust_id = -1;
                        read(new_socketDescriptor, &cust_id, sizeof(int));
                        int offset = currentOffset(cust_id, fd_custs);

                        write(new_socketDescriptor, &offset, sizeof(int));

                        if (offset == -1)
                        {
                            continue;
                        }

                        struct flock lock_cart;

                        int i = -1;
                        CartLock(fd_cart, lock_cart, offset, 1);
                        struct cart c;
                        read(fd_cart, &c, sizeof(struct cart));

                        struct flock lock_prod;
                        readLock(fd, lock_prod);

                        struct product p;
                        read(new_socketDescriptor, &p, sizeof(struct product));

                        int prod_id = p.id;
                        int qty = p.qty;

                        struct product p1;
                        int found = 0;
                        while (read(fd, &p1, sizeof(struct product)))
                        {
                            if (p1.id == p.id)
                            {
                                if (p1.qty >= p.qty)
                                {
                                    // p1.qty -= p.qty;
                                    found = 1;
                                    break;
                                }
                            }
                        }
                        unlock(fd_cart, lock_cart);
                        unlock(fd, lock_prod);

                        if (!found)
                        {
                            write(new_socketDescriptor, "Product id invalid or out of stock\n", sizeof("Product id invalid or out of stock\n"));
                            continue;
                        }

                        int flg = 0;

                        int flg1 = 0;
                        for (int i = 0; i < 20; i++)
                        {
                            if (c.products[i].id == p.id)
                            {
                                flg1 = 1;
                                break;
                            }
                        }

                        if (flg1)
                        {
                            write(new_socketDescriptor, "Product already exists in cart\n", sizeof("Product already exists in cart\n"));
                            continue;
                        }

                        for (int i = 0; i < 20; i++)
                        {
                            if (c.products[i].id == -1)
                            {
                                flg = 1;
                                c.products[i].id = p.id;
                                c.products[i].qty = p.qty;
                                strcpy(c.products[i].name, p1.name);
                                c.products[i].price = p1.price;
                                break;
                            }
                            else if (c.products[i].qty <= 0)
                            {
                                flg = 1;
                                c.products[i].id = p.id;
                                c.products[i].qty = p.qty;
                                strcpy(c.products[i].name, p1.name);
                                c.products[i].price = p1.price;
                                break;
                            }
                        }

                        if (!flg)
                        {
                            write(new_socketDescriptor, "Cart limit reached\n", sizeof("Cart limit reached\n"));
                            continue;
                        }

                        write(new_socketDescriptor, "Item added to cart\n", sizeof("Item added to cart\n"));

                        CartLock(fd_cart, lock_cart, offset, 2);
                        write(fd_cart, &c, sizeof(struct cart));
                        unlock(fd_cart, lock_cart);
                    }

                    else if (ch == '4')
                    {

                        int cust_id = -1;
                        read(new_socketDescriptor, &cust_id, sizeof(int));

                        int offset = currentOffset(cust_id, fd_custs);

                        write(new_socketDescriptor, &offset, sizeof(int));
                        if (offset == -1)
                        {
                            continue;
                        }

                        struct flock lock_cart;
                        CartLock(fd_cart, lock_cart, offset, 1);
                        struct cart c;
                        read(fd_cart, &c, sizeof(struct cart));

                        int pid, qty;
                        struct product p;
                        read(new_socketDescriptor, &p, sizeof(struct product));

                        pid = p.id;
                        qty = p.qty;

                        int flg = 0;
                        int i;
                        for (i = 0; i < 20; i++)
                        {
                            if (c.products[i].id == pid)
                            {

                                struct flock lock_prod;
                                readLock(fd, lock_prod);

                                struct product p1;
                                while (read(fd, &p1, sizeof(struct product)))
                                {
                                    if (p1.id == pid && p1.qty > 0)
                                    {
                                        if (p1.qty >= qty)
                                        {
                                            flg = 1;
                                            break;
                                        }
                                        else
                                        {
                                            flg = 0;
                                            break;
                                        }
                                    }
                                }

                                unlock(fd, lock_prod);
                                break;
                            }
                        }
                        unlock(fd_cart, lock_cart);

                        if (!flg)
                        {
                            write(new_socketDescriptor, "Product id not in the order or out of stock\n", sizeof("Product id not in the order or out of stock\n"));
                            continue;
                        }

                        c.products[i].qty = qty;
                        write(new_socketDescriptor, "Update successful\n", sizeof("Update successful\n"));
                        CartLock(fd_cart, lock_cart, offset, 2);
                        write(fd_cart, &c, sizeof(struct cart));
                        unlock(fd_cart, lock_cart);
                    }

                    else if (ch == '5')
                    {
                        int cust_id = -1;
                        read(new_socketDescriptor, &cust_id, sizeof(int));

                        int offset;
                        offset = currentOffset(cust_id, fd_custs);

                        write(new_socketDescriptor, &offset, sizeof(int));
                        if (offset == -1)
                        {
                            continue;
                        }

                        struct flock lock_cart;
                        CartLock(fd_cart, lock_cart, offset, 1);

                        struct cart c;
                        read(fd_cart, &c, sizeof(struct cart));
                        unlock(fd_cart, lock_cart);
                        write(new_socketDescriptor, &c, sizeof(struct cart));

                        int total = 0;

                        for (int i = 0; i < 20; i++)
                        {

                            if (c.products[i].id != -1)
                            {
                                write(new_socketDescriptor, &c.products[i].qty, sizeof(int));

                                struct flock lock_prod;
                                readLock(fd, lock_prod);
                                lseek(fd, 0, SEEK_SET);

                                struct product p;
                                int flg = 0;
                                while (read(fd, &p, sizeof(struct product)))
                                {

                                    if (p.id == c.products[i].id && p.qty > 0)
                                    {
                                        int min;
                                        if (c.products[i].qty >= p.qty)
                                        {
                                            min = p.qty;
                                        }
                                        else
                                        {
                                            min = c.products[i].qty;
                                        }

                                        flg = 1;
                                        write(new_socketDescriptor, &min, sizeof(int));
                                        write(new_socketDescriptor, &p.price, sizeof(int));
                                        break;
                                    }
                                }

                                unlock(fd, lock_prod);

                                if (!flg)
                                {
                                    // product got deleted midway
                                    int val = 0;
                                    write(new_socketDescriptor, &val, sizeof(int));
                                    write(new_socketDescriptor, &val, sizeof(int));
                                }
                            }
                        }

                        char ch;
                        read(new_socketDescriptor, &ch, sizeof(char));

                        for (int i = 0; i < 20; i++)
                        {

                            struct flock lock_prod;
                            readLock(fd, lock_prod);
                            lseek(fd, 0, SEEK_SET);

                            struct product p;
                            while (read(fd, &p, sizeof(struct product)))
                            {

                                if (p.id == c.products[i].id)
                                {
                                    int min;
                                    if (c.products[i].qty >= p.qty)
                                    {
                                        min = p.qty;
                                    }
                                    else
                                    {
                                        min = c.products[i].qty;
                                    }
                                    unlock(fd, lock_prod);
                                    // product write lock
                                    lseek(fd, (-1) * sizeof(struct product), SEEK_CUR);
                                    lock_prod.l_type = F_WRLCK;
                                    lock_prod.l_whence = SEEK_CUR;
                                    lock_prod.l_start = 0;
                                    lock_prod.l_len = sizeof(struct product);

                                    fcntl(fd, F_SETLKW, &lock_prod);
                                    p.qty = p.qty - min;

                                    write(fd, &p, sizeof(struct product));
                                    unlock(fd, lock_prod);
                                }
                            }

                            unlock(fd, lock_prod);
                        }

                        CartLock(fd_cart, lock_cart, offset, 2);

                        for (int i = 0; i < 20; i++)
                        {
                            c.products[i].id = -1;
                            strcpy(c.products[i].name, "");
                            c.products[i].price = -1;
                            c.products[i].qty = -1;
                        }

                        write(fd_cart, &c, sizeof(struct cart));
                        write(new_socketDescriptor, &ch, sizeof(char));
                        unlock(fd_cart, lock_cart);

                        read(new_socketDescriptor, &total, sizeof(int));
                        read(new_socketDescriptor, &c, sizeof(struct cart));

                        int fd_rec = open("Bill.txt", O_CREAT | O_RDWR, 0777);
                        write(fd_rec, "ProductID\tProductName\tQuantity\tPrice\n", strlen("ProductID\tProductName\tQuantity\tPrice\n"));
                        char temp[100];
                        for (int i = 0; i < 20; i++)
                        {
                            if (c.products[i].id != -1)
                            {
                                sprintf(temp, "%d\t%s\t%d\t%d\n", c.products[i].id, c.products[i].name, c.products[i].qty, c.products[i].price);
                                write(fd_rec, temp, strlen(temp));
                            }
                        }
                        sprintf(temp, "Total - %d\n", total);
                        write(fd_rec, temp, strlen(temp));
                        close(fd_rec);
                    }

                    else if (ch == '6')
                    {

                        struct flock lock;
                        lock.l_len = 0;
                        lock.l_type = F_RDLCK;
                        lock.l_start = 0;
                        lock.l_whence = SEEK_SET;
                        fcntl(fd_custs, F_SETLKW, &lock);

                        int max_id = -1;
                        struct index id;
                        while (read(fd_custs, &id, sizeof(struct index)))
                        {
                            if (id.id > max_id)
                            {
                                max_id = id.id;
                            }
                        }

                        max_id++;

                        id.id = max_id;
                        id.offset = lseek(fd_cart, 0, SEEK_END);
                        lseek(fd_custs, 0, SEEK_END);
                        write(fd_custs, &id, sizeof(struct index));

                        struct cart c;
                        c.custid = max_id;
                        for (int i = 0; i < 20; i++)
                        {
                            c.products[i].id = -1;
                            strcpy(c.products[i].name, "");
                            c.products[i].qty = -1;
                            c.products[i].price = -1;
                        }
                        lseek(fd_cart, 0, SEEK_END);
                        write(fd_cart, &c, sizeof(struct cart));
                        unlock(fd_custs, lock);
                        write(new_socketDescriptor, &max_id, sizeof(int));
                    }
                }
                printf("Connection terminated\n");
            }
            else if (user == 2)
            {
                char ch;
                while (1)
                {
                    read(new_socketDescriptor, &ch, sizeof(ch));

                    lseek(fd, 0, SEEK_SET);
                    lseek(fd_cart, 0, SEEK_SET);
                    lseek(fd_custs, 0, SEEK_SET);

                    if (ch == '1')
                    {
                        char name[50];
                        char response[100];
                        int id, qty, price;

                        struct product p1;
                        int n = read(new_socketDescriptor, &p1, sizeof(struct product));

                        strcpy(name, p1.name);
                        id = p1.id;
                        qty = p1.qty;
                        price = p1.price;

                        struct flock lock;
                        readLock(fd, lock);

                        struct product p;

                        int flg = 0;
                        while (read(fd, &p, sizeof(struct product)))
                        {

                            if (p.id == id && p.qty > 0)
                            {
                                write(new_socketDescriptor, "Duplicate product\n", sizeof("Duplicate product\n"));
                                sprintf(response, "Adding product with product id %d failed as product id is duplicate\n", id);
                                write(fd_admin, response, strlen(response));
                                unlock(fd, lock);
                                flg = 1;
                                break;
                            }
                        }

                        if (!flg)
                        {

                            lseek(fd, 0, SEEK_END);
                            p.id = id;
                            strcpy(p.name, name);
                            p.price = price;
                            p.qty = qty;

                            write(fd, &p, sizeof(struct product));
                            write(new_socketDescriptor, "Added successfully\n", sizeof("Added succesfully\n"));
                            sprintf(response, "New product with product id %d added successfully\n", id);
                            write(fd_admin, response, strlen(response));
                            unlock(fd, lock);
                        }
                    }
                    else if (ch == '2')
                    {
                        int id;
                        read(new_socketDescriptor, &id, sizeof(int));
                        struct flock lock;
                        readLock(fd, lock);
                        char response[100];

                        struct product p;
                        int flg = 0;
                        while (read(fd, &p, sizeof(struct product)))
                        {
                            if (p.id == id)
                            {

                                unlock(fd, lock);
                                // product write lock
                                lseek(fd, (-1) * sizeof(struct product), SEEK_CUR);
                                lock.l_type = F_WRLCK;
                                lock.l_whence = SEEK_CUR;
                                lock.l_start = 0;
                                lock.l_len = sizeof(struct product);

                                fcntl(fd, F_SETLKW, &lock);

                                p.id = -1;
                                strcpy(p.name, "");
                                p.price = -1;
                                p.qty = -1;

                                write(fd, &p, sizeof(struct product));
                                write(new_socketDescriptor, "Delete successful", sizeof("Delete successful"));
                                sprintf(response, "Product with product id %d deleted succesfully\n", id);
                                write(fd_admin, response, strlen(response));

                                unlock(fd, lock);
                                flg = 1;
                                break;
                            }
                        }

                        if (!flg)
                        {
                            sprintf(response, "Deleting product with product id %d failed as product does not exist\n", id);
                            write(fd_admin, response, strlen(response));
                            write(new_socketDescriptor, "Product id invalid", sizeof("Product id invalid"));
                            unlock(fd, lock);
                        }
                    }
                    else if (ch == '3')
                    {
                        int id;
                        int val = -1;
                        struct product p1;
                        read(new_socketDescriptor, &p1, sizeof(struct product));
                        id = p1.id;

                        char response[100];

                        val = p1.price;

                        struct flock lock;
                        readLock(fd, lock);

                        int flg = 0;

                        struct product p;
                        while (read(fd, &p, sizeof(struct product)))
                        {
                            if (p.id == id)
                            {

                                unlock(fd, lock);
                                // product write lock
                                lseek(fd, (-1) * sizeof(struct product), SEEK_CUR);
                                lock.l_type = F_WRLCK;
                                lock.l_whence = SEEK_CUR;
                                lock.l_start = 0;
                                lock.l_len = sizeof(struct product);

                                fcntl(fd, F_SETLKW, &lock);
                                int old;

                                old = p.price;
                                p.price = val;

                                write(fd, &p, sizeof(struct product));

                                write(new_socketDescriptor, "The price of the product has been successfully modified.", sizeof("The price of the product has been successfully modified."));

                                sprintf(response, "Price of product with ID %d has been successfully modified from %d to %d.\n", id, old, val);
                                write(fd_admin, response, strlen(response));

                                unlock(fd, lock);
                                flg = 1;
                                break;
                            }
                        }

                        if (!flg)
                        {
                            write(new_socketDescriptor, "Product id invalid", sizeof("Product id invalid"));
                            unlock(fd, lock);
                        }
                    }

                    else if (ch == '4')
                    {
                        int id;
                        int val = -1;
                        struct product p1;
                        read(new_socketDescriptor, &p1, sizeof(struct product));
                        id = p1.id;

                        char response[100];

                        val = p1.qty;

                        struct flock lock;
                        readLock(fd, lock);

                        int flg = 0;

                        struct product p;
                        while (read(fd, &p, sizeof(struct product)))
                        {
                            if (p.id == id)
                            {

                                unlock(fd, lock);
                                // product write lock
                                lseek(fd, (-1) * sizeof(struct product), SEEK_CUR);
                                lock.l_type = F_WRLCK;
                                lock.l_whence = SEEK_CUR;
                                lock.l_start = 0;
                                lock.l_len = sizeof(struct product);

                                fcntl(fd, F_SETLKW, &lock);
                                int old;

                                old = p.qty;
                                p.qty = val;

                                write(fd, &p, sizeof(struct product));

                                sprintf(response, "Quantity of product with product id %d modified from %d to %d \n", id, old, val);
                                write(fd_admin, response, strlen(response));
                                write(new_socketDescriptor, "Quantity modified successfully", sizeof("Quantity modified successfully"));

                                unlock(fd, lock);
                                flg = 1;
                                break;
                            }
                        }

                        if (!flg)
                        {
                            write(new_socketDescriptor, "Product id invalid", sizeof("Product id invalid"));
                            unlock(fd, lock);
                        }
                    }

                    else if (ch == '5')
                    {
                        struct flock lock;
                        readLock(fd, lock);

                        struct product p;
                        while (read(fd, &p, sizeof(struct product)))
                        {
                            if (p.id != -1)
                            {
                                write(new_socketDescriptor, &p, sizeof(struct product));
                            }
                        }

                        p.id = -1;
                        write(new_socketDescriptor, &p, sizeof(struct product));
                        unlock(fd, lock);
                    }

                    else if (ch == '6')
                    {
                        close(new_socketDescriptor);
                        struct flock lock;
                        readLock(fd, lock);
                        write(fd_admin, "Current Inventory:\n", strlen("Current Inventory:\n"));
                        write(fd_admin, "ProductID\tProductName\tQuantity\tPrice\n", strlen("ProductID\tProductName\tQuantity\tPrice\n"));

                        lseek(fd, 0, SEEK_SET);
                        struct product p;
                        while (read(fd, &p, sizeof(struct product)))
                        {
                            if (p.id != -1)
                            {
                                char temp[100];
                                sprintf(temp, "%d\t%s\t%d\t%d\n", p.id, p.name, p.qty, p.price);
                                write(fd_admin, temp, strlen(temp));
                            }
                        }
                        unlock(fd, lock);
                        break;
                    }
                    else
                    {
                        continue;
                    }
                }
            }
            printf("Connection terminated\n");
        }
        else
        {
            close(new_socketDescriptor);
        }
    }
}