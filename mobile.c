#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <assert.h>
#include <stdarg.h>


/* Atributos para GCC. */
#ifdef __GNUC__
#define attribute(attrs)	\
	__attribute__(attrs)
#else
/* não faz nada em outros compiladores */
#define attribute(attrs)
#endif

/* Ponteiro restrito (C99). */
#define restrict __restrict__


/* * * * * * * * * * */
/* ESTRUTURAS GERAIS */

typedef struct mobile mobile_t;

/**
 * Um objeto de um móbile pode ser um peso
 * simples ou um (ponteiro para um) submóbile.
 *
 * Implementado como uma "safe" `union`.
 */
typedef struct objeto {
	/* O objeto propriamente. */
	union dado {
		/* Que pode ser um peso simples */
		size_t peso;
		/* ou um móbile */
		mobile_t *mobile;
	} dado;
	/* Tipo do objeto: peso simples ou submóbile. */
	enum tipo { PESO, MOBILE } tipo;
} objeto_t;


/* Implementação do móbile. */
struct mobile {
	/* Objeto em cada lado. */
	objeto_t Pe, Pd;
	/* Distância do objeto na haste. */
	size_t De, Dd;
};


/* * * * * * * * * * *  * * * */
/* TRATEMENTO DAS ESTRUTURAS  */

attribute((const))
/**
 * Retorna o ponteiro para o móbile a partir
 * de um objeto. Retorna NULL se o objeto
 * não for um móbile.
 */
mobile_t *acessa_mobile(objeto_t obj) {
	if (obj.tipo == MOBILE) {
		return obj.dado.mobile;
	} else {
		return NULL;
	}
}

static attribute((nonnull))
/**
 * Desaloca toda a memória utilizada por um
 * móbile e seus possíveis submóbiles.
 */
void free_mobile(mobile_t *mob) {
	/* guarda o erro para a apresentá-lo depois */
	int error = errno;

	/* desaloca filho esquerdo */
	mobile_t *filho = acessa_mobile(mob->Pe);
	if (filho != NULL) {
		free_mobile(filho);
	}
	/* e o filho direito */
	filho = acessa_mobile(mob->Pd);
	if (filho != NULL) {
		free_mobile(filho);
	}
	/* por fim, o próprio móbile */
	free(mob);

	errno = error;
}

attribute((const))
/**
 * Cria um objeto não inicializado, seguro
 * para uso com a `free_mobile`.
 */
objeto_t objeto_nao_inicializado(void) {
	objeto_t obj;
	obj.dado.peso = 0;
	obj.tipo = PESO;
	return obj;
}

static attribute((const))
/**
 * Cria um móbile não inicializado, seguro
 * para uso com a `free_mobile`.
 */
mobile_t mobile_nao_inicializado(void) {
	mobile_t mob;
	mob.Pe = objeto_nao_inicializado();
	mob.Pd = objeto_nao_inicializado();
	mob.De = mob.Dd = 0;
	return mob;
}


/* * * * * * * * * * */
/* LEITURA DOS DADOS */

/* Função executada sem problemas. */
#define OK 	 0
/* Resultado da função com erro genérico. */
/* Erro específico guardado em `errno`. */
#define ERR -1
/* Erro especial para entradas inválidas. */
#define ENTINV 0x1234

static attribute((format(scanf, 2, 3), nonnull))
/**
 * Checked `scanf`.
 *
 * Checa se o `scanf` fez todas as leituras esperadas,
 * como recebido pelo parâmetro `expect`.
 *
 * Retorna `OK` em caso de sucesso. Para erros, o valor
 * do erro é marcado em `errno` e retornado.
 */
int cscanf(unsigned expect, const char *restrict fmt, ...) {
	int rv;
	va_list args;
	va_start(args, fmt);
	/* usa `vscanf` para tratar argumentos
	variados com mais facilidade */
	rv = vscanf(fmt, args);
	va_end(args);

	/* erro de leitura */
	if (rv < 0) {
		return rv;
	}
	/* leitura incompleta */
	if (rv < expect) {
		errno = ENTINV;
		return ENTINV;
	}
	return OK;
}


/* Marcador de objeto como submóbile, para a leitura. */
#define SUBMOBILE 	0UL


static
int monta_objeto(objeto_t *obj, size_t peso)
attribute((nonnull));

static
/*  Alocação e leitura de um móbile da
 * entrada padrão.
 *
 * Retorna NULL em caso de erro, marcando
 * o valor do erro em `errno`.
 */
mobile_t *ler_mobile(void) {
	mobile_t *mob;
	size_t Pe, De, Pd, Dd;
	/* leitura da entrada padrão (checada) */
	int rv = cscanf(4, "%lu %lu %lu %lu\n", &Pe, &De, &Pd, &Dd);
	if (rv != OK) {
		return NULL;
	}

	/* alocação da memória */
	mob = malloc(sizeof(mobile_t));
	if (mob == NULL) {
		return NULL;
	}
	/* objeto padrão, para desalocação
	segura em caso de erro */
	*mob = mobile_nao_inicializado();
	/* dados já lidos */
	mob->De = De;
	mob->Dd = Dd;

	/* leitura do filho esquerdo */
	rv = monta_objeto(&mob->Pe, Pe);
	if (rv != OK) {
		/* desaloca antes de retornar erro */
		free_mobile(mob);
		return NULL;
	}
	/* e do filho direito */
	rv = monta_objeto(&mob->Pd, Pd);
	if (rv != OK) {
		free_mobile(mob);
		return NULL;
	}
	/* mobile alocado e inicializado (lido) */
	return mob;
}

