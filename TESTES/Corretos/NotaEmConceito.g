programa {
	int nota;
	car conceito;
	escreva "Digite um valor inteiro para a nota de um aluno";
	novalinha;
	leia nota;
	
	se (nota<6)
	entao{
		car conceito;
		conceito='D';
		escreva "Conceito: ";
		escreva conceito;
		novalinha;
	}
	senao{
		se(nota<7)
		entao{
			car conceito;
			conceito='C';
			escreva "Conceito: ";
			escreva conceito;
			novalinha;
		}
		senao{
			 se (nota<9)
			 entao{
				car conceito;
				conceito='B';
				escreva "Conceito: ";
				escreva conceito;
				novalinha;
			}
			senao{
				car conceito;
				conceito='A';
				escreva "Conceito: ";
				escreva conceito;
				novalinha;
			}
		}
	}
	
}
