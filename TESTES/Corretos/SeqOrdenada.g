car checaOrd(int quant)
	{
	int cont, valAtual;
	car ordenado;
	ordenado='v';
	cont=1;
	leia valAtual;
	escreva "digite uma sequencia de ";
	escreva quant;
	escreva " numeros inteiros separados entre si por um espaco";
	
	enquanto (cont < quant) execute
	{	int proxVal; /*Variavel declarada dentro de bloco */
		leia proxVal;
		se (valAtual < proxVal)
		entao 
			valAtual=proxVal;
		senao
			{
				ordenado='f';
				cont=quant;
			}
		cont=cont+1;		
	}
    retorne ordenado;
}

programa{

	int quant;
	escreva "digite o tamanho de uma sequencia de numeros inteiros - digite 0 para terminar.";
	leia quant;
	
	enquanto (quant != 0) execute{
		se (checaOrd(quant)=='v')
		entao{ 
			escreva "ORDENADA";
			novalinha;
		}
		senao{
			escreva "DESORDENADA";
			novalinha;
		}
		leia quant;	
	}
}
