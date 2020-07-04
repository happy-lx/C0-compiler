const int a = 0 , c = 3 ; 

int fib(int a)
{
    int result;

    if(a == 1)
    {
        return (1);
    }
    result = a * fib(a - 1);

    return (result);
}

void main()
{
    
    int c ;
    c = fib(5);
    printf(c);
}