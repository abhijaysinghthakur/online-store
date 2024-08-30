#include <stdio.h>

struct product
{
    int id;
    char name[50];
    int qty;
    int price;
};
struct cart
{
    int custid;
    struct product products[20];
};
struct index
{
    int id;
    int offset;
};