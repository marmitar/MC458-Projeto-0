#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <assert.h>
#include <stdarg.h>


// Atributos para GCC.
#define attribute(...) \
	__attribute__((__VA_ARGS__))


/* * * * * * * * * * */
/* ESTRUTURAS GERAIS */

typedef struct mobile mobile_t;

/*  Um objeto de um móbile pode ser um peso
 * simples ou um (ponteiro para um) submóbile.
 *
 * Implementado como uma "safe" `union`.
 */
typedef struct objeto {
	// O objeto propriamente.
	union dado {
		// Peso simples.
		size_t peso;
		// Um submóbile.
		mobile_t *filho;
	} dado;
	// Tipo do objeto: peso simples ou submóbile.
	enum tipo { PESO, FILHO } tipo;
} objeto_t;


/* Implementação do móbile. */
struct mobile {
	// Objeto em cada lado.
	objeto_t Pe, Pd;
	// Distância do objeto na haste.
	size_t De, Dd;
};


/* * * * * * * * * * *  * * * */
/* TRATEMENTO DAS ESTRUTURAS  */

attribute(const)
/*  Ponteiro para o móbile em um objeto de
 * outro móbile.
 *
 * Retorna NULL se o objeto for um peso simples.
 */
mobile_t *mobile_filho(objeto_t obj) {
	if (obj.tipo == FILHO) {
		return obj.dado.filho;
	} else {
		return NULL;
	}
}

static attribute(nonnull)
/*  Desaloca toda a memória utilizada por um
 * móbile e seus possíveis submóbiles.
 */
void free_mobile(mobile_t *mob) {
	// guarda o erro para a apresentá-lo depois
	int error = errno;

	// desaloca filho esquerdo
	mobile_t *filho = mobile_filho(mob->Pe);
	if (filho != NULL) {
		free_mobile(filho);
	}
	// e o filho direito
	filho = mobile_filho(mob->Pd);
	if (filho != NULL) {
		free_mobile(filho);
	}
	// por fim, o próprio móbile
	free(mob);

	errno = error;
}

attribute(const)
/*  Cria um objeto não inicializado, seguro
 * para uso com a `free_mobile`.
 */
objeto_t objeto_nao_inicializado(void) {
	return (objeto_t) {
		.dado = {.peso = 0},
		.tipo = PESO
	};
}

static attribute(const)
/*  Cria um móbile não inicializado, seguro
 * para uso com a `free_mobile`.
 */
mobile_t mobile_nao_inicializado(void) {
	return (mobile_t) {
		.Pe = objeto_nao_inicializado(),
		.Pd = objeto_nao_inicializado(),
		.De = 0, .Dd = 0
	};
}


/* * * * * * * * * * */
/* LEITURA DOS DADOS */

// Função executada sem problemas.
#define OK 	 0
// Resultado da função com erro genérico.
// Erro específico guardado em `errno`.
#define ERR -1
// Erro especial para entradas inválidas.
#define ENTINV 0x1234

static attribute(format(scanf, 2, 3), nonnull)
/* Checked `scanf`.
 *
 * Checa se o `scanf` fez todas as leituras esperadas,
 * como recebido pelo parâmetro `expect`.
 *
 * Retorna `OK` em caso de sucesso. Para erros, o valor
 * do erro é marcado em `errno` e retornado.
 */
int cscanf(unsigned expect, const char *restrict fmt, ...) {
	va_list args;
	va_start(args, fmt);
	// usa `vscanf` para tratar argumentos variados
	// com mais facilidade
	int rv = vscanf(fmt, args);
	va_end(args);

	// erro de leitura
	if (rv < 0) {
		return rv;
	}
	// leitura incompleta
	if (rv < expect) {
		errno = ENTINV;
		return ENTINV;
	}
	return OK;
}


// Marcador de objeto como submóbile,
// para a leitura.
#define SUBMOBILE 	0ULL

static mobile_t *ler_mobile(void);

static attribute(nonnull)
/*  Monta um objeto de um móbile dado o
 * peso lido.
 *
 * Caso necessário, faz a laitura de um
 * submóbile.
 *
 * Retorna `OK` em caso de sucesso. Para erros,
 * o valor do erro é marcado em `errno` e
 * nada é escrito em `obj`.
 */
int ler_filho(objeto_t *obj, size_t peso) {
	// leitura de submóbile necessária
	if (peso == SUBMOBILE) {
		mobile_t *filho = ler_mobile();
		if (filho == NULL) {
			return ERR;
		}
		obj->dado.filho = filho;
		obj->tipo = FILHO;

		return OK;
	// objeto é um peso simples
	} else {
		obj->dado.peso = peso;
		obj->tipo = PESO;

		return OK;
	}
}

