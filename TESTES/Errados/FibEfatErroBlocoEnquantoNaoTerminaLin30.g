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
		senao
			retorne fibonacci(seq-1)+fibonacci(seq-2);
}
int  fat, fib;
programa {
	int n;
	enquanto (n<0) execute {
       escreva "digite um numero";
       novalinha;
       leia n;    
	/*o bloco dentro do comando enquanto nao termina	*/
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
