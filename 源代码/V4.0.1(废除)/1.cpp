int add(int a,int b)
{
    return (a + b);
}
int minus(int a,int b)
{
    return (a - b);
}

void main()
{
    int a , b ;
    scanf(a);
    scanf(b);
    printf("a+b is ",add(a,b));
    printf("\n");
    printf("a-b is ",minus(a,b));
    printf("\n");
}