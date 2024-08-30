#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "structures.h"
#include <fcntl.h>
#include <sys/stat.h>

void displayProduct(struct product p)
{
    if (p.id != -1 && p.qty > 0)
    {

        printf("| %10d | %20s | %16d | %16.2d |\n", p.id, p.name, p.qty, p.price);
    }
}
int main()
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock == -1)
    {
        perror("Error: ");
        return -1;
    }

    struct sockaddr_in serv;

    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = INADDR_ANY;
    serv.sin_port = htons(5555);

    if (connect(sock, (struct sockaddr *)&serv, sizeof(serv)) == -1)
    {
        perror("Error: ");
        return -1;
    }

    printf("Success\n");
    printf("************************************************************\n");
    printf("*                                                          *\n");
    printf("*                 Welcome to our store, customer!          *\n");
    printf("*                                                          *\n");
    printf("************************************************************\n");
    printf("Are you a who or the admin? Press 1 for user, any other number for admin\n");
    int who;
    scanf("%d", &who);
    write(sock, &who, sizeof(who));

    if (who == 1)
    {
        while (1)
        {

            printf("\n==================================================================================\n");
            printf("\x1b[1mWelcome to our Online Store!\x1b[0m\n");
            printf("==================================================================================\n\n");

            printf("\x1b[1mMenu:\x1b[0m\n");
            printf("\x1b[33m╔═════════════════════════════════════════════════════════════════════════════╗\n");
            printf("║  \x1b[34m[ 1 ]\x1b[0m  \x1b[1mList availabe items\x1b[0m                                                 ║\n");
            printf("║  \x1b[34m[ 2 ]\x1b[0m  \x1b[1mView your cart\x1b[0m                                                      ║\n");
            printf("║  \x1b[34m[ 3 ]\x1b[0m  \x1b[1mAdd products to your cart\x1b[0m                                           ║\n");
            printf("║  \x1b[34m[ 4 ]\x1b[0m  \x1b[1mEdit products in your cart\x1b[0m                                          ║\n");
            printf("║  \x1b[34m[ 5 ]\x1b[0m  \x1b[1mProceed to checkout\x1b[0m                                                 ║\n");
            printf("║  \x1b[34m[ 6 ]\x1b[0m  \x1b[1mRegister\x1b[0m                                                            ║\n");
            printf("║  \x1b[34m[ 7 ]\x1b[0m  \x1b[1mExit Menu\x1b[0m                                                           ║\n");
            printf("╚═════════════════════════════════════════════════════════════════════════════╝\x1b[0m\n\n");

            printf("Enter the number corresponding to your choice:");

            char ch;
            scanf("%c", &ch);
            scanf("%c", &ch);

            write(sock, &ch, sizeof(char));

            if (ch == '7')
            {
                break;
            }
            else if (ch == '1')
            {

                printf("+------------+----------------------+------------------+------------------+\n");
                printf("| ProductID  |     ProductName      |  Quantity |       Price      |\n");
                printf("+------------+----------------------+------------------+------------------+\n");

                while (1)
                {
                    struct product p;
                    read(sock, &p, sizeof(struct product));
                    if (p.id != -1)
                    {
                        if (p.id != -1 && p.qty > 0)
                        {

                            printf("| %10d | %20s | %16d | %16.2d |\n", p.id, p.name, p.qty, p.price);
                        }
                    }
                    else
                    {
                        break;
                    }
                }

                printf("+------------+----------------------+------------------+------------------+\n");
            }
            else if (ch == '2')
            {
                int customerID = -1;

                printf("Enter customer id\n");
                scanf("%d", &customerID);

                write(sock, &customerID, sizeof(int));

                struct cart o;
                read(sock, &o, sizeof(struct cart));

                if (o.custid != -1)
                {
                    printf("Customer ID: %d\n\n", o.custid);
                    printf("+------------+----------------------+------------------+------------------+\n");
                    printf("| Product ID |     Product Name     | Quantity  |       Price      |\n");
                    printf("+------------+----------------------+------------------+------------------+\n");
                    for (int i = 0; i < 20; i++)
                    {
                        // displayProduct(o.products[i]);
                        if (o.products[i].id != -1 && o.products[i].qty > 0)
                        {

                            printf("| %10d | %20s | %16d | %16.2d |\n", o.products[i].id, o.products[i].name, o.products[i].qty, o.products[i].price);
                        }
                    }
                    printf("+------------+----------------------+------------------+------------------+\n");
                }

                else
                {
                    printf("Wrong customer id provided\n");
                }
            }

            else if (ch == '3')
            {
                int customerID = -1;

                printf("╔════════════════════════════╗\n");
                printf("║       Enter Customer ID     ║\n");
                printf("╚════════════════════════════╝\n");
                scanf("%d", &customerID);

                write(sock, &customerID, sizeof(int));

                int res;
                read(sock, &res, sizeof(int));
                if (res == -1)
                {
                    printf("If the customer ID you entered is invalid, the system cannot find any matching customer information. Please make sure that you have entered the correct ID and try again. If you continue to encounter this issue, please contact customer support for further assistance.\n");
                    continue;
                }
                char reply[80];
                int pid, qty;

                printf("╔════════════════════════════╗\n");
                printf("║       Enter Product ID      ║\n");
                printf("╚════════════════════════════╝\n");
                scanf("%d", &pid);

                while (1)
                {
                    printf("╔════════════════════════════╗\n");
                    printf("║       Enter Quantity        ║\n");
                    printf("╚════════════════════════════╝\n");
                    scanf("%d", &qty);

                    if (qty <= 0)
                    {
                        printf("The quantity entered is invalid. It must be greater than 0. Please try again.\n");
                    }
                    else
                    {
                        break;
                    }
                }

                struct product p;
                p.id = pid;
                p.qty = qty;

                write(sock, &p, sizeof(struct product));
                read(sock, reply, sizeof(reply));
                printf("%s", reply);
            }

            else if (ch == '4')
            {
                int customerID = -1;

                printf("╔════════════════════════════╗\n");
                printf("║       Enter Customer ID      ║\n");
                printf("╚════════════════════════════╝\n");
                scanf("%d", &customerID);

                write(sock, &customerID, sizeof(int));

                int res;
                read(sock, &res, sizeof(int));
                if (res == -1)
                {
                    printf("If the customer ID you entered is invalid, the system cannot find any matching customer information. Please make sure that you have entered the correct ID and try again. If you continue to encounter this issue, please contact customer support for further assistance.\n");
                    continue;
                }

                int pid, qty;

                printf("╔════════════════════════════╗\n");
                printf("║       Enter Product ID      ║\n");
                printf("╚════════════════════════════╝\n");
                scanf("%d", &pid);

                while (1)
                {
                    printf("Enter quantity\n");
                    scanf("%d", &qty);

                    if (qty < 0)
                    {
                        printf("Please enter a +ve quantity\n");
                    }
                    else
                    {
                        break;
                    }
                }

                struct product p;
                p.id = pid;
                p.qty = qty;

                write(sock, &p, sizeof(struct product));

                char reply[80];
                read(sock, reply, sizeof(reply));
                printf("%s", reply);
            }

            else if (ch == '5')
            {
                int customerID = -1;

                printf("╔════════════════════════════╗\n");
                printf("║       Enter Customer ID     ║\n");
                printf("╚════════════════════════════╝\n");
                scanf("%d", &customerID);

                write(sock, &customerID, sizeof(int));

                int res;
                read(sock, &res, sizeof(int));
                if (res == -1)
                {
                    printf("Invalid customer id\n");
                    continue;
                }

                struct cart c;
                read(sock, &c, sizeof(struct cart));

                int total = 0;
                for (int i = 0; i < 20; i++)
                {
                    if (c.products[i].id != -1)
                    {
                        total = total + c.products[i].qty * c.products[i].price;
                    }
                }

                printf("Value of your cart:");
                printf("%d\n", total);
                while (1)
                {
                    printf("Select your payment method:\n ");
                    printf("1) Credit Card\n");
                    printf("2) Debit Card\n");
                    printf("3) Cash\n");
                    printf("4) UPI\n");
                    int method;
                    scanf("%d", &method);
                    if (method > 4 || method < 1)
                    {
                        printf("Invalid method, try again\n");
                    }
                    else
                    {
                        switch (method)
                        {
                        case 1:
                            printf("You selected Credit Card payment method.\n");
                            printf("Enter Bank Name: ");
                            char bankName[100];
                            scanf(" %[^\n]", bankName);
                            printf("Enter Card Number: ");
                            char cardNumber[20];
                            scanf(" %[^\n]", cardNumber);
                            printf("Enter PIN: ");
                            int pin;
                            scanf("%d", &pin);

                            // Code for credit card payment processing

                            break;
                        case 2:
                            printf("You selected Debit Card payment method.\n");
                            printf("Enter Bank Name: ");
                            char bankNamee[100];
                            scanf(" %[^\n]", bankNamee);
                            printf("Enter Card Number: ");
                            char cardNumberr[20];
                            scanf(" %[^\n]", cardNumberr);
                            printf("Enter PIN: ");
                            int pinn;
                            scanf("%d", &pinn);

                            break;
                        case 3:
                            printf("You selected Cash payment method.\n");
                            // Add your code for Cash payment here
                            break;
                        case 4:
                            printf("You selected UPI payment method.\n");
                            printf("Enter UPI ID: ");
                            char upiID[100];
                            scanf(" %[^\n]", upiID);
                            printf("Enter UPI PIN: ");
                            int upiPIN;
                            scanf("%d", &upiPIN);

                            // Code for UPI payment processing
                            printf("Processing UPI payment...\n");
                            break;
                        }
                        break;
                    }
                }
                ch = "y";
                printf("Payment Successfull!\n");
                write(sock, &ch, sizeof(char));
                read(sock, &ch, sizeof(char));
                write(sock, &total, sizeof(int));
                write(sock, &c, sizeof(struct cart));

                // Print the receipt data on the terminal
                printf("\n╔═════════════════════════╗\n");
                printf("║          RECEIPT        ║\n");
                printf("╠═════════════════════════╣\n");
                printf("║ Total:%-18d║\n", total);
                printf("╠════════════╦════════════╣═════════║\n");
                printf("║ Product ID ║  Quantity  ║  Price  ║\n");
                printf("╠════════════╬════════════╬═════════╣\n");

                for (int i = 0; i < 20; i++)
                {
                    if (c.products[i].id == -1)
                    {
                        break;
                    }
                    printf("║ %-10d ║ %-10d ║ $%-6d ║\n", c.products[i].id, c.products[i].qty, c.products[i].price);
                }

                printf("╚════════════╩════════════╩═════════╝\n");
            }

            else if (ch == '6')
            {

                int id;
                read(sock, &id, sizeof(int));
                printf("Your new customer id : %d\n", id);
            }
            else
            {
                printf("Invalid choice, try again\n");
            }
        }
    }
    else
    {

        while (1)
        {
            printf("\n------------------\n");
            printf("\x1b[1mMenu to choose from:\x1b[0m\n");
            printf("\x1b[33m1.\x1b[0m To add a product\n");
            printf("\x1b[33m2.\x1b[0m To delete a product\n");
            printf("\x1b[33m3.\x1b[0m To update the price of an existing product\n");
            printf("\x1b[33m4.\x1b[0m To update the quantity of an existing product\n");
            printf("\x1b[33m5.\x1b[0m To see your inventory\n");
            printf("\x1b[33m6.\x1b[0m To exit the program\n");
            printf("Please enter your choice\n");
            char ch;
            scanf("%c", &ch);
            scanf("%c", &ch);
            write(sock, &ch, sizeof(ch));

            if (ch == '1')
            {
                // add a product
                int id, qty;
                char name[50];

                printf("╔════════════════════════════╗\n");
                printf("║       Enter Product Name      ║\n");
                printf("╚════════════════════════════╝\n");
                scanf("%s", name);

                printf("╔════════════════════════════╗\n");
                printf("║       Enter Product ID      ║\n");
                printf("╚════════════════════════════╝\n");
                scanf("%d", &id);

                while (1)
                {
                    printf("\n╔════════════════════════════╗\n");
                    printf("║     Enter Quantity (+ve)    ║\n");
                    printf("╚════════════════════════════╝\n");
                    printf("            ");
                    scanf("%d", &qty);

                    if (qty < 0)
                    {
                        printf("\nPlease enter a positive quantity.\n");
                    }
                    else
                    {
                        break;
                    }
                }
                int price = -1;
                while (1)
                {
                    printf("╔════════════════════════════╗\n");
                    printf("║       Enter Price ID        ║\n");
                    printf("╚════════════════════════════╝\n");
                    scanf("%d", &price);

                    if (price < 0)
                    {
                        printf("Please enter a +ve price\n");
                    }
                    else
                    {
                        break;
                    }
                }

                struct product p;
                p.id = id;
                strcpy(p.name, name);
                p.qty = qty;
                p.price = price;

                int n1 = write(sock, &p, sizeof(struct product));

                char reply[80];
                int n = read(sock, reply, sizeof(reply));
                reply[n] = '\0';

                printf("%s", reply);
            }

            else if (ch == '2')
            {
                // printf("Enter product id to be deleted\n");
                int id = -1;

                printf("╔════════════════════════════╗\n");
                printf("║       Enter Product ID      ║\n");
                printf("╚════════════════════════════╝\n");
                scanf("%d", &id);

                write(sock, &id, sizeof(int));
                // deleting is equivalent to setting everything as -1

                char reply[80];
                read(sock, reply, sizeof(reply));
                printf("%s\n", reply);
            }

            else if (ch == '3')
            {
                int id = -1;

                printf("╔════════════════════════════╗\n");
                printf("║       Enter Product ID      ║\n");
                printf("╚════════════════════════════╝\n");
                printf("            ");
                scanf("%d", &id);

                int price = -1;
                while (1)
                {
                    printf("╔════════════════════════════╗\n");
                    printf("║       Enter Price           ║\n");
                    printf("╚════════════════════════════╝\n");
                    scanf("%d", &price);

                    if (price < 0)
                    {
                        printf("Please enter a +ve price\n");
                    }
                    else
                    {
                        break;
                    }
                }

                struct product p;
                p.id = id;
                p.price = price;
                write(sock, &p, sizeof(struct product));

                char reply[80];
                read(sock, reply, sizeof(reply));
                printf("%s\n", reply);
            }

            else if (ch == '4')
            {
                int id = -1;

                printf("╔════════════════════════════╗\n");
                printf("║       Enter Product ID      ║\n");
                printf("╚════════════════════════════╝\n");
                printf("            ");
                scanf("%d", &id);

                int qty = -1;
                while (1)
                {
                    printf("\n╔════════════════════════════╗\n");
                    printf("║     Enter Quantity (+ve)    ║\n");
                    printf("╚════════════════════════════╝\n");
                    printf("            ");
                    scanf("%d", &qty);

                    if (qty < 0)
                    {
                        printf("\nPlease enter a positive quantity.\n");
                    }
                    else
                    {
                        break;
                    }
                }

                struct product p;
                p.id = id;
                p.qty = qty;
                write(sock, &p, sizeof(struct product));

                char reply[80];
                read(sock, reply, sizeof(reply));
                printf("%s\n", reply);
            }

            else if (ch == '5')
            {

                printf("+------------+----------------------+------------------+------------------+\n");
                printf("| ProductID  |     ProductName      |  Quantity |       Price      |\n");
                printf("+------------+----------------------+------------------+------------------+\n");

                while (1)
                {
                    struct product p;
                    read(sock, &p, sizeof(struct product));
                    if (p.id != -1)
                    {
                        if (p.id != -1 && p.qty > 0)
                        {

                            printf("| %10d | %20s | %16d | %16.2d |\n", p.id, p.name, p.qty, p.price);
                        }
                    }
                    else
                    {
                        break;
                    }
                }
                printf("+------------+----------------------+------------------+------------------+\n");
            }

            else if (ch == '6')
            {
                break;
            }

            else
            {
                printf("Wrong choice, try again\n");
            }
        }
    }

    printf("Bye!!\n");
    close(sock);
    return 0;
}