static
/*  Alocação e leitura de um móbile da
 * entrada padrão.
 *
 * Retorna NULL em caso de erro, marcando
 * o valor do erro em `errno`.
 */
mobile_t *ler_mobile(void) {
	size_t Pe, De, Pd, Dd;
	// leitura da entrada padrão (checada)
	int rv = cscanf(4, "%zu %zu %zu %zu\n", &Pe, &De, &Pd, &Dd);
	if (rv != OK) {
		return NULL;
	}

	// alocação da memória
	mobile_t *mob = malloc(sizeof(mobile_t));
	if (mob == NULL) {
		return NULL;
	}
	// objeto padrão, para desalocação segura
	// em caso de erro
	*mob = mobile_nao_inicializado();
	// dados já lidos
	mob->De = De;
	mob->Dd = Dd;

	// leitura do filho esquerdo
	rv = ler_filho(&mob->Pe, Pe);
	if (rv != OK) {
		// desaloca antes de retornar erro
		free_mobile(mob);
		return NULL;
	}
	// e do filho direito
	rv = ler_filho(&mob->Pd, Pd);
	if (rv != OK) {
		free_mobile(mob);
		return NULL;
	}
	// objeto alocado e inicializado (lido)
	return mob;
}


/* * * * * * * * * * * */
/* TESTE DE EQUILÌBRIO */

/* Resultado do teste de equilíbrio
 * de um móbile.
 */
typedef struct equilibrio {
	// Peso total do móbile
	// e seus objetos.
	size_t peso;
	// Se o móbile e submóbiles
	// estão em equilíbrio.
	bool equilibrio;
} equilibrio_t;


static
equilibrio_t mobile_estavel(const mobile_t *mob)
attribute(pure, nonnull);

static attribute(pure, nonnull)
/* Teste se um objeto (peso ou submóbile)
 * é estável. Retorna também a massa
 * total do objeto.
 */
equilibrio_t objeto_estavel(const objeto_t objeto) {
	mobile_t *filho = mobile_filho(objeto);
	// se for um filho, basta
	// fazer o teste de móbile
	if (filho != NULL) {
		return mobile_estavel(filho);
	// senão, é apenas um peso simples
	// considerado estável
	} else {
		return (equilibrio_t) {
			.peso = objeto.dado.peso,
			.equilibrio = true
		};
	}
}

static attribute(pure, nonnull)
/* Teste se um mébile é estável.
 * Retorna também a massa total
 * compreendida no móbile.
 */
equilibrio_t mobile_estavel(const mobile_t *mob) {
	// testes dos objetos esquerdo e direito
	equilibrio_t esq, dir;
	esq = objeto_estavel(mob->Pe);
	dir = objeto_estavel(mob->Pd);

	// estabilidade geral do móbile
	bool estavel = (esq.peso * mob->De == dir.peso * mob->Dd);

	return (equilibrio_t) {
		// peso toal, considerando os dois objetos
		.peso = esq.peso + dir.peso,
		// equilíbrio total, considerando os dois objetos
		// e o equilíbrio geral do móbile
		.equilibrio = esq.equilibrio && dir.equilibrio && estavel
	};
}


/* * * * * */
/* SAÍDAS  */

static attribute(nonnull)
// Apresenta o erro marcado em `errno` na saída de erro.
void show_error(const char *prog) {
	// erro especial nesse programa
	if (errno == ENTINV) {
		(void) fprintf(stderr, "%s: entrada inválida\n", prog);
	// erros gerais da libc
	} else {
		perror(prog);
	}
}

static
/**
 * Imprime "S" se emBalanco for maior que 0, e "N" caso contrario.
 */
void imprimeSaida(bool emBalanco) {
	(void) printf("%s\n", (emBalanco ? "S" : "N"));
}


/* * * * */
/* MAIN  */

int main(int argc, const char *argv[]) {
	// nome do programa, para saída de erro
	assert(argc > 0);
	const char *prog = argv[0];

	// leitura da quantidade de linhas
	// (ignorada no decorrer do programa)
	size_t _len;
	int rv = cscanf(1, "%zu\n", &_len);
	if (rv != OK) {
		show_error(prog);
		return EXIT_FAILURE;
	}

	// leitura do mobile
	mobile_t *mobile = ler_mobile();
	if (mobile == NULL) {
		show_error(prog);
		return EXIT_FAILURE;
	}
	// teste de estabilidade dele
	equilibrio_t estabilidade = mobile_estavel(mobile);
	// e apresentação do resultado
	imprimeSaida(estabilidade.equilibrio);

	// encerramento do programa
	free_mobile(mobile);
	return EXIT_SUCCESS;
}