static attribute((nonnull))
/**
 * Monta um objeto de um móbile dado o
 * peso lido.
 *
 * Caso necessário, faz a leitura de um
 * submóbile.
 *
 * Retorna `OK` em caso de sucesso. Para erros,
 * o valor do erro é marcado em `errno` e
 * nada é escrito em `obj`.
 */
int monta_objeto(objeto_t *obj, size_t peso) {
	/* leitura de submóbile necessária */
	if (peso == SUBMOBILE) {
		mobile_t *filho = ler_mobile();
		if (filho == NULL) {
			return ERR;
		}
		obj->dado.mobile = filho;
		obj->tipo = MOBILE;

		return OK;
	/* objeto é um peso simples */
	} else {
		obj->dado.peso = peso;
		obj->tipo = PESO;

		return OK;
	}
}


/* * * * * * * * * * * */
/* TESTE DE EQUILÌBRIO */

/**
 * Resultado do teste de equilíbrio
 * de um objeto.
 */
typedef struct resultado {
	/* Peso total do móbile
	e seus objetos. */
	size_t peso;
	/* Se o móbile e submóbiles
	estão em equilíbrio. */
	bool equilibrio;
} resultado_t;


static
resultado_t teste_mobile(const mobile_t *mob)
attribute((pure, nonnull));

static attribute((pure))
/**
 * Teste se um objeto (peso ou móbile)
 * está em equilíbrio. Retorna também
 * a massa total do objeto.
 */
resultado_t teste_objeto(const objeto_t objeto) {
	mobile_t *mob = acessa_mobile(objeto);
	/* se for um móbile, basta
	fazer o teste recursivo */
	if (mob != NULL) {
		return teste_mobile(mob);
	/* senão, é apenas um peso simples
	considerado estável */
	} else {
		resultado_t res;
		res.peso = objeto.dado.peso;
		res.equilibrio = true;
		return res;
	}
}

static attribute((pure, nonnull))
/**
 * Teste se um móbile está em equilíbrio.
 * Retorna também a massa total presa
 * no móbile.
 */
resultado_t teste_mobile(const mobile_t *mob) {
	bool estavel;
	/* testes dos objetos esquerdo e direito */
	resultado_t esq, dir, total;
	esq = teste_objeto(mob->Pe);
	dir = teste_objeto(mob->Pd);

	/* estabilidade geral do móbile */
	estavel = (esq.peso * mob->De == dir.peso * mob->Dd);

	/* peso toal, considerando os dois objetos */
	total.peso = esq.peso + dir.peso;
	/* equilíbrio total, considerando os dois
	objetos e o equilíbrio geral do móbile */
	total.equilibrio = esq.equilibrio && dir.equilibrio && estavel;

	return total;
}

static attribute((pure, nonnull))
/**
 * Teste se um móbile está em equilíbrio.
 */
bool mobile_em_equilibrio(const mobile_t *mob) {
	return teste_mobile(mob).equilibrio;
}


/* * * * * */
/* SAÍDAS  */

static attribute((nonnull))
/**
 * Apresenta o erro marcado em `errno` na saída de erro.
 */
void printerr(const char *prog) {
	/* erro especial nesse programa */
	if (errno == ENTINV) {
		(void) fprintf(stderr, "%s: entrada inválida\n", prog);
	/* erros gerais da libc */
	} else {
		perror(prog);
	}
}

static
/**
 * Imprime "S" se emBalanco for maior que 0, e "N" caso contrario.
 */
void imprime_saida(bool emBalanco) {
	(void) printf("%s\n", (emBalanco ? "S" : "N"));
}


/* * * * */
/* MAIN  */

int main(int argc, const char *argv[]) {
	size_t _len;
	const char *prog;
	mobile_t *mobile;
	bool equilibrio;

	/* nome do programa, para saída de erro */
	assert(argc > 0);
	prog = argv[0];

	/* leitura da quantidade de linhas
	(ignorada no decorrer do programa) */
	if (cscanf(1, "%lu\n", &_len) != OK) {
		printerr(prog);
		return EXIT_FAILURE;
	}

	/* leitura do mobile */
	mobile = ler_mobile();
	if (mobile == NULL) {
		printerr(prog);
		return EXIT_FAILURE;
	}
	/* teste de estabilidade dele */
	equilibrio = mobile_em_equilibrio(mobile);
	/* e apresentação do resultado */
	imprime_saida(equilibrio);

	/* encerramento do programa */
	free_mobile(mobile);
	return EXIT_SUCCESS;
}
