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
    const int t = 1;
    const int kk = -1;
    const char b = 'c';
    int c ;
    int d ;
    c = fib(5);
    d = a + c;
    c = a + c;
    c = kk;
    if(t)
   {
	t  = 3 + 2;
       if(t>3)
	{
	t = 5 - 1 ;
	}
       else
	{
	b = 'A' + 'A';
	}
   }
    printf('c');
}