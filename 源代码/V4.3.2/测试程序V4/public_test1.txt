const int a_const_int = 1;
const char A_const_char = 'A';
int a_var_int;
int a_var_int_arr[5];
char a_var_char_arr[5];

int fibo(int n){
    if(n==1)
        return (1);
    else if(n==2)
        return (1);
    else
        return(fibo(n-1)+fibo(n-2));
}

void test_base(int input1,int input2){
    const int a = 1;
    int A;
    A=-+11;
    if(a==1){
        if(a>0){
            if(a)
                printf(" A(-+11)= ",A);
        }
    }
    A=--11-+11+-11+fibo(5)+1*-2-((9))/3;
    printf(" calc A =",A);


    if(input1>input2){
        printf("input1>input2 ");
        return;
    }
    else if(input1<input2){
        printf("input1<input2 ");
        return;
    }
    else if(input1==input2){
        printf("input1==input2 ");
        return;
    }
}

int get_gcd(int x,int y){
    int temp;
    int z;
    int i;
    if(x>y){
        temp=x;
        x=y;
        y=temp;
    }
    z=y/x;
    z=z*x;
    if(y-z == 0){
        return (x);
    }
    for(i=x/2;i>1;i=i-1){
        z=x/i;
        z=z*i;
        if(x-z == 0){
            z=y/i;
            z=z*i;
            if(y-z == 0)
                return(i);
        }
    }
    return (1);
}

char fourparam(int a,int b,char c,char d){
    int temp;
    temp=+-22;
    a=a+b;
    b=a+b;
    a=a*b+a;
    a=c+a;
    a=d+a;
    a=temp+a;
    printf(" four param cal=",a);
    ;;;;;{}{}{};{;};;
    return(d);
}



void main(){
    int A,a;
    char C,c;
    int len,i;
    i=0;
    len=5;
    for(i=0;i<len;i=i+1){
        a_var_int_arr[i]=i;
    }

    i=0;
    do{
        a_var_char_arr[i]='a';
        i=i+1;
    }while(i<len)

    printf("scanf two int ");
    scanf(a,A);
    test_base(a,A);
    a_var_int=get_gcd(a,A);
    printf("get_gcd a and A= ",a_var_int);
    scanf(c,C);
    C=fourparam(a,A,c,C);
    printf(" fourparam return= ",C);

    printf("\n\n\n const A_const_char= ",A_const_char);
    printf("\\n\\n\\n a_var_char_arr[4]= ",a_var_char_arr[1+1*2+1]);

    return;
}
