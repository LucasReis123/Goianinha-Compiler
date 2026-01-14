/* Este  e um programa com ERRO */
int fatorial(int n){
	int n; /* Erro: nome de variavel redeclarado no mesmo escopo de nomes dos parametros*/
	se (n==0)
	entao
		retorne 1;
	senao
		retorne n* fatorial(n-1);
} 

programa {
	int n;
	n = 1-0;
	enquanto (n>0) execute {
       		escreva "digite um numero";
       		novalinha;
       		leia n;    
	}	
	escreva "O fatorial de ";
	escreva n;
        	escreva " e: ";
	escreva fatorial(n);
	novalinha;
}
