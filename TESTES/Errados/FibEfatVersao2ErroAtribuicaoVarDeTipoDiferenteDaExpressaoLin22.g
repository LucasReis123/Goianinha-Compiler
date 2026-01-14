/* Este é um programa em Goianinha que calcula que possui varias funcoes misturadas com
varias declaracoes de variaveis globais. */
int fatorial(int n){
	se (n==0)
	entao
		retorne 1;
	senao
		retorne n* fatorial(n-1);
}
int somaFunc;

int fibonacci (int seq){
	se(seq==0)
	entao   
		retorne 0;
	senao 
		se (seq==1)
		entao
			retorne 1;
		senao{
			car somaFunc;
			somaFunc=fibonacci(seq-1)+fibonacci(n-2);/*ERRO:somaFunc DO TIPO CAR RECEBE EXPR DO TIPO INT*/ 
			retorne somaFunc;
			}
}
int  fat, fib;
programa {
int n;
n = 1-0;
enquanto (n<0) execute {
       escreva "digite um numero";
       novalinha;
       leia n;    
}	
    fat=fatorial(n);
	escreva "O fatorial de ";
	escreva n;
    escreva " e: ";
	escreva fat;
	novalinha;
	fib=fibonacci(n);
	escreva "Fibonacci de ";
	escreva n;
	escreva " e: ";
	escreva fib;
	novalinha;
	escreva "A soma do valor do fatorial com o valor de fibonacci e: ";
	somaFunc=fat+fib;
	escreva somaFunc;
	novalinha;
	escreva "A subtracao do valor do fatorial pelo valor de finbonacci e: ";
	escreva fat-fib;

}